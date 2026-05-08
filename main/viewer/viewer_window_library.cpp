#include "viewer_window_common.h"

using namespace viewer_window_detail;

bool SpineLoveWindow::ConfirmExitLoadedSpineMode()
{
	return !IsLoadedSpineReplaceConfirmationNeeded();
}

bool SpineLoveWindow::IsLoadedSpineReplaceConfirmationNeeded() const
{
	if (m_bypassLoadedSpineReplaceConfirm) return false;
	const SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	if (runtime != nullptr && runtime->LoadedSkeletonCount() > 1)
		return true;
	return m_loadedSpineEntries.size() > 1;
}

bool SpineLoveWindow::QueueLoadedSpineReplaceAction(std::function<void()> action)
{
	if (!IsLoadedSpineReplaceConfirmationNeeded()) return false;
	m_pendingLoadedSpineReplaceAction = std::move(action);
	m_panelState.loadedSpineReplaceConfirmMessage =
		"Loading in this way will exit multi-spine mode and replace the currently loaded spines.";
	m_panelState.showLoadedSpineReplaceConfirm = true;
	return true;
}

void SpineLoveWindow::ConfirmLoadedSpineReplaceAction()
{
	m_panelState.showLoadedSpineReplaceConfirm = false;
	if (!m_pendingLoadedSpineReplaceAction) return;
	auto action = std::move(m_pendingLoadedSpineReplaceAction);
	m_pendingLoadedSpineReplaceAction = {};
	m_bypassLoadedSpineReplaceConfirm = true;
	action();
	m_bypassLoadedSpineReplaceConfirm = false;
}

void SpineLoveWindow::CancelLoadedSpineReplaceAction()
{
	m_panelState.showLoadedSpineReplaceConfirm = false;
	m_pendingLoadedSpineReplaceAction = {};
}

void SpineLoveWindow::RefreshFolderBrowser(const std::wstring& folderPath)
{
	m_folderBrowser.activeFolder = folderPath;
	m_folderBrowser.skeletonFiles.clear();
	m_folderBrowser.previewIndex = 0;

	path_util::ScanSkeletonFilesRecursive(folderPath, m_folderBrowser.skeletonFiles);
	std::sort(m_folderBrowser.skeletonFiles.begin(), m_folderBrowser.skeletonFiles.end());

	m_panelState.skelFileList = m_folderBrowser.skeletonFiles;
	if (!m_folderBrowser.skeletonFiles.empty())
	{
		const auto selected = std::find(
			m_folderBrowser.skeletonFiles.begin(),
			m_folderBrowser.skeletonFiles.end(),
			m_panelState.currentSkelFile);
		if (selected != m_folderBrowser.skeletonFiles.end())
			m_folderBrowser.previewIndex = static_cast<size_t>(std::distance(m_folderBrowser.skeletonFiles.begin(), selected));
	}
}

void SpineLoveWindow::RememberFolderNeighborhood(const std::wstring& folderPath)
{
	m_folderBrowser.siblingFolders.clear();
	m_folderBrowser.siblingIndex = 0;
	path_util::CollectNeighborFiles(folderPath, nullptr, m_folderBrowser.siblingFolders, &m_folderBrowser.siblingIndex);
}

void SpineLoveWindow::ClearFolderNeighborhood()
{
	m_folderBrowser.siblingFolders.clear();
	m_folderBrowser.siblingIndex = 0;
}

void SpineLoveWindow::FavoriteLoadCache()
{
	m_favoriteSkelFiles.clear();
	if (m_favoriteCachePath.empty() || !FileExistsW(m_favoriteCachePath)) return;

	FILE* fp = nullptr;
	if (_wfopen_s(&fp, m_favoriteCachePath.c_str(), L"rt, ccs=UTF-8") != 0 || !fp) return;

	wchar_t line[4096] = {};
	while (fgetws(line, _countof(line), fp))
	{
		std::wstring path = line;
		while (!path.empty() && (path.back() == L'\r' || path.back() == L'\n'))
			path.pop_back();
		if (path.empty() || !FileExistsW(path)) continue;
		if (std::find(m_favoriteSkelFiles.begin(), m_favoriteSkelFiles.end(), path) == m_favoriteSkelFiles.end())
			m_favoriteSkelFiles.push_back(path);
	}
	fclose(fp);
	std::sort(m_favoriteSkelFiles.begin(), m_favoriteSkelFiles.end());
}

void SpineLoveWindow::FavoriteSaveCache() const
{
	if (m_favoriteCachePath.empty()) return;

	FILE* fp = nullptr;
	if (_wfopen_s(&fp, m_favoriteCachePath.c_str(), L"wt, ccs=UTF-8") != 0 || !fp) return;
	for (const std::wstring& path : m_favoriteSkelFiles)
	{
		if (!FileExistsW(path)) continue;
		fwprintf(fp, L"%s\n", path.c_str());
	}
	fclose(fp);
}

void SpineLoveWindow::FavoriteSyncDatum()
{
	m_panelState.favoriteSkelFileList.clear();
	for (const std::wstring& path : m_favoriteSkelFiles)
	{
		if (FileExistsW(path))
			m_panelState.favoriteSkelFileList.push_back(path);
	}
	m_panelState.showFavoriteSkelFiles = m_showFavoriteSkelFiles;
}

void SpineLoveWindow::FavoriteToggleFile(const std::wstring& path)
{
	auto it = std::find(m_favoriteSkelFiles.begin(), m_favoriteSkelFiles.end(), path);
	if (it == m_favoriteSkelFiles.end())
	{
		m_favoriteSkelFiles.push_back(path);
		std::sort(m_favoriteSkelFiles.begin(), m_favoriteSkelFiles.end());
	}
	else
	{
		m_favoriteSkelFiles.erase(it);
	}
	FavoriteSaveCache();
	FavoriteSyncDatum();
}

void SpineLoveWindow::ResetLoadedSpineEntries(const std::vector<SkeletonOpenItem>& items)
{
	m_loadedSpineEntries.clear();
	m_loadedSpineVisibility.clear();
	m_showLoadedSpinePanel = false;
	for (const SkeletonOpenItem& item : items)
	{
		m_loadedSpineEntries.push_back({ item.skeletonPath, item.atlasPath, item.displayName });
		m_loadedSpineVisibility.push_back(true);
	}
	m_selectedLoadedSpine = 0;
	SyncLoadedSpineDatum();
}

void SpineLoveWindow::SyncLoadedSpineDatum()
{
	m_panelState.loadedSpineFileList.clear();
	m_panelState.loadedSpineNames.clear();
	for (const auto& entry : m_loadedSpineEntries)
	{
		m_panelState.loadedSpineFileList.push_back(entry.skelPath);
		m_panelState.loadedSpineNames.push_back(entry.displayName);
	}
	m_panelState.loadedSpineVisibility = m_loadedSpineVisibility;
	m_panelState.showLoadedSpinePanel = m_showLoadedSpinePanel;
	m_panelState.selectedLoadedSpine = static_cast<int>(m_selectedLoadedSpine);
}

void SpineLoveWindow::SelectLoadedSpine(size_t index)
{
	if (index >= m_loadedSpineEntries.size()) return;

	SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	if (runtime == nullptr) return;

	m_selectedLoadedSpine = index;
	runtime->ChooseSkeleton(index);
	m_panelState.currentSkelFile = m_loadedSpineEntries[index].skelPath;
	m_panelState.currentFileName = m_loadedSpineEntries[index].displayName;
	m_panelState.titleBar.subTitle = m_loadedSpineEntries[index].displayName;
	SyncLoadedSpineDatum();
}

void SpineLoveWindow::ToggleLoadedSpineVisibility(size_t index)
{
	if (index >= m_loadedSpineVisibility.size()) return;

	SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	if (runtime == nullptr) return;

	const bool newVisible = !m_loadedSpineVisibility[index];
	if (!runtime->SetSkeletonLayerVisible(index, newVisible))
		return;

	m_loadedSpineVisibility[index] = newVisible;
	SyncLoadedSpineDatum();
}

void SpineLoveWindow::MoveLoadedSpineUp(size_t index)
{
	if (index == 0 || index >= m_loadedSpineEntries.size()) return;

	SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	if (runtime == nullptr) return;
	if (!runtime->PromoteSkeleton(index)) return;

	const size_t target = index - 1;
	std::swap(m_loadedSpineEntries[index], m_loadedSpineEntries[target]);
	std::swap(m_loadedSpineVisibility[index], m_loadedSpineVisibility[target]);
	if (m_selectedLoadedSpine == index)
		m_selectedLoadedSpine = target;
	else if (m_selectedLoadedSpine == target)
		m_selectedLoadedSpine = index;
	SyncLoadedSpineDatum();
}

void SpineLoveWindow::MoveLoadedSpineDown(size_t index)
{
	if (index + 1 >= m_loadedSpineEntries.size()) return;

	SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	if (runtime == nullptr) return;
	if (!runtime->DemoteSkeleton(index)) return;

	const size_t target = index + 1;
	std::swap(m_loadedSpineEntries[index], m_loadedSpineEntries[target]);
	std::swap(m_loadedSpineVisibility[index], m_loadedSpineVisibility[target]);
	if (m_selectedLoadedSpine == index)
		m_selectedLoadedSpine = target;
	else if (m_selectedLoadedSpine == target)
		m_selectedLoadedSpine = index;
	SyncLoadedSpineDatum();
}

bool SpineLoveWindow::MoveFolderNeighborhood(int delta)
{
	const size_t count = m_folderBrowser.siblingFolders.size();
	if (count == 0 || delta == 0)
		return false;

	const int current = static_cast<int>(m_folderBrowser.siblingIndex % count);
	const int wrapped = (current + delta % static_cast<int>(count) + static_cast<int>(count)) % static_cast<int>(count);
	m_folderBrowser.siblingIndex = static_cast<size_t>(wrapped);
	const std::wstring folder = m_folderBrowser.siblingFolders[m_folderBrowser.siblingIndex];
	RefreshFolderBrowser(folder);
	return StartSkeletonOpenFromFolder(folder);
}

bool SpineLoveWindow::PreviewSkeletonByOffset(int delta)
{
	const auto& fileList = m_folderBrowser.skeletonFiles.empty()
		? m_panelState.skelFileList
		: m_folderBrowser.skeletonFiles;
	if (fileList.empty() || delta == 0)
		return false;

	const auto current = std::find(fileList.begin(), fileList.end(), m_panelState.currentSkelFile);
	const size_t baseIndex = current == fileList.end()
		? m_folderBrowser.previewIndex % fileList.size()
		: static_cast<size_t>(std::distance(fileList.begin(), current));
	const int count = static_cast<int>(fileList.size());
	const int next = (static_cast<int>(baseIndex) + delta % count + count) % count;
	m_folderBrowser.previewIndex = static_cast<size_t>(next);

	m_panelState.currentSkelFile = fileList[m_folderBrowser.previewIndex];
	const std::string stem = ExtractStemUtf8(m_panelState.currentSkelFile);
	m_panelState.currentFileName = stem;
	m_panelState.titleBar.subTitle = stem;
	return true;
}

void SpineLoveWindow::OpenPreviewedSkeleton()
{
	if (!m_panelState.currentSkelFile.empty())
		StartSkeletonOpenFromPaths({ m_panelState.currentSkelFile });
}

void SpineLoveWindow::ClearFolderBrowser()
{
	m_folderBrowser = {};
	m_panelState.skelFileList.clear();
}
