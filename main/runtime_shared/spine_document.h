#ifndef SPINELOVE_RUNTIME_SHARED_SPINE_DOCUMENT_H_
#define SPINELOVE_RUNTIME_SHARED_SPINE_DOCUMENT_H_

#include <string>
#include <vector>

struct SlFileBundleRequest
{
	std::vector<std::string> atlasFiles;
	std::vector<std::string> skeletonFiles;
	bool binarySkeleton = false;
};

struct SlMemoryBundleRequest
{
	std::vector<std::string> atlasText;
	std::vector<std::string> textureRoots;
	std::vector<std::string> skeletonBytes;
	bool binarySkeleton = false;
};

struct SlLayerFileRequest
{
	const char* atlasPath = nullptr;
	const char* skeletonPath = nullptr;
	bool binarySkeleton = false;
};

class SlAssetIntake
{
public:
	virtual ~SlAssetIntake() = default;

	virtual bool LoadBundleFromFiles(const SlFileBundleRequest& request) = 0;
	virtual bool LoadBundleFromMemory(const SlMemoryBundleRequest& request) = 0;
	virtual bool AddLayerFromFiles(const SlLayerFileRequest& request) = 0;
	virtual bool ContainsDrawableContent() const noexcept = 0;
	virtual std::string LastRuntimeIssue() const = 0;
};

#endif
