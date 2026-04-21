
#include <Windows.h>
#include <shlwapi.h>
#include <algorithm>

#include "sl_path_util.h"

#pragma comment(lib, "Shlwapi.lib")

namespace path_util
{

	static bool CreateFilaNameList(const wchar_t* pwzFolderPath, const wchar_t* pwzFileNamePattern, std::vector<std::wstring>& wstrNames)
	{
		if (pwzFolderPath == nullptr)return false;

		std::wstring wstrPath = pwzFolderPath;
		if (pwzFileNamePattern != nullptr)
		{
			if (wcschr(pwzFileNamePattern, L'*') == nullptr)
			{
				wstrPath += L'*';
			}
			wstrPath += pwzFileNamePattern;
		}
		else
		{
			wstrPath += L'*';
		}

		WIN32_FIND_DATAW sFindData;

		HANDLE hFind = ::FindFirstFileW(wstrPath.c_str(), &sFindData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			if (pwzFileNamePattern != nullptr)
			{
				do
				{

					if (!(sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						wstrNames.push_back(sFindData.cFileName);
					}
				} while (::FindNextFileW(hFind, &sFindData));
			}
			else
			{
				do
				{

					if ((sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						if (wcscmp(sFindData.cFileName, L".") != 0 && wcscmp(sFindData.cFileName, L"..") != 0)
						{
							wstrNames.push_back(sFindData.cFileName);
						}
					}
				} while (::FindNextFileW(hFind, &sFindData));
			}

			::FindClose(hFind);
		}
		return wstrNames.size() > 0;
	}
}


bool path_util::CreateFilePathList(const wchar_t* pwzFolderPath, const wchar_t* pwzFileSpec, std::vector<std::wstring>& paths)
{
	if (pwzFolderPath == nullptr || *pwzFolderPath == L'\0')return false;

	std::wstring wstrParent = pwzFolderPath;
	if (wstrParent.back() != L'\\')
	{
		wstrParent += L"\\";
	}
	std::vector<std::wstring> wstrNames;

	if (pwzFileSpec != nullptr)
	{
		const auto SplitSpecs = [](const wchar_t* pwzFileSpec, std::vector<std::wstring>& specs)
			-> void
			{
				std::wstring wstrTemp;
				for (const wchar_t* p = pwzFileSpec; *p != L'\0' && p != nullptr; ++p)
				{
					if (*p == L';')
					{
						if (!wstrTemp.empty())
						{
							specs.push_back(wstrTemp);
							wstrTemp.clear();
						}
						continue;
					}

					wstrTemp.push_back(*p);
				}

				if (!wstrTemp.empty())
				{
					specs.push_back(wstrTemp);
				}
			};
		std::vector<std::wstring> specs;
		SplitSpecs(pwzFileSpec, specs);

		for (const auto& spec : specs)
		{
			CreateFilaNameList(wstrParent.c_str(), spec.c_str(), wstrNames);
		}
	}
	else
	{
		CreateFilaNameList(wstrParent.c_str(), pwzFileSpec, wstrNames);
	}


	std::sort(wstrNames.begin(), wstrNames.end(), [](const std::wstring& a, const std::wstring& b) {
		return ::StrCmpLogicalW(a.c_str(), b.c_str()) < 0;
	});

	for (const std::wstring& wstr : wstrNames)
	{
		paths.push_back(wstrParent + wstr);
	}

	return paths.size() > 0;
}

bool path_util::GetFilePathListAndIndex(const std::wstring& wstrPath, const wchar_t* pwzFileSpec, std::vector<std::wstring>& paths, size_t* nIndex)
{
	std::wstring wstrParent;

	size_t nPos = wstrPath.find_last_of(L"\\/");
	if (nPos != std::wstring::npos)
	{
		wstrParent = wstrPath.substr(0, nPos);
	}

	path_util::CreateFilePathList(wstrParent.c_str(), pwzFileSpec, paths);

	const auto& iter = std::find(paths.begin(), paths.end(), wstrPath);
	if (iter != paths.end())
	{
		*nIndex = std::distance(paths.begin(), iter);
	}

	return iter != paths.end();
}

std::string path_util::LoadFileAsString(const wchar_t* pwzFilePath)
{
	HANDLE hFile = ::CreateFileW(pwzFilePath, GENERIC_READ, FILE_SHARE_READ,
		nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE) return {};

	LARGE_INTEGER liSize{};
	const bool gotSize = ::GetFileSizeEx(hFile, &liSize) && liSize.QuadPart > 0;
	if (!gotSize)
	{
		::CloseHandle(hFile);
		return {};
	}

	std::string content(static_cast<size_t>(liSize.QuadPart), '\0');
	DWORD ulRead = 0;
	const BOOL ok = ::ReadFile(hFile, &content[0], static_cast<DWORD>(liSize.QuadPart), &ulRead, nullptr);
	::CloseHandle(hFile);

	if (!ok || ulRead == 0) return {};
	content.resize(ulRead);
	return content;
}

std::wstring path_util::GetCurrentProcessPath()
{
	wchar_t buf[MAX_PATH]{};
	const DWORD len = ::GetModuleFileNameW(nullptr, buf, MAX_PATH);
	if (len == 0) return {};


	const wchar_t* end = buf + len;
	while (end != buf && *(end - 1) != L'\\' && *(end - 1) != L'/') --end;
	return std::wstring(buf, end == buf ? len : static_cast<size_t>(end - buf - 1));
}

bool path_util::FileExists(const std::wstring& path)
{
	if (path.empty()) return false;
	const DWORD attr = ::GetFileAttributesW(path.c_str());
	return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

std::wstring path_util::GetBundledFontPath()
{
	const std::wstring exeDir = GetCurrentProcessPath();
	if (exeDir.empty()) return {};

	const std::wstring directPath = exeDir + L"\\NotoSansSC-Regular.ttf";
	if (FileExists(directPath)) return directPath;

	const std::wstring fontsPath = exeDir + L"\\fonts\\NotoSansSC-Regular.ttf";
	if (FileExists(fontsPath)) return fontsPath;

	return {};
}

std::wstring path_util::CreateWorkFolder(const std::wstring& wstrRelativePath)
{
	if (wstrRelativePath.empty())return std::wstring();

	std::wstring wstrPath = GetCurrentProcessPath();
	if (wstrPath.empty())return std::wstring{};

	wstrPath.push_back(L'\\');
	size_t nRead = 0;
	if (wstrRelativePath[0] == L'\\' || wstrRelativePath[0] == L'/')++nRead;

	for (const wchar_t* pStart = wstrRelativePath.data();;)
	{
		size_t nPos = wstrRelativePath.find_first_of(L"\\/", nRead);
		if (nPos == std::wstring::npos)
		{
			wstrPath.append(pStart + nRead, wstrRelativePath.size() - nRead).push_back(L'\\');
			::CreateDirectoryW(wstrPath.c_str(), nullptr);

			break;
		}
		wstrPath.append(pStart + nRead, nPos - nRead).push_back(L'\\');
		::CreateDirectoryW(wstrPath.c_str(), nullptr);

		nRead = nPos + 1;
	}

	return wstrPath;
}

static void ScanSkeletonFilesRecursiveImpl(const std::wstring& folder, std::vector<std::wstring>& outPaths, int depth)
{
	if (depth <= 0) return;
	WIN32_FIND_DATAW fd;
	HANDLE h = ::FindFirstFileW((folder + L"\\*").c_str(), &fd);
	if (h == INVALID_HANDLE_VALUE) return;
	do {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcscmp(fd.cFileName, L".") != 0 && wcscmp(fd.cFileName, L"..") != 0)
				ScanSkeletonFilesRecursiveImpl(folder + L"\\" + fd.cFileName, outPaths, depth - 1);
		}
		else
		{
			std::wstring name(fd.cFileName);
			auto endsWith = [&](const wchar_t* ext) {
				size_t el = wcslen(ext), nl = name.size();
				return nl >= el && name.compare(nl - el, el, ext) == 0;
			};
			if (endsWith(L".json") || endsWith(L".skel") || endsWith(L".bin"))
			{

				std::wstring stem = name.substr(0, name.rfind(L'.'));
				std::wstring atlasPath = folder + L"\\" + stem + L".atlas";
				DWORD attr = ::GetFileAttributesW(atlasPath.c_str());
				if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY))
				{

					atlasPath = folder + L"\\" + stem + L".atlas.txt";
					attr = ::GetFileAttributesW(atlasPath.c_str());
				}
				if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY))
					outPaths.push_back(folder + L"\\" + name);
			}
		}
	} while (::FindNextFileW(h, &fd));
	::FindClose(h);
}

void path_util::ScanSkeletonFilesRecursive(const std::wstring& folder, std::vector<std::wstring>& outPaths)
{
	ScanSkeletonFilesRecursiveImpl(folder, outPaths, 8);
}
