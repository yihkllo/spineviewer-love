#ifndef SPINE_LOADER_C_H_
#define SPINE_LOADER_C_H_

#include <memory>

#include <spine/spine.h>

namespace spine_loader_c
{
	std::shared_ptr<spAtlas> CreateAtlasFromFile(const char* filePath, void* rendererObject);
	std::shared_ptr<spAtlas> CreateAtlasFromMemory(const char* atlasData, int atlasLength, const char* fileDirectory, void* rendererObject);

	std::shared_ptr<spSkeletonData> ReadTextSkeletonFromFile(const char* filePath, spAtlas* atlas, float scale = 1.f);
	std::shared_ptr<spSkeletonData> ReadBinarySkeletonFromFile(const char* filePath, spAtlas* atlas, float scale = 1.f);

	std::shared_ptr<spSkeletonData> ReadTextSkeletonFromMemory(const char* skeletonJson, spAtlas* atlas, float scale = 1.f);
	std::shared_ptr<spSkeletonData> ReadBinarySkeletonFromMemory(const unsigned char* skeletonBinary, int skeletonLength, spAtlas* atlas, float scale = 1.f);
}
#endif
