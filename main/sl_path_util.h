#ifndef SL_PATH_UTIL_H_
#define SL_PATH_UTIL_H_

#include <string>
#include <vector>

namespace path_util
{
	bool CreateFilePathList(const wchar_t* pwzFolderPath, const wchar_t* pwzFileSpec, std::vector<std::wstring>& paths);
	bool GetFilePathListAndIndex(const std::wstring& wstrPath, const wchar_t* pwzFileSpec, std::vector<std::wstring>& paths, size_t* nIndex);
	std::string LoadFileAsString(const wchar_t* pwzFilePath);
	std::wstring GetCurrentProcessPath();
	bool FileExists(const std::wstring& path);
	std::wstring GetBundledFontPath();
	std::wstring CreateWorkFolder(const std::wstring &wstrRelativePath);
	void ScanSkeletonFilesRecursive(const std::wstring& folder, std::vector<std::wstring>& outPaths);
}
#endif
