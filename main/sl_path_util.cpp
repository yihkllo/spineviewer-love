#include "sl_path_util.h"

#include <Windows.h>
#include <shlwapi.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <system_error>

#pragma comment(lib, "Shlwapi.lib")

namespace fs = std::filesystem;

namespace
{
	fs::path MakePath(const wchar_t* text)
	{
		return (text == nullptr || *text == L'\0') ? fs::path() : fs::path(text);
	}

	std::wstring FileNameOf(const std::wstring& path)
	{
		return fs::path(path).filename().wstring();
	}

	bool IsSkeletonExtension(const fs::path& path)
	{
		const std::wstring ext = path.extension().wstring();
		return _wcsicmp(ext.c_str(), L".json") == 0 ||
			_wcsicmp(ext.c_str(), L".skel") == 0;
	}

	bool ContainsWildcard(const std::wstring& text)
	{
		return text.find_first_of(L"*?") != std::wstring::npos;
	}

	std::vector<std::wstring> ParsePatternList(const wchar_t* specs)
	{
		std::vector<std::wstring> patterns;
		if (specs == nullptr || *specs == L'\0')
			return patterns;

		std::wstring current;
		for (const wchar_t* p = specs; ; ++p)
		{
			const bool atEnd = *p == L'\0';
			if (!atEnd && *p != L';')
			{
				current.push_back(*p);
				continue;
			}

			if (!current.empty())
			{
				if (!ContainsWildcard(current))
					current.insert(current.begin(), L'*');
				patterns.push_back(current);
				current.clear();
			}

			if (atEnd)
				break;
		}
		return patterns;
	}

	bool MatchesPatterns(const fs::path& path, const std::vector<std::wstring>& patterns)
	{
		if (patterns.empty())
			return true;

		const std::wstring name = path.filename().wstring();
		for (const std::wstring& pattern : patterns)
		{
			if (::PathMatchSpecW(name.c_str(), pattern.c_str()))
				return true;
		}
		return false;
	}

	void SortByDisplayName(std::vector<std::wstring>& paths, size_t beginIndex)
	{
		if (beginIndex >= paths.size())
			return;

		std::sort(paths.begin() + static_cast<std::ptrdiff_t>(beginIndex), paths.end(),
			[](const std::wstring& left, const std::wstring& right) {
				return ::StrCmpLogicalW(FileNameOf(left).c_str(), FileNameOf(right).c_str()) < 0;
			});
	}

	bool HasSiblingAtlas(const fs::path& skeleton)
	{
		const fs::path folder = skeleton.parent_path();
		const fs::path stem = skeleton.stem();
		return path_util::FileExists((folder / stem).wstring() + L".atlas") ||
			path_util::FileExists((folder / stem).wstring() + L".atlas.txt");
	}

	template <typename Iterator>
	bool NextEntry(Iterator& iterator, const Iterator& end, std::error_code& ec)
	{
		if (iterator == end)
			return false;

		iterator.increment(ec);
		if (ec)
			ec.clear();
		return iterator != end;
	}
}

bool path_util::CollectFolderEntries(const wchar_t* folderPath, const wchar_t* filterSpec, std::vector<std::wstring>& outPaths)
{
	const fs::path folder = MakePath(folderPath);
	if (folder.empty())
		return false;

	std::error_code ec;
	if (!fs::is_directory(folder, ec))
		return false;

	const bool wantDirectories = filterSpec == nullptr;
	const std::vector<std::wstring> patterns = ParsePatternList(filterSpec);
	const size_t originalCount = outPaths.size();

	fs::directory_iterator it(folder, fs::directory_options::skip_permission_denied, ec);
	fs::directory_iterator end;
	for (; it != end; NextEntry(it, end, ec))
	{
		const fs::directory_entry& entry = *it;
		const bool accept = wantDirectories
			? entry.is_directory(ec)
			: (entry.is_regular_file(ec) && MatchesPatterns(entry.path(), patterns));
		if (accept)
			outPaths.push_back(entry.path().wstring());
	}

	SortByDisplayName(outPaths, originalCount);
	return outPaths.size() > originalCount;
}

bool path_util::CollectNeighborFiles(const std::wstring& targetPath, const wchar_t* filterSpec, std::vector<std::wstring>& outPaths, size_t* targetIndex)
{
	const size_t originalCount = outPaths.size();
	const fs::path target(targetPath);
	CollectFolderEntries(target.parent_path().c_str(), filterSpec, outPaths);

	const auto begin = outPaths.begin() + static_cast<std::ptrdiff_t>(originalCount);
	const auto found = std::find(begin, outPaths.end(), target.wstring());
	if (found == outPaths.end())
		return false;

	if (targetIndex != nullptr)
		*targetIndex = static_cast<size_t>(std::distance(outPaths.begin(), found));
	return true;
}

std::string path_util::ReadBinaryTextFile(const wchar_t* filePath)
{
	const fs::path path = MakePath(filePath);
	if (path.empty())
		return {};

	std::ifstream input(path, std::ios::binary);
	if (!input)
		return {};

	input.seekg(0, std::ios::end);
	const std::streamoff size = input.tellg();
	if (size <= 0)
		return {};

	std::string content(static_cast<size_t>(size), '\0');
	input.seekg(0, std::ios::beg);
	input.read(&content[0], static_cast<std::streamsize>(content.size()));
	content.resize(static_cast<size_t>(input.gcount()));
	return content;
}

std::wstring path_util::ApplicationDirectory()
{
	std::wstring modulePath(MAX_PATH, L'\0');
	for (;;)
	{
		const DWORD copied = ::GetModuleFileNameW(nullptr, &modulePath[0], static_cast<DWORD>(modulePath.size()));
		if (copied == 0)
			return {};
		if (copied < modulePath.size() - 1)
		{
			modulePath.resize(copied);
			return fs::path(modulePath).parent_path().wstring();
		}
		modulePath.resize(modulePath.size() * 2);
	}
}

bool path_util::FileExists(const std::wstring& path)
{
	std::error_code ec;
	return fs::is_regular_file(fs::path(path), ec);
}

std::wstring path_util::GetBundledFontPath()
{
	const fs::path exeDir = fs::path(ApplicationDirectory());
	if (exeDir.empty())
		return {};

	const fs::path direct = exeDir / L"NotoSansSC-Regular.ttf";
	if (FileExists(direct.wstring()))
		return direct.wstring();

	const fs::path nested = exeDir / L"fonts" / L"NotoSansSC-Regular.ttf";
	return FileExists(nested.wstring()) ? nested.wstring() : std::wstring();
}

void path_util::ScanSkeletonFilesRecursive(const std::wstring& folder, std::vector<std::wstring>& outPaths)
{
	const fs::path root(folder);
	std::error_code ec;
	if (!fs::is_directory(root, ec))
		return;

	const size_t originalCount = outPaths.size();
	fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, ec);
	fs::recursive_directory_iterator end;
	for (; it != end; NextEntry(it, end, ec))
	{
		if (it.depth() >= 7)
			it.disable_recursion_pending();

		const fs::directory_entry& entry = *it;
		if (!entry.is_regular_file(ec))
			continue;

		const fs::path path = entry.path();
		if (IsSkeletonExtension(path) && HasSiblingAtlas(path))
			outPaths.push_back(path.wstring());
	}

	SortByDisplayName(outPaths, originalCount);
}
