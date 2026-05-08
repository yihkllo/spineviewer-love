#include "runtime_api.h"

#include <memory>
#include <string>
#include <vector>

#include <spine/Animation.h>
#include <spine/AnimationState.h>
#include <spine/AnimationStateData.h>
#include <spine/Atlas.h>
#include <spine/BlendMode.h>
#include <spine/Bone.h>
#include <spine/ClippingAttachment.h>
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

struct RuntimeTexture
{
	TextureInfo info;
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

class CppRuntimeAdapter final : public IRuntime
{
public:
	RuntimeInfo Info() const noexcept override
	{
		return RuntimeInfo{ RuntimeKindValue(), SL_RUNTIME_DISPLAY_VERSION, SL_RUNTIME_DISPLAY_VERSION };
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

		m_skeleton.reset(new sp::Skeleton(m_skeletonData.get()));
		m_skeleton->setToSetupPose();
		UpdateWorldTransform();

		m_animationStateData.reset(new sp::AnimationStateData(m_skeletonData.get()));
		m_animationState.reset(new sp::AnimationState(m_animationStateData.get()));

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
		UpdateWorldTransform();
	}

	void BuildFrame(int width, int height, Frame& outFrame) override
	{
		outFrame.width = width;
		outFrame.height = height;
		outFrame.draws.clear();
		if (!m_hasSkeleton || !m_skeleton)
			return;

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
	}

	const std::vector<std::string>& MotionNames() const noexcept override { return m_animationNames; }
	const std::vector<std::string>& LookNames() const noexcept override { return m_skinNames; }
	const std::vector<std::string>& SlotCatalog() const noexcept override { return m_slotNames; }
	const std::vector<TextureInfo>& TextureInfos() const noexcept override { return m_textureInfos; }
	void StartMotion(const char* name, bool loop) override
	{
		if (!m_animationState || name == nullptr || name[0] == '\0')
			return;
		m_animationState->setAnimation(0, sp::String(name), loop);
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
			m_animationState->setAnimation(trackIndex, sp::String(names[i].c_str()), loop);
			++trackIndex;
		}
	}

	void ApplyLook(const char* name) override
	{
		if (!m_skeleton || name == nullptr || name[0] == '\0')
			return;
		m_skeleton->setSkin(sp::String(name));
		m_skeleton->setSlotsToSetupPose();
	}

	std::string LastError() const override { return m_lastError; }

private:
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

		DrawCommand command;
		command.textureId = textureId;
		command.slotName = ToStdString(slot.getData().getName());
		command.blendMode = ConvertBlendMode(slot.getData().getBlendMode());
		command.premultipliedAlpha = premultipliedAlpha;
		command.vertices.resize(vertexCount);
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
		outFrame.draws.push_back(std::move(command));
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
		m_skeletonData.reset();
		m_atlas.reset();
		m_textureLoader.reset();
		m_textureRecords.clear();
		m_worldVertices.clear();
	}

	bool m_hasSkeleton = false;
	std::vector<std::string> m_animationNames;
	std::vector<std::string> m_skinNames;
	std::vector<std::string> m_slotNames;
	std::vector<TextureInfo> m_textureInfos;
	std::vector<std::unique_ptr<RuntimeTexture>> m_textureRecords;
	sp::Vector<float> m_worldVertices;
	sp::Vector<unsigned short> m_quadIndices;
	sp::SkeletonClipping m_clipper;
	std::string m_lastError;
	std::unique_ptr<sp::TextureLoader> m_textureLoader;
	std::unique_ptr<sp::Atlas> m_atlas;
	std::unique_ptr<sp::SkeletonData> m_skeletonData;
	std::unique_ptr<sp::Skeleton> m_skeleton;
	std::unique_ptr<sp::AnimationStateData> m_animationStateData;
	std::unique_ptr<sp::AnimationState> m_animationState;
};

}

std::unique_ptr<IRuntime> SL_RUNTIME_FACTORY_NAME()
{
	return std::unique_ptr<IRuntime>(new CppRuntimeAdapter());
}

}
