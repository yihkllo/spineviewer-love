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
#include <spine/Sequence.h>
#include <spine/SlotData.h>
#include <spine/TextureLoader.h>

#include <spine/Physics.h>


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
		texture->info.sourcePremultipliedAlpha = page.pma;
		texture->info.renderPremultipliedAlpha = true;
		texture->info.hasPremultipliedAlphaMetadata = true;

		RuntimeTexture* rawTexture = texture.get();
		m_textures.push_back(std::move(texture));
		page.texture = rawTexture;

	}

	void unload(void*) override {}

private:
	std::vector<std::unique_ptr<RuntimeTexture>>& m_textures;
};

class CppRuntimeAdapter final : public IRuntime, public sp::AnimationStateListenerObject
{
public:
	CppRuntimeAdapter()
	{
		static const bool initialized = [] { sp::Bone::setYDown(false); return true; }();
		(void)initialized;
	}
	void callback(sp::AnimationState*, sp::EventType type, sp::TrackEntry* entry, sp::Event* event) override
	{
		if (m_collectCompletions && type == sp::EventType_Complete && entry)
			m_pendingCompletions.push_back({ToStdString(entry->getAnimation().getName()),entry->getTrackIndex(),entry->getTrackTime()});
		if (type != sp::EventType_Event || event == nullptr)
			return;
		AnimationEvent value;
		value.name = ToStdString(event->getData().getName());
		value.stringValue = ToStdString(event->getString());
		value.intValue = event->getInt();
		value.floatValue = event->getFloat();
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
		value = { bone->getAppliedPose().getA(), bone->getAppliedPose().getB(), bone->getAppliedPose().getC(), bone->getAppliedPose().getD(), bone->getAppliedPose().getWorldX(), bone->getAppliedPose().getWorldY() };
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
			sp::SkeletonBinary binary(*m_atlas);
			loadedData = binary.readSkeletonDataFile(sp::String(request.skeletonPaths.front().c_str()));
			if (!loadedData)
				m_lastError = ToStdString(binary.getError());
		}
		else
		{
			sp::SkeletonJson json(*m_atlas);
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
			sp::SkeletonBinary binary(*m_atlas);
			loadedData = binary.readSkeletonData(reinterpret_cast<const unsigned char*>(skeletonText.data()), static_cast<int>(skeletonText.size()));
			if (!loadedData)
				m_lastError = ToStdString(binary.getError());
		}
		else
		{
			sp::SkeletonJson json(*m_atlas);
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

		m_skeleton.reset(new sp::Skeleton(*m_skeletonData));
		m_skeleton->setupPose();
		UpdateWorldTransform();

		m_animationStateData.reset(new sp::AnimationStateData(*m_skeletonData));
		m_animationState.reset(new sp::AnimationState(*m_animationStateData));
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
		m_slotOverrides.clear();
		m_lastError.clear();
		ClearLoadedObjects();
	}

	bool HasSkeleton() const noexcept override { return m_hasSkeleton; }
	void Update(float deltaSeconds) override
	{
		if (!m_hasSkeleton || !m_skeleton)
			return;

		m_skeleton->update(deltaSeconds);

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
		sp::Array<sp::Slot*>& drawOrder = m_skeleton->getDrawOrder().getAppliedPose();
		for (size_t i = 0; i < drawOrder.size(); ++i)
		{
			sp::Slot* slot = drawOrder[i];
			if (!slot)
				continue;

			sp::Attachment* attachment = slot->getAppliedPose().getAttachment();
			if (!attachment)
			{
				m_clipper.clipEnd(*slot);
				continue;
			}

			const bool isClipAttachment = attachment->getRTTI().isExactly(sp::ClippingAttachment::rtti);
			if ((slot->getAppliedPose().getColor().a <= 0.0f || !slot->getBone().isActive()) && !isClipAttachment)
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
				m_clipper.clipStart(*m_skeleton, *slot, static_cast<sp::ClippingAttachment*>(attachment));
			}
		}
		m_clipper.clipEnd();
		outFrame.draws.resize(m_drawCursor);
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
		sp::TrackEntry* entry = &m_animationState->setAnimation(0, sp::String(name), loop);
		if (entry && mixSeconds >= 0.0f) entry->setMixDuration(mixSeconds);
		return entry != nullptr;
	}
	bool QueueMotion(const char* name, bool loop, float mixSeconds) override
	{
		if (!m_animationState || name == nullptr || name[0] == '\0' ||
			(m_skeletonData && m_skeletonData->findAnimation(sp::String(name)) == nullptr))
			return false;
		sp::TrackEntry* entry = &m_animationState->addAnimation(0, sp::String(name), loop, 0.0f);
		if (entry && mixSeconds >= 0.0f) entry->setMixDuration(mixSeconds);
		return entry != nullptr;
	}
	bool SetCurrentMotionTimeScale(float timeScale) noexcept override
	{
		if (!m_animationState) return false;
		sp::TrackEntry* entry = m_animationState->getTrack(0);
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

		sp::Array<sp::TrackEntry*>& tracks = m_animationState->getTracks();
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
		auto* entry = &m_animationState->addAnimation(track, sp::String(name), loop, .01f);
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
		if (!m_animationState || !m_skeletonData || value.track < 0 || value.holdPrevious) return false;
		if (value.animation.empty() && !value.emptyAnimation) {
			m_animationState->clearTrack(value.track);
			return true;
		}
		sp::TrackEntry* entry = nullptr;
		if (value.emptyAnimation) entry = &m_animationState->setEmptyAnimation(value.track,(std::max)(0.f,value.mix));
		else {
			auto* animation = m_skeletonData->findAnimation(sp::String(value.animation.c_str()));
			if (!animation) return false;
			entry = &m_animationState->setAnimation(value.track,*animation,value.loop);
		}
		if (!entry) return false;
		entry->setTrackTime((std::max)(0.f,value.time));
		entry->setTimeScale(value.speed);
		entry->setEventThreshold(value.eventThreshold);
		entry->setMixAttachmentThreshold(value.attachmentThreshold);
		entry->setMixDrawOrderThreshold(value.drawOrderThreshold);

		
		if (value.mix >= 0) entry->setMixDuration(value.mix);
		return true;
	}
	bool HoldMotionTrack(int track, float time) override
	{
		if (!m_animationState || track < 0) return false;
		auto* entry = m_animationState->getTrack(track);
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
		ApplySkinChange(newSkin);
		m_compositeSkin.reset();
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
		size_t added = 0;
		for (const std::string& name : names)
		{
			sp::Skin* skin = m_skeletonData->findSkin(sp::String(name.c_str()));
			if (!skin)
				continue;
			combined->addSkin(*skin);
			++added;
		}
		if (added == 0)
			return;

		ApplySkinChange(combined.get());
		m_compositeSkin = std::move(combined);
	}

	bool SetNativeSkinMix(const std::vector<std::string>& names) override
	{
		if (!m_skeleton || !m_skeletonData) return false;
		std::unique_ptr<sp::Skin> combined(new sp::Skin(sp::String("__spinelove_native_composite")));
		for (const auto& name : names)
		{
			if (auto* skin = m_skeletonData->findSkin(sp::String(name.c_str()))) combined->addSkin(*skin);
		}
		m_skeleton->setSkin(combined.get());
		m_skeleton->setupPoseSlots();
		if (m_animationState) m_animationState->apply(*m_skeleton);
		m_compositeSkin = std::move(combined);
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
			slot.getPose().setAttachment(nullptr);
		else if (state.attachmentMode == SlotAttachmentMode::SetupIfEmpty && !slot.getAppliedPose().getAttachment())
			slot.setupPose();
		else if (state.attachmentMode == SlotAttachmentMode::NamedIfEmpty && !slot.getAppliedPose().getAttachment())
		{
			sp::SlotData& data = slot.getData();
			sp::Attachment* attachment = m_skeleton
				? m_skeleton->getAttachment(data.getIndex(), data.getName()) : nullptr;
			if (attachment) slot.getPose().setAttachment(attachment);
			else slot.setupPose();
		}
		slot.getPose().getColor().a = state.alpha;
	}

	void ApplySlotOverrides()
	{
		if (!m_skeleton) return;
		for (const auto& item : m_slotOverrides)
			if (sp::Slot* slot = m_skeleton->findSlot(sp::String(item.first.c_str())))
				ApplySlotOverride(*slot, item.second);
	}

	void ApplySkinChange(sp::Skin* skin)
	{
		m_skeleton->setSkin(skin);
		m_skeleton->setupPoseSlots();
		if (m_animationState) m_animationState->apply(*m_skeleton);
		ApplySlotOverrides();
		UpdateWorldTransform();
	}
	static RuntimeKind RuntimeKindValue() noexcept { return RuntimeKind::Cpp43; }

	void CollectNames()
	{
		m_animationNames.clear();
		m_skinNames.clear();
		m_slotNames.clear();
		if (!m_skeletonData)
			return;

		sp::Array<sp::Animation*>& animations = m_skeletonData->getAnimations();
		for (size_t i = 0; i < animations.size(); ++i)
		{
			if (animations[i])
				m_animationNames.push_back(ToStdString(animations[i]->getName()));
		}

		sp::Array<sp::Skin*>& skins = m_skeletonData->getSkins();
		for (size_t i = 0; i < skins.size(); ++i)
		{
			if (skins[i])
				m_skinNames.push_back(ToStdString(skins[i]->getName()));
		}

		sp::Array<sp::SlotData*>& slots = m_skeletonData->getSlots();
		for (size_t i = 0; i < slots.size(); ++i)
		{
			if (slots[i])
				m_slotNames.push_back(ToStdString(slots[i]->getName()));
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

	unsigned long long TextureIdFromRegion(sp::TextureRegion* region) const noexcept
	{
		return region ? TextureIdFromObject(region->getRendererObject()) : 0;
	}

	bool TexturePremultipliedFromRegion(sp::TextureRegion* region) const noexcept
	{
		return region ? TexturePremultipliedFromObject(region->getRendererObject()) : false;
	}


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
		region.computeWorldVertices(slot, region.getOffsets(slot.getAppliedPose()), m_worldVertices, 0, 2);
		auto* atlasRegion = static_cast<sp::AtlasRegion*>(region.getSequence().getRegion(region.getSequence().resolveIndex(slot.getAppliedPose())));

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
		sp::Array<float>& uvs = region.getSequence().getUVs(region.getSequence().resolveIndex(slot.getAppliedPose()));
		const Color color = CombineColor(m_skeleton->getColor(), slot.getAppliedPose().getColor(), region.getColor());
		AppendGeometryCommand(textureId, TexturePremultipliedFromRegion(atlasRegion), slot, color, m_worldVertices, uvs, m_quadIndices, outFrame);
	}

	void AppendMeshCommand(sp::MeshAttachment& mesh, sp::Slot& slot, Frame& outFrame)
	{
		const size_t vertexValueCount = mesh.getWorldVerticesLength();
		if (vertexValueCount == 0)
			return;

		m_worldVertices.setSize(vertexValueCount, 0.0f);
		mesh.computeWorldVertices(*m_skeleton, slot, 0, vertexValueCount, m_worldVertices.buffer(), 0, 2);
		auto* atlasRegion = static_cast<sp::AtlasRegion*>(mesh.getSequence().getRegion(mesh.getSequence().resolveIndex(slot.getAppliedPose())));

		const unsigned long long textureId = TextureIdFromRegion(atlasRegion);
		if (textureId == 0)
			return;

		sp::Array<float>& uvs = mesh.getSequence().getUVs(mesh.getSequence().resolveIndex(slot.getAppliedPose()));
		sp::Array<unsigned short>& indices = mesh.getTriangles();
		const Color color = CombineColor(m_skeleton->getColor(), slot.getAppliedPose().getColor(), mesh.getColor());
		AppendGeometryCommand(textureId, TexturePremultipliedFromRegion(atlasRegion), slot, color, m_worldVertices, uvs, indices, outFrame);
	}

	void AppendGeometryCommand(
		unsigned long long textureId,
		bool premultipliedAlpha,
		sp::Slot& slot,
		const Color& color,
		sp::Array<float>& worldVertices,
		sp::Array<float>& uvs,
		sp::Array<unsigned short>& indices,
		Frame& outFrame)
	{
		sp::Array<float>* finalVertices = &worldVertices;
		sp::Array<float>* finalUvs = &uvs;
		sp::Array<unsigned short>* finalIndices = &indices;
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
		if (slot.getAppliedPose().getAttachment())
			AssignString(command.attachmentName, slot.getAppliedPose().getAttachment()->getName());
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

	void UpdateWorldTransform()
	{
		if (!m_skeleton)
			return;
		m_skeleton->updateWorldTransform(sp::Physics_Update);

	}

	void ClearLoadedObjects() noexcept
	{
		m_animationState.reset();
		m_animationStateData.reset();
		m_skeleton.reset();
		m_compositeSkin.reset();
		m_skeletonData.reset();
		m_atlas.reset();
		m_textureLoader.reset();
		m_textureRecords.clear();
		m_worldVertices.clear();
		m_pendingEvents.clear();
		m_pendingCompletions.clear();
	}

	bool m_hasSkeleton = false;
	std::vector<std::string> m_animationNames;
	std::vector<std::string> m_skinNames;
	std::vector<std::string> m_slotNames;
	std::vector<TextureInfo> m_textureInfos;
	std::vector<std::unique_ptr<RuntimeTexture>> m_textureRecords;
	std::unordered_map<std::string, RuntimeSlotOverride> m_slotOverrides;
	sp::Array<float> m_worldVertices;
	size_t m_drawCursor = 0;
	sp::Array<unsigned short> m_quadIndices;
	sp::SkeletonClipping m_clipper;
	std::string m_lastError;
	std::unique_ptr<sp::TextureLoader> m_textureLoader;
	std::unique_ptr<sp::Atlas> m_atlas;
	std::unique_ptr<sp::SkeletonData> m_skeletonData;
	std::unique_ptr<sp::Skeleton> m_skeleton;
	std::unique_ptr<sp::Skin> m_compositeSkin;
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
