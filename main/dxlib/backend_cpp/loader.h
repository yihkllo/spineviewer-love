#ifndef SPINE_LOADER_H_
#define SPINE_LOADER_H_

#include <memory>
#include <spine/spine.h>

namespace spine_loader
{
	std::shared_ptr<spine::SkeletonData> ReadTextSkeletonFromFile(const char* filePath, spine::Atlas* atlas, float scale = 1.f);
	std::shared_ptr<spine::SkeletonData> ReadBinarySkeletonFromFile(const char* filePath, spine::Atlas* atlas, float scale = 1.f);

	std::shared_ptr<spine::SkeletonData> ReadTextSkeletonFromMemory(const char* skeletonJson, spine::Atlas* atlas, float scale = 1.f);
	std::shared_ptr<spine::SkeletonData> ReadBinarySkeletonFromMemory(const unsigned char* skeletonBinary, int skeletonLength, spine::Atlas* atlas, float scale = 1.f);
}

#endif
