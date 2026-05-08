#ifndef SL_PATH_UTIL_H_
#define SL_PATH_UTIL_H_

#include <string>
#include <vector>

namespace path_util
{
	bool CollectFolderEntries(const wchar_t* folderPath, const wchar_t* filterSpec, std::vector<std::wstring>& outPaths);
	bool CollectNeighborFiles(const std::wstring& targetPath, const wchar_t* filterSpec, std::vector<std::wstring>& outPaths, size_t* targetIndex);
	std::string ReadBinaryTextFile(const wchar_t* filePath);
	std::wstring ApplicationDirectory();
	bool FileExists(const std::wstring& path);
	std::wstring GetBundledFontPath();
	void ScanSkeletonFilesRecursive(const std::wstring& folder, std::vector<std::wstring>& outPaths);
}
#endif
