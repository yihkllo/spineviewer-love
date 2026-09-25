#include "spinelove/spine_runtime_registry.h"


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
#include "spinelove/sl_gfx_draw.h"
#if defined(SL_RENDER_D3D11)
#include "common/sl_text_codec.h"
#include "render_d3d11/d3d11_renderer.h"
#endif

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

	const std::array<RuntimeDescriptor, 10>& RuntimeCatalog()
	{
		static const std::array<RuntimeDescriptor, 10> catalog =
		{
			MakeRuntimeDescriptor(SlRuntimeHub::RuntimeLane::Runtime21, "2.1"),
			MakeRuntimeDescriptor(SlRuntimeHub::RuntimeLane::Runtime31, "3.1"),
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
		float SkeletonLayerOpacity(size_t) const noexcept override { return 1.0f; }
		bool SetSkeletonLayerOpacity(size_t, float) noexcept override { return false; }
		SlColor SkeletonLayerTint(size_t) const noexcept override { return SlColor{}; }
		bool SetSkeletonLayerTint(size_t, const SlColor&) noexcept override { return false; }
		SlColor SkeletonLayerLerpTint(size_t) const noexcept override { return SlColor(1.0f, 1.0f, 1.0f, 0.0f); }
		bool SetSkeletonLayerLerpTint(size_t, const SlColor&) noexcept override { return false; }
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
		bool StartMotionByName(const char*, bool, float = -1.0f) override { return false; }
		bool QueueMotionByName(const char*, bool, float = -1.0f) override { return false; }
		bool SetActiveMotionTimeScale(float) noexcept override { return false; }
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
		void DrainMotionEventsAt(size_t, SlMotionEventList& events) override { events.clear(); }

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
		bool SetSkeletonTransformAt(size_t, float, float, float) noexcept override { return false; }
		float CanvasScale() const noexcept override { return 1.f; }
		void SetCanvasScale(float) override {}
		float TimeScale() const noexcept override { return 1.f; }
		void SetTimeScale(float) override {}
		bool SetTimeScaleAt(size_t, float) noexcept override { return false; }
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
		case SlRuntimeHub::RuntimeLane::Runtime31: return sl_runtime_v2::CreateCpp31Runtime();
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

			const size_t appendIndex = m_layers.size();
			sl_runtime_v2::LoadRequest request;
			request.atlasPaths.push_back(files.atlasPath);
			request.skeletonPaths.push_back(files.skeletonPath);
			request.binarySkeleton = files.binarySkeleton;
			if (!AppendRuntime(request))
				return false;
			if (appendIndex < m_layers.size() && m_layers[appendIndex].runtime)
                ApplyOpeningAnimation(m_layers[appendIndex]);
			return true;
		}

		bool AddLayerFromMemory(const SlMemoryBundleRequest& memory) override
		{
			if (memory.atlasText.size() != 1 || memory.skeletonBytes.size() != 1)
			{
				m_lastError = "One Spine data pair is required to append a layer.";
				return false;
			}
			const size_t appendIndex = m_layers.size();
			sl_runtime_v2::LoadRequest request;
			request.atlasData = memory.atlasText;
			request.skeletonData = memory.skeletonBytes;
			request.textureDirectories = memory.textureRoots;
			request.binarySkeleton = memory.binarySkeleton;
			if (!AppendRuntime(request)) return false;
			if (appendIndex < m_layers.size() && m_layers[appendIndex].runtime)
                ApplyOpeningAnimation(m_layers[appendIndex]);
			return true;
		}

		size_t LoadedSkeletonCount() const noexcept override { return m_layers.size(); }
		size_t ActiveSkeletonIndex() const noexcept override { return m_selectedSpine; }
		bool ChooseSkeleton(size_t index) noexcept override
		{
			if (index >= m_layers.size())
				return false;
			m_selectedSpine = index;
			InvalidateProbeFrame();
			const SpineLayer& layer = m_layers[index];
            m_currentAnimationName = layer.animationName;
            m_currentSkinName = layer.skinName;
			m_currentAnimationTime = layer.animationTime;
			m_currentMotionDuration = layer.motionDuration;
			return true;
		}

		bool SkeletonLayerVisible(size_t index) const noexcept override
		{
			const SpineLayer* layer = LayerAt(index);
			return layer != nullptr && layer->visible;
		}

		bool SetSkeletonLayerVisible(size_t index, bool visible) noexcept override
		{
			SpineLayer* layer = LayerAt(index);
			if (layer == nullptr)
				return false;
			layer->visible = visible;
			return true;
		}

		float SkeletonLayerOpacity(size_t index) const noexcept override
		{
			const SpineLayer* layer = LayerAt(index);
			return layer != nullptr ? layer->opacity : 1.0f;
		}

		bool SetSkeletonLayerOpacity(size_t index, float opacity) noexcept override
		{
			SpineLayer* layer = LayerAt(index);
			if (layer == nullptr) return false;
			layer->opacity = (std::max)(0.0f, (std::min)(1.0f, opacity));
			return true;
		}
		SlColor SkeletonLayerTint(size_t index) const noexcept override
		{
			const SpineLayer* layer = LayerAt(index);
			return layer != nullptr ? layer->tint : SlColor{};
		}
		bool SetSkeletonLayerTint(size_t index, const SlColor& color) noexcept override
		{
			SpineLayer* layer = LayerAt(index);
			if (layer == nullptr) return false;
			layer->tint = ClampColor(color);
			return true;
		}
		SlColor SkeletonLayerLerpTint(size_t index) const noexcept override
		{
			const SpineLayer* layer = LayerAt(index);
			return layer != nullptr ? layer->lerpTint : SlColor(1.0f, 1.0f, 1.0f, 0.0f);
		}
		bool SetSkeletonLayerLerpTint(size_t index, const SlColor& color) noexcept override
		{
			SpineLayer* layer = LayerAt(index);
			if (layer == nullptr) return false;
			layer->lerpTint = ClampColor(color);
			return true;
		}

		bool PromoteSkeleton(size_t index) noexcept override
		{
			if (index == 0 || index >= m_layers.size())
				return false;
			std::swap(m_layers[index], m_layers[index - 1]);
			InvalidateProbeFrame();
			if (m_selectedSpine == index) m_selectedSpine = index - 1;
			else if (m_selectedSpine == index - 1) m_selectedSpine = index;
			SyncActiveMotionBookkeeping();
			return true;
		}

		bool DemoteSkeleton(size_t index) noexcept override
		{
			if (index + 1 >= m_layers.size())
				return false;
			std::swap(m_layers[index], m_layers[index + 1]);
			InvalidateProbeFrame();
			if (m_selectedSpine == index) m_selectedSpine = index + 1;
			else if (m_selectedSpine == index + 1) m_selectedSpine = index;
			SyncActiveMotionBookkeeping();
			return true;
		}

		bool ContainsDrawableContent() const noexcept override { return !m_layers.empty(); }

		void TickPlayback(float fDelta) override
		{
			InvalidateProbeFrame();
			for (SpineLayer& layer : m_layers)
			{
				if (!layer.visible || !layer.runtime)
					continue;
				const float step = fDelta * m_timeScale * layer.timeScale;
				layer.runtime->Update(step);
				layer.animationTime += step;
			}
			if (const SpineLayer* layer = ActiveLayer())
				m_currentAnimationTime = layer->animationTime;
		}

		bool RenderD3D11(SlSceneRenderer& renderer)
		{
			ReleaseQueuedD3D11Textures(renderer);
			m_lastRenderBoundsValid = false;

			bool drewAny = false;
			for (size_t spineOffset = m_layers.size(); spineOffset > 0; --spineOffset)
			{
				const size_t spineIndex = spineOffset - 1;
				SpineLayer& layer = m_layers[spineIndex];
				if (!layer.visible || !layer.runtime)
					continue;
				if (!EnsureD3D11Textures(spineIndex, renderer))
					continue;

				layer.runtime->BuildFrame(m_renderWidth, m_renderHeight, m_renderFrame);
				BuildDrawList(m_renderFrame, m_renderDrawList, spineIndex);
				renderer.Submit(m_renderDrawList, layer.textures);
				drewAny = true;
			}
			return drewAny;
		}

		bool QueryLastRenderedBounds(SlRect& outBounds) const noexcept
		{
			if (!m_lastRenderBoundsValid)
				return false;
			outBounds = m_lastRenderBounds;
			return true;
		}

		void ResetViewScale() override
		{
			ActiveLayerScale() = 1.0f;
			ActiveLayerOffset() = SlVec2{ 0.f, 0.f };
			m_canvasScale = 1.0f;
		}
		void ResetViewScaleAll() override
		{
			for (SpineLayer& layer : m_layers)
			{
				if (!layer.visible)
					continue;
				layer.scale = 1.0f;
				layer.offset = SlVec2{ 0.f, 0.f };
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
			for (SpineLayer& layer : m_layers)
			{
				if (!layer.visible)
					continue;
				layer.offset.x += static_cast<float>(iX);
				layer.offset.y += static_cast<float>(iY);
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
				InvalidateProbeFrame();
				runtime->StartMotion(szAnimationName, true);
				RecordActiveMotion(szAnimationName ? szAnimationName : "");
			}
		}
		bool StartMotionByName(const char* animationName, bool loop, float mixSeconds = -1.0f) override
		{
			auto* runtime = SelectedRuntime();
			if (!runtime || !runtime->StartMotionWithMix(animationName, loop, mixSeconds)) return false;
			InvalidateProbeFrame();
			RecordActiveMotion(animationName ? animationName : "");
			return true;
		}
		bool QueueMotionByName(const char* animationName, bool loop, float mixSeconds = -1.0f) override
		{
			auto* runtime = SelectedRuntime();
			return runtime && runtime->QueueMotion(animationName, loop, mixSeconds);
		}
		bool SetActiveMotionTimeScale(float timeScale) noexcept override
		{
			auto* runtime = SelectedRuntime();
			return runtime && runtime->SetCurrentMotionTimeScale(timeScale);
		}

		void RestartMotion(bool loop = true) override
		{
			const SpineLayer* activeLayer = ActiveLayer();
			const std::string animationName = activeLayer != nullptr
				? activeLayer->animationName : m_currentAnimationName;
			if (!animationName.empty())
			{
				if (auto* runtime = SelectedRuntime())
				{
					InvalidateProbeFrame();
					runtime->StartMotion(animationName.c_str(), loop);
					RecordActiveMotion(animationName);
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
				InvalidateProbeFrame();
                runtime->ApplyLook(szSkinName);
                m_currentSkinName = szSkinName ? szSkinName : "";
                if (SpineLayer* layer = ActiveLayer()) layer->skinName = m_currentSkinName;
			}
		}

		void TogglePremultiplyMode() override
		{
			m_sourceIsPremultiplied = !m_sourceIsPremultiplied;
		}
		bool UsesPremultipliedAlpha(size_t = 0) override { return m_sourceIsPremultiplied; }
		bool SetPremultipliedAlpha(bool isToBePremultiplied, size_t = 0) override
		{
			m_sourceIsPremultiplied = isToBePremultiplied;
			return true;
		}

		std::string ActiveMotionName() override
		{
			const SpineLayer* layer = ActiveLayer();
			return layer != nullptr ? layer->animationName : m_currentAnimationName;
		}
        std::string ActiveLookName() override
        {
            const SpineLayer* layer = ActiveLayer();
            return layer != nullptr ? layer->skinName : m_currentSkinName;
        }
		std::string LastRuntimeIssue() const override { return m_lastError; }
		void ReadMotionClock(float* fTrack, float* fLast, float* fStart, float* fEnd) override
		{
			const std::string animationName = ActiveMotionName();
			SpineLayer* activeLayer = ActiveLayer();
			float track = activeLayer != nullptr ? activeLayer->animationTime : m_currentAnimationTime;
			float duration = activeLayer != nullptr ? activeLayer->motionDuration : m_currentMotionDuration;
			if (duration <= 0.0f && !animationName.empty())
			{
				const auto* runtime = SelectedRuntime();
				if (runtime)
					duration = runtime->MotionDuration(animationName.c_str());
			}
			if (activeLayer != nullptr)
				activeLayer->motionDuration = duration;
			m_currentAnimationTime = track;
			m_currentMotionDuration = duration;
			if (fTrack) *fTrack = track;
			if (fLast) *fLast = duration > 0.f ? std::fmod(track, duration) : track;
			if (fStart) *fStart = 0.f;
			if (fEnd) *fEnd = duration;
		}
		float MotionDuration(const char* name) override
		{
			const auto* runtime = SelectedRuntime();
			return runtime ? runtime->MotionDuration(name) : 0.0f;
		}
		void DrainMotionEventsAt(size_t index, SlMotionEventList& events) override
		{
			events.clear();
			if (index >= m_layers.size() || !m_layers[index].runtime) return;
			std::vector<sl_runtime_v2::AnimationEvent> runtimeEvents;
			m_layers[index].runtime->DrainAnimationEvents(runtimeEvents);
			events.reserve(runtimeEvents.size());
			for (auto& event : runtimeEvents)
			{
				SlMotionEvent value;
				value.name = std::move(event.name);
				value.stringValue = std::move(event.stringValue);
				value.intValue = event.intValue;
				value.floatValue = event.floatValue;
				value.time = event.time;
				events.push_back(std::move(value));
			}
		}

		const std::vector<std::string>& SlotCatalog() const noexcept override { return CurrentSlotCatalog(); }
		const SlNameList& LookNames() const noexcept override { return ActiveLookNames(); }
		const SlNameList& MotionNames() const noexcept override { return ActiveMotionNames(); }
		bool SetSlotOverride(const std::string& slotName, float alpha,
			SlSlotAttachmentMode attachmentMode) override
		{
			auto* runtime = SelectedRuntime();
			if (!runtime) return false;
			InvalidateProbeFrame();
			sl_runtime_v2::SlotAttachmentMode mode = sl_runtime_v2::SlotAttachmentMode::Preserve;
			if (attachmentMode == SlSlotAttachmentMode::SetupIfEmpty)
				mode = sl_runtime_v2::SlotAttachmentMode::SetupIfEmpty;
			else if (attachmentMode == SlSlotAttachmentMode::NamedIfEmpty)
				mode = sl_runtime_v2::SlotAttachmentMode::NamedIfEmpty;
			else if (attachmentMode == SlSlotAttachmentMode::Clear)
				mode = sl_runtime_v2::SlotAttachmentMode::Clear;
			return runtime->SetSlotOverride(slotName.c_str(), alpha, mode);
		}
		void ClearSlotOverrides() override
		{
			if (auto* runtime = SelectedRuntime())
			{
				InvalidateProbeFrame();
				runtime->ClearSlotOverrides();
			}
		}
		void SetHiddenSlots(const std::vector<std::string>& slotNames) override
		{
			if (SpineLayer* layer = ActiveLayer())
				layer->excludedSlots = slotNames;
		}
		void ComposeLooks(const SlNameList& looks) override
		{
			if (auto* runtime = SelectedRuntime())
			{
				InvalidateProbeFrame();
                runtime->ComposeLooks(looks);
                m_currentSkinName = looks.empty() ? std::string{} : looks.back();
                if (SpineLayer* layer = ActiveLayer()) layer->skinName = m_currentSkinName;
			}
		}
		bool SetNativeSkinMix(const SlNameList& names) override
		{
			if (auto* runtime = SelectedRuntime())
			{
				InvalidateProbeFrame();
				return runtime->SetNativeSkinMix(names);
			}
			return false;
		}
		void SetLayeredMotions(const SlNameList& animationNames, bool loop = false) override
		{
			if (auto* runtime = SelectedRuntime())
				runtime->SetSecondaryMotions(animationNames, loop);
		}
		bool StartMotionOnTrack(int track, const char* name, bool loop, float mix) override
		{
			if (auto* r = SelectedRuntime()) { InvalidateProbeFrame(); return r->StartMotionOnTrack(track, name, loop, mix); }
			return false;
		}
		bool ClearMotionTrack(int track, float mix) override
		{
			if (auto* r = SelectedRuntime()) { InvalidateProbeFrame(); return r->ClearMotionTrack(track, mix); }
			return false;
		}
		bool HoldMotionTrack(int track, float time) override
		{
			if (auto* r = SelectedRuntime()) { InvalidateProbeFrame(); return r->HoldMotionTrack(track, time); }
			return false;
		}
		void EnableMotionCompletions(bool enabled) override
		{
			if (auto* r = SelectedRuntime()) r->EnableMotionCompletions(enabled);
		}
		bool SetNativeMotionTrack(const SlNativeMotionTrack& value) override
		{
			if (auto* r = SelectedRuntime()) { InvalidateProbeFrame(); return r->SetNativeMotionTrack(value); }
			return false;
		}
		void DrainMotionCompletions(std::vector<SlMotionCompletion>& events) override
		{
			events.clear();
			if (auto* r = SelectedRuntime()) {
				std::vector<sl_runtime_v2::AnimationCompletion> completed;
				r->DrainMotionCompletions(completed);
				for (const auto& item : completed) events.push_back({item.animation,item.track,item.time});
			}
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

			if (m_probeFrameGeneration != m_stateGeneration)
			{
				const_cast<sl_runtime_v2::IRuntime*>(runtime)->BuildFrame(m_renderWidth, m_renderHeight, m_probeFrame);
				m_probeFrameGeneration = m_stateGeneration;
			}

			PoseBounds bounds{};
			if (!FindPoseBounds(m_probeFrame, bounds))
				return SlVec2{ 0.f, 0.f };

			return SlVec2{ bounds.maxX - bounds.minX, bounds.maxY - bounds.minY };
		}
		SlVec2 ViewOffset() const noexcept override { return LayerOffset(m_selectedSpine); }
		void SetViewOffset(float fX, float fY) noexcept override { ActiveLayerOffset() = SlVec2{ fX, fY }; }
		SlVec2 ViewOffsetAt(size_t index) const noexcept override { return LayerOffset(index); }
		bool SetViewOffsetAt(size_t index, float fX, float fY) noexcept override
		{
			SpineLayer* layer = LayerAt(index);
			if (layer == nullptr)
				return false;
			layer->offset = SlVec2{ fX, fY };
			return true;
		}
		bool CenterOpeningPoseInView() noexcept override
		{
			auto* runtime = SelectedRuntime();
			if (runtime == nullptr)
				return false;

			InvalidateProbeFrame();
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
			SpineLayer* layer = LayerAt(index);
			if (layer == nullptr)
				return false;
			layer->scale = fScale;
			return true;
		}
		bool SetSkeletonTransformAt(size_t index, float scaleX, float scaleY,
			float rotationDegrees) noexcept override
		{
			SpineLayer* layer = LayerAt(index);
			if (layer == nullptr) return false;
			layer->transformScale = SlVec2{ scaleX, scaleY };
			layer->angle = rotationDegrees;
			return true;
		}
		float CanvasScale() const noexcept override { return m_canvasScale; }
		void SetCanvasScale(float fScale) override { m_canvasScale = fScale; }
		float TimeScale() const noexcept override { return m_timeScale; }
		void SetTimeScale(float fTimeScale) override { m_timeScale = fTimeScale; }
		bool SetTimeScaleAt(size_t index, float timeScale) noexcept override
		{
			SpineLayer* layer = LayerAt(index);
			if (layer == nullptr) return false;
			layer->timeScale = (std::max)(0.0f, timeScale);
			return true;
		}
		void SetBlendWindowSeconds(float seconds) override
		{
			m_mixSeconds = seconds > 0.0f ? seconds : 0.0f;
			for (SpineLayer& layer : m_layers)
			{
				if (layer.runtime)
					layer.runtime->SetMotionBlendSeconds(m_mixSeconds);
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
			const SpineLayer* layer = LayerAt(spineIndex);
			const SlVec2 offset = LayerOffset(spineIndex);
			const float centerX = static_cast<float>(m_renderWidth) * 0.5f + offset.x;
			const float centerY = static_cast<float>(m_renderHeight) * 0.5f + offset.y;
			const float scale = LayerScale(spineIndex) * m_canvasScale;
			const SlVec2 transformScale = layer != nullptr ? layer->transformScale : SlVec2{ 1.0f, 1.0f };
			const float extraAngle = layer != nullptr ? layer->angle : 0.0f;
			const float radians = (static_cast<float>(LayerRotation(spineIndex) & 3) * 90.0f + extraAngle) *
				3.1415926535f / 180.0f;
			const float cosine = std::cos(radians);
			const float sine = std::sin(radians);
			SlMatrix4 matrix{};
			matrix.m[2][2] = 1.0f;
			matrix.m[3][3] = 1.0f;
			const float signedScaleX = (LayerMirror(spineIndex) ? -scale : scale) * transformScale.x;
			const float signedScaleY = -scale * transformScale.y;
			matrix.m[0][0] = signedScaleX * cosine;
			matrix.m[0][1] = signedScaleX * sine;
			matrix.m[1][0] = -signedScaleY * sine;
			matrix.m[1][1] = signedScaleY * cosine;
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
		bool ReadBoneTransform(const std::string& name, SlMatrix4& matrix) const override
		{
			const auto* runtime = SelectedRuntime();
			std::array<float, 6> pose{};
			if (!runtime || !runtime->ReadBoneTransform(name.c_str(), pose)) return false;
			matrix = SlIdentityMatrix4();
			matrix.m[0][0] = pose[0]; matrix.m[1][0] = pose[1];
			matrix.m[0][1] = pose[2]; matrix.m[1][1] = pose[3];
			matrix.m[3][0] = pose[4]; matrix.m[3][1] = pose[5];
			return true;
		}
		bool ReadSlotMesh(const std::string& slotName, ReadSlotMeshData& outData) const override
		{
			const auto* runtime = SelectedRuntime();
			if (runtime == nullptr || slotName.empty())
				return false;

			if (m_probeFrameGeneration != m_stateGeneration)
			{
				const_cast<sl_runtime_v2::IRuntime*>(runtime)->BuildFrame(m_renderWidth, m_renderHeight, m_probeFrame);
				m_probeFrameGeneration = m_stateGeneration;
			}
			const sl_runtime_v2::Frame& frame = m_probeFrame;
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
		void SetViewportSize(int width, int height) noexcept override
		{
			if (width != m_renderWidth || height != m_renderHeight)
				InvalidateProbeFrame();
			m_renderWidth = width;
			m_renderHeight = height;
		}

	private:
		struct SpineLayer;

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

			InvalidateProbeFrame();
			SpineLayer layer;
			layer.offset = LayerOffset(m_selectedSpine);
			layer.scale = LayerScale(m_selectedSpine);
			layer.mirror = LayerMirror(m_selectedSpine) ? 1 : 0;
			layer.rotation = LayerRotation(m_selectedSpine);
            const auto& motionNames = runtime->MotionNames();
            layer.animationName = motionNames.empty() ? std::string{} : motionNames.front();
            const auto& skinNames = runtime->LookNames();
            layer.skinName = skinNames.empty() ? std::string{} : skinNames.front();
			layer.runtime = std::move(runtime);
			m_layers.push_back(std::move(layer));
			m_lastError.clear();
			return true;
		}

		void ClearSpines()
		{
			InvalidateProbeFrame();
			QueueD3D11TextureRelease();
			m_layers.clear();
			m_selectedSpine = 0;
			m_currentAnimationName.clear();
			m_currentSkinName.clear();
			m_currentAnimationTime = 0.0f;
			m_currentMotionDuration = 0.0f;
			m_slotExcludeCallback = nullptr;
		}

		void InvalidateProbeFrame() noexcept { ++m_stateGeneration; }

		SpineLayer* LayerAt(size_t index) noexcept
		{
			return index < m_layers.size() ? &m_layers[index] : nullptr;
		}

		const SpineLayer* LayerAt(size_t index) const noexcept
		{
			return index < m_layers.size() ? &m_layers[index] : nullptr;
		}

		SpineLayer* ActiveLayer() noexcept { return LayerAt(m_selectedSpine); }
		const SpineLayer* ActiveLayer() const noexcept { return LayerAt(m_selectedSpine); }

		void RecordActiveMotion(const std::string& name)
		{
			m_currentAnimationName = name;
			m_currentAnimationTime = 0.0f;
			m_currentMotionDuration = 0.0f;
			if (SpineLayer* layer = ActiveLayer())
			{
				layer->animationName = name;
				layer->animationTime = 0.0f;
				layer->motionDuration = 0.0f;
			}
		}

		void SyncActiveMotionBookkeeping()
		{
			if (const SpineLayer* layer = ActiveLayer())
			{
                m_currentAnimationName = layer->animationName;
                m_currentSkinName = layer->skinName;
				m_currentAnimationTime = layer->animationTime;
				m_currentMotionDuration = layer->motionDuration;
			}
		}

		static SlColor ClampColor(const SlColor& color) noexcept
		{
			return SlColor{
				(std::max)(0.0f, (std::min)(1.0f, color.r)),
				(std::max)(0.0f, (std::min)(1.0f, color.g)),
				(std::max)(0.0f, (std::min)(1.0f, color.b)),
				(std::max)(0.0f, (std::min)(1.0f, color.a)) };
		}

		sl_runtime_v2::IRuntime* SelectedRuntime() const noexcept
		{
			const SpineLayer* layer = ActiveLayer();
			return layer != nullptr ? layer->runtime.get() : nullptr;
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
            for (SpineLayer& layer : m_layers)
            {
                if (layer.runtime)
                    ApplyOpeningAnimation(layer);
            }
            SyncActiveMotionBookkeeping();
        }

        void ApplyOpeningAnimation(SpineLayer& layer)
        {
            auto& runtime = *layer.runtime;
            const std::vector<std::string>& animationNames = runtime.MotionNames();
			if (!animationNames.empty())
			{
				const auto current = std::find(animationNames.begin(), animationNames.end(), m_currentAnimationName);
                const std::string& animationName = current == animationNames.end() ? animationNames.front() : *current;
                runtime.StartMotion(animationName.c_str(), true);
                layer.animationName = animationName;
                layer.animationTime = 0.0f;
                layer.motionDuration = runtime.MotionDuration(animationName.c_str());
			}

			const std::vector<std::string>& skinNames = runtime.LookNames();
			if (!skinNames.empty())
			{
				const auto current = std::find(skinNames.begin(), skinNames.end(), m_currentSkinName);
                const std::string& skinName = current == skinNames.end() ? skinNames.front() : *current;
                runtime.ApplyLook(skinName.c_str());
                layer.skinName = skinName;
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

		void BuildDrawList(const sl_runtime_v2::Frame& frame, SlDrawList& outDrawList, size_t spineIndex)
		{
			outDrawList.width = frame.width;
			outDrawList.height = frame.height;

			const SlMatrix4 transform = ViewTransformFor(spineIndex);
			const SlColor tint = SkeletonLayerTint(spineIndex);
			const SlColor lerpTint = SkeletonLayerLerpTint(spineIndex);
			const float layerOpacity = SkeletonLayerOpacity(spineIndex);

			size_t commandCursor = 0;
			for (const auto& command : frame.draws)
			{
				if (command.vertices.empty() || command.indices.empty())
					continue;
				if (IsSlotExcluded(command.slotName, spineIndex))
					continue;

				if (commandCursor >= outDrawList.commands.size())
					outDrawList.commands.emplace_back();
				SlDrawCommand& outCommand = outDrawList.commands[commandCursor];
				++commandCursor;
				outCommand.textureId = command.textureId;
				outCommand.slotName = command.slotName;
				outCommand.blendMode = ConvertBlendMode(command.blendMode);
				outCommand.premultipliedAlpha = command.premultipliedAlpha;
				outCommand.invertedMask = command.invertedMask;
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
					const float baseR = source.color.r * tint.r;
					const float baseG = source.color.g * tint.g;
					const float baseB = source.color.b * tint.b;
					dest.color.r = baseR + (lerpTint.r - baseR) * lerpTint.a;
					dest.color.g = baseG + (lerpTint.g - baseG) * lerpTint.a;
					dest.color.b = baseB + (lerpTint.b - baseB) * lerpTint.a;
					dest.color.a = source.color.a * tint.a * layerOpacity;
					if (command.premultipliedAlpha)
					{
						dest.color.r *= dest.color.a;
						dest.color.g *= dest.color.a;
						dest.color.b *= dest.color.a;
					}
					dest.uv.x = source.u;
					dest.uv.y = source.v;
					if (dest.color.a > 0.001f &&
						std::isfinite(transformed.x) && std::isfinite(transformed.y))
					{
						if (!m_lastRenderBoundsValid)
						{
							m_lastRenderBounds = SlRect(transformed.x, transformed.y, 0.0f, 0.0f);
							m_lastRenderBoundsValid = true;
						}
						else
						{
							const float right = (std::max)(m_lastRenderBounds.x + m_lastRenderBounds.w, transformed.x);
							const float bottom = (std::max)(m_lastRenderBounds.y + m_lastRenderBounds.h, transformed.y);
							m_lastRenderBounds.x = (std::min)(m_lastRenderBounds.x, transformed.x);
							m_lastRenderBounds.y = (std::min)(m_lastRenderBounds.y, transformed.y);
							m_lastRenderBounds.w = right - m_lastRenderBounds.x;
							m_lastRenderBounds.h = bottom - m_lastRenderBounds.y;
						}
					}
				}
				size_t maskCursor = 0;
				for (const auto& mask : command.masks)
				{
					if (mask.vertices.empty() || mask.indices.empty())
						continue;

					if (maskCursor >= outCommand.masks.size())
						outCommand.masks.emplace_back();
					SlMaskDrawCommand& outMask = outCommand.masks[maskCursor];
					++maskCursor;
					outMask.textureId = mask.textureId;
					outMask.premultipliedAlpha = mask.premultipliedAlpha;
					outMask.vertices.resize(mask.vertices.size());
					outMask.indices = mask.indices;
					for (size_t i = 0; i < mask.vertices.size(); ++i)
					{
						const sl_runtime_v2::Vertex& source = mask.vertices[i];
						SlVertex2D& dest = outMask.vertices[i];
						const SlVec3 transformed = SlVec3Transform(SlVec3(source.x, source.y, 0.0f), transform);
						dest.pos.x = transformed.x;
						dest.pos.y = transformed.y;
						dest.pos.z = 0.0f;
						dest.rhw = 1.0f;
						dest.color.r = source.color.r * tint.r;
						dest.color.g = source.color.g * tint.g;
						dest.color.b = source.color.b * tint.b;
						dest.color.a = source.color.a * tint.a * layerOpacity;
						if (mask.premultipliedAlpha)
						{
							dest.color.r *= dest.color.a;
							dest.color.g *= dest.color.a;
							dest.color.b *= dest.color.a;
						}
						dest.uv.x = source.u;
						dest.uv.y = source.v;
					}
				}
				outCommand.masks.resize(maskCursor);
			}
			outDrawList.commands.resize(commandCursor);
		}

		bool IsSlotExcluded(const std::string& slotName, size_t spineIndex) const
		{
			if (slotName.empty())
				return false;
			if (m_slotExcludeCallback && m_slotExcludeCallback(slotName.c_str(), slotName.size()))
				return true;
			const SpineLayer* layer = LayerAt(spineIndex);
			if (layer == nullptr) return false;
			const auto& excluded = layer->excludedSlots;
			return std::find(excluded.begin(), excluded.end(), slotName) != excluded.end();
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
				if (IsSlotExcluded(command.slotName, m_selectedSpine))
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
				if (IsSlotExcluded(command.slotName, m_selectedSpine))
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
			if (const SpineLayer* layer = ActiveLayer())
			{
				const auto it = layer->textures.find(command.textureId);
				if (it != layer->textures.end())
					return static_cast<int>(it->second);
			}
			return 1;
		}

		bool EnsureD3D11Textures(size_t spineIndex, SlSceneRenderer& renderer)
		{
			SpineLayer* layer = LayerAt(spineIndex);
			if (layer == nullptr || !layer->runtime)
				return false;

			const bool premultiply = !m_sourceIsPremultiplied;
			if (!layer->textures.empty() && layer->texturesPremultiplied == premultiply)
				return true;

			ReleaseD3D11TextureMap(renderer, layer->textures);
			layer->texturesPremultiplied = premultiply;

			for (const auto& texture : layer->runtime->TextureInfos())
			{
				const SlTextureId handle = renderer.LoadTextureUtf8(texture.path.c_str(), ShouldPremultiplyTexture(texture));
				if (handle != 0)
					layer->textures[texture.id] = handle;
			}
			return !layer->textures.empty();
		}

		static void ReleaseD3D11TextureMap(SlSceneRenderer& renderer, std::unordered_map<std::uint64_t, SlTextureId>& textures) noexcept
		{
			for (const auto& entry : textures)
				renderer.ReleaseTexture(entry.second);
			textures.clear();
		}

		void QueueD3D11TextureRelease()
		{
			for (SpineLayer& layer : m_layers)
			{
				if (!layer.textures.empty())
					m_d3d11TextureReleaseQueue.push_back(std::move(layer.textures));
			}
		}

		void ReleaseQueuedD3D11Textures(SlSceneRenderer& renderer)
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

		SlVec2 LayerOffset(size_t index) const noexcept
		{
			const SpineLayer* layer = LayerAt(index);
			return layer != nullptr ? layer->offset : m_offset;
		}

		SlVec2& ActiveLayerOffset() noexcept
		{
			SpineLayer* layer = ActiveLayer();
			return layer != nullptr ? layer->offset : m_offset;
		}

		float LayerScale(size_t index) const noexcept
		{
			const SpineLayer* layer = LayerAt(index);
			return layer != nullptr ? layer->scale : m_skeletonScale;
		}

		float& ActiveLayerScale() noexcept
		{
			SpineLayer* layer = ActiveLayer();
			return layer != nullptr ? layer->scale : m_skeletonScale;
		}

		bool LayerMirror(size_t index) const noexcept
		{
			const SpineLayer* layer = LayerAt(index);
			return layer != nullptr ? layer->mirror != 0 : m_flipX;
		}

		unsigned char& ActiveLayerMirrorFlag() noexcept
		{
			SpineLayer* layer = ActiveLayer();
			return layer != nullptr ? layer->mirror : m_fallbackMirror;
		}

		int LayerRotation(size_t index) const noexcept
		{
			const SpineLayer* layer = LayerAt(index);
			return layer != nullptr ? layer->rotation : m_rotationSteps;
		}

		int& ActiveLayerRotation() noexcept
		{
			SpineLayer* layer = ActiveLayer();
			return layer != nullptr ? layer->rotation : m_rotationSteps;
		}

		struct SpineLayer
		{
			std::unique_ptr<sl_runtime_v2::IRuntime> runtime;
			std::unordered_map<std::uint64_t, SlTextureId> textures;
			bool texturesPremultiplied = false;
			bool visible = true;
			SlVec2 offset{ 0.f, 0.f };
			float scale = 1.0f;
			unsigned char mirror = 0;
			int rotation = 0;
			float opacity = 1.0f;
			SlVec2 transformScale{ 1.0f, 1.0f };
			float angle = 0.0f;
			SlColor tint{};
			SlColor lerpTint{ 1.0f, 1.0f, 1.0f, 0.0f };
			float timeScale = 1.0f;
			std::vector<std::string> excludedSlots;
            std::string animationName;
            std::string skinName;
			float animationTime = 0.0f;
			float motionDuration = 0.0f;
		};

		SlRuntimeHub::RuntimeLane m_slot;
		std::vector<SpineLayer> m_layers;
		std::vector<std::unordered_map<std::uint64_t, SlTextureId>> m_d3d11TextureReleaseQueue;
		unsigned char m_fallbackMirror = 0;
		size_t m_selectedSpine = 0;
		std::string m_lastError;
		std::string m_currentAnimationName;
		std::string m_currentSkinName;
		std::vector<std::string> m_emptyNames;
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
		SlRect m_lastRenderBounds{};
		bool m_lastRenderBoundsValid = false;
		sl_runtime_v2::Frame m_renderFrame;
		SlDrawList m_renderDrawList;
		mutable sl_runtime_v2::Frame m_probeFrame;
		mutable unsigned int m_probeFrameGeneration = 0;
		unsigned int m_stateGeneration = 1;
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

bool SlRuntimeHub::RenderCurrentRuntime(SlSceneRenderer& renderer)
{
	SlPlaybackRuntime* runtime = CurrentRuntime();
	auto* staticRuntime = dynamic_cast<StaticRuntimeAdapter*>(runtime);
	if (staticRuntime == nullptr)
		return false;
	return staticRuntime->RenderD3D11(renderer);
}

#if defined(SL_RENDER_D3D11)
bool SlRuntimeHub::RenderCurrentRuntimeD3D11(sl_d3d11::D3D11Renderer& renderer)
{
	class D3D11Bridge final : public SlSceneRenderer
	{
	public:
		explicit D3D11Bridge(sl_d3d11::D3D11Renderer& renderer) : m_renderer(renderer) {}
		SlTextureId LoadTextureUtf8(const char* path, bool premultiply) override
		{
			return m_renderer.LoadTexture(sl_text::Utf8ToWide(std::string(path)).c_str(), premultiply);
		}
		void ReleaseTexture(SlTextureId id) noexcept override { m_renderer.ReleaseTexture(id); }
		void Submit(const SlDrawList& list, const std::unordered_map<std::uint64_t, SlTextureId>& textures) override
		{
			m_renderer.Submit(list, textures);
		}
	private:
		sl_d3d11::D3D11Renderer& m_renderer;
	} bridge(renderer);
	return RenderCurrentRuntime(bridge);
}
#endif

bool SlRuntimeHub::QueryLastRenderedBounds(SlRect& outBounds) const noexcept
{
	SlPlaybackRuntime* runtime = CurrentRuntime();
	auto* staticRuntime = dynamic_cast<StaticRuntimeAdapter*>(runtime);
	return staticRuntime != nullptr && staticRuntime->QueryLastRenderedBounds(outBounds);
}
