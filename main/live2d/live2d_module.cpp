#include "live2d_module.h"
#include "unity_playback.h"

#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <malloc.h>
#include <mutex>
#include <set>
#include <vector>
#include <unordered_map>
#include <utility>

#include "../common/module_audio.h"
#include "nlohmann/json.hpp"
#include "../render_d3d11/d3d11_renderer.h"
#include "../common/sl_text_codec.h"

#include <CubismDefaultParameterId.hpp>
#include <CubismFramework.hpp>
#include <CubismModelSettingJson.hpp>
#include <Effect/CubismBreath.hpp>
#include <Effect/CubismEyeBlink.hpp>
#include <Id/CubismIdManager.hpp>
#include <Id/CubismId.hpp>
#include <Math/CubismMatrix44.hpp>
#include <Math/CubismTargetPoint.hpp>
#include <Model/CubismUserModel.hpp>
#include <Motion/CubismExpressionMotionManager.hpp>
#include <Motion/CubismMotion.hpp>
#include <Physics/CubismPhysics.hpp>
#include <Rendering/D3D11/CubismRenderer_D3D11.hpp>

namespace fs = std::filesystem;
namespace Csm = Live2D::Cubism::Framework;
namespace CsmCore = Live2D::Cubism::Core;

namespace
{
	using Csm::csmByte;
	using Csm::csmSizeInt;

	constexpr float kClippingMaskBufferSize = 2048.0f;
	constexpr float kMaxFrameDeltaSeconds = 0.1f;
	constexpr int kMaxExportPrerollSteps = 900;
	constexpr unsigned int kExportRandomSeed = 0x5EEDu;

	bool EndsWithInsensitive(const std::wstring& text, const wchar_t* suffix)
	{
		const size_t suffixLength = std::wcslen(suffix);
		if (text.size() < suffixLength)
			return false;
		return _wcsicmp(text.c_str() + text.size() - suffixLength, suffix) == 0;
	}

	std::string ModelDisplayName(const std::wstring& path)
	{
		std::wstring name = fs::path(path).filename().wstring();
		constexpr wchar_t suffix[] = L".model3.json";
		if (EndsWithInsensitive(name, suffix))
			name.resize(name.size() - std::wcslen(suffix));
		return sl_text::WideToUtf8(name);
	}

	bool ReadBytes(const fs::path& path, std::vector<csmByte>& bytes)
	{
		bytes.clear();
		std::ifstream input(path, std::ios::binary);
		if (!input)
			return false;
		input.seekg(0, std::ios::end);
		const std::streamoff size = input.tellg();
		if (size <= 0)
			return false;
		bytes.resize(static_cast<size_t>(size));
		input.seekg(0, std::ios::beg);
		input.read(reinterpret_cast<char*>(bytes.data()), size);
		return input.good() || input.gcount() == size;
	}

	fs::path AssetPath(const fs::path& directory, const char* relativeUtf8)
	{
		if (relativeUtf8 == nullptr || *relativeUtf8 == '\0')
			return {};
		return directory / fs::path(sl_text::Utf8ToWide(relativeUtf8));
	}

	class CubismAllocator final : public Csm::ICubismAllocator
	{
	public:
		void* Allocate(const Csm::csmSizeType size) override { return std::malloc(size); }
		void Deallocate(void* memory) override { std::free(memory); }
		void* AllocateAligned(const Csm::csmSizeType size, const Csm::csmUint32 alignment) override
		{
			return _aligned_malloc(size, alignment);
		}
		void DeallocateAligned(void* memory) override { _aligned_free(memory); }
	};

	void CubismLog(const char* message)
	{
		if (message == nullptr)
			return;
		::OutputDebugStringA("[Live2D] ");
		::OutputDebugStringA(message);
		::OutputDebugStringA("\n");
	}

	CubismAllocator sharedAllocator;
	std::mutex frameworkMutex;
	unsigned frameworkUsers = 0;
	ID3D11Device* frameworkDevice = nullptr;

	class Live2DModel final : public Csm::CubismUserModel
	{
	public:
		struct MotionEntry
		{
			std::string displayName;
			std::string group;
			int groupIndex = 0;
			Csm::CubismMotion* motion = nullptr;
			float duration = 0.0f;
			float fadeInSeconds = 0.0f;
			float fadeOutSeconds = 0.0f;
			fs::path soundFile;
		};

		struct ExpressionEntry
		{
			std::string name;
			Csm::ACubismMotion* motion = nullptr;
		};

		struct ParameterOverride
		{
			float value = 0.0f;
			bool enabled = false;
		};

		Live2DModel(sl_d3d11::D3D11Renderer* textureRenderer, slaudio::audio_deck* audio)
			: m_textureRenderer(textureRenderer)
			, m_audio(audio)
		{
			using namespace Csm::DefaultParameterId;
			Csm::CubismIdManager* ids = Csm::CubismFramework::GetIdManager();
			m_idParamAngleX = ids->GetId(ParamAngleX);
			m_idParamAngleY = ids->GetId(ParamAngleY);
			m_idParamAngleZ = ids->GetId(ParamAngleZ);
			m_idParamBodyAngleX = ids->GetId(ParamBodyAngleX);
			m_idParamEyeBallX = ids->GetId(ParamEyeBallX);
			m_idParamEyeBallY = ids->GetId(ParamEyeBallY);
			m_idParamBreath = ids->GetId(ParamBreath);
		}

		~Live2DModel() override
		{
			if (m_audio != nullptr)
				m_audio->voice_stop();
			if (_motionManager != nullptr)
				_motionManager->StopAllMotions();
			DeleteRenderer();
			for (MotionEntry& entry : m_motions)
			{
				if (entry.motion != nullptr)
					Csm::ACubismMotion::Delete(entry.motion);
			}
			m_motions.clear();
			for (ExpressionEntry& entry : m_expressions)
			{
				if (entry.motion != nullptr)
					Csm::ACubismMotion::Delete(entry.motion);
			}
			m_expressions.clear();
			if (m_textureRenderer != nullptr)
			{
				for (SlTextureId texture : m_textures)
					m_textureRenderer->ReleaseTexture(texture);
			}
		}

		bool Load(const std::wstring& manifestPath, std::string& error)
		{
			m_directory = fs::path(manifestPath).parent_path();
			std::vector<csmByte> bytes;
			if (!ReadBytes(fs::path(manifestPath), bytes))
			{
				error = "Could not read the model3 file.";
				return false;
			}

			m_setting = std::make_unique<Csm::CubismModelSettingJson>(bytes.data(), static_cast<csmSizeInt>(bytes.size()));
			const auto metadata = nlohmann::json::parse(bytes.begin(), bytes.end(), nullptr, false);
			m_nativeCoordinates = !metadata.is_discarded() && metadata.contains("SpineLove")
				&& metadata["SpineLove"].value("NativeCoordinates", false);
			const char* mocFile = m_setting->GetModelFileName();
			if (mocFile == nullptr || mocFile[0] == '\0')
			{
				error = "This model3.json does not reference a moc3 file. "
					"Note that Cubism 2.x models (.model.json / .moc) are not supported.";
				return false;
			}
			const fs::path mocPath = AssetPath(m_directory, mocFile);
			if (!ReadBytes(mocPath, bytes))
			{
				error = "The moc3 file referenced by this model could not be read.";
				return false;
			}
            LoadModel(bytes.data(), static_cast<csmSizeInt>(bytes.size()), true);
			if (_model == nullptr)
			{
				error = "Cubism could not create the model from its moc3 file.";
				return false;
			}

			if (!metadata.is_discarded() && metadata.contains("SpineLove") && metadata["SpineLove"].contains("UnityPlayback")) {
				try {
					const auto path = AssetPath(m_directory, metadata["SpineLove"]["UnityPlayback"].get<std::string>().c_str());
					if (!ReadBytes(path, bytes)) throw std::runtime_error("Unity playback data is missing.");
					std::vector<std::string> parameterIds,partIds;
					for (int i=0;i<_model->GetParameterCount();++i) parameterIds.emplace_back(_model->GetParameterId(i)->GetString().GetRawString());
					for (int i=0;i<_model->GetPartCount();++i) partIds.emplace_back(_model->GetPartId(i)->GetString().GetRawString());
					m_unity.load(nlohmann::json::parse(bytes.begin(),bytes.end()),parameterIds,partIds);
					m_unity.update(0,CsmCore::csmGetParameterValues(_model->GetModel()),CsmCore::csmGetPartOpacities(_model->GetModel()));
				} catch (const std::exception& exception) { error=exception.what();return false; }
			}
			LoadOptionalComponents(bytes);
			_model->Update();
			SetupLayout();
			LoadMotions(bytes);
			LoadExpressions(bytes);
			std::set<std::vector<Csm::csmInt32>> maskSets;
			{
				const auto* counts=_model->GetDrawableMaskCounts();const auto** masks=_model->GetDrawableMasks();
				for(Csm::csmInt32 i=0;i<_model->GetDrawableCount();++i)if(counts[i]>0){std::vector<Csm::csmInt32> set(masks[i],masks[i]+counts[i]);std::sort(set.begin(),set.end());maskSets.insert(std::move(set));}
			}
			CreateRenderer((std::max)(1,static_cast<int>((maskSets.size()+35)/36)));
			if (auto* renderer = GetRenderer<Csm::Rendering::CubismRenderer_D3D11>())
			{
				renderer->SetClippingMaskBufferSize(kClippingMaskBufferSize, kClippingMaskBufferSize);
				if(!metadata.is_discarded()&&metadata.contains("SpineLove"))
					for(const auto& value:metadata["SpineLove"].value("DisabledDrawables",nlohmann::json::array()))
						if(value.is_number_integer()){const int index=value.get<int>();if(index>=0&&index<_model->GetDrawableCount())renderer->SetDrawableDisabled(index);}
			}
			if (!LoadTextures(error))
				return false;

			_model->SaveParameters();
			BuildParameterCatalog();
			BuildPartCatalog();
			LoadDisplayNames(manifestPath);
			_updating = false;
			_initialized = true;
			for (size_t i = 0; i < m_motions.size(); ++i)
			{
				if (IsIdleGroupName(m_motions[i].group))
				{
					m_hasIdleGroup = true;
					if (m_lastIdleMotion < 0)
						m_lastIdleMotion = static_cast<int>(i);
				}
			}
			if (!m_motions.empty())
				PlayMotion(m_lastIdleMotion >= 0 ? static_cast<size_t>(m_lastIdleMotion) : 0);
			return true;
		}

		void Update(float deltaSeconds, float timeScale)
		{
			UpdateInternal(deltaSeconds, timeScale, true);
		}

		void Draw(int width, int height, float centerOffsetX, float centerOffsetY,
			float scale, float offsetX, float offsetY, live2d::RenderBounds* outBounds,const std::array<float,6>* sceneTransform=nullptr)
		{
			if (_model == nullptr || width <= 0 || height <= 0)
				return;
			auto* renderer = GetRenderer<Csm::Rendering::CubismRenderer_D3D11>();
			if (renderer == nullptr)
				return;

			Csm::CubismMatrix44 view;
			view.LoadIdentity();
			if (width < height)
				view.Scale(scale, scale * static_cast<float>(width) / static_cast<float>(height));
			else
				view.Scale(scale * static_cast<float>(height) / static_cast<float>(width), scale);
			view.Translate(centerOffsetX + (2.0f * offsetX / static_cast<float>(width)),
				centerOffsetY - (2.0f * offsetY / static_cast<float>(height)));
			m_lastView = view;
			m_lastViewValid = true;

			Csm::CubismMatrix44 mvp = view;
			if (m_nativeCoordinates && sceneTransform) {
				const auto& a=*sceneTransform;const float aspect=float(width)/float(height);
				float matrix[16]={a[0]/aspect,a[1],0,0,a[2]/aspect,a[3],0,0,0,0,1,0,a[4]/aspect,a[5],0,1};
				mvp.SetMatrix(matrix);
			} else if (!m_nativeCoordinates) mvp.MultiplyByMatrix(_modelMatrix);

			if (outBounds != nullptr)
				*outBounds = ComputePixelBounds(mvp, width, height);

            renderer->SetMvpMatrix(&mvp);
            if(m_unity.enabled())renderer->SetModelColor(1,1,1,m_unity.opacity());
            renderer->DrawModel();
		}

		bool PlayMotion(size_t index, bool forceOneShot = false)
		{
			if (index >= m_motions.size() || m_motions[index].motion == nullptr || _motionManager == nullptr)
				return false;

			const bool isIdle = IsIdleGroupName(m_motions[index].group);
			const bool loop = !forceOneShot && (m_forceLoop || isIdle || !m_hasIdleGroup);
			if (isIdle)
				m_lastIdleMotion = static_cast<int>(index);
			if (!loop)
			{
				if (!m_oneShotActive)
					m_returnMotion = m_lastIdleMotion >= 0 ? m_lastIdleMotion : m_currentMotion;
				m_oneShotActive = true;
			}
			else
			{
				m_oneShotActive = false;
			}

			StartMotionEntry(index, loop);
			PlayMotionSound(m_motions[index]);
			return true;
		}
		bool NativeCoordinates() const noexcept { return m_nativeCoordinates; }
		bool PlayNativeMotion(size_t index,bool loop,double mix,double time) {
			if(index>=m_motions.size())return false;
			if(!m_unity.enabled())return loop?PlayMotion(index):PlayMotionOnce(index);
			_motionManager->StopAllMotions();m_oneShotActive=false;m_currentMotion=int(index);
			return m_unity.play(m_motions[index].group,loop,mix,time);
		}
		void SeekNativeMotion(double time){if(m_unity.enabled())m_unity.seek(time);}
		void SetNativeLayers(const std::vector<std::string>& names){if(m_unity.enabled()){_motionManager->StopAllMotions();m_oneShotActive=false;m_currentMotion=-1;m_unity.setLayers(names);}}
		void SetNativeLayerStates(const std::vector<live2d::NativeLayerState>& layers,const std::unordered_map<std::string,float>& overrides,const std::unordered_map<std::string,float>& parts){if(m_unity.enabled()){_motionManager->StopAllMotions();m_oneShotActive=false;m_currentMotion=-1;m_unity.setLayerStates(layers,overrides,parts);}}

		bool PlayMotionOnce(size_t index)
		{
			if (index >= m_motions.size() || m_motions[index].motion == nullptr || _motionManager == nullptr)
				return false;
			m_oneShotActive = false;
			StartMotionEntry(index, false);
			PlayMotionSound(m_motions[index]);
			return true;
		}

		bool IsMotionFinished() const noexcept
		{
			if(m_unity.enabled())return m_unity.finished();
			return _motionManager != nullptr && _motionManager->IsFinished();
		}

		void SetForceLoop(bool enabled) noexcept { m_forceLoop = enabled; }
		bool ForceLoop() const noexcept { return m_forceLoop; }

		bool PlayExpression(size_t index)
		{
			if (index >= m_expressions.size() || m_expressions[index].motion == nullptr ||
				_expressionManager == nullptr)
				return false;
			_expressionManager->StartMotionPriority(m_expressions[index].motion, false, 3);
			m_currentExpression = static_cast<int>(index);
			return true;
		}

		bool PlayRandomExpression()
		{
			if (m_expressions.empty())
				return false;
			size_t index = static_cast<size_t>(std::rand()) % m_expressions.size();
			if (m_expressions.size() > 1 && static_cast<int>(index) == m_currentExpression)
				index = (index + 1) % m_expressions.size();
			return PlayExpression(index);
		}

		void ClearExpression()
		{
			CSM_DELETE(_expressionManager);
			_expressionManager = CSM_NEW Csm::CubismExpressionMotionManager();
			m_currentExpression = -1;
		}

		bool PlayRandomTapMotion()
		{
			if (m_motions.empty())
				return false;
			std::vector<size_t> candidates;
			for (size_t i = 0; i < m_motions.size(); ++i)
			{
				if (_strnicmp(m_motions[i].group.c_str(), "tap", 3) == 0)
					candidates.push_back(i);
			}
			if (candidates.empty())
			{
				for (size_t i = 0; i < m_motions.size(); ++i)
					candidates.push_back(i);
			}
			const size_t pick = candidates[static_cast<size_t>(std::rand()) % candidates.size()];
			return PlayMotion(pick, true);
		}

		static bool IsIdleGroupName(const std::string& group)
		{
			return _strnicmp(group.c_str(), "idle", 4) == 0;
		}

		std::string HitTest(float normalizedX, float normalizedY) const
		{
			if (_model == nullptr || m_setting == nullptr || !m_lastViewValid)
				return {};
			Csm::CubismMatrix44 view = m_lastView;
			const float viewX = view.InvertTransformX(normalizedX);
			const float viewY = view.InvertTransformY(normalizedY);
			for (int i = 0; i < m_setting->GetHitAreasCount(); ++i)
			{
				const Csm::CubismIdHandle id = m_setting->GetHitAreaId(i);
				if (id == nullptr)
					continue;
				if (const_cast<Live2DModel*>(this)->IsHit(id, viewX, viewY))
				{
					const char* name = m_setting->GetHitAreaName(i);
					return name != nullptr ? name : "";
				}
			}
			return {};
		}

        std::vector<live2d::NativeDrawableHit> NativeRaycast(float x,float y,const std::vector<live2d::NativeHitCandidate>& candidates)const
        {
            std::vector<live2d::NativeDrawableHit> hits;
            if(!_model||!std::isfinite(x)||!std::isfinite(y))return hits;
            auto ordered=candidates;
            for(auto& candidate:ordered)if(!candidate.drawableId.empty())candidate.index=_model->GetDrawableIndex(Csm::CubismFramework::GetIdManager()->GetId(candidate.drawableId.c_str()));
            std::stable_sort(ordered.begin(),ordered.end(),[](const auto& a,const auto& b){return a.index<b.index;});
            int previous=-1;
            for(const auto& candidate:ordered){
                const int index=candidate.index;
                if(index<0||index>=_model->GetDrawableCount()||index==previous)continue;
                const bool updated=size_t(index)<m_nativeVisibilityUpdated.size()&&m_nativeVisibilityUpdated[size_t(index)];
                if(!(updated?_model->GetDrawableDynamicFlagIsVisible(index):candidate.enabled))continue;
                previous=index;const int count=_model->GetDrawableVertexCount(index);const auto* vertices=_model->GetDrawableVertexPositions(index);if(!vertices||count<=0)continue;
                float minX=vertices[0].X,maxX=minX,minY=vertices[0].Y,maxY=minY;
                for(int i=1;i<count;++i){minX=(std::min)(minX,vertices[i].X);maxX=(std::max)(maxX,vertices[i].X);minY=(std::min)(minY,vertices[i].Y);maxY=(std::max)(maxY,vertices[i].Y);}
                if(x<minX||x>maxX||y<minY||y>maxY)continue;
                bool inside=candidate.precision==0;
                if(candidate.precision==1){const auto* indices=_model->GetDrawableVertexIndices(index);const int length=_model->GetDrawableVertexIndexCount(index);
                    for(int i=0;indices&&i+2<length;i+=3){if(indices[i]>=count||indices[i+1]>=count||indices[i+2]>=count)continue;const auto a=vertices[indices[i]],b=vertices[indices[i+1]],c=vertices[indices[i+2]];
                        const float ab=(b.X-a.X)*(y-b.Y)-(b.Y-a.Y)*(x-b.X),bc=(c.X-b.X)*(y-c.Y)-(c.Y-b.Y)*(x-c.X),ca=(a.X-c.X)*(y-a.Y)-(a.Y-c.Y)*(x-a.X);
                        if((ab>0&&bc>0&&ca>0)||(ab<0&&bc<0&&ca<0)){inside=true;break;}
                    }
                }
                if(inside){hits.push_back({_model->GetDrawableId(index)->GetString().GetRawString(),index,candidate.partType});if(hits.size()==4)break;}
            }
            return hits;
        }
		bool TapAt(float normalizedX, float normalizedY)
		{
			const std::string area = HitTest(normalizedX, normalizedY);
			if (!area.empty())
			{
				if (_strnicmp(area.c_str(), "head", 4) == 0 && !m_expressions.empty())
					return PlayRandomExpression();
				return PlayRandomTapMotion();
			}

			if (m_motions.empty())
				return false;
			const size_t next = m_currentMotion < 0
				? 0
				: (static_cast<size_t>(m_currentMotion) + 1) % m_motions.size();
			return PlayMotion(next);
		}

		void SetVoiceVolume(float volume) noexcept
		{
			m_voiceVolume = (std::max)(0.0f, (std::min)(1.0f, volume));
			if (m_audio != nullptr && m_audio->ready())
				m_audio->voice_set_volume(m_voiceVolume);
		}
		float VoiceVolume() const noexcept { return m_voiceVolume; }

		const std::vector<MotionEntry>& Motions() const noexcept { return m_motions; }
		const std::vector<ExpressionEntry>& Expressions() const noexcept { return m_expressions; }
		int CurrentMotion() const noexcept { return m_currentMotion; }
		int CurrentExpression() const noexcept { return m_currentExpression; }
		const std::vector<live2d::ParameterState>& Parameters() const noexcept { return m_parameters; }
		const std::vector<live2d::PartState>& Parts() const noexcept { return m_parts; }

		bool SetParameter(size_t index, float value)
		{
			if (index >= m_parameterOverrides.size())
				return false;
			const float clamped = (std::max)(m_parameters[index].minimum, (std::min)(m_parameters[index].maximum, value));
			m_parameterOverrides[index].value = clamped;
			m_parameterOverrides[index].enabled = true;
			m_parameters[index].value = clamped;
			m_parameters[index].overridden = true;
			return true;
		}

		bool SetParameterOverride(size_t index, bool enabled)
		{
			if (index >= m_parameterOverrides.size())
				return false;
			if (enabled && !m_parameterOverrides[index].enabled)
				m_parameterOverrides[index].value = m_parameters[index].value;
			m_parameterOverrides[index].enabled = enabled;
			m_parameters[index].overridden = enabled;
			return true;
		}

		bool SetPartOpacity(size_t index, float opacity)
		{
			if (index >= m_partOverrides.size())
				return false;
			const float clamped = (std::max)(0.0f, (std::min)(1.0f, opacity));
			m_partOverrides[index].value = clamped;
			m_partOverrides[index].enabled = true;
			m_parts[index].opacity = clamped;
			m_parts[index].overridden = true;
			return true;
		}

		bool SetPartOverride(size_t index, bool enabled)
		{
			if (index >= m_partOverrides.size())
				return false;
			if (enabled && !m_partOverrides[index].enabled)
				m_partOverrides[index].value = m_parts[index].opacity;
			m_partOverrides[index].enabled = enabled;
			m_parts[index].overridden = enabled;
			if (!enabled && _model != nullptr)
			{
				_model->SetPartOpacity(static_cast<int>(index), m_parts[index].defaultOpacity);
				m_parts[index].opacity = m_parts[index].defaultOpacity;
			}
			return true;
		}

		bool ResetPart(size_t index)
		{
			return SetPartOverride(index, false);
		}

		bool ResetParameter(size_t index)
		{
			if (index >= m_parameterOverrides.size())
				return false;
			m_parameterOverrides[index].enabled = false;
			m_parameters[index].overridden = false;
			if (_model != nullptr)
			{
				_model->SetParameterValue(static_cast<int>(index), m_parameters[index].defaultValue);
				m_parameters[index].value = m_parameters[index].defaultValue;
			}
			return true;
		}

		void ClearParameterOverrides() noexcept
		{
			for (size_t i = 0; i < m_parameterOverrides.size(); ++i)
			{
				m_parameterOverrides[i].enabled = false;
				m_parameters[i].overridden = false;
			}
		}

		void SetDragTarget(float x, float y) noexcept
		{
			if (_dragManager != nullptr && !m_export.active)
				_dragManager->Set((std::max)(-1.0f, (std::min)(1.0f, x * m_dragSettings.sensitivity)),
					(std::max)(-1.0f, (std::min)(1.0f, y * m_dragSettings.sensitivity)));
		}

		void SetDragSettings(const live2d::DragSettings& settings) noexcept { m_dragSettings = settings; }
		std::array<int, 6> GazeParameterIndices() const noexcept { return m_gazeParameterIndices; }
		void SetGazePose(const live2d::GazePose& pose) noexcept { m_gazePose = pose; }
		void SetEffects(const live2d::EffectSettings& settings) noexcept { m_effects = settings; }
		const live2d::EffectSettings& Effects() const noexcept { return m_effects; }

		bool BeginExport(size_t motionIndex, float fps)
		{
			if (motionIndex >= m_motions.size() || m_motions[motionIndex].motion == nullptr || _model == nullptr)
				return false;
			if (m_export.active)
				EndExport();

			m_export.active = true;
			m_export.previousMotion = m_currentMotion;
			m_export.previousOneShotActive = m_oneShotActive;
			m_export.previousReturnMotion = m_returnMotion;
			m_export.touchedMotions.clear();

			if (m_audio != nullptr && m_audio->ready())
				m_audio->voice_stop();
			std::srand(kExportRandomSeed);

			ResetDragManager();
			CreateBreath();
			CreateEyeBlink();
			if (_physics != nullptr)
				_physics->Reset();

			const int parameterCount = _model->GetParameterCount();
			for (int i = 0; i < parameterCount; ++i)
				_model->SetParameterValue(i, _model->GetParameterDefaultValue(i));
			_model->SaveParameters();

			StartExportMotion(motionIndex);
			UpdateInternal(0.0f, 1.0f, false);
			if (_physics != nullptr)
				_physics->Stabilization(_model);
			const float step = fps > 0.0f ? 1.0f / fps : 1.0f / 30.0f;
			const float duration = m_motions[motionIndex].duration;
			int prerollSteps = duration > 0.0f ? static_cast<int>(std::ceil(duration / step)) : 0;
			prerollSteps = (std::min)(prerollSteps, kMaxExportPrerollSteps);
			for (int i = 0; i < prerollSteps; ++i)
				UpdateInternal(step, 1.0f, false);
			StartExportMotion(motionIndex);
			return true;
		}

		bool ExportSwitchMotion(size_t motionIndex)
		{
			if (!m_export.active || motionIndex >= m_motions.size() || m_motions[motionIndex].motion == nullptr)
				return false;
			StartExportMotion(motionIndex);
			return true;
		}

		void EndExport()
		{
			if (!m_export.active)
				return;
			for (const TouchedMotion& touched : m_export.touchedMotions)
			{
				if (touched.index >= m_motions.size() || m_motions[touched.index].motion == nullptr)
					continue;
				Csm::CubismMotion* motion = m_motions[touched.index].motion;
				motion->SetFadeInTime(touched.fadeIn);
				motion->SetFadeOutTime(touched.fadeOut);
				motion->IsLoopFadeIn(touched.loopFadeIn);
			}
			m_export.touchedMotions.clear();
			std::srand(::GetTickCount());
			ResetDragManager();

			m_oneShotActive = false;
			if (m_export.previousMotion >= 0 && static_cast<size_t>(m_export.previousMotion) < m_motions.size())
			{
				PlayMotion(static_cast<size_t>(m_export.previousMotion));
				if (m_export.previousOneShotActive)
				{
					m_oneShotActive = true;
					m_returnMotion = m_export.previousReturnMotion;
				}
			}
			else if (!m_motions.empty())
			{
				PlayMotion(0);
			}
			m_export.active = false;
		}

		bool ExportActive() const noexcept { return m_export.active; }

		float MotionDuration(size_t index) const noexcept
		{
			return index < m_motions.size() ? m_motions[index].duration : 0.0f;
		}

	private:
		struct TouchedMotion
		{
			size_t index = 0;
			float fadeIn = 0.0f;
			float fadeOut = 0.0f;
			bool loopFadeIn = true;
		};

		struct ExportState
		{
			bool active = false;
			int previousMotion = -1;
			bool previousOneShotActive = false;
			int previousReturnMotion = -1;
			std::vector<TouchedMotion> touchedMotions;
		};

		void UpdateInternal(float deltaSeconds, float timeScale, bool computeVertices)
		{
			if (_model == nullptr)
				return;
			const float frameDelta = (std::max)(0.0f, (std::min)(deltaSeconds, kMaxFrameDeltaSeconds));
			const float motionDelta = frameDelta * (std::max)(0.0f, timeScale);

			if (m_oneShotActive && IsMotionFinished())
			{
				m_oneShotActive = false;
				if (m_returnMotion >= 0 && static_cast<size_t>(m_returnMotion) < m_motions.size())
					PlayMotion(static_cast<size_t>(m_returnMotion));
			}

			_model->LoadParameters();
			bool motionUpdated = false;
			if(m_unity.enabled()){
				m_unity.update(motionDelta,CsmCore::csmGetParameterValues(_model->GetModel()),CsmCore::csmGetPartOpacities(_model->GetModel()));motionUpdated=m_unity.active();
			}else if (_motionManager != nullptr && !_motionManager->IsFinished())
				motionUpdated = _motionManager->UpdateMotion(_model, motionDelta);
			_model->SaveParameters();

			if (!motionUpdated && _eyeBlink != nullptr && m_effects.eyeBlink)
				_eyeBlink->UpdateParameters(_model, motionDelta);
			if (_expressionManager != nullptr)
				_expressionManager->UpdateMotion(_model, motionDelta);
			if (_dragManager != nullptr && m_effects.gazeFollow && !m_gazePose.enabled && !m_export.active)
			{
				_dragManager->Update(frameDelta);
				const float dragX = _dragManager->GetX();
				const float dragY = _dragManager->GetY();
				const std::array<float, 6> values{dragX * m_dragSettings.angleX, dragY * m_dragSettings.angleY,
					dragX * dragY * m_dragSettings.angleZ, dragX * m_dragSettings.bodyAngleX,
					dragX * m_dragSettings.eyeBallX, dragY * m_dragSettings.eyeBallY};
				for (size_t i = 0; i < values.size(); ++i)
					if (m_gazeParameterIndices[i] >= 0) _model->AddParameterValue(m_gazeParameterIndices[i], values[i]);
			}
			ApplyGazePose();
			if (m_effects.lipSync && !m_export.active && m_audio != nullptr && m_audio->ready() &&
				m_audio->voice_is_active() && m_lipSyncIds.GetSize() > 0)
			{
				const float level = (std::min)(1.0f, m_audio->voice_level() * 4.0f);
				for (Csm::csmUint32 i = 0; i < m_lipSyncIds.GetSize(); ++i)
					_model->AddParameterValue(m_lipSyncIds[i], level, 0.8f);
			}
			if (_breath != nullptr && m_effects.breath)
				_breath->UpdateParameters(_model, motionDelta);
			if (_physics != nullptr && m_effects.physics)
				_physics->Evaluate(_model, motionDelta);
			if (_pose != nullptr)
				_pose->UpdateParameters(_model, motionDelta);
			ApplyGazePose();
			for (size_t i = 0; i < m_parameterOverrides.size(); ++i)
			{
				if (m_parameterOverrides[i].enabled)
					_model->SetParameterValue(static_cast<int>(i), m_parameterOverrides[i].value);
				m_parameters[i].value = _model->GetParameterValue(static_cast<int>(i));
				m_parameters[i].overridden = m_parameterOverrides[i].enabled;
			}
			for (size_t i = 0; i < m_partOverrides.size(); ++i)
			{
				if (m_partOverrides[i].enabled)
					_model->SetPartOpacity(static_cast<int>(i), m_partOverrides[i].value);
				m_parts[i].opacity = _model->GetPartOpacity(static_cast<int>(i));
				m_parts[i].overridden = m_partOverrides[i].enabled;
			}
			if (computeVertices){
				_model->Update();
                m_nativeVisibilityUpdated.resize(size_t(_model->GetDrawableCount()),false);
                for(int i=0;i<_model->GetDrawableCount();++i)if(_model->GetDrawableDynamicFlagVisibilityDidChange(i))m_nativeVisibilityUpdated[size_t(i)]=true;
            }
		}

		void ApplyGazePose()
		{
			if (!m_gazePose.enabled || m_export.active) return;
			for (size_t i = 0; i < m_gazeParameterIndices.size(); ++i)
				if (m_gazeParameterIndices[i] >= 0) _model->SetParameterValue(m_gazeParameterIndices[i], m_gazePose.values[i]);
		}

		void StartMotionEntry(size_t index, bool loop)
		{
			if(m_unity.enabled()){
				_motionManager->StopAllMotions();m_unity.play(m_motions[index].group,loop);m_currentMotion=int(index);return;
			}
			Csm::CubismMotion* motion = m_motions[index].motion;
			_motionManager->StopAllMotions();
			_motionManager->SetReservePriority(3);
			motion->IsLoop(loop);
			_motionManager->StartMotionPriority(motion, false, 3);
			m_currentMotion = static_cast<int>(index);
		}

		void StartExportMotion(size_t index)
		{
			Csm::CubismMotion* motion = m_motions[index].motion;
			bool alreadyTouched = false;
			for (const TouchedMotion& touched : m_export.touchedMotions)
				alreadyTouched = alreadyTouched || touched.index == index;
			if (!alreadyTouched)
			{
				TouchedMotion touched;
				touched.index = index;
				touched.fadeIn = motion->GetFadeInTime();
				touched.fadeOut = motion->GetFadeOutTime();
				touched.loopFadeIn = motion->IsLoopFadeIn();
				m_export.touchedMotions.push_back(touched);
			}
			motion->SetFadeInTime(0.0f);
			motion->SetFadeOutTime(0.0f);
			motion->IsLoopFadeIn(false);
			m_oneShotActive = false;
			StartMotionEntry(index, true);
		}

		void ResetDragManager()
		{
			CSM_DELETE(_dragManager);
			_dragManager = CSM_NEW Csm::CubismTargetPoint();
		}

		void CreateBreath()
		{
			Csm::CubismBreath::Delete(_breath);
			_breath = Csm::CubismBreath::Create();
			Csm::csmVector<Csm::CubismBreath::BreathParameterData> breath;
			breath.PushBack({ m_idParamAngleX, 0.0f, 15.0f, 6.5345f, 0.5f });
			breath.PushBack({ m_idParamAngleY, 0.0f, 8.0f, 3.5345f, 0.5f });
			breath.PushBack({ m_idParamAngleZ, 0.0f, 10.0f, 5.5345f, 0.5f });
			breath.PushBack({ m_idParamBodyAngleX, 0.0f, 4.0f, 15.5345f, 0.5f });
			breath.PushBack({ m_idParamBreath, 0.5f, 0.5f, 3.2345f, 0.5f });
			_breath->SetParameters(breath);
		}

		void CreateEyeBlink()
		{
			Csm::CubismEyeBlink::Delete(_eyeBlink);
			_eyeBlink = nullptr;
			if (m_setting != nullptr && m_setting->GetEyeBlinkParameterCount() > 0)
				_eyeBlink = Csm::CubismEyeBlink::Create(m_setting.get());
		}

		live2d::RenderBounds ComputePixelBounds(Csm::CubismMatrix44& mvp, int width, int height) const
		{
			live2d::RenderBounds bounds{ 0.0f, 0.0f, -1.0f, -1.0f };
			bool hasBounds = false;
			float minimumX = 0.0f;
			float minimumY = 0.0f;
			float maximumX = 0.0f;
			float maximumY = 0.0f;
			for (int drawableIndex = 0; drawableIndex < _model->GetDrawableCount(); ++drawableIndex)
			{
				if (!_model->GetDrawableDynamicFlagIsVisible(drawableIndex) ||
					_model->GetDrawableOpacity(drawableIndex) <= 0.001f)
				{
					continue;
				}
				const int vertexCount = _model->GetDrawableVertexCount(drawableIndex);
				const float* vertices = _model->GetDrawableVertices(drawableIndex);
				if (vertexCount <= 0 || vertices == nullptr)
					continue;

				for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
				{
					const float clipX = mvp.TransformX(vertices[vertexIndex * 2]);
					const float clipY = mvp.TransformY(vertices[vertexIndex * 2 + 1]);
					const float pixelX = (clipX + 1.0f) * 0.5f * static_cast<float>(width);
					const float pixelY = (1.0f - clipY) * 0.5f * static_cast<float>(height);
					if (!std::isfinite(pixelX) || !std::isfinite(pixelY))
						continue;
					if (!hasBounds)
					{
						minimumX = maximumX = pixelX;
						minimumY = maximumY = pixelY;
						hasBounds = true;
					}
					else
					{
						minimumX = (std::min)(minimumX, pixelX);
						minimumY = (std::min)(minimumY, pixelY);
						maximumX = (std::max)(maximumX, pixelX);
						maximumY = (std::max)(maximumY, pixelY);
					}
				}
			}
			if (hasBounds)
				bounds = { minimumX, minimumY, maximumX - minimumX, maximumY - minimumY };
			return bounds;
		}

		void BuildParameterCatalog()
		{
			const int count = _model == nullptr ? 0 : _model->GetParameterCount();
			m_parameters.clear();
			m_parameterOverrides.clear();
			m_parameters.reserve(static_cast<size_t>(count));
			m_parameterOverrides.reserve(static_cast<size_t>(count));
			const std::array<Csm::CubismIdHandle, 6> gazeIds{m_idParamAngleX, m_idParamAngleY, m_idParamAngleZ,
				m_idParamBodyAngleX, m_idParamEyeBallX, m_idParamEyeBallY};
			m_gazeParameterIndices.fill(-1);
			for (int i = 0; i < count; ++i)
			{
				live2d::ParameterState state;
				const Csm::CubismIdHandle id = _model->GetParameterId(static_cast<Csm::csmUint32>(i));
				for (size_t axis = 0; axis < gazeIds.size(); ++axis)
					if (id == gazeIds[axis]) m_gazeParameterIndices[axis] = i;
				state.id = id == nullptr ? ("Parameter " + std::to_string(i)) : id->GetString().GetRawString();
				state.displayName = state.id;
				state.value = _model->GetParameterValue(i);
				state.minimum = _model->GetParameterMinimumValue(static_cast<Csm::csmUint32>(i));
				state.maximum = _model->GetParameterMaximumValue(static_cast<Csm::csmUint32>(i));
				state.defaultValue = _model->GetParameterDefaultValue(static_cast<Csm::csmUint32>(i));
				m_parameters.push_back(state);
				m_parameterOverrides.push_back({ state.value, false });
			}
		}

		void LoadOptionalComponents(std::vector<csmByte>& bytes)
		{
			auto loadOptional = [&](const char* file, const auto& loader) {
				const fs::path path = AssetPath(m_directory, file);
				if (!path.empty() && ReadBytes(path, bytes))
					loader(bytes.data(), static_cast<csmSizeInt>(bytes.size()));
			};

			loadOptional(m_setting->GetPhysicsFileName(), [this](const csmByte* data, csmSizeInt size) { LoadPhysics(data, size); });
			loadOptional(m_setting->GetPoseFileName(), [this](const csmByte* data, csmSizeInt size) { LoadPose(data, size); });
			loadOptional(m_setting->GetUserDataFile(), [this](const csmByte* data, csmSizeInt size) { LoadUserData(data, size); });

			for (int i = 0; i < m_setting->GetEyeBlinkParameterCount(); ++i)
				m_eyeBlinkIds.PushBack(m_setting->GetEyeBlinkParameterId(i));
			for (int i = 0; i < m_setting->GetLipSyncParameterCount(); ++i)
				m_lipSyncIds.PushBack(m_setting->GetLipSyncParameterId(i));
			if (m_lipSyncIds.GetSize() == 0)
				m_lipSyncIds.PushBack(Csm::CubismFramework::GetIdManager()->GetId(
					Csm::DefaultParameterId::ParamMouthOpenY));

			CreateEyeBlink();
			CreateBreath();
		}

		void SetupLayout()
		{
			Csm::csmMap<Csm::csmString, Csm::csmFloat32> layout;
			if (m_setting->GetLayoutMap(layout))
				_modelMatrix->SetupFromLayout(layout);

			bool hasBounds = false;
			float minimumX = 0.0f;
			float minimumY = 0.0f;
			float maximumX = 0.0f;
			float maximumY = 0.0f;
			for (int drawableIndex = 0; drawableIndex < _model->GetDrawableCount(); ++drawableIndex)
			{
				if (_model->GetDrawableOpacity(drawableIndex) <= 0.001f)
					continue;
				const int vertexCount = _model->GetDrawableVertexCount(drawableIndex);
				const float* vertices = _model->GetDrawableVertices(drawableIndex);
				if (vertexCount <= 0 || vertices == nullptr)
					continue;
				for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
				{
					const float x = vertices[vertexIndex * 2];
					const float y = vertices[vertexIndex * 2 + 1];
					if (!std::isfinite(x) || !std::isfinite(y))
						continue;
					if (!hasBounds)
					{
						minimumX = maximumX = x;
						minimumY = maximumY = y;
						hasBounds = true;
					}
					else
					{
						minimumX = (std::min)(minimumX, x);
						minimumY = (std::min)(minimumY, y);
						maximumX = (std::max)(maximumX, x);
						maximumY = (std::max)(maximumY, y);
					}
				}
			}

			if (hasBounds)
			{
				const float centerX = (minimumX + maximumX) * 0.5f;
				const float centerY = (minimumY + maximumY) * 0.5f;
				_modelMatrix->TranslateX(-centerX * _modelMatrix->GetScaleX());
				_modelMatrix->TranslateY(-centerY * _modelMatrix->GetScaleY());
			}
			else
			{
				_modelMatrix->CenterX(0.0f);
				_modelMatrix->CenterY(0.0f);
			}
		}

		void LoadMotions(std::vector<csmByte>& bytes)
		{
			for (int groupIndex = 0; groupIndex < m_setting->GetMotionGroupCount(); ++groupIndex)
			{
				const char* group = m_setting->GetMotionGroupName(groupIndex);
				for (int motionIndex = 0; motionIndex < m_setting->GetMotionCount(group); ++motionIndex)
				{
					const char* file = m_setting->GetMotionFileName(group, motionIndex);
					if (!ReadBytes(AssetPath(m_directory, file), bytes))
						continue;
					const std::string key = std::string(group) + "_" + std::to_string(motionIndex);
					auto* motion = static_cast<Csm::CubismMotion*>(LoadMotion(bytes.data(), static_cast<csmSizeInt>(bytes.size()), key.c_str()));
					if (motion == nullptr)
						continue;
					const float fadeIn = m_setting->GetMotionFadeInTimeValue(group, motionIndex);
					const float fadeOut = m_setting->GetMotionFadeOutTimeValue(group, motionIndex);
					if (fadeIn >= 0.0f) motion->SetFadeInTime(fadeIn);
					if (fadeOut >= 0.0f) motion->SetFadeOutTime(fadeOut);
					motion->SetEffectIds(m_eyeBlinkIds, m_lipSyncIds);
					motion->IsLoop(true);

					std::string fileName = file == nullptr ? key : fs::path(sl_text::Utf8ToWide(file)).filename().replace_extension().string();
					if (EndsWithInsensitive(sl_text::Utf8ToWide(fileName), L".motion3"))
						fileName.resize(fileName.size() - 8);
					MotionEntry entry;
					entry.displayName = std::string(group) + " / " + fileName;
					entry.group = group;
					entry.groupIndex = motionIndex;
					entry.motion = motion;
					entry.duration = (std::max)(0.0f, motion->GetLoopDuration());
					entry.fadeInSeconds = motion->GetFadeInTime();
					entry.fadeOutSeconds = motion->GetFadeOutTime();
					const char* sound = m_setting->GetMotionSoundFileName(group, motionIndex);
					if (sound != nullptr && sound[0] != '\0')
						entry.soundFile = AssetPath(m_directory, sound);
					m_motions.push_back(std::move(entry));
				}
			}
		}

		void BuildPartCatalog()
		{
			const int count = _model == nullptr ? 0 : _model->GetPartCount();
			m_parts.clear();
			m_partOverrides.clear();
			m_parts.reserve(static_cast<size_t>(count));
			m_partOverrides.reserve(static_cast<size_t>(count));
			for (int i = 0; i < count; ++i)
			{
				live2d::PartState state;
				const Csm::CubismIdHandle id = _model->GetPartId(static_cast<Csm::csmUint32>(i));
				state.id = id == nullptr ? ("Part " + std::to_string(i)) : id->GetString().GetRawString();
				state.displayName = state.id;
				state.opacity = _model->GetPartOpacity(i);
				state.defaultOpacity = state.opacity;
				m_parts.push_back(state);
				m_partOverrides.push_back({ state.opacity, false });
			}
		}

		void LoadDisplayNames(const std::wstring& manifestPath)
		{
			try
			{
				std::ifstream manifestFile(fs::path(manifestPath), std::ios::binary);
				if (!manifestFile)
					return;
				const nlohmann::json manifest = nlohmann::json::parse(manifestFile, nullptr, false);
				if (manifest.is_discarded())
					return;
				const std::string displayFile = manifest
					.value("FileReferences", nlohmann::json::object())
					.value("DisplayInfo", std::string());
				if (displayFile.empty())
					return;

				std::ifstream displayStream(m_directory / fs::path(sl_text::Utf8ToWide(displayFile)), std::ios::binary);
				if (!displayStream)
					return;
				const nlohmann::json displayInfo = nlohmann::json::parse(displayStream, nullptr, false);
				if (displayInfo.is_discarded())
					return;

				std::unordered_map<std::string, std::string> names;
				for (const auto& parameter : displayInfo.value("Parameters", nlohmann::json::array()))
				{
					const std::string id = parameter.value("Id", std::string());
					const std::string name = parameter.value("Name", std::string());
					if (!id.empty() && !name.empty())
						names[id] = name;
				}
				for (live2d::ParameterState& state : m_parameters)
				{
					const auto found = names.find(state.id);
					if (found != names.end())
						state.displayName = found->second;
				}

				std::unordered_map<std::string, std::string> partNames;
				for (const auto& part : displayInfo.value("Parts", nlohmann::json::array()))
				{
					const std::string id = part.value("Id", std::string());
					const std::string name = part.value("Name", std::string());
					if (!id.empty() && !name.empty())
						partNames[id] = name;
				}
				for (live2d::PartState& state : m_parts)
				{
					const auto found = partNames.find(state.id);
					if (found != partNames.end())
						state.displayName = found->second;
				}
			}
			catch (...)
			{
			}
		}

		void LoadExpressions(std::vector<csmByte>& bytes)
		{
			for (int i = 0; i < m_setting->GetExpressionCount(); ++i)
			{
				const char* file = m_setting->GetExpressionFileName(i);
				if (!ReadBytes(AssetPath(m_directory, file), bytes))
					continue;
				const char* rawName = m_setting->GetExpressionName(i);
				std::string name = rawName != nullptr && rawName[0] != '\0'
					? rawName
					: fs::path(sl_text::Utf8ToWide(file != nullptr ? file : "")).filename().string();
				Csm::ACubismMotion* motion = LoadExpression(bytes.data(), static_cast<csmSizeInt>(bytes.size()), name.c_str());
				if (motion == nullptr)
					continue;
				ExpressionEntry entry;
				entry.name = std::move(name);
				entry.motion = motion;
				m_expressions.push_back(std::move(entry));
			}
		}

		void PlayMotionSound(const MotionEntry& entry)
		{
			if (m_audio == nullptr || m_export.active)
				return;
			if (!m_audio->ready() && !m_audio->boot())
				return;
			m_audio->voice_stop();
			if (!entry.soundFile.empty())
				m_audio->voice_start(entry.soundFile.wstring(), m_voiceVolume, false);
		}

		bool LoadTextures(std::string& error)
		{
			if (m_textureRenderer == nullptr)
			{
				error = "The D3D11 texture renderer is unavailable.";
				return false;
			}
			auto* renderer = GetRenderer<Csm::Rendering::CubismRenderer_D3D11>();
			for (int i = 0; i < m_setting->GetTextureCount(); ++i)
			{
				const fs::path texturePath = AssetPath(m_directory, m_setting->GetTextureFileName(i));
				const SlTextureId texture = m_textureRenderer->LoadTexture(texturePath.c_str(), false, true);
				if (texture == 0)
				{
					error = "A texture referenced by the model could not be loaded.";
					return false;
				}
				m_textures.push_back(texture);
				auto* srv = static_cast<ID3D11ShaderResourceView*>(m_textureRenderer->GetTextureSrv(texture));
				renderer->BindTexture(i, srv);
			}
			renderer->IsPremultipliedAlpha(false);
			return true;
		}

		fs::path m_directory;
		bool m_nativeCoordinates = false;
		live2d::UnityPlayback m_unity;
		std::unique_ptr<Csm::CubismModelSettingJson> m_setting;
		sl_d3d11::D3D11Renderer* m_textureRenderer = nullptr;
		slaudio::audio_deck* m_audio = nullptr;
		std::vector<SlTextureId> m_textures;
		std::vector<MotionEntry> m_motions;
		std::vector<ExpressionEntry> m_expressions;
		std::vector<live2d::ParameterState> m_parameters;
		std::vector<ParameterOverride> m_parameterOverrides;
		std::vector<live2d::PartState> m_parts;
		std::vector<ParameterOverride> m_partOverrides;
		Csm::csmVector<Csm::CubismIdHandle> m_eyeBlinkIds;
		Csm::csmVector<Csm::CubismIdHandle> m_lipSyncIds;
		Csm::CubismIdHandle m_idParamAngleX = nullptr;
		Csm::CubismIdHandle m_idParamAngleY = nullptr;
		Csm::CubismIdHandle m_idParamAngleZ = nullptr;
		Csm::CubismIdHandle m_idParamBodyAngleX = nullptr;
		Csm::CubismIdHandle m_idParamEyeBallX = nullptr;
		Csm::CubismIdHandle m_idParamEyeBallY = nullptr;
		Csm::CubismIdHandle m_idParamBreath = nullptr;
		live2d::DragSettings m_dragSettings;
		live2d::GazePose m_gazePose;
		std::array<int, 6> m_gazeParameterIndices{-1, -1, -1, -1, -1, -1};
		live2d::EffectSettings m_effects;
		ExportState m_export;
		Csm::CubismMatrix44 m_lastView;
		bool m_lastViewValid = false;
        std::vector<bool> m_nativeVisibilityUpdated;
		bool m_oneShotActive = false;
		bool m_forceLoop = false;
		bool m_hasIdleGroup = false;
		int m_lastIdleMotion = -1;
		int m_returnMotion = -1;
		int m_currentMotion = -1;
		int m_currentExpression = -1;
		float m_voiceVolume = 0.8f;
	};
}

struct live2d::Live2DModule::Impl
{
	CubismAllocator allocator;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;
	sl_d3d11::D3D11Renderer* textureRenderer = nullptr;
	slaudio::audio_deck audio;
	std::unique_ptr<Live2DModel> model;
	std::wstring manifestPath;
	std::string displayName;
	std::string lastError;
	std::vector<std::string> motionNames;
	std::vector<float> motionDurations;
	std::vector<std::string> expressionNames;
	float timeScale = 1.0f;
	float modelScale = 1.0f;
	float viewOffsetX = 0.0f;
	float viewOffsetY = 0.0f;
	std::array<float,6> sceneTransform{};
	bool hasSceneTransform=false;
	live2d::RenderBounds lastRenderBounds{};
	bool lastRenderBoundsValid = false;
	live2d::DragSettings dragSettings;
	live2d::GazePose gazePose;
	live2d::EffectSettings effects;
	float exportRestoreTimeScale = 1.0f;
	bool initialized = false;
	bool loopAll = false;
	unsigned int modelGeneration = 0;
};

live2d::Live2DModule::Live2DModule() : m_impl(std::make_unique<Impl>()) {}
live2d::Live2DModule::~Live2DModule() { Shutdown(); }

bool live2d::Live2DModule::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, sl_d3d11::D3D11Renderer* textureRenderer)
{
	if (m_impl->initialized)
		return true;
	if (device == nullptr || context == nullptr || textureRenderer == nullptr)
	{
		m_impl->lastError = "The D3D11 rendering device is unavailable.";
		return false;
	}

	{
		std::lock_guard<std::mutex> guard(frameworkMutex);
		if (frameworkUsers && frameworkDevice != device) {
			m_impl->lastError = "Cubism is waiting for the previous rendering device to close.";
			return false;
		}
		if (!frameworkUsers) {
			Csm::CubismFramework::Option option{};
			option.LogFunction = &CubismLog;
			option.LoggingLevel = Csm::CubismFramework::Option::LogLevel_Warning;
			if (!Csm::CubismFramework::StartUp(&sharedAllocator, &option)) {
				m_impl->lastError = "Cubism Framework failed to start.";
				return false;
			}
			Csm::CubismFramework::Initialize();
			Csm::Rendering::CubismRenderer_D3D11::InitializeConstantSettings(1, device);
			frameworkDevice = device;
		}
		++frameworkUsers;
	}
	std::srand(::GetTickCount());
	m_impl->device = device;
	m_impl->context = context;
	m_impl->textureRenderer = textureRenderer;
	m_impl->initialized = true;
	return true;
}

void live2d::Live2DModule::Shutdown() noexcept
{
	if (!m_impl)
		return;
	Clear();
	if (m_impl->initialized)
	{
		std::lock_guard<std::mutex> guard(frameworkMutex);
		if (--frameworkUsers == 0) {
			Csm::Rendering::CubismRenderer_D3D11::OnDeviceLost();
			Csm::Rendering::CubismRenderer_D3D11::DeleteShaderManager();
			Csm::Rendering::CubismRenderer_D3D11::DeleteRenderStateManager();
			Csm::CubismFramework::Dispose();
			Csm::CubismFramework::CleanUp();
			frameworkDevice = nullptr;
		}
	}
	m_impl->device = nullptr;
	m_impl->context = nullptr;
	m_impl->textureRenderer = nullptr;
	m_impl->initialized = false;
}

bool live2d::Live2DModule::ImportModel(const std::wstring& manifestPath)
{
	Clear();
	if (!m_impl->initialized)
	{
		m_impl->lastError = "Cubism rendering has not been initialized.";
		return false;
	}
	if (!EndsWithInsensitive(manifestPath, L".model3.json"))
	{
		m_impl->lastError = EndsWithInsensitive(manifestPath, L".model.json") || EndsWithInsensitive(manifestPath, L".moc")
			? "Cubism 2.x models (.model.json / .moc) are not supported; please use a Cubism 3+ .model3.json."
			: "Please select a Cubism .model3.json file.";
		return false;
	}

	auto model = std::make_unique<Live2DModel>(m_impl->textureRenderer, &m_impl->audio);
	if (!model->Load(manifestPath, m_impl->lastError))
		return false;
	m_impl->manifestPath = manifestPath;
	m_impl->displayName = ModelDisplayName(manifestPath);
	for (const Live2DModel::MotionEntry& motion : model->Motions())
	{
		m_impl->motionNames.push_back(motion.displayName);
		m_impl->motionDurations.push_back(motion.duration);
	}
	for (const Live2DModel::ExpressionEntry& expression : model->Expressions())
		m_impl->expressionNames.push_back(expression.name);
	model->SetDragSettings(m_impl->dragSettings);
	model->SetEffects(m_impl->effects);
	model->SetForceLoop(m_impl->loopAll);
	m_impl->model = std::move(model);
	const auto gazeIndices = GazeParameterIndices();
	for (size_t i = 0; i < gazeIndices.size(); ++i)
		m_impl->gazePose.values[i] = gazeIndices[i] >= 0 ? Parameters()[gazeIndices[i]].defaultValue : 0.0f;
	SetGazePose(m_impl->gazePose);
	++m_impl->modelGeneration;
	return true;
}

void live2d::Live2DModule::Clear() noexcept
{
	m_impl->model.reset();
	m_impl->manifestPath.clear();
	m_impl->displayName.clear();
	m_impl->lastError.clear();
	m_impl->motionNames.clear();
	m_impl->motionDurations.clear();
	m_impl->expressionNames.clear();
	++m_impl->modelGeneration;
	m_impl->timeScale = 1.0f;
	m_impl->modelScale = 1.0f;
	m_impl->viewOffsetX = 0.0f;
	m_impl->viewOffsetY = 0.0f;
	m_impl->hasSceneTransform=false;
	m_impl->lastRenderBoundsValid = false;
}

bool live2d::Live2DModule::TickAndRender(float deltaSeconds, int viewportWidth, int viewportHeight,
	float centerOffsetX, float centerOffsetY, bool captureBounds)
{
	m_impl->lastRenderBoundsValid = false;
	if (!m_impl->initialized || !m_impl->model || viewportWidth <= 0 || viewportHeight <= 0)
		return false;
	Csm::Rendering::CubismRenderer_D3D11::StartFrame(
		m_impl->device, m_impl->context,
		static_cast<Csm::csmUint32>(viewportWidth), static_cast<Csm::csmUint32>(viewportHeight));
	m_impl->model->Update(deltaSeconds, m_impl->timeScale);
	if (captureBounds)
		m_impl->lastRenderBounds = { 0.0f, 0.0f, -1.0f, -1.0f };
	m_impl->model->Draw(viewportWidth, viewportHeight, centerOffsetX, centerOffsetY, m_impl->modelScale,
		m_impl->viewOffsetX, m_impl->viewOffsetY,
		captureBounds ? &m_impl->lastRenderBounds : nullptr,m_impl->hasSceneTransform?&m_impl->sceneTransform:nullptr);
	m_impl->lastRenderBoundsValid = captureBounds &&
		m_impl->lastRenderBounds.width >= 0.0f && m_impl->lastRenderBounds.height >= 0.0f;
	Csm::Rendering::CubismRenderer_D3D11::EndFrame(m_impl->device);
	return true;
}

bool live2d::Live2DModule::QueryLastRenderedBounds(RenderBounds& outBounds) const noexcept
{
	if (!m_impl->lastRenderBoundsValid)
		return false;
	outBounds = m_impl->lastRenderBounds;
	return true;
}

bool live2d::Live2DModule::PlayMotion(size_t index)
{
	return m_impl->model != nullptr && m_impl->model->PlayMotion(index);
}

bool live2d::Live2DModule::PlayMotionOnce(size_t index)
{
	return m_impl->model != nullptr && m_impl->model->PlayMotionOnce(index);
}
bool live2d::Live2DModule::PlayNativeMotion(size_t index,bool loop,double mix,double time){return m_impl->model&&m_impl->model->PlayNativeMotion(index,loop,mix,time);}
void live2d::Live2DModule::SeekNativeMotion(double time){if(m_impl->model)m_impl->model->SeekNativeMotion(time);}
void live2d::Live2DModule::SetNativeLayers(const std::vector<std::string>& names){if(m_impl->model)m_impl->model->SetNativeLayers(names);}
void live2d::Live2DModule::SetNativeLayerStates(const std::vector<NativeLayerState>& layers,const std::unordered_map<std::string,float>& overrides,const std::unordered_map<std::string,float>& parts){if(m_impl->model)m_impl->model->SetNativeLayerStates(layers,overrides,parts);}
void live2d::Live2DModule::SetSceneTransform(const std::array<float,6>& matrix){m_impl->sceneTransform=matrix;m_impl->hasSceneTransform=true;}

bool live2d::Live2DModule::IsMotionFinished() const noexcept
{
	return m_impl->model != nullptr && m_impl->model->IsMotionFinished();
}

bool live2d::Live2DModule::PlayExpression(size_t index)
{
	return m_impl->model != nullptr && m_impl->model->PlayExpression(index);
}

bool live2d::Live2DModule::PlayRandomExpression()
{
	return m_impl->model != nullptr && m_impl->model->PlayRandomExpression();
}

void live2d::Live2DModule::ClearExpression()
{
	if (m_impl->model != nullptr)
		m_impl->model->ClearExpression();
}

bool live2d::Live2DModule::TapAt(float normalizedX, float normalizedY)
{
	return m_impl->model != nullptr && m_impl->model->TapAt(normalizedX, normalizedY);
}

std::string live2d::Live2DModule::HitAreaAt(float normalizedX, float normalizedY) const
{
	return m_impl->model != nullptr ? m_impl->model->HitTest(normalizedX, normalizedY) : std::string();
}

std::vector<live2d::NativeDrawableHit> live2d::Live2DModule::NativeRaycast(float x,float y,const std::vector<NativeHitCandidate>& candidates)const
{
    return m_impl->model?m_impl->model->NativeRaycast(x,y,candidates):std::vector<NativeDrawableHit>{};
}

void live2d::Live2DModule::StopVoice() noexcept
{
	if (m_impl->audio.ready()) m_impl->audio.voice_stop();
}

void live2d::Live2DModule::SetVoiceVolume(float volume) noexcept
{
	if (m_impl->model != nullptr)
		m_impl->model->SetVoiceVolume(volume);
}

float live2d::Live2DModule::VoiceVolume() const noexcept
{
	return m_impl->model != nullptr ? m_impl->model->VoiceVolume() : 0.8f;
}

void live2d::Live2DModule::SetLoopAll(bool enabled) noexcept
{
	m_impl->loopAll = enabled;
	if (m_impl->model != nullptr)
		m_impl->model->SetForceLoop(enabled);
}

bool live2d::Live2DModule::LoopAll() const noexcept { return m_impl->loopAll; }

bool live2d::Live2DModule::BeginExportSession(size_t motionIndex, float fps)
{
	if (m_impl->model == nullptr)
		return false;
	if (!m_impl->model->ExportActive())
		m_impl->exportRestoreTimeScale = m_impl->timeScale;
	if (!m_impl->model->BeginExport(motionIndex, fps))
		return false;
	m_impl->timeScale = 1.0f;
	return true;
}

bool live2d::Live2DModule::ExportSessionSwitchMotion(size_t motionIndex)
{
	return m_impl->model != nullptr && m_impl->model->ExportSwitchMotion(motionIndex);
}

void live2d::Live2DModule::EndExportSession()
{
	if (m_impl->model == nullptr || !m_impl->model->ExportActive())
		return;
	m_impl->model->EndExport();
	m_impl->timeScale = m_impl->exportRestoreTimeScale;
}

bool live2d::Live2DModule::ExportSessionActive() const noexcept
{
	return m_impl->model != nullptr && m_impl->model->ExportActive();
}

float live2d::Live2DModule::MotionDuration(size_t index) const noexcept
{
	return m_impl->model != nullptr ? m_impl->model->MotionDuration(index) : 0.0f;
}

void live2d::Live2DModule::SetTimeScale(float value) noexcept
{
	const float clamped = (std::max)(0.0f, (std::min)(5.0f, value));
	if (ExportSessionActive())
		m_impl->exportRestoreTimeScale = clamped;
	else
		m_impl->timeScale = clamped;
}
float live2d::Live2DModule::TimeScale() const noexcept
{
	return ExportSessionActive() ? m_impl->exportRestoreTimeScale : m_impl->timeScale;
}
void live2d::Live2DModule::SetModelScale(float value) noexcept {
	const bool native = m_impl->model && m_impl->model->NativeCoordinates();
	m_impl->modelScale = (std::max)(native ? 0.005f : 0.1f, (std::min)(native ? 100.f : 5.0f, value));
}
float live2d::Live2DModule::ModelScale() const noexcept { return m_impl->modelScale; }
void live2d::Live2DModule::PanByPixels(float deltaX, float deltaY) noexcept { m_impl->viewOffsetX += deltaX; m_impl->viewOffsetY += deltaY; }
void live2d::Live2DModule::SetViewOffset(float x, float y) noexcept { m_impl->viewOffsetX = x; m_impl->viewOffsetY = y; }
float live2d::Live2DModule::ViewOffsetX() const noexcept { return m_impl->viewOffsetX; }
float live2d::Live2DModule::ViewOffsetY() const noexcept { return m_impl->viewOffsetY; }
void live2d::Live2DModule::ResetView() noexcept { m_impl->modelScale = 1.0f; m_impl->viewOffsetX = 0.0f; m_impl->viewOffsetY = 0.0f; }
void live2d::Live2DModule::SetDragTarget(float normalizedX, float normalizedY) noexcept { if (m_impl->model != nullptr) m_impl->model->SetDragTarget(normalizedX, normalizedY); }
void live2d::Live2DModule::EndDrag() noexcept { if (m_impl->model != nullptr) m_impl->model->SetDragTarget(0.0f, 0.0f); }
void live2d::Live2DModule::SetDragSettings(const DragSettings& settings) noexcept
{
	m_impl->dragSettings.sensitivity = (std::max)(0.0f, (std::min)(3.0f, settings.sensitivity));
	m_impl->dragSettings.angleX = (std::max)(-60.0f, (std::min)(60.0f, settings.angleX));
	m_impl->dragSettings.angleY = (std::max)(-60.0f, (std::min)(60.0f, settings.angleY));
	m_impl->dragSettings.angleZ = (std::max)(-60.0f, (std::min)(60.0f, settings.angleZ));
	m_impl->dragSettings.bodyAngleX = (std::max)(-30.0f, (std::min)(30.0f, settings.bodyAngleX));
	m_impl->dragSettings.eyeBallX = (std::max)(-2.0f, (std::min)(2.0f, settings.eyeBallX));
	m_impl->dragSettings.eyeBallY = (std::max)(-2.0f, (std::min)(2.0f, settings.eyeBallY));
	if (m_impl->model != nullptr)
		m_impl->model->SetDragSettings(m_impl->dragSettings);
}
live2d::DragSettings live2d::Live2DModule::GetDragSettings() const noexcept { return m_impl->dragSettings; }
void live2d::Live2DModule::ResetDragSettings() noexcept { SetDragSettings(DragSettings{}); }
std::array<int, 6> live2d::Live2DModule::GazeParameterIndices() const noexcept
{
	return m_impl->model ? m_impl->model->GazeParameterIndices() : std::array<int, 6>{-1, -1, -1, -1, -1, -1};
}
void live2d::Live2DModule::SetGazePose(const GazePose& pose) noexcept
{
	m_impl->gazePose = pose;
	const auto indices = GazeParameterIndices();
	const auto& parameters = Parameters();
	for (size_t i = 0; i < indices.size(); ++i) {
		if (indices[i] < 0) { m_impl->gazePose.values[i] = 0.0f; continue; }
		const auto& parameter = parameters[indices[i]];
		const float value = std::isfinite(pose.values[i]) ? pose.values[i] : parameter.defaultValue;
		m_impl->gazePose.values[i] = (std::max)(parameter.minimum, (std::min)(parameter.maximum, value));
	}
	if (m_impl->model) m_impl->model->SetGazePose(m_impl->gazePose);
}
live2d::GazePose live2d::Live2DModule::GetGazePose() const noexcept { return m_impl->gazePose; }
void live2d::Live2DModule::SetEffects(const EffectSettings& settings) noexcept
{
	m_impl->effects = settings;
	if (m_impl->model != nullptr)
		m_impl->model->SetEffects(settings);
}
live2d::EffectSettings live2d::Live2DModule::Effects() const noexcept { return m_impl->effects; }
bool live2d::Live2DModule::SetParameter(size_t index, float value) { return m_impl->model != nullptr && m_impl->model->SetParameter(index, value); }
bool live2d::Live2DModule::SetParameterOverride(size_t index, bool enabled) { return m_impl->model != nullptr && m_impl->model->SetParameterOverride(index, enabled); }
bool live2d::Live2DModule::ResetParameter(size_t index) { return m_impl->model != nullptr && m_impl->model->ResetParameter(index); }
bool live2d::Live2DModule::SetPartOpacity(size_t index, float opacity) { return m_impl->model != nullptr && m_impl->model->SetPartOpacity(index, opacity); }
bool live2d::Live2DModule::SetPartOverride(size_t index, bool enabled) { return m_impl->model != nullptr && m_impl->model->SetPartOverride(index, enabled); }
bool live2d::Live2DModule::ResetPart(size_t index) { return m_impl->model != nullptr && m_impl->model->ResetPart(index); }
const std::vector<live2d::PartState>& live2d::Live2DModule::Parts() const noexcept
{
	static const std::vector<live2d::PartState> empty;
	return m_impl->model ? m_impl->model->Parts() : empty;
}
unsigned int live2d::Live2DModule::ModelGeneration() const noexcept { return m_impl->modelGeneration; }
void live2d::Live2DModule::ClearParameterOverrides() noexcept { if (m_impl->model != nullptr) m_impl->model->ClearParameterOverrides(); }

bool live2d::Live2DModule::HasImportedModel() const noexcept { return m_impl->model != nullptr; }
bool live2d::Live2DModule::RenderingBackendAvailable() const noexcept { return m_impl->initialized; }
const std::wstring& live2d::Live2DModule::ManifestPath() const noexcept { return m_impl->manifestPath; }
const std::string& live2d::Live2DModule::DisplayName() const noexcept { return m_impl->displayName; }
const std::string& live2d::Live2DModule::LastError() const noexcept { return m_impl->lastError; }
const std::vector<std::string>& live2d::Live2DModule::MotionNames() const noexcept { return m_impl->motionNames; }
const std::vector<float>& live2d::Live2DModule::MotionDurations() const noexcept { return m_impl->motionDurations; }
const std::vector<std::string>& live2d::Live2DModule::ExpressionNames() const noexcept { return m_impl->expressionNames; }
const std::vector<live2d::ParameterState>& live2d::Live2DModule::Parameters() const noexcept
{
	static const std::vector<live2d::ParameterState> empty;
	return m_impl->model ? m_impl->model->Parameters() : empty;
}
int live2d::Live2DModule::CurrentMotionIndex() const noexcept { return m_impl->model ? m_impl->model->CurrentMotion() : -1; }
int live2d::Live2DModule::CurrentExpressionIndex() const noexcept { return m_impl->model ? m_impl->model->CurrentExpression() : -1; }
