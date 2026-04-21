
#include "loader.h"

namespace spine_loader
{
namespace detail
{

	template <typename Reader>
	std::shared_ptr<spine::SkeletonData> LoadFromFile(spine::Atlas* atlas, float scale, const char* path)
	{
		Reader r(atlas);
		r.setScale(scale);
		spine::SkeletonData* p = r.readSkeletonDataFile(path);
		return p ? std::shared_ptr<spine::SkeletonData>(p) : nullptr;
	}

	template <typename Reader, typename... DataArgs>
	std::shared_ptr<spine::SkeletonData> LoadFromMemory(spine::Atlas* atlas, float scale, DataArgs... args)
	{
		Reader r(atlas);
		r.setScale(scale);
		spine::SkeletonData* p = r.readSkeletonData(args...);
		return p ? std::shared_ptr<spine::SkeletonData>(p) : nullptr;
	}
}

std::shared_ptr<spine::SkeletonData> ReadTextSkeletonFromFile(const char* filePath, spine::Atlas* atlas, float scale)
{
	return detail::LoadFromFile<spine::SkeletonJson>(atlas, scale, filePath);
}

std::shared_ptr<spine::SkeletonData> ReadBinarySkeletonFromFile(const char* filePath, spine::Atlas* atlas, float scale)
{
	return detail::LoadFromFile<spine::SkeletonBinary>(atlas, scale, filePath);
}

std::shared_ptr<spine::SkeletonData> ReadTextSkeletonFromMemory(const char* skeletonJson, spine::Atlas* atlas, float scale)
{
	return detail::LoadFromMemory<spine::SkeletonJson>(atlas, scale, skeletonJson);
}

std::shared_ptr<spine::SkeletonData> ReadBinarySkeletonFromMemory(const unsigned char* skeletonBinary, int skeletonLength, spine::Atlas* atlas, float scale)
{
	return detail::LoadFromMemory<spine::SkeletonBinary>(atlas, scale, skeletonBinary, skeletonLength);
}
}
