#include "runtime_api.h"

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <spine/Animation.h>
#include <spine/AnimationState.h>
#include <spine/AnimationStateData.h>
#include <spine/Atlas.h>
#include <spine/BlendMode.h>
#include <spine/Bone.h>
#include <spine/ClippingAttachment.h>
#include <spine/Event.h>
#include <spine/EventData.h>
#include <spine/Extension.h>
#include <spine/MeshAttachment.h>
#include <spine/RegionAttachment.h>
#include <spine/Skeleton.h>
#include <spine/SkeletonBinary.h>
#include <spine/SkeletonClipping.h>
#include <spine/SkeletonData.h>
#include <spine/SkeletonJson.h>
#include <spine/Skin.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>
#include <spine/TextureLoader.h>

#if defined(SL_SPINE_WORLD_TRANSFORM_HAS_PHYSICS)
#include <spine/Physics.h>
#endif

#ifndef SL_RUNTIME_FACTORY_NAME
#error SL_RUNTIME_FACTORY_NAME must be defined.
#endif

#ifndef SL_RUNTIME_DISPLAY_VERSION
#error SL_RUNTIME_DISPLAY_VERSION must be defined.
#endif

#ifndef SL_SPINE_NAMESPACE
#error SL_SPINE_NAMESPACE must be defined.
#endif

namespace SL_SPINE_NAMESPACE {

SpineExtension* getDefaultExtension()
{
	static DefaultSpineExtension extension;
	return &extension;
}

}

namespace sl_runtime_v2 {
namespace {

namespace sp = SL_SPINE_NAMESPACE;

std::string ToStdString(const sp::String& value)
{
	const char* text = value.buffer();
	return text ? std::string(text) : std::string();
}

void AssignString(std::string& out, const sp::String& value)
{
	const char* text = value.buffer();
	if (text)
		out.assign(text);
	else
		out.clear();
}

struct RuntimeTexture
{
	TextureInfo info;
};

struct Live2DClippingBinding
{
	std::vector<std::string> masks;
	bool inverted = false;
};

struct RuntimeSlotOverride
{
	float alpha = 1.0f;
	SlotAttachmentMode attachmentMode = SlotAttachmentMode::Preserve;
};

BlendMode ConvertBlendMode(sp::BlendMode blendMode)
{
	switch (blendMode)
	{
	case sp::BlendMode_Additive: return BlendMode::Additive;
	case sp::BlendMode_Multiply: return BlendMode::Multiply;
	case sp::BlendMode_Screen: return BlendMode::Screen;
	case sp::BlendMode_Normal:
	default: return BlendMode::Normal;
	}
}

class LoadedTextureCollector final : public sp::TextureLoader
{
public:
	explicit LoadedTextureCollector(std::vector<std::unique_ptr<RuntimeTexture>>& textures)
		: m_textures(textures)
	{}

	void load(sp::AtlasPage& page, const sp::String& path) override
	{
		std::unique_ptr<RuntimeTexture> texture(new RuntimeTexture());
		texture->info.id = static_cast<unsigned long long>(m_textures.size() + 1);
		texture->info.path = ToStdString(path);
#if defined(SL_SPINE_ATLAS_PAGE_HAS_PMA)
		texture->info.sourcePremultipliedAlpha = page.pma;
		texture->info.renderPremultipliedAlpha = true;
		texture->info.hasPremultipliedAlphaMetadata = true;
#endif
		RuntimeTexture* rawTexture = texture.get();
		m_textures.push_back(std::move(texture));
#if defined(SL_SPINE_ATLAS_PAGE_HAS_TEXTURE)
		page.texture = rawTexture;
#else
		page.setRendererObject(rawTexture);
#endif
	}

	void unload(void*) override {}

private:
	std::vector<std::unique_ptr<RuntimeTexture>>& m_textures;
};

class CppRuntimeAdapter final : public IRuntime, public sp::AnimationStateListenerObject
{
public:
	void callback(sp::AnimationState*, sp::EventType type, sp::TrackEntry* entry, sp::Event* event) override
	{
		if (m_collectCompletions && type == sp::EventType_Complete && entry && entry->getAnimation())
			m_pendingCompletions.push_back({ToStdString(entry->getAnimation()->getName()),entry->getTrackIndex(),entry->getTrackTime()});
		if (type != sp::EventType_Event || event == nullptr)
			return;
		AnimationEvent value;
		value.name = ToStdString(event->getData().getName());
		value.stringValue = ToStdString(event->getStringValue());
		value.intValue = event->getIntValue();
		value.floatValue = event->getFloatValue();
		value.time = event->getTime();
		m_pendingEvents.push_back(std::move(value));
	}

	RuntimeInfo Info() const noexcept override
	{
		return RuntimeInfo{ RuntimeKindValue(), SL_RUNTIME_DISPLAY_VERSION, SL_RUNTIME_DISPLAY_VERSION };
	}
	bool ReadBoneTransform(const char* name, std::array<float, 6>& value) const override
	{
		if (!m_skeleton || !name) return false;
		auto* bone = m_skeleton->findBone(name);
		if (!bone) return false;
		value = { bone->getA(), bone->getB(), bone->getC(), bone->getD(), bone->getWorldX(), bone->getWorldY() };
		return true;
	}

	bool Load(const LoadRequest& request) override
	{
		Clear();

		const bool useMemory = !request.atlasData.empty() || !request.skeletonData.empty();
		if (useMemory)
			return LoadFromMemory(request);
		return LoadFromFiles(request);
	}

	bool LoadFromFiles(const LoadRequest& request)
	{
		if (request.atlasPaths.empty())
		{
			m_lastError = "No atlas file was provided.";
			return false;
		}

		if (request.skeletonPaths.empty())
		{
			m_lastError = "No skeleton file was provided.";
			return false;
		}

		m_textureRecords.clear();
		m_textureLoader.reset(new LoadedTextureCollector(m_textureRecords));
		m_atlas.reset(new sp::Atlas(sp::String(request.atlasPaths.front().c_str()), m_textureLoader.get(), true));
		RefreshTextureInfos();

		sp::SkeletonData* loadedData = nullptr;
		if (request.binarySkeleton)
		{
			sp::SkeletonBinary binary(m_atlas.get());
			loadedData = binary.readSkeletonDataFile(sp::String(request.skeletonPaths.front().c_str()));
			if (!loadedData)
				m_lastError = ToStdString(binary.getError());
		}
		else
		{
			sp::SkeletonJson json(m_atlas.get());
			loadedData = json.readSkeletonDataFile(sp::String(request.skeletonPaths.front().c_str()));
			if (!loadedData)
				m_lastError = ToStdString(json.getError());
		}

		return AcceptLoadedSkeleton(loadedData);
	}

	bool LoadFromMemory(const LoadRequest& request)
	{
		if (request.atlasData.empty())
		{
			m_lastError = "No atlas data was provided.";
			return false;
		}

		if (request.skeletonData.empty())
		{
			m_lastError = "No skeleton data was provided.";
			return false;
		}

		const std::string& atlasText = request.atlasData.front();
		const char* textureDir = request.textureDirectories.empty() ? "" : request.textureDirectories.front().c_str();

		m_textureRecords.clear();
		m_textureLoader.reset(new LoadedTextureCollector(m_textureRecords));
		m_atlas.reset(new sp::Atlas(atlasText.data(), static_cast<int>(atlasText.size()), textureDir, m_textureLoader.get(), true));
		RefreshTextureInfos();

		sp::SkeletonData* loadedData = nullptr;
		const std::string& skeletonText = request.skeletonData.front();
		if (request.binarySkeleton)
		{
			sp::SkeletonBinary binary(m_atlas.get());
			loadedData = binary.readSkeletonData(reinterpret_cast<const unsigned char*>(skeletonText.data()), static_cast<int>(skeletonText.size()));
			if (!loadedData)
				m_lastError = ToStdString(binary.getError());
		}
		else
		{
			sp::SkeletonJson json(m_atlas.get());
			loadedData = json.readSkeletonData(skeletonText.c_str());
			if (!loadedData)
				m_lastError = ToStdString(json.getError());
		}

		return AcceptLoadedSkeleton(loadedData);
	}

	bool AcceptLoadedSkeleton(sp::SkeletonData* loadedData)
	{
		if (!loadedData)
		{
			if (m_lastError.empty())
				m_lastError = std::string("C++ runtime ") + SL_RUNTIME_DISPLAY_VERSION + " could not load the skeleton.";
			ClearLoadedObjects();
			return false;
		}

		m_skeletonData.reset(loadedData);
		CollectNames();
		CollectLive2DClipping();

		m_skeleton.reset(new sp::Skeleton(m_skeletonData.get()));
		m_skeleton->setToSetupPose();
		UpdateWorldTransform();

		m_animationStateData.reset(new sp::AnimationStateData(m_skeletonData.get()));
		m_animationState.reset(new sp::AnimationState(m_animationStateData.get()));
		m_animationState->setListener(this);

		m_hasSkeleton = true;
		m_lastError.clear();
		return true;
	}

	void Clear() noexcept override
	{
		m_hasSkeleton = false;
		m_animationNames.clear();
		m_skinNames.clear();
		m_slotNames.clear();
		m_textureInfos.clear();
		m_live2DClipping.clear();
		m_slotOverrides.clear();
		m_lastError.clear();
		ClearLoadedObjects();
	}

	bool HasSkeleton() const noexcept override { return m_hasSkeleton; }
	void Update(float deltaSeconds) override
	{
		if (!m_hasSkeleton || !m_skeleton)
			return;

#if defined(SL_SPINE_SKELETON_HAS_UPDATE)
		m_skeleton->update(deltaSeconds);
#else
		(void)deltaSeconds;
#endif
		if (m_animationState)
		{
			m_animationState->update(deltaSeconds);
			m_animationState->apply(*m_skeleton);
		}
		ApplySlotOverrides();
		UpdateWorldTransform();
	}

	void BuildFrame(int width, int height, Frame& outFrame) override
	{
		outFrame.width = width;
		outFrame.height = height;
		m_drawCursor = 0;
		if (!m_hasSkeleton || !m_skeleton)
		{
			outFrame.draws.clear();
			return;
		}

		m_clipper.clipEnd();
		sp::Vector<sp::Slot*>& drawOrder = m_skeleton->getDrawOrder();
		for (size_t i = 0; i < drawOrder.size(); ++i)
		{
			sp::Slot* slot = drawOrder[i];
			if (!slot)
				continue;

			sp::Attachment* attachment = slot->getAttachment();
			if (!attachment)
			{
				m_clipper.clipEnd(*slot);
				continue;
			}

			const bool isClipAttachment = attachment->getRTTI().isExactly(sp::ClippingAttachment::rtti);
			if ((slot->getColor().a <= 0.0f || !slot->getBone().isActive()) && !isClipAttachment)
			{
				m_clipper.clipEnd(*slot);
				continue;
			}

			if (attachment->getRTTI().isExactly(sp::RegionAttachment::rtti))
			{
				AppendRegionCommand(*static_cast<sp::RegionAttachment*>(attachment), *slot, outFrame);
				m_clipper.clipEnd(*slot);
			}
			else if (attachment->getRTTI().isExactly(sp::MeshAttachment::rtti))
			{
				AppendMeshCommand(*static_cast<sp::MeshAttachment*>(attachment), *slot, outFrame);
				m_clipper.clipEnd(*slot);
			}
			else if (isClipAttachment)
			{
				m_clipper.clipStart(*slot, static_cast<sp::ClippingAttachment*>(attachment));
			}
		}
		m_clipper.clipEnd();
		outFrame.draws.resize(m_drawCursor);
		AttachLive2DMasks(outFrame);
	}

	const std::vector<std::string>& MotionNames() const noexcept override { return m_animationNames; }
	const std::vector<std::string>& LookNames() const noexcept override { return m_skinNames; }
	const std::vector<std::string>& SlotCatalog() const noexcept override { return m_slotNames; }
	const std::vector<TextureInfo>& TextureInfos() const noexcept override { return m_textureInfos; }
	void StartMotion(const char* name, bool loop) override
	{
		if (!m_animationState || name == nullptr || name[0] == '\0')
			return;
		if (m_skeletonData && m_skeletonData->findAnimation(sp::String(name)) == nullptr)
			return;
		m_pendingEvents.clear();
		m_pendingCompletions.clear();
		m_animationState->setAnimation(0, sp::String(name), loop);
	}
	bool StartMotionWithMix(const char* name, bool loop, float mixSeconds) override
	{
		if (!m_animationState || name == nullptr || name[0] == '\0' ||
			(m_skeletonData && m_skeletonData->findAnimation(sp::String(name)) == nullptr))
			return false;
		m_pendingEvents.clear();
		m_pendingCompletions.clear();
		sp::TrackEntry* entry = m_animationState->setAnimation(0, sp::String(name), loop);
		if (entry && mixSeconds >= 0.0f) entry->setMixDuration(mixSeconds);
		return entry != nullptr;
	}
	bool QueueMotion(const char* name, bool loop, float mixSeconds) override
	{
		if (!m_animationState || name == nullptr || name[0] == '\0' ||
			(m_skeletonData && m_skeletonData->findAnimation(sp::String(name)) == nullptr))
			return false;
		sp::TrackEntry* entry = m_animationState->addAnimation(0, sp::String(name), loop, 0.0f);
		if (entry && mixSeconds >= 0.0f) entry->setMixDuration(mixSeconds);
		return entry != nullptr;
	}
	bool SetCurrentMotionTimeScale(float timeScale) noexcept override
	{
		if (!m_animationState) return false;
		sp::TrackEntry* entry = m_animationState->getCurrent(0);
		if (!entry) return false;
		entry->setTimeScale((std::max)(0.0f, timeScale));
		return true;
	}

	float MotionDuration(const char* name) const override
	{
		if (!m_skeletonData || name == nullptr || name[0] == '\0')
			return 0.0f;
		sp::Animation* animation = m_skeletonData->findAnimation(sp::String(name));
		return animation ? animation->getDuration() : 0.0f;
	}

	void SetMotionBlendSeconds(float seconds) override
	{
		if (m_animationStateData)
			m_animationStateData->setDefaultMix(seconds > 0.0f ? seconds : 0.0f);
	}

	void SetSecondaryMotions(const std::vector<std::string>& names, bool loop) override
	{
		if (!m_animationState)
			return;

		sp::Vector<sp::TrackEntry*>& tracks = m_animationState->getTracks();
		for (size_t track = 1; track < tracks.size(); ++track)
			m_animationState->clearTrack(track);

		if (names.empty())
			return;

		size_t trackIndex = 1;
		for (size_t i = 0; i < names.size(); ++i)
		{
			if (m_skeletonData && m_skeletonData->findAnimation(sp::String(names[i].c_str())) == nullptr) { ++trackIndex; continue; }
			m_animationState->setAnimation(trackIndex, sp::String(names[i].c_str()), loop);
			++trackIndex;
		}
	}

	bool StartMotionOnTrack(int track, const char* name, bool loop, float mix) override
	{
		if (!m_animationState || !m_skeletonData || track < 0 || !name || !m_skeletonData->findAnimation(sp::String(name))) return false;
		m_animationState->setEmptyAnimation(track, (std::max)(0.f, mix));
		auto* entry = m_animationState->addAnimation(track, sp::String(name), loop, .01f);
		if (entry) entry->setMixDuration((std::max)(0.f, mix));
		return entry != nullptr;
	}
	bool ClearMotionTrack(int track, float mix) override
	{
		if (!m_animationState || track < 0) return false;
		m_animationState->setEmptyAnimation(track, (std::max)(0.f, mix));
		return true;
	}
	bool SetNativeMotionTrack(const SlNativeMotionTrack& value) override
	{
		if (!m_animationState || !m_skeletonData || value.track < 0) return false;
		if (value.animation.empty() && !value.emptyAnimation) {
			m_animationState->clearTrack(value.track);
			return true;
		}
		sp::TrackEntry* entry = nullptr;
		if (value.emptyAnimation) entry = m_animationState->setEmptyAnimation(value.track,(std::max)(0.f,value.mix));
		else {
			auto* animation = m_skeletonData->findAnimation(sp::String(value.animation.c_str()));
			if (!animation) return false;
			entry = m_animationState->setAnimation(value.track,animation,value.loop);
		}
		if (!entry) return false;
		entry->setTrackTime((std::max)(0.f,value.time));
		entry->setTimeScale(value.speed);
		entry->setEventThreshold(value.eventThreshold);
#if defined(SL_SPINE_WORLD_TRANSFORM_HAS_PHYSICS)
		entry->setMixAttachmentThreshold(value.attachmentThreshold);
		entry->setMixDrawOrderThreshold(value.drawOrderThreshold);
#else
		entry->setAttachmentThreshold(value.attachmentThreshold);
		entry->setDrawOrderThreshold(value.drawOrderThreshold);
#endif
		entry->setHoldPrevious(value.holdPrevious);
		if (value.mix >= 0) entry->setMixDuration(value.mix);
		return true;
	}
	bool HoldMotionTrack(int track, float time) override
	{
		if (!m_animationState || track < 0) return false;
		auto* entry = m_animationState->getCurrent(track);
		if (!entry) return false;
		entry->setLoop(false); entry->setTrackTime((std::max)(0.f, time)); entry->setTimeScale(0);
		return true;
	}
	void ApplyLook(const char* name) override
	{
		if (!m_skeleton || name == nullptr || name[0] == '\0')
			return;
		sp::Skin* newSkin = m_skeletonData ? m_skeletonData->findSkin(sp::String(name)) : nullptr;
		if (!newSkin)
			return;
		ApplySkinChange(newSkin, IsFaceSkinName(name), m_compositeContainsFace);
		m_compositeSkin.reset();
		m_compositeContainsFace = false;
	}

	void DrainAnimationEvents(std::vector<AnimationEvent>& events) override
	{
		events.clear();
		events.swap(m_pendingEvents);
	}
	void EnableMotionCompletions(bool enabled) override
	{
		m_collectCompletions = enabled;
		m_pendingCompletions.clear();
	}
	void DrainMotionCompletions(std::vector<AnimationCompletion>& events) override
	{
		events.clear();
		events.swap(m_pendingCompletions);
	}

	void ComposeLooks(const std::vector<std::string>& names) override
	{
		if (!m_skeleton || !m_skeletonData || names.empty())
			return;

		std::unique_ptr<sp::Skin> combined(new sp::Skin(sp::String("__spinelove_composite")));
		bool containsFaceSkin = false;
		size_t added = 0;
		for (const std::string& name : names)
		{
			sp::Skin* skin = m_skeletonData->findSkin(sp::String(name.c_str()));
			if (!skin)
				continue;
			combined->addSkin(skin);
			containsFaceSkin = containsFaceSkin || IsFaceSkinName(name);
			++added;
		}
		if (added == 0)
			return;

		ApplySkinChange(combined.get(), containsFaceSkin, m_compositeContainsFace);
		m_compositeSkin = std::move(combined);
		m_compositeContainsFace = containsFaceSkin;
	}

	bool SetNativeSkinMix(const std::vector<std::string>& names) override
	{
		if (!m_skeleton || !m_skeletonData) return false;
		std::unique_ptr<sp::Skin> combined(new sp::Skin(sp::String("__spinelove_native_composite")));
		for (const auto& name : names)
		{
			if (auto* skin = m_skeletonData->findSkin(sp::String(name.c_str()))) combined->addSkin(skin);
		}
		m_skeleton->setSkin(combined.get());
		m_skeleton->setSlotsToSetupPose();
		if (m_animationState) m_animationState->apply(*m_skeleton);
		m_compositeSkin = std::move(combined);
		m_compositeContainsFace = false;
		ApplySlotOverrides();
		UpdateWorldTransform();
		return true;
	}
	bool SetSlotOverride(const char* slotName, float alpha,
		SlotAttachmentMode attachmentMode) override
	{
		if (!m_skeleton || slotName == nullptr || slotName[0] == '\0') return false;
		sp::Slot* slot = m_skeleton->findSlot(sp::String(slotName));
		if (!slot) return false;
		RuntimeSlotOverride state;
		state.alpha = (std::max)(0.0f, (std::min)(1.0f, alpha));
		state.attachmentMode = attachmentMode;
		m_slotOverrides[slotName] = state;
		ApplySlotOverride(*slot, state);
		UpdateWorldTransform();
		return true;
	}

	void ClearSlotOverrides() override { m_slotOverrides.clear(); }

	std::string LastError() const override { return m_lastError; }

private:
	void ApplySlotOverride(sp::Slot& slot, const RuntimeSlotOverride& state)
	{
		if (state.attachmentMode == SlotAttachmentMode::Clear)
			slot.setAttachment(nullptr);
		else if (state.attachmentMode == SlotAttachmentMode::SetupIfEmpty && !slot.getAttachment())
			slot.setToSetupPose();
		else if (state.attachmentMode == SlotAttachmentMode::NamedIfEmpty && !slot.getAttachment())
		{
			sp::SlotData& data = slot.getData();
			sp::Attachment* attachment = m_skeleton
				? m_skeleton->getAttachment(data.getIndex(), data.getName()) : nullptr;
			if (attachment) slot.setAttachment(attachment);
			else slot.setToSetupPose();
		}
		slot.getColor().a = state.alpha;
	}

	void ApplySlotOverrides()
	{
		if (!m_skeleton) return;
		for (const auto& item : m_slotOverrides)
			if (sp::Slot* slot = m_skeleton->findSlot(sp::String(item.first.c_str())))
				ApplySlotOverride(*slot, item.second);
	}

	void ApplySkinChange(sp::Skin* newSkin, bool isFaceSkin, bool wasCompositeFaceSkin)
	{
		sp::Skin* oldSkin = m_skeleton->getSkin();
		const bool wasFaceSkin = wasCompositeFaceSkin ||
			(oldSkin && IsFaceSkinName(ToStdString(oldSkin->getName())));
		const bool preserveConvertedFacePose =
			RuntimeKindValue() == RuntimeKind::Cpp42 && (wasFaceSkin || isFaceSkin);
		sp::Vector<sp::Slot*>& slots = m_skeleton->getSlots();
		std::vector<sp::Attachment*> previousAttachments(slots.size(), nullptr);
		for (size_t i = 0; i < slots.size(); ++i)
			previousAttachments[i] = slots[i]->getAttachment();

		m_skeleton->setSkin(newSkin);
		if (preserveConvertedFacePose)
		{
			ReconcileFaceAttachments(oldSkin, newSkin, previousAttachments, wasFaceSkin, isFaceSkin);
		}
		else
		{
			m_skeleton->setSlotsToSetupPose();
		}

		if (m_animationState)
			m_animationState->apply(*m_skeleton);
		ApplySlotOverrides();
		UpdateWorldTransform();
	}
	static bool IsFaceSkinName(const std::string& name)
	{
		return name.size() >= 4 && name.compare(0, 4, "Face") == 0;
	}

	static void MarkSkinSlots(sp::Skin* skin, std::vector<bool>& marked)
	{
		if (!skin)
			return;
		sp::Skin::AttachmentMap::Entries entries = skin->getAttachments();
		while (entries.hasNext())
		{
			sp::Skin::AttachmentMap::Entry& entry = entries.next();
			if (entry._slotIndex < marked.size())
				marked[entry._slotIndex] = true;
		}
	}

	static bool FindAttachmentName(
		sp::Skin* skin,
		size_t slotIndex,
		sp::Attachment* attachment,
		std::string& outName)
	{
		if (!skin || !attachment)
			return false;
		sp::Skin::AttachmentMap::Entries entries = skin->getAttachments();
		while (entries.hasNext())
		{
			sp::Skin::AttachmentMap::Entry& entry = entries.next();
			if (entry._slotIndex == slotIndex && entry._attachment == attachment)
			{
				outName = ToStdString(entry._name);
				return true;
			}
		}
		return false;
	}

	void ReconcileFaceAttachments(
		sp::Skin* oldSkin,
		sp::Skin* newSkin,
		const std::vector<sp::Attachment*>& previousAttachments,
		bool oldContainsFace,
		bool newContainsFace)
	{
		sp::Vector<sp::Slot*>& slots = m_skeleton->getSlots();
		std::vector<bool> affectedSlots(slots.size(), false);
		if (oldSkin && oldContainsFace)
			MarkSkinSlots(oldSkin, affectedSlots);
		if (newSkin && newContainsFace)
			MarkSkinSlots(newSkin, affectedSlots);

		sp::Skin* defaultSkin = m_skeletonData ? m_skeletonData->getDefaultSkin() : nullptr;
		for (size_t slotIndex = 0; slotIndex < slots.size(); ++slotIndex)
		{
			if (!affectedSlots[slotIndex] || slotIndex >= previousAttachments.size())
				continue;

			std::string attachmentName;
			if (!FindAttachmentName(oldSkin, slotIndex, previousAttachments[slotIndex], attachmentName) &&
				!FindAttachmentName(defaultSkin, slotIndex, previousAttachments[slotIndex], attachmentName))
				continue;

			const sp::String key(attachmentName.c_str());
			sp::Attachment* target = newSkin ? newSkin->getAttachment(slotIndex, key) : nullptr;
			if (!target && defaultSkin)
				target = defaultSkin->getAttachment(slotIndex, key);
			if (target && slots[slotIndex]->getAttachment() != target)
				slots[slotIndex]->setAttachment(target);
		}
	}

	static RuntimeKind RuntimeKindValue() noexcept
	{
		const char* version = SL_RUNTIME_DISPLAY_VERSION;
		if (version[0] == '3') return RuntimeKind::Cpp38;
		if (version[0] == '4' && version[2] == '0') return RuntimeKind::Cpp40;
		if (version[0] == '4' && version[2] == '1') return RuntimeKind::Cpp41;
		return RuntimeKind::Cpp42;
	}

	void CollectNames()
	{
		m_animationNames.clear();
		m_skinNames.clear();
		m_slotNames.clear();
		if (!m_skeletonData)
			return;

		sp::Vector<sp::Animation*>& animations = m_skeletonData->getAnimations();
		for (size_t i = 0; i < animations.size(); ++i)
		{
			if (animations[i])
				m_animationNames.push_back(ToStdString(animations[i]->getName()));
		}

		sp::Vector<sp::Skin*>& skins = m_skeletonData->getSkins();
		for (size_t i = 0; i < skins.size(); ++i)
		{
			if (skins[i])
				m_skinNames.push_back(ToStdString(skins[i]->getName()));
		}

		sp::Vector<sp::SlotData*>& slots = m_skeletonData->getSlots();
		for (size_t i = 0; i < slots.size(); ++i)
		{
			if (slots[i])
				m_slotNames.push_back(ToStdString(slots[i]->getName()));
		}
	}

	void CollectLive2DClipping()
	{
		m_live2DClipping.clear();
		if (!m_skeletonData)
			return;

		const std::string prefix = "Clipping_";
		sp::Vector<sp::EventData*>& events = m_skeletonData->getEvents();
		for (size_t i = 0; i < events.size(); ++i)
		{
			sp::EventData* eventData = events[i];
			if (!eventData)
				continue;
			const std::string eventName = ToStdString(eventData->getName());
			if (eventName.compare(0, prefix.size(), prefix) != 0 || eventName.size() <= prefix.size())
				continue;

			Live2DClippingBinding binding;
			const std::string value = ToStdString(eventData->getStringValue());
			size_t begin = 0;
			while (begin <= value.size())
			{
				const size_t comma = value.find(',', begin);
				const size_t end = comma == std::string::npos ? value.size() : comma;
				size_t first = begin;
				while (first < end && std::isspace(static_cast<unsigned char>(value[first]))) ++first;
				size_t last = end;
				while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1]))) --last;
				if (last > first)
					binding.masks.push_back(value.substr(first, last - first));
				if (comma == std::string::npos)
					break;
				begin = comma + 1;
			}
			binding.inverted = eventData->getIntValue() != 0;
			if (!binding.masks.empty())
				m_live2DClipping[eventName.substr(prefix.size())] = std::move(binding);
		}
	}

	void RefreshTextureInfos()
	{
		m_textureInfos.clear();
		for (const auto& texture : m_textureRecords)
		{
			if (texture)
				m_textureInfos.push_back(texture->info);
		}
	}

	const RuntimeTexture* RuntimeTextureFromObject(void* object) const noexcept
	{
		return static_cast<const RuntimeTexture*>(object);
	}

	unsigned long long TextureIdFromObject(void* object) const noexcept
	{
		const RuntimeTexture* texture = RuntimeTextureFromObject(object);
		return texture ? texture->info.id : 0;
	}

	bool TexturePremultipliedFromObject(void* object) const noexcept
	{
		const RuntimeTexture* texture = RuntimeTextureFromObject(object);
		return texture ? texture->info.renderPremultipliedAlpha : false;
	}

#if defined(SL_SPINE_TEXTURE_REGION_API)
	unsigned long long TextureIdFromRegion(sp::TextureRegion* region) const noexcept
	{
		return region ? TextureIdFromObject(region->rendererObject) : 0;
	}

	bool TexturePremultipliedFromRegion(sp::TextureRegion* region) const noexcept
	{
		return region ? TexturePremultipliedFromObject(region->rendererObject) : false;
	}
#else
	unsigned long long TextureIdFromRegion(sp::AtlasRegion* region) const noexcept
	{
		return (region && region->page) ? TextureIdFromObject(region->page->getRendererObject()) : 0;
	}

	bool TexturePremultipliedFromRegion(sp::AtlasRegion* region) const noexcept
	{
		return (region && region->page) ? TexturePremultipliedFromObject(region->page->getRendererObject()) : false;
	}
#endif

	Color CombineColor(const sp::Color& skeleton, const sp::Color& slot, const sp::Color& attachment) const noexcept
	{
		Color color;
		color.r = skeleton.r * slot.r * attachment.r;
		color.g = skeleton.g * slot.g * attachment.g;
		color.b = skeleton.b * slot.b * attachment.b;
		color.a = skeleton.a * slot.a * attachment.a;
		return color;
	}

	void AppendRegionCommand(sp::RegionAttachment& region, sp::Slot& slot, Frame& outFrame)
	{
		m_worldVertices.setSize(8, 0.0f);
#if defined(SL_SPINE_REGION_COMPUTE_USES_SLOT)
		region.computeWorldVertices(slot, m_worldVertices, 0, 2);
		auto* atlasRegion = static_cast<sp::AtlasRegion*>(region.getRegion());
#else
		region.computeWorldVertices(slot.getBone(), m_worldVertices, 0, 2);
		auto* atlasRegion = static_cast<sp::AtlasRegion*>(region.getRendererObject());
#endif
		const unsigned long long textureId = TextureIdFromRegion(atlasRegion);
		if (textureId == 0)
			return;

		m_quadIndices.clear();
		m_quadIndices.add(0);
		m_quadIndices.add(1);
		m_quadIndices.add(2);
		m_quadIndices.add(2);
		m_quadIndices.add(3);
		m_quadIndices.add(0);
		sp::Vector<float>& uvs = region.getUVs();
		const Color color = CombineColor(m_skeleton->getColor(), slot.getColor(), region.getColor());
		AppendGeometryCommand(textureId, TexturePremultipliedFromRegion(atlasRegion), slot, color, m_worldVertices, uvs, m_quadIndices, outFrame);
	}

	void AppendMeshCommand(sp::MeshAttachment& mesh, sp::Slot& slot, Frame& outFrame)
	{
		const size_t vertexValueCount = mesh.getWorldVerticesLength();
		if (vertexValueCount == 0)
			return;

		m_worldVertices.setSize(vertexValueCount, 0.0f);
		mesh.computeWorldVertices(slot, 0, vertexValueCount, m_worldVertices.buffer(), 0, 2);
#if defined(SL_SPINE_TEXTURE_REGION_API)
		auto* atlasRegion = static_cast<sp::AtlasRegion*>(mesh.getRegion());
#else
		auto* atlasRegion = static_cast<sp::AtlasRegion*>(mesh.getRendererObject());
#endif
		const unsigned long long textureId = TextureIdFromRegion(atlasRegion);
		if (textureId == 0)
			return;

		sp::Vector<float>& uvs = mesh.getUVs();
		sp::Vector<unsigned short>& indices = mesh.getTriangles();
		const Color color = CombineColor(m_skeleton->getColor(), slot.getColor(), mesh.getColor());
		AppendGeometryCommand(textureId, TexturePremultipliedFromRegion(atlasRegion), slot, color, m_worldVertices, uvs, indices, outFrame);
	}

	void AppendGeometryCommand(
		unsigned long long textureId,
		bool premultipliedAlpha,
		sp::Slot& slot,
		const Color& color,
		sp::Vector<float>& worldVertices,
		sp::Vector<float>& uvs,
		sp::Vector<unsigned short>& indices,
		Frame& outFrame)
	{
		sp::Vector<float>* finalVertices = &worldVertices;
		sp::Vector<float>* finalUvs = &uvs;
		sp::Vector<unsigned short>* finalIndices = &indices;
		if (m_clipper.isClipping())
		{
			m_clipper.clipTriangles(worldVertices, indices, uvs, 2);
			finalVertices = &m_clipper.getClippedVertices();
			finalUvs = &m_clipper.getClippedUVs();
			finalIndices = &m_clipper.getClippedTriangles();
		}

		const size_t vertexCount = finalVertices->size() / 2;
		if (vertexCount == 0 || finalIndices->size() == 0)
			return;

		if (m_drawCursor >= outFrame.draws.size())
			outFrame.draws.emplace_back();
		DrawCommand& command = outFrame.draws[m_drawCursor];
		++m_drawCursor;
		command.textureId = textureId;
		AssignString(command.slotName, slot.getData().getName());
		if (slot.getAttachment())
			AssignString(command.attachmentName, slot.getAttachment()->getName());
		else
			command.attachmentName.clear();
		command.blendMode = ConvertBlendMode(slot.getData().getBlendMode());
		command.premultipliedAlpha = premultipliedAlpha;
		command.invertedMask = false;
		command.masks.clear();
		command.vertices.resize(vertexCount);
		command.indices.clear();
		command.indices.reserve(finalIndices->size());
		for (size_t i = 0; i < finalIndices->size(); ++i)
			command.indices.push_back((*finalIndices)[i]);
		for (size_t i = 0; i < command.vertices.size(); ++i)
		{
			command.vertices[i].x = (*finalVertices)[i * 2 + 0];
			command.vertices[i].y = (*finalVertices)[i * 2 + 1];
			command.vertices[i].u = (*finalUvs)[i * 2 + 0];
			command.vertices[i].v = (*finalUvs)[i * 2 + 1];
			command.vertices[i].color = color;
		}
	}

	bool BuildLive2DMaskCommand(const std::string& attachmentName, MaskDrawCommand& outCommand)
	{
		if (!m_skeleton)
			return false;

		sp::Vector<sp::Slot*>& slots = m_skeleton->getSlots();
		for (size_t i = 0; i < slots.size(); ++i)
		{
			sp::Slot* slot = slots[i];
			if (!slot || !slot->getBone().isActive())
				continue;
			sp::Attachment* attachment = slot->getAttachment();
			if (!attachment || ToStdString(attachment->getName()) != attachmentName ||
				!attachment->getRTTI().isExactly(sp::MeshAttachment::rtti))
				continue;

			sp::MeshAttachment& mesh = *static_cast<sp::MeshAttachment*>(attachment);
			const size_t vertexValueCount = mesh.getWorldVerticesLength();
			if (vertexValueCount == 0)
				return false;
			m_maskWorldVertices.setSize(vertexValueCount, 0.0f);
			mesh.computeWorldVertices(*slot, 0, vertexValueCount, m_maskWorldVertices.buffer(), 0, 2);
#if defined(SL_SPINE_TEXTURE_REGION_API)
			auto* atlasRegion = static_cast<sp::AtlasRegion*>(mesh.getRegion());
#else
			auto* atlasRegion = static_cast<sp::AtlasRegion*>(mesh.getRendererObject());
#endif
			const unsigned long long textureId = TextureIdFromRegion(atlasRegion);
			if (textureId == 0)
				return false;

			sp::Vector<float>& uvs = mesh.getUVs();
			sp::Vector<unsigned short>& indices = mesh.getTriangles();
			const size_t vertexCount = m_maskWorldVertices.size() / 2;
			if (vertexCount == 0 || indices.size() == 0 || uvs.size() < vertexCount * 2)
				return false;

			outCommand.textureId = textureId;
			outCommand.premultipliedAlpha = TexturePremultipliedFromRegion(atlasRegion);
			outCommand.vertices.resize(vertexCount);
			outCommand.indices.assign(indices.buffer(), indices.buffer() + indices.size());
			const Color color;
			for (size_t vertex = 0; vertex < vertexCount; ++vertex)
			{
				Vertex& outVertex = outCommand.vertices[vertex];
				outVertex.x = m_maskWorldVertices[vertex * 2];
				outVertex.y = m_maskWorldVertices[vertex * 2 + 1];
				outVertex.u = uvs[vertex * 2];
				outVertex.v = uvs[vertex * 2 + 1];
				outVertex.color = color;
			}
			return true;
		}
		return false;
	}

	void AttachLive2DMasks(Frame& frame)
	{
		if (m_live2DClipping.empty())
			return;
		for (DrawCommand& command : frame.draws)
		{
			auto binding = m_live2DClipping.find(command.attachmentName);
			if (binding == m_live2DClipping.end())
				continue;
			command.invertedMask = binding->second.inverted;
			for (const std::string& maskName : binding->second.masks)
			{
				MaskDrawCommand mask;
				if (BuildLive2DMaskCommand(maskName, mask))
					command.masks.push_back(std::move(mask));
			}
		}
	}

	void UpdateWorldTransform()
	{
		if (!m_skeleton)
			return;
#if defined(SL_SPINE_WORLD_TRANSFORM_HAS_PHYSICS)
		m_skeleton->updateWorldTransform(sp::Physics_Update);
#else
		m_skeleton->updateWorldTransform();
#endif
	}

	void ClearLoadedObjects() noexcept
	{
		m_animationState.reset();
		m_animationStateData.reset();
		m_skeleton.reset();
		m_compositeSkin.reset();
		m_compositeContainsFace = false;
		m_skeletonData.reset();
		m_atlas.reset();
		m_textureLoader.reset();
		m_textureRecords.clear();
		m_worldVertices.clear();
		m_maskWorldVertices.clear();
		m_pendingEvents.clear();
		m_pendingCompletions.clear();
	}

	bool m_hasSkeleton = false;
	std::vector<std::string> m_animationNames;
	std::vector<std::string> m_skinNames;
	std::vector<std::string> m_slotNames;
	std::vector<TextureInfo> m_textureInfos;
	std::vector<std::unique_ptr<RuntimeTexture>> m_textureRecords;
	std::unordered_map<std::string, Live2DClippingBinding> m_live2DClipping;
	std::unordered_map<std::string, RuntimeSlotOverride> m_slotOverrides;
	sp::Vector<float> m_worldVertices;
	size_t m_drawCursor = 0;
	sp::Vector<float> m_maskWorldVertices;
	sp::Vector<unsigned short> m_quadIndices;
	sp::SkeletonClipping m_clipper;
	std::string m_lastError;
	std::unique_ptr<sp::TextureLoader> m_textureLoader;
	std::unique_ptr<sp::Atlas> m_atlas;
	std::unique_ptr<sp::SkeletonData> m_skeletonData;
	std::unique_ptr<sp::Skeleton> m_skeleton;
	std::unique_ptr<sp::Skin> m_compositeSkin;
	bool m_compositeContainsFace = false;
	std::unique_ptr<sp::AnimationStateData> m_animationStateData;
	std::unique_ptr<sp::AnimationState> m_animationState;
	std::vector<AnimationEvent> m_pendingEvents;
	bool m_collectCompletions = false;
	std::vector<AnimationCompletion> m_pendingCompletions;
};

}

std::unique_ptr<IRuntime> SL_RUNTIME_FACTORY_NAME()
{
	return std::unique_ptr<IRuntime>(new CppRuntimeAdapter());
}

}
