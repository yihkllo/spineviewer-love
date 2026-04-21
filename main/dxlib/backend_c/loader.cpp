

#include "loader.h"

namespace spine_loader_c
{
namespace detail
{

	template <typename T, typename Disposer>
	std::shared_ptr<T> WrapShared(T* raw, Disposer disposer)
	{
		if (!raw) return nullptr;
		return std::shared_ptr<T>(raw, [disposer](T* p) { if (p) disposer(p); });
	}


	template <typename Creator, typename Reader, typename Disposer>
	std::shared_ptr<spSkeletonData> ReadSkelFromFile(
		Creator create, Reader readFile, Disposer dispose,
		const char* filePath, spAtlas* atlas, float scale)
	{
		auto* ctx = create(atlas);
		ctx->scale = scale;
		auto data = WrapShared(readFile(ctx, filePath), spSkeletonData_dispose);
		dispose(ctx);
		return data;
	}


	template <typename Creator, typename Reader, typename Disposer, typename... Extra>
	std::shared_ptr<spSkeletonData> ReadSkelFromMemory(
		Creator create, Reader readMem, Disposer dispose,
		spAtlas* atlas, float scale, Extra... args)
	{
		auto* ctx = create(atlas);
		ctx->scale = scale;
		auto data = WrapShared(readMem(ctx, args...), spSkeletonData_dispose);
		dispose(ctx);
		return data;
	}
}

std::shared_ptr<spAtlas> CreateAtlasFromFile(const char* filePath, void* rendererObject)
{
	return detail::WrapShared(spAtlas_createFromFile(filePath, rendererObject), spAtlas_dispose);
}

std::shared_ptr<spAtlas> CreateAtlasFromMemory(const char* atlasData, int atlasLength, const char* fileDirectory, void* rendererObject)
{
	return detail::WrapShared(spAtlas_create(atlasData, atlasLength, fileDirectory, rendererObject), spAtlas_dispose);
}

std::shared_ptr<spSkeletonData> ReadTextSkeletonFromFile(const char* filePath, spAtlas* atlas, float scale)
{
	return detail::ReadSkelFromFile(spSkeletonJson_create, spSkeletonJson_readSkeletonDataFile, spSkeletonJson_dispose,
		filePath, atlas, scale);
}

std::shared_ptr<spSkeletonData> ReadBinarySkeletonFromFile(const char* filePath, spAtlas* atlas, float scale)
{
	return detail::ReadSkelFromFile(spSkeletonBinary_create, spSkeletonBinary_readSkeletonDataFile, spSkeletonBinary_dispose,
		filePath, atlas, scale);
}

std::shared_ptr<spSkeletonData> ReadTextSkeletonFromMemory(const char* skeletonJson, spAtlas* atlas, float scale)
{
	return detail::ReadSkelFromMemory(spSkeletonJson_create, spSkeletonJson_readSkeletonData, spSkeletonJson_dispose,
		atlas, scale, skeletonJson);
}

std::shared_ptr<spSkeletonData> ReadBinarySkeletonFromMemory(const unsigned char* skeletonBinary, int skeletonLength, spAtlas* atlas, float scale)
{
	return detail::ReadSkelFromMemory(spSkeletonBinary_create, spSkeletonBinary_readSkeletonData, spSkeletonBinary_dispose,
		atlas, scale, skeletonBinary, skeletonLength);
}
}
