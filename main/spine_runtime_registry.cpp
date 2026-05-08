#include "spine_runtime_registry.h"


#include <array>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "runtime_v2/runtime_api.h"
#include "sl_gfx_draw.h"
#include "sl_text_codec.h"

namespace
{
	struct RuntimeDescriptor
	{
		SlRuntimeHub::RuntimeLane slot;
		const char* versionPrefix;
		size_t versionPrefixLength;
	};

	RuntimeDescriptor MakeRuntimeDescriptor(
		SlRuntimeHub::RuntimeLane slot,
		const char* versionPrefix)
	{
		RuntimeDescriptor descriptor{};
		descriptor.slot = slot;
		descriptor.versionPrefix = versionPrefix;
		descriptor.versionPrefixLength = std::strlen(versionPrefix);
		return descriptor;
	}

	const std::array<RuntimeDescriptor, 9>& RuntimeCatalog()
	{
		static const std::array<RuntimeDescriptor, 9> catalog =
		{
			MakeRuntimeDescriptor(SlRuntimeHub::RuntimeLane::Runtime21, "2.1"),
			MakeRuntimeDescriptor(SlRuntimeHub::RuntimeLane::Runtime34, "3.4"),
			MakeRuntimeDescriptor(SlRuntimeHub::RuntimeLane::Runtime35, "3.5"),
			MakeRuntimeDescriptor(SlRuntimeHub::RuntimeLane::Runtime36, "3.6"),
			MakeRuntimeDescriptor(SlRuntimeHub::RuntimeLane::Runtime37, "3.7"),
			MakeRuntimeDescriptor(SlRuntimeHub::RuntimeLane::Runtime38, "3.8"),
			MakeRuntimeDescriptor(SlRuntimeHub::RuntimeLane::Runtime40, "4.0"),
			MakeRuntimeDescriptor(SlRuntimeHub::RuntimeLane::Runtime41, "4.1"),
			MakeRuntimeDescriptor(SlRuntimeHub::RuntimeLane::Runtime42, "4.2")
		};
		return catalog;
	}

	const RuntimeDescriptor* FindRuntimeDescriptor(SlRuntimeHub::RuntimeLane slot)
	{
		const auto& catalog = RuntimeCatalog();
		for (size_t index = 0; index < catalog.size(); ++index)
		{
			if (catalog[index].slot == slot)
				return &catalog[index];
		}
		return nullptr;
	}

	const RuntimeDescriptor* MatchRuntimeVersion(const char* version)
	{
		if (version == nullptr)
			return nullptr;

		const auto& catalog = RuntimeCatalog();
		for (size_t index = 0; index < catalog.size(); ++index)
		{
			const RuntimeDescriptor& descriptor = catalog[index];
			if (std::strncmp(version, descriptor.versionPrefix, descriptor.versionPrefixLength) == 0)
				return &descriptor;
		}
		return nullptr;
	}

	class UnavailableRuntimeAdapter final : public SlPlaybackRuntime
	{
	public:
		explicit UnavailableRuntimeAdapter(const char* versionName)
			: m_versionName(versionName ? versionName : "unknown")
		{}

		bool LoadBundleFromFiles(const SlFileBundleRequest&) override
		{
			m_lastError = "Static runtime " + m_versionName + " is not implemented yet.";
			return false;
		}

		bool LoadBundleFromMemory(const SlMemoryBundleRequest&) override
		{
			m_lastError = "Static runtime " + m_versionName + " is not implemented yet.";
			return false;
		}

		bool AddLayerFromFiles(const SlLayerFileRequest&) override
		{
			m_lastError = "Static runtime " + m_versionName + " is not implemented yet.";
			return false;
		}

		size_t LoadedSkeletonCount() const noexcept override { return 0; }
		size_t ActiveSkeletonIndex() const noexcept override { return 0; }
		bool ChooseSkeleton(size_t) noexcept override { return false; }
		bool SkeletonLayerVisible(size_t) const noexcept override { return false; }
		bool SetSkeletonLayerVisible(size_t, bool) noexcept override { return false; }
		bool PromoteSkeleton(size_t) noexcept override { return false; }
		bool DemoteSkeleton(size_t) noexcept override { return false; }
		bool ContainsDrawableContent() const noexcept override { return false; }

		void TickPlayback(float) override {}
		void ResetViewScale() override {}
		void ResetViewScaleAll() override {}
		void PanByPixels(int, int) override {}
		void PanAllByPixels(int, int) override {}
		void StepToNextMotion() override {}
		void StepToPreviousMotion() override {}
		void StepToNextLook() override {}

		void PlayMotionByIndex(size_t) override {}
		void PlayMotionByName(const char*) override {}
		void RestartMotion(bool = true) override {}
		void ApplyLookByIndex(size_t) override {}
		void ApplyLookByName(const char*) override {}

		void TogglePremultiplyMode() override {}
		bool UsesPremultipliedAlpha(size_t = 0) override { return false; }
		bool SetPremultipliedAlpha(bool, size_t = 0) override { return false; }

		std::string ActiveMotionName() override { return {}; }
		std::string ActiveLookName() override { return {}; }
		std::string LastRuntimeIssue() const override { return m_lastError; }
		void ReadMotionClock(float* fTrack, float* fLast, float* fStart, float* fEnd) override
		{
			if (fTrack) *fTrack = 0.f;
			if (fLast) *fLast = 0.f;
			if (fStart) *fStart = 0.f;
			if (fEnd) *fEnd = 0.f;
		}
		float MotionDuration(const char*) override { return 0.f; }

		const std::vector<std::string>& SlotCatalog() const noexcept override { return m_emptyNames; }
		const SlNameList& LookNames() const noexcept override { return m_emptyNames; }
		const SlNameList& MotionNames() const noexcept override { return m_emptyNames; }

		void SetHiddenSlots(const std::vector<std::string>&) override {}
		void ComposeLooks(const SlNameList&) override {}
		void SetLayeredMotions(const SlNameList&, bool = false) override {}
		void SetSlotVisibilityRule(bool (*)(const char*, size_t)) override {}
		SlVec2 BaseSize() const noexcept override { return SlVec2{ 0.f, 0.f }; }
		void SetBaseSize(float, float) override {}
		void ClearBaseSize() override {}
		SlVec2 SkeletonContentSize() const override { return SlVec2{ 0.f, 0.f }; }
		SlVec2 ViewOffset() const noexcept override { return SlVec2{ 0.f, 0.f }; }
		void SetViewOffset(float, float) noexcept override {}
		SlVec2 ViewOffsetAt(size_t) const noexcept override { return SlVec2{ 0.f, 0.f }; }
		bool SetViewOffsetAt(size_t, float, float) noexcept override { return false; }
		bool CenterOpeningPoseInView() noexcept override { return false; }

		float SkeletonScale() const noexcept override { return 1.f; }
		void SetSkeletonScale(float) override {}
		float SkeletonScaleAt(size_t) const noexcept override { return 1.f; }
		bool SetSkeletonScaleAt(size_t, float) noexcept override { return false; }
		float CanvasScale() const noexcept override { return 1.f; }
		void SetCanvasScale(float) override {}
		float TimeScale() const noexcept override { return 1.f; }
		void SetTimeScale(float) override {}
		void SetBlendWindowSeconds(float) override {}

		bool IsMirroredHorizontally() const noexcept override { return false; }
		void ToggleMirrorX() noexcept override {}
		int QuarterTurns() const noexcept override { return 0; }
		void RotateClockwise() noexcept override {}
		bool ResetsViewOnLoad() const noexcept override { return false; }
		void SetResetViewOnLoad(bool) noexcept override {}

		SlMatrix4 ViewTransform() const noexcept override
		{
			return SlIdentityMatrix4();
		}

		SlVec4 MeasureSlotBounds(const std::string&) const override
		{
			return SlVec4{ 0.f, 0.f, 0.f, 0.f };
		}

		bool ReadSlotMesh(const std::string&, ReadSlotMeshData&) const override { return false; }
		void SetViewportSize(int, int) noexcept override {}

	private:
		std::string m_versionName;
		std::string m_lastError;
		std::vector<std::string> m_emptyNames;
	};

	std::unique_ptr<sl_runtime_v2::IRuntime> CreateStaticSpineRuntime(SlRuntimeHub::RuntimeLane slot)
	{
		switch (slot)
		{
		case SlRuntimeHub::RuntimeLane::Runtime21: return sl_runtime_v2::CreateCpp21Runtime();
		case SlRuntimeHub::RuntimeLane::Runtime34: return sl_runtime_v2::CreateCpp34Runtime();
		case SlRuntimeHub::RuntimeLane::Runtime35: return sl_runtime_v2::CreateCpp35Runtime();
		case SlRuntimeHub::RuntimeLane::Runtime36: return sl_runtime_v2::CreateCpp36Runtime();
		case SlRuntimeHub::RuntimeLane::Runtime37: return sl_runtime_v2::CreateCpp37Runtime();
		case SlRuntimeHub::RuntimeLane::Runtime38: return sl_runtime_v2::CreateCpp38Runtime();
		case SlRuntimeHub::RuntimeLane::Runtime40: return sl_runtime_v2::CreateCpp40Runtime();
		case SlRuntimeHub::RuntimeLane::Runtime41: return sl_runtime_v2::CreateCpp41Runtime();
		case SlRuntimeHub::RuntimeLane::Runtime42: return sl_runtime_v2::CreateCpp42Runtime();
		default: return nullptr;
		}
	}

	SlBlendMode ConvertBlendMode(sl_runtime_v2::BlendMode blendMode) noexcept
	{
		switch (blendMode)
		{
		case sl_runtime_v2::BlendMode::Additive: return SlBlendMode::Additive;
		case sl_runtime_v2::BlendMode::Multiply: return SlBlendMode::Multiply;
		case sl_runtime_v2::BlendMode::Screen: return SlBlendMode::Screen;
		case sl_runtime_v2::BlendMode::Normal:
		default: return SlBlendMode::Normal;
		}
	}

	class StaticRuntimeAdapter final : public SlPlaybackRuntime
	{
	public:
		explicit StaticRuntimeAdapter(SlRuntimeHub::RuntimeLane slot)
			: m_slot(slot)
		{}

		~StaticRuntimeAdapter() override
		{
			ClearSpines();
		}

		bool LoadBundleFromFiles(const SlFileBundleRequest& files) override
		{
			ClearSpines();
			const size_t count = files.atlasFiles.size() < files.skeletonFiles.size() ? files.atlasFiles.size() : files.skeletonFiles.size();
			if (count == 0)
			{
				m_lastError = "No Spine file pair was provided.";
				return false;
			}

			for (size_t i = 0; i < count; ++i)
			{
				sl_runtime_v2::LoadRequest request;
				request.atlasPaths.push_back(files.atlasFiles[i]);
				request.skeletonPaths.push_back(files.skeletonFiles[i]);
				request.binarySkeleton = files.binarySkeleton;
				if (!AppendRuntime(request))
					return false;
			}
			m_selectedSpine = 0;
			ApplyDefaultAnimation();
			return true;
		}

		bool LoadBundleFromMemory(const SlMemoryBundleRequest& memory) override
		{
			ClearSpines();
			const size_t count = memory.atlasText.size() < memory.skeletonBytes.size() ? memory.atlasText.size() : memory.skeletonBytes.size();
			if (count == 0)
			{
				m_lastError = "No Spine data pair was provided.";
				return false;
			}

			for (size_t i = 0; i < count; ++i)
			{
				sl_runtime_v2::LoadRequest request;
				request.atlasData.push_back(memory.atlasText[i]);
				if (i < memory.textureRoots.size())
					request.textureDirectories.push_back(memory.textureRoots[i]);
				request.skeletonData.push_back(memory.skeletonBytes[i]);
				request.binarySkeleton = memory.binarySkeleton;
				if (!AppendRuntime(request))
					return false;
			}
			m_selectedSpine = 0;
			ApplyDefaultAnimation();
			return true;
		}

		bool AddLayerFromFiles(const SlLayerFileRequest& files) override
		{
			if (files.atlasPath == nullptr || files.skeletonPath == nullptr)
			{
				m_lastError = "No Spine file pair was provided.";
				return false;
			}

			const size_t appendIndex = m_spines.size();
			sl_runtime_v2::LoadRequest request;
			request.atlasPaths.push_back(files.atlasPath);
			request.skeletonPaths.push_back(files.skeletonPath);
			request.binarySkeleton = files.binarySkeleton;
			if (!AppendRuntime(request))
				return false;
			if (appendIndex < m_spines.size() && m_spines[appendIndex])
				ApplyOpeningAnimation(*m_spines[appendIndex]);
			return true;
		}

		size_t LoadedSkeletonCount() const noexcept override { return m_spines.size(); }
		size_t ActiveSkeletonIndex() const noexcept override { return m_selectedSpine; }
		bool ChooseSkeleton(size_t index) noexcept override
		{
			if (index >= m_spines.size())
				return false;
			m_selectedSpine = index;
			return true;
		}

		bool SkeletonLayerVisible(size_t index) const noexcept override
		{
			return index < m_visible.size() ? m_visible[index] : false;
		}

		bool SetSkeletonLayerVisible(size_t index, bool visible) noexcept override
		{
			if (index >= m_visible.size())
				return false;
			m_visible[index] = visible;
			return true;
		}

		bool PromoteSkeleton(size_t index) noexcept override
		{
			if (index == 0 || index >= m_spines.size())
				return false;
			std::swap(m_spines[index], m_spines[index - 1]);
			std::swap(m_textureHandles[index], m_textureHandles[index - 1]);
			std::swap(m_d3d11TextureHandles[index], m_d3d11TextureHandles[index - 1]);
			std::swap(m_d3d11TexturePremul[index], m_d3d11TexturePremul[index - 1]);
			std::swap(m_layerOffsets[index], m_layerOffsets[index - 1]);
			std::swap(m_layerScales[index], m_layerScales[index - 1]);
			std::swap(m_layerMirrors[index], m_layerMirrors[index - 1]);
			std::swap(m_layerRotations[index], m_layerRotations[index - 1]);
			SwapVisible(index, index - 1);
			if (m_selectedSpine == index) m_selectedSpine = index - 1;
			else if (m_selectedSpine == index - 1) m_selectedSpine = index;
			return true;
		}

		bool DemoteSkeleton(size_t index) noexcept override
		{
			if (index + 1 >= m_spines.size())
				return false;
			std::swap(m_spines[index], m_spines[index + 1]);
			std::swap(m_textureHandles[index], m_textureHandles[index + 1]);
			std::swap(m_d3d11TextureHandles[index], m_d3d11TextureHandles[index + 1]);
			std::swap(m_d3d11TexturePremul[index], m_d3d11TexturePremul[index + 1]);
			std::swap(m_layerOffsets[index], m_layerOffsets[index + 1]);
			std::swap(m_layerScales[index], m_layerScales[index + 1]);
			std::swap(m_layerMirrors[index], m_layerMirrors[index + 1]);
			std::swap(m_layerRotations[index], m_layerRotations[index + 1]);
			SwapVisible(index, index + 1);
			if (m_selectedSpine == index) m_selectedSpine = index + 1;
			else if (m_selectedSpine == index + 1) m_selectedSpine = index;
			return true;
		}

		bool ContainsDrawableContent() const noexcept override { return !m_spines.empty(); }

		void TickPlayback(float fDelta) override
		{
			for (size_t i = 0; i < m_spines.size(); ++i)
			{
				if (i < m_visible.size() && m_visible[i])
					m_spines[i]->Update(fDelta * m_timeScale);
			}
			if (m_selectedSpine < m_visible.size() && m_visible[m_selectedSpine])
				m_currentAnimationTime += fDelta * m_timeScale;
		}

		bool RenderD3D11(sl_d3d11::D3D11Renderer& renderer)
		{
			ReleaseQueuedD3D11Textures(renderer);

			bool drewAny = false;
			for (size_t spineOffset = m_spines.size(); spineOffset > 0; --spineOffset)
			{
				const size_t spineIndex = spineOffset - 1;
				if (spineIndex >= m_visible.size() || !m_visible[spineIndex])
					continue;
				if (!EnsureD3D11Textures(spineIndex, renderer))
					continue;

				sl_runtime_v2::Frame frame;
				m_spines[spineIndex]->BuildFrame(m_renderWidth, m_renderHeight, frame);
				SlDrawList drawList;
				BuildDrawList(frame, drawList, spineIndex);
				renderer.Submit(drawList, m_d3d11TextureHandles[spineIndex]);
				drewAny = true;
			}
			return drewAny;
		}
		void ResetViewScale() override
		{
			ActiveLayerScale() = 1.0f;
			ActiveLayerOffset() = SlVec2{ 0.f, 0.f };
			m_canvasScale = 1.0f;
		}
		void ResetViewScaleAll() override
		{
			if (m_layerScales.size() < m_spines.size())
				m_layerScales.resize(m_spines.size(), m_skeletonScale);
			if (m_layerOffsets.size() < m_spines.size())
				m_layerOffsets.resize(m_spines.size(), m_offset);
			for (size_t i = 0; i < m_spines.size(); ++i)
			{
				if (i < m_visible.size() && !m_visible[i])
					continue;
				m_layerScales[i] = 1.0f;
				m_layerOffsets[i] = SlVec2{ 0.f, 0.f };
			}
			m_canvasScale = 1.0f;
		}
		void PanByPixels(int iX, int iY) override
		{
			SlVec2& offset = ActiveLayerOffset();
			offset.x += static_cast<float>(iX);
			offset.y += static_cast<float>(iY);
		}
		void PanAllByPixels(int iX, int iY) override
		{
			if (m_layerOffsets.size() < m_spines.size())
				m_layerOffsets.resize(m_spines.size(), m_offset);
			for (size_t i = 0; i < m_spines.size(); ++i)
			{
				if (i < m_visible.size() && !m_visible[i])
					continue;
				m_layerOffsets[i].x += static_cast<float>(iX);
				m_layerOffsets[i].y += static_cast<float>(iY);
			}
		}
		void StepToNextMotion() override { ShiftName(ActiveMotionNames(), true, false); }
		void StepToPreviousMotion() override { ShiftName(ActiveMotionNames(), false, false); }
		void StepToNextLook() override { ShiftName(ActiveLookNames(), true, true); }

		void PlayMotionByIndex(size_t nIndex) override
		{
			const auto& names = ActiveMotionNames();
			if (nIndex < names.size())
				PlayMotionByName(names[nIndex].c_str());
		}

		void PlayMotionByName(const char* szAnimationName) override
		{
			if (auto* runtime = SelectedRuntime())
			{
				runtime->StartMotion(szAnimationName, true);
				m_currentAnimationName = szAnimationName ? szAnimationName : "";
				m_currentAnimationTime = 0.0f;
				m_currentMotionDuration = 0.0f;
			}
		}

		void RestartMotion(bool loop = true) override
		{
			if (!m_currentAnimationName.empty())
			{
				if (auto* runtime = SelectedRuntime())
				{
					runtime->StartMotion(m_currentAnimationName.c_str(), loop);
					m_currentAnimationTime = 0.0f;
					m_currentMotionDuration = 0.0f;
				}
			}
		}

		void ApplyLookByIndex(size_t nIndex) override
		{
			const auto& names = ActiveLookNames();
			if (nIndex < names.size())
				ApplyLookByName(names[nIndex].c_str());
		}

		void ApplyLookByName(const char* szSkinName) override
		{
			if (auto* runtime = SelectedRuntime())
			{
				runtime->ApplyLook(szSkinName);
				m_currentSkinName = szSkinName ? szSkinName : "";
			}
		}

		void TogglePremultiplyMode() override
		{
			m_sourceIsPremultiplied = !m_sourceIsPremultiplied;
			ReloadTextureHandles();
		}
		bool UsesPremultipliedAlpha(size_t = 0) override { return m_sourceIsPremultiplied; }
		bool SetPremultipliedAlpha(bool isToBePremultiplied, size_t = 0) override
		{
			if (m_sourceIsPremultiplied != isToBePremultiplied)
			{
				m_sourceIsPremultiplied = isToBePremultiplied;
				ReloadTextureHandles();
			}
			return true;
		}

		std::string ActiveMotionName() override { return m_currentAnimationName; }
		std::string ActiveLookName() override { return m_currentSkinName; }
		std::string LastRuntimeIssue() const override { return m_lastError; }
		void ReadMotionClock(float* fTrack, float* fLast, float* fStart, float* fEnd) override
		{
			if (m_currentMotionDuration <= 0.0f && !m_currentAnimationName.empty())
			{
				const auto* runtime = SelectedRuntime();
				if (runtime)
					m_currentMotionDuration = runtime->MotionDuration(m_currentAnimationName.c_str());
			}
			if (fTrack) *fTrack = m_currentAnimationTime;
			if (fLast) *fLast = m_currentMotionDuration > 0.f ? std::fmod(m_currentAnimationTime, m_currentMotionDuration) : m_currentAnimationTime;
			if (fStart) *fStart = 0.f;
			if (fEnd) *fEnd = m_currentMotionDuration;
		}
		float MotionDuration(const char* name) override
		{
			const auto* runtime = SelectedRuntime();
			return runtime ? runtime->MotionDuration(name) : 0.0f;
		}

		const std::vector<std::string>& SlotCatalog() const noexcept override { return CurrentSlotCatalog(); }
		const SlNameList& LookNames() const noexcept override { return ActiveLookNames(); }
		const SlNameList& MotionNames() const noexcept override { return ActiveMotionNames(); }
		void SetHiddenSlots(const std::vector<std::string>& slotNames) override { m_excludedSlots = slotNames; }
		void ComposeLooks(const SlNameList&) override {}
		void SetLayeredMotions(const SlNameList& animationNames, bool loop = false) override
		{
			if (auto* runtime = SelectedRuntime())
				runtime->SetSecondaryMotions(animationNames, loop);
		}
		void SetSlotVisibilityRule(bool (*pFunc)(const char*, size_t)) override { m_slotExcludeCallback = pFunc; }
		SlVec2 BaseSize() const noexcept override { return m_baseSize; }
		void SetBaseSize(float fWidth, float fHeight) override { m_baseSize = SlVec2{ fWidth, fHeight }; }
		void ClearBaseSize() override { m_baseSize = SlVec2{ 0.f, 0.f }; }
		SlVec2 SkeletonContentSize() const override
		{
			const auto* runtime = SelectedRuntime();
			if (runtime == nullptr)
				return SlVec2{ 0.f, 0.f };

			sl_runtime_v2::Frame frame;
			const_cast<sl_runtime_v2::IRuntime*>(runtime)->BuildFrame(
				m_renderWidth > 0 ? m_renderWidth : 1,
				m_renderHeight > 0 ? m_renderHeight : 1,
				frame);

			PoseBounds bounds{};
			if (!FindPoseBounds(frame, bounds))
				return SlVec2{ 0.f, 0.f };

			return SlVec2{ bounds.maxX - bounds.minX, bounds.maxY - bounds.minY };
		}
		SlVec2 ViewOffset() const noexcept override { return LayerOffset(m_selectedSpine); }
		void SetViewOffset(float fX, float fY) noexcept override { ActiveLayerOffset() = SlVec2{ fX, fY }; }
		SlVec2 ViewOffsetAt(size_t index) const noexcept override { return LayerOffset(index); }
		bool SetViewOffsetAt(size_t index, float fX, float fY) noexcept override
		{
			if (index >= m_spines.size())
				return false;
			if (m_layerOffsets.size() < m_spines.size())
				m_layerOffsets.resize(m_spines.size(), m_offset);
			m_layerOffsets[index] = SlVec2{ fX, fY };
			return true;
		}
		bool CenterOpeningPoseInView() noexcept override
		{
			auto* runtime = SelectedRuntime();
			if (runtime == nullptr)
				return false;

			runtime->Update(0.0f);

			sl_runtime_v2::Frame frame;
			runtime->BuildFrame(
				m_renderWidth > 0 ? m_renderWidth : 1,
				m_renderHeight > 0 ? m_renderHeight : 1,
				frame);

			SlVec2 poseCenter{};
			if (!FindOpeningPoseCenter(frame, poseCenter))
			{
				ActiveLayerOffset() = SlVec2{ 0.f, 0.f };
				return false;
			}

			const float scale = ActiveLayerScale() * m_canvasScale;
			if (!std::isfinite(scale) || scale <= 0.0f)
			{
				ActiveLayerOffset() = SlVec2{ 0.f, 0.f };
				return false;
			}

			SlVec2& offset = ActiveLayerOffset();
			offset.x = (LayerMirror(m_selectedSpine) ? poseCenter.x : -poseCenter.x) * scale;
			offset.y = poseCenter.y * scale;
			return true;
		}
		float SkeletonScale() const noexcept override { return LayerScale(m_selectedSpine); }
		void SetSkeletonScale(float fScale) override { ActiveLayerScale() = fScale; }
		float SkeletonScaleAt(size_t index) const noexcept override { return LayerScale(index); }
		bool SetSkeletonScaleAt(size_t index, float fScale) noexcept override
		{
			if (index >= m_layerScales.size())
				return false;
			m_layerScales[index] = fScale;
			return true;
		}
		float CanvasScale() const noexcept override { return m_canvasScale; }
		void SetCanvasScale(float fScale) override { m_canvasScale = fScale; }
		float TimeScale() const noexcept override { return m_timeScale; }
		void SetTimeScale(float fTimeScale) override { m_timeScale = fTimeScale; }
		void SetBlendWindowSeconds(float seconds) override
		{
			m_mixSeconds = seconds > 0.0f ? seconds : 0.0f;
			for (auto& runtime : m_spines)
			{
				if (runtime)
					runtime->SetMotionBlendSeconds(m_mixSeconds);
			}
		}
		bool IsMirroredHorizontally() const noexcept override { return LayerMirror(m_selectedSpine); }
		void ToggleMirrorX() noexcept override { ActiveLayerMirrorFlag() = LayerMirror(m_selectedSpine) ? 0 : 1; }
		int QuarterTurns() const noexcept override { return LayerRotation(m_selectedSpine); }
		void RotateClockwise() noexcept override { ActiveLayerRotation() = (ActiveLayerRotation() + 1) % 4; }
		bool ResetsViewOnLoad() const noexcept override { return m_resetOffsetOnLoad; }
		void SetResetViewOnLoad(bool bReset) noexcept override { m_resetOffsetOnLoad = bReset; }
		SlMatrix4 ViewTransform() const noexcept override
		{
			return ViewTransformFor(m_selectedSpine);
		}
		SlMatrix4 ViewTransformFor(size_t spineIndex) const noexcept
		{
			const SlVec2 offset = LayerOffset(spineIndex);
			const float centerX = static_cast<float>(m_renderWidth) * 0.5f + offset.x;
			const float centerY = static_cast<float>(m_renderHeight) * 0.5f + offset.y;
			const float scale = LayerScale(spineIndex) * m_canvasScale;
			SlMatrix4 matrix{};
			matrix.m[2][2] = 1.0f;
			matrix.m[3][3] = 1.0f;
			const float signedScaleX = LayerMirror(spineIndex) ? -scale : scale;
			const float signedScaleY = -scale;
			switch (LayerRotation(spineIndex) & 3)
			{
			case 1:
				matrix.m[0][1] = signedScaleX;
				matrix.m[1][0] = -signedScaleY;
				break;
			case 2:
				matrix.m[0][0] = -signedScaleX;
				matrix.m[1][1] = -signedScaleY;
				break;
			case 3:
				matrix.m[0][1] = -signedScaleX;
				matrix.m[1][0] = signedScaleY;
				break;
			default:
				matrix.m[0][0] = signedScaleX;
				matrix.m[1][1] = signedScaleY;
				break;
			}
			matrix.m[3][0] = centerX;
			matrix.m[3][1] = centerY;
			return matrix;
		}
		SlVec4 MeasureSlotBounds(const std::string& slotName) const override
		{
			ReadSlotMeshData meshData;
			if (!ReadSlotMesh(slotName, meshData) || meshData.worldVertices.size() < 2)
				return SlVec4{ 0.f, 0.f, 0.f, 0.f };
			float minX = meshData.worldVertices[0];
			float maxX = minX;
			float minY = meshData.worldVertices[1];
			float maxY = minY;
			for (size_t i = 2; i + 1 < meshData.worldVertices.size(); i += 2)
			{
				const float x = meshData.worldVertices[i];
				const float y = meshData.worldVertices[i + 1];
				if (x < minX) minX = x;
				if (x > maxX) maxX = x;
				if (y < minY) minY = y;
				if (y > maxY) maxY = y;
			}
			return SlVec4{ minX, minY, maxX - minX, maxY - minY };
		}
		bool ReadSlotMesh(const std::string& slotName, ReadSlotMeshData& outData) const override
		{
			const auto* runtime = SelectedRuntime();
			if (runtime == nullptr || slotName.empty())
				return false;

			sl_runtime_v2::Frame frame;
			const_cast<sl_runtime_v2::IRuntime*>(runtime)->BuildFrame(m_renderWidth, m_renderHeight, frame);
			for (const auto& command : frame.draws)
			{
				if (command.slotName != slotName || command.vertices.empty())
					continue;
				outData.worldVertices.clear();
				outData.uvs.clear();
				outData.triangles = command.indices;
				outData.worldVertices.reserve(command.vertices.size() * 2);
				outData.uvs.reserve(command.vertices.size() * 2);
				for (const auto& vertex : command.vertices)
				{
					outData.worldVertices.push_back(vertex.x);
					outData.worldVertices.push_back(vertex.y);
					outData.uvs.push_back(vertex.u);
					outData.uvs.push_back(vertex.v);
				}
				outData.hullLength = command.vertices.size() == 4 ? 4 : 0;
				outData.textureHandle = TextureHandleForCommand(command);
				outData.isRegion = command.vertices.size() == 4;
				return true;
			}
			return false;
		}
		void SetViewportSize(int width, int height) noexcept override { m_renderWidth = width; m_renderHeight = height; }

	private:
		bool AppendRuntime(const sl_runtime_v2::LoadRequest& request)
		{
			std::unique_ptr<sl_runtime_v2::IRuntime> runtime = CreateStaticSpineRuntime(m_slot);
			if (!runtime)
			{
				m_lastError = "This Spine version does not have a static runtime yet.";
				return false;
			}

			if (!runtime->Load(request))
			{
				m_lastError = runtime->LastError();
				return false;
			}
			runtime->SetMotionBlendSeconds(m_mixSeconds);

			m_spines.push_back(std::move(runtime));
			m_textureHandles.push_back({});
			m_d3d11TextureHandles.push_back({});
			m_d3d11TexturePremul.push_back(false);
			m_visible.push_back(true);
			m_layerOffsets.push_back(LayerOffset(m_selectedSpine));
			m_layerScales.push_back(LayerScale(m_selectedSpine));
			m_layerMirrors.push_back(LayerMirror(m_selectedSpine) ? 1 : 0);
			m_layerRotations.push_back(LayerRotation(m_selectedSpine));
			m_lastError.clear();
			return true;
		}

		void ClearSpines()
		{
			for (auto& spineTextures : m_textureHandles)
				DeleteTextureHandles(spineTextures);
			m_textureHandles.clear();
			QueueD3D11TextureRelease();
			m_d3d11TextureHandles.clear();
			m_d3d11TexturePremul.clear();
			m_spines.clear();
			m_visible.clear();
			m_layerOffsets.clear();
			m_layerScales.clear();
			m_layerMirrors.clear();
			m_layerRotations.clear();
			m_selectedSpine = 0;
			m_currentAnimationName.clear();
			m_currentSkinName.clear();
			m_currentAnimationTime = 0.0f;
			m_currentMotionDuration = 0.0f;
			m_excludedSlots.clear();
			m_slotExcludeCallback = nullptr;
		}

		void LoadTextureHandles(const sl_runtime_v2::IRuntime& runtime, std::unordered_map<unsigned long long, int>& outHandles) const
		{
			outHandles.clear();
			(void)runtime;
		}

		static void DeleteTextureHandles(std::unordered_map<unsigned long long, int>& handles)
		{
			handles.clear();
		}

		void ReloadTextureHandles()
		{
			for (size_t i = 0; i < m_spines.size() && i < m_textureHandles.size(); ++i)
			{
				DeleteTextureHandles(m_textureHandles[i]);
				if (m_spines[i])
					LoadTextureHandles(*m_spines[i], m_textureHandles[i]);
			}
		}

		sl_runtime_v2::IRuntime* SelectedRuntime() const noexcept
		{
			if (m_selectedSpine >= m_spines.size())
				return nullptr;
			return m_spines[m_selectedSpine].get();
		}

		const std::vector<std::string>& ActiveMotionNames() const noexcept
		{
			const auto* runtime = SelectedRuntime();
			return runtime ? runtime->MotionNames() : m_emptyNames;
		}

		const std::vector<std::string>& ActiveLookNames() const noexcept
		{
			const auto* runtime = SelectedRuntime();
			return runtime ? runtime->LookNames() : m_emptyNames;
		}

		const std::vector<std::string>& CurrentSlotCatalog() const noexcept
		{
			const auto* runtime = SelectedRuntime();
			return runtime ? runtime->SlotCatalog() : m_emptyNames;
		}

		void ApplyDefaultAnimation()
		{
			for (const auto& runtime : m_spines)
			{
				if (runtime)
					ApplyOpeningAnimation(*runtime);
			}

			const auto& animationNames = ActiveMotionNames();
			m_currentAnimationName = animationNames.empty() ? std::string{} : animationNames.front();
			m_currentAnimationTime = 0.0f;
			m_currentMotionDuration = 0.0f;

			const auto& skinNames = ActiveLookNames();
			m_currentSkinName = skinNames.empty() ? std::string{} : skinNames.front();
		}

		void ApplyOpeningAnimation(sl_runtime_v2::IRuntime& runtime)
		{
			const std::vector<std::string>& animationNames = runtime.MotionNames();
			if (!animationNames.empty())
			{
				const auto current = std::find(animationNames.begin(), animationNames.end(), m_currentAnimationName);
				const std::string& animationName = current == animationNames.end() ? animationNames.front() : *current;
				runtime.StartMotion(animationName.c_str(), true);
			}

			const std::vector<std::string>& skinNames = runtime.LookNames();
			if (!skinNames.empty())
			{
				const auto current = std::find(skinNames.begin(), skinNames.end(), m_currentSkinName);
				const std::string& skinName = current == skinNames.end() ? skinNames.front() : *current;
				runtime.ApplyLook(skinName.c_str());
			}
		}

		void ShiftName(const std::vector<std::string>& names, bool forward, bool skin)
		{
			if (names.empty())
				return;
			const std::string& current = skin ? m_currentSkinName : m_currentAnimationName;
			auto it = std::find(names.begin(), names.end(), current);
			size_t index = it == names.end() ? 0 : static_cast<size_t>(it - names.begin());
			if (forward)
				index = (index + 1) % names.size();
			else
				index = index == 0 ? names.size() - 1 : index - 1;
			if (skin)
				ApplyLookByName(names[index].c_str());
			else
				PlayMotionByName(names[index].c_str());
		}

		void BuildDrawList(const sl_runtime_v2::Frame& frame, SlDrawList& outDrawList, size_t spineIndex) const
		{
			outDrawList.width = frame.width;
			outDrawList.height = frame.height;
			outDrawList.commands.clear();

			const SlMatrix4 transform = ViewTransformFor(spineIndex);

			for (const auto& command : frame.draws)
			{
				if (command.vertices.empty() || command.indices.empty())
					continue;
				if (IsSlotExcluded(command.slotName))
					continue;

				SlDrawCommand outCommand;
				outCommand.textureId = command.textureId;
				outCommand.slotName = command.slotName;
				outCommand.blendMode = ConvertBlendMode(command.blendMode);
				outCommand.premultipliedAlpha = command.premultipliedAlpha;
				outCommand.vertices.resize(command.vertices.size());
				outCommand.indices = command.indices;
				for (size_t i = 0; i < command.vertices.size(); ++i)
				{
					const sl_runtime_v2::Vertex& source = command.vertices[i];
					SlVertex2D& dest = outCommand.vertices[i];
					const SlVec3 transformed = SlVec3Transform(SlVec3(source.x, source.y, 0.0f), transform);
					dest.pos.x = transformed.x;
					dest.pos.y = transformed.y;
					dest.pos.z = 0.0f;
					dest.rhw = 1.0f;
					dest.color.r = source.color.r;
					dest.color.g = source.color.g;
					dest.color.b = source.color.b;
					dest.color.a = source.color.a;
					dest.uv.x = source.u;
					dest.uv.y = source.v;
				}
				outDrawList.commands.push_back(std::move(outCommand));
			}
		}

		bool IsSlotExcluded(const std::string& slotName) const
		{
			if (slotName.empty())
				return false;
			if (m_slotExcludeCallback && m_slotExcludeCallback(slotName.c_str(), slotName.size()))
				return true;
			return std::find(m_excludedSlots.begin(), m_excludedSlots.end(), slotName) != m_excludedSlots.end();
		}

		static bool IsUsablePoseCoordinate(float value) noexcept
		{
			constexpr float kReasonablePoseLimit = 1000000.0f;
			return std::isfinite(value) && value > -kReasonablePoseLimit && value < kReasonablePoseLimit;
		}

		struct PoseBounds
		{
			float minX = 0.0f;
			float minY = 0.0f;
			float maxX = 0.0f;
			float maxY = 0.0f;
		};

		bool FindPoseBounds(const sl_runtime_v2::Frame& frame, PoseBounds& outBounds) const
		{
			float minX = (std::numeric_limits<float>::max)();
			float minY = (std::numeric_limits<float>::max)();
			float maxX = -(std::numeric_limits<float>::max)();
			float maxY = -(std::numeric_limits<float>::max)();
			size_t acceptedVertices = 0;

			for (const auto& command : frame.draws)
			{
				if (command.vertices.empty() || command.indices.empty())
					continue;
				if (IsSlotExcluded(command.slotName))
					continue;

				for (unsigned short index : command.indices)
				{
					if (index >= command.vertices.size())
						continue;

					const sl_runtime_v2::Vertex& vertex = command.vertices[index];
					if (vertex.color.a <= 0.001f)
						continue;
					if (!IsUsablePoseCoordinate(vertex.x) || !IsUsablePoseCoordinate(vertex.y))
						continue;

					if (vertex.x < minX) minX = vertex.x;
					if (vertex.x > maxX) maxX = vertex.x;
					if (vertex.y < minY) minY = vertex.y;
					if (vertex.y > maxY) maxY = vertex.y;
					++acceptedVertices;
				}
			}

			if (acceptedVertices == 0 || minX > maxX || minY > maxY)
				return false;

			outBounds.minX = minX;
			outBounds.minY = minY;
			outBounds.maxX = maxX;
			outBounds.maxY = maxY;
			return true;
		}

		bool FindOpeningPoseCenter(const sl_runtime_v2::Frame& frame, SlVec2& outCenter) const
		{
			std::vector<float> xs;
			std::vector<float> ys;

			for (const auto& command : frame.draws)
			{
				if (command.vertices.empty() || command.indices.empty())
					continue;
				if (IsSlotExcluded(command.slotName))
					continue;

				for (unsigned short index : command.indices)
				{
					if (index >= command.vertices.size())
						continue;

					const sl_runtime_v2::Vertex& vertex = command.vertices[index];
					if (vertex.color.a <= 0.001f)
						continue;
					if (!IsUsablePoseCoordinate(vertex.x) || !IsUsablePoseCoordinate(vertex.y))
						continue;

					xs.push_back(vertex.x);
					ys.push_back(vertex.y);
				}
			}

			if (xs.empty() || ys.empty())
				return false;

			std::sort(xs.begin(), xs.end());
			std::sort(ys.begin(), ys.end());
			outCenter.x = RobustAxisCenter(xs);
			outCenter.y = RobustAxisCenter(ys);
			return std::isfinite(outCenter.x) && std::isfinite(outCenter.y);
		}

		static float RobustAxisCenter(const std::vector<float>& sortedValues) noexcept
		{
			const size_t count = sortedValues.size();
			if (count == 0)
				return 0.0f;
			if (count < 16)
				return (sortedValues.front() + sortedValues.back()) * 0.5f;

			const size_t trim = count / 10;
			const size_t lowIndex = trim < count ? trim : 0;
			const size_t highIndex = count > trim + 1 ? count - trim - 1 : count - 1;
			const float trimmedCenter = (sortedValues[lowIndex] + sortedValues[highIndex]) * 0.5f;

			const float fullSpan = sortedValues.back() - sortedValues.front();
			const float trimmedSpan = sortedValues[highIndex] - sortedValues[lowIndex];
			if (trimmedSpan > 0.0f && fullSpan > trimmedSpan * 3.0f)
				return sortedValues[count / 2];

			return trimmedCenter;
		}

		int TextureHandleForCommand(const sl_runtime_v2::DrawCommand& command) const
		{
			if (m_selectedSpine < m_d3d11TextureHandles.size())
			{
				const auto it = m_d3d11TextureHandles[m_selectedSpine].find(command.textureId);
				if (it != m_d3d11TextureHandles[m_selectedSpine].end())
					return static_cast<int>(it->second);
			}
			return 1;
		}

		bool EnsureD3D11Textures(size_t spineIndex, sl_d3d11::D3D11Renderer& renderer)
		{
			if (spineIndex >= m_spines.size())
				return false;
			if (spineIndex >= m_d3d11TextureHandles.size() || spineIndex >= m_d3d11TexturePremul.size())
				return false;

			const bool premultiply = !m_sourceIsPremultiplied;
			if (!m_d3d11TextureHandles[spineIndex].empty() && m_d3d11TexturePremul[spineIndex] == premultiply)
				return true;

			ReleaseD3D11TextureMap(renderer, m_d3d11TextureHandles[spineIndex]);
			m_d3d11TextureHandles[spineIndex].clear();
			m_d3d11TexturePremul[spineIndex] = premultiply;

			for (const auto& texture : m_spines[spineIndex]->TextureInfos())
			{
				const std::wstring widePath = sl_text::Utf8ToWide(texture.path);
				const SlTextureId handle = renderer.LoadTexture(widePath.c_str(), ShouldPremultiplyTexture(texture));
				if (handle != 0)
					m_d3d11TextureHandles[spineIndex][texture.id] = handle;
			}
			return !m_d3d11TextureHandles[spineIndex].empty();
		}

		static void ReleaseD3D11TextureMap(sl_d3d11::D3D11Renderer& renderer, std::unordered_map<std::uint64_t, SlTextureId>& textures) noexcept
		{
			for (const auto& entry : textures)
				renderer.ReleaseTexture(entry.second);
			textures.clear();
		}

		void QueueD3D11TextureRelease()
		{
			for (auto& textures : m_d3d11TextureHandles)
			{
				if (!textures.empty())
					m_d3d11TextureReleaseQueue.push_back(std::move(textures));
			}
		}

		void ReleaseQueuedD3D11Textures(sl_d3d11::D3D11Renderer& renderer)
		{
			for (auto& textures : m_d3d11TextureReleaseQueue)
				ReleaseD3D11TextureMap(renderer, textures);
			m_d3d11TextureReleaseQueue.clear();
		}

		bool ShouldPremultiplyTexture(const sl_runtime_v2::TextureInfo& texture) const noexcept
		{
			return texture.renderPremultipliedAlpha && !SourcePremultipliedForTexture(texture);
		}

		bool SourcePremultipliedForTexture(const sl_runtime_v2::TextureInfo&) const noexcept
		{
			return m_sourceIsPremultiplied;
		}

		void SwapVisible(size_t a, size_t b) noexcept
		{
			const bool visibleA = m_visible[a];
			m_visible[a] = m_visible[b];
			m_visible[b] = visibleA;
		}

		SlVec2 LayerOffset(size_t index) const noexcept
		{
			return index < m_layerOffsets.size() ? m_layerOffsets[index] : m_offset;
		}

		SlVec2& ActiveLayerOffset() noexcept
		{
			if (m_selectedSpine >= m_layerOffsets.size())
			{
				if (m_layerOffsets.size() < m_spines.size())
					m_layerOffsets.resize(m_spines.size(), m_offset);
				if (m_selectedSpine >= m_layerOffsets.size())
					return m_offset;
			}
			return m_layerOffsets[m_selectedSpine];
		}

		float LayerScale(size_t index) const noexcept
		{
			return index < m_layerScales.size() ? m_layerScales[index] : m_skeletonScale;
		}

		float& ActiveLayerScale() noexcept
		{
			if (m_selectedSpine >= m_layerScales.size())
			{
				if (m_layerScales.size() < m_spines.size())
					m_layerScales.resize(m_spines.size(), m_skeletonScale);
				if (m_selectedSpine >= m_layerScales.size())
					return m_skeletonScale;
			}
			return m_layerScales[m_selectedSpine];
		}

		bool LayerMirror(size_t index) const noexcept
		{
			return index < m_layerMirrors.size() ? m_layerMirrors[index] : m_flipX;
		}

		unsigned char& ActiveLayerMirrorFlag() noexcept
		{
			if (m_selectedSpine >= m_layerMirrors.size())
			{
				if (m_layerMirrors.size() < m_spines.size())
					m_layerMirrors.resize(m_spines.size(), m_flipX ? 1 : 0);
				if (m_selectedSpine >= m_layerMirrors.size())
					return m_fallbackMirror;
			}
			return m_layerMirrors[m_selectedSpine];
		}

		int LayerRotation(size_t index) const noexcept
		{
			return index < m_layerRotations.size() ? m_layerRotations[index] : m_rotationSteps;
		}

		int& ActiveLayerRotation() noexcept
		{
			if (m_selectedSpine >= m_layerRotations.size())
			{
				if (m_layerRotations.size() < m_spines.size())
					m_layerRotations.resize(m_spines.size(), m_rotationSteps);
				if (m_selectedSpine >= m_layerRotations.size())
					return m_rotationSteps;
			}
			return m_layerRotations[m_selectedSpine];
		}

		SlRuntimeHub::RuntimeLane m_slot;
		std::vector<std::unique_ptr<sl_runtime_v2::IRuntime>> m_spines;
		std::vector<std::unordered_map<unsigned long long, int>> m_textureHandles;
		std::vector<std::unordered_map<std::uint64_t, SlTextureId>> m_d3d11TextureHandles;
		std::vector<std::unordered_map<std::uint64_t, SlTextureId>> m_d3d11TextureReleaseQueue;
		std::vector<bool> m_d3d11TexturePremul;
		std::vector<bool> m_visible;
		std::vector<SlVec2> m_layerOffsets;
		std::vector<float> m_layerScales;
		std::vector<unsigned char> m_layerMirrors;
		std::vector<int> m_layerRotations;
		unsigned char m_fallbackMirror = 0;
		size_t m_selectedSpine = 0;
		std::string m_lastError;
		std::string m_currentAnimationName;
		std::string m_currentSkinName;
		std::vector<std::string> m_emptyNames;
		std::vector<std::string> m_excludedSlots;
		bool (*m_slotExcludeCallback)(const char*, size_t) = nullptr;
		SlVec2 m_baseSize{ 0.f, 0.f };
		SlVec2 m_offset{ 0.f, 0.f };
		float m_skeletonScale = 1.f;
		float m_canvasScale = 1.f;
		float m_timeScale = 1.f;
		float m_mixSeconds = 0.f;
		float m_currentAnimationTime = 0.f;
		float m_currentMotionDuration = 0.f;
		bool m_sourceIsPremultiplied = true;
		bool m_flipX = false;
		bool m_resetOffsetOnLoad = false;
		int m_rotationSteps = 0;
		int m_renderWidth = 0;
		int m_renderHeight = 0;
	};

	std::unique_ptr<SlPlaybackRuntime> CreateStaticRuntime(SlRuntimeHub::RuntimeLane slot)
	{
		const RuntimeDescriptor* descriptor = FindRuntimeDescriptor(slot);
		if (CreateStaticSpineRuntime(slot))
			return std::unique_ptr<SlPlaybackRuntime>(new StaticRuntimeAdapter(slot));
		return std::unique_ptr<SlPlaybackRuntime>(
			new UnavailableRuntimeAdapter(descriptor ? descriptor->versionPrefix : "unknown"));
	}
}

SlRuntimeHub::SlRuntimeHub()
{
	RebuildRuntimePool();
}

SlRuntimeHub::~SlRuntimeHub() = default;

bool SlRuntimeHub::RebuildRuntimePool()
{
	m_runtimePoolReady = true;

	const auto& catalog = RuntimeCatalog();
	for (size_t descriptorIndex = 0; descriptorIndex < catalog.size(); ++descriptorIndex)
	{
		const RuntimeDescriptor& descriptor = catalog[descriptorIndex];
		const size_t runtimeIndex = static_cast<size_t>(descriptor.slot);
		m_runtimeSlots[runtimeIndex] = CreateStaticRuntime(descriptor.slot);
		if (!m_runtimeSlots[runtimeIndex])
			m_runtimePoolReady = false;
	}

	return m_runtimePoolReady;
}

bool SlRuntimeHub::RuntimePoolReady() const noexcept
{
	return m_runtimePoolReady;
}

SlRuntimeHub::RuntimeLane SlRuntimeHub::LaneForVersionText(const char* version) const noexcept
{
	const RuntimeDescriptor* descriptor = MatchRuntimeVersion(version);
	return descriptor != nullptr ? descriptor->slot : RuntimeLane::Unknown;
}

bool SlRuntimeHub::ActivateLane(RuntimeLane slot) noexcept
{
	if (slot == RuntimeLane::Unknown)
		return false;
	if (FindRuntimeDescriptor(slot) == nullptr)
		return false;

	m_currentLane = slot;
	return true;
}

SlRuntimeHub::RuntimeLane SlRuntimeHub::CurrentLane() const noexcept
{
	return m_currentLane;
}

SlPlaybackRuntime* SlRuntimeHub::RuntimeForLane(RuntimeLane slot) const
{
	if (slot == RuntimeLane::Unknown)
		return nullptr;

	const size_t index = static_cast<size_t>(slot);
	if (index >= RuntimeLaneCount || !m_runtimeSlots[index])
		return nullptr;
	return m_runtimeSlots[index].get();
}

bool SlRuntimeHub::LaneIsReady(RuntimeLane slot) const
{
	return RuntimeForLane(slot) != nullptr;
}

SlPlaybackRuntime* SlRuntimeHub::CurrentRuntime() const
{
	return RuntimeForLane(m_currentLane);
}

bool SlRuntimeHub::RenderCurrentRuntimeD3D11(sl_d3d11::D3D11Renderer& renderer)
{
	SlPlaybackRuntime* runtime = CurrentRuntime();
	auto* staticRuntime = dynamic_cast<StaticRuntimeAdapter*>(runtime);
	if (staticRuntime == nullptr)
		return false;
	return staticRuntime->RenderD3D11(renderer);
}
