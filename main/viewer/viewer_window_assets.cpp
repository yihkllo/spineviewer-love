#include "viewer_window_common.h"

using namespace viewer_window_detail;

void SpineLoveWindow::BrowseSkeletonFiles()
{
	SModalGuard guard(m_isModalOpen);
	::LockWindowUpdate(m_windowHandle);
	shell_dialogs::FileDialogRequest skeletonRequest{};
	skeletonRequest.title = L"Select skeleton files";
	skeletonRequest.primaryFilter = { L"skeleton files", L"*.json;*.skel" };
	skeletonRequest.allowAnyFile = true;
	std::vector<std::wstring> skelFilePaths = m_fileDialogs.PickMultipleFiles(skeletonRequest, m_windowHandle);
	::LockWindowUpdate(nullptr);
	::InvalidateRect(m_windowHandle, nullptr, FALSE);
	if (skelFilePaths.empty())return;

	std::sort(skelFilePaths.begin(), skelFilePaths.end());
	StartSkeletonOpenFromPaths(skelFilePaths);
}

void SpineLoveWindow::StartSkeletonOpenFromPaths(const std::vector<std::wstring>& skelFilePaths)
{
	if (skelFilePaths.empty())return;
	if (QueueLoadedSpineReplaceAction([this, skelFilePaths]() { StartSkeletonOpenFromPaths(skelFilePaths); })) return;

	ClearFolderNeighborhood();

	SkeletonAssetBundle bundle;
	if (!ReadSkeletonAssetBundle(skelFilePaths, bundle, false))
		return;

	if (!OpenSkeletonAssetBundle(bundle))
		return;
	ResetLoadedSpineEntries(bundle.items);
	SelectLoadedSpine(0);
}

bool SpineLoveWindow::BuildSkeletonOpenItem(const std::wstring& skeletonPath, SkeletonOpenItem& item, bool requireCurrentRuntime)
{
	const size_t lastSlash = skeletonPath.find_last_of(L"\\/");
	const size_t lastDot = skeletonPath.find_last_of(L'.');
	const size_t nameStart = lastSlash == std::wstring::npos ? 0 : lastSlash + 1;
	const std::wstring dir = lastSlash == std::wstring::npos ? L"" : skeletonPath.substr(0, nameStart);
	const std::wstring baseName = lastDot == std::wstring::npos || lastDot <= nameStart
		? skeletonPath.substr(nameStart)
		: skeletonPath.substr(nameStart, lastDot - nameStart);

	std::wstring atlasPath = dir + baseName + L".atlas";
	if (!FileExistsW(atlasPath))
	{
		atlasPath = dir + baseName + L".atlas.txt";
		if (!FileExistsW(atlasPath))
		{
			m_fileDialogs.ShowOwnerError((L"Atlas file not found for: " + baseName).c_str(), m_windowHandle);
			return false;
		}
	}

	std::string skeletonFileDatum = path_util::ReadBinaryTextFile(skeletonPath.c_str());
	const sl_skeleton_probe::Result skeletonInfo = sl_skeleton_probe::Inspect(reinterpret_cast<const unsigned char*>(skeletonFileDatum.data()), skeletonFileDatum.size());
	if (!skeletonInfo.IsSpineSkeleton())
	{
		m_fileDialogs.ShowOwnerError(L"This seems not to be valid Spine skeleton file.", m_windowHandle);
		return false;
	}

	if (requireCurrentRuntime)
	{
		if (SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime())
		{
			if (runtime->ContainsDrawableContent())
			{
				const auto versionIndex = m_runtimeHub.LaneForVersionText(skeletonInfo.version.c_str());
				if (versionIndex != m_runtimeHub.CurrentLane())
				{
					m_fileDialogs.ShowOwnerError(L"The file to be added should have the same Spine version as that of being loaded.", m_windowHandle);
					return false;
				}
			}
		}
	}

	item.skeletonPath = skeletonPath;
	item.atlasPath = atlasPath;
	item.textureDirectory = dir;
	item.displayName = ExtractStemUtf8(skeletonPath);
	item.version = skeletonInfo.version;
	item.binarySkeleton = skeletonInfo.kind == sl_skeleton_probe::FileKind::Binary;
	return true;
}

bool SpineLoveWindow::ReadSkeletonAssetBundle(const std::vector<std::wstring>& skeletonPaths, SkeletonAssetBundle& bundle, bool requireCurrentRuntime)
{
	bundle = {};
	if (skeletonPaths.empty())
		return false;

	for (const std::wstring& skeletonPath : skeletonPaths)
	{
		SkeletonOpenItem item;
		if (!BuildSkeletonOpenItem(skeletonPath, item, requireCurrentRuntime))
			return false;

		if (!bundle.items.empty() && bundle.binarySkeleton != item.binarySkeleton)
		{
			m_fileDialogs.ShowOwnerError(L"Please open either binary .skel files or JSON files together, not both.", m_windowHandle);
			return false;
		}
		if (!bundle.items.empty() && bundle.items.front().version != item.version)
		{
			m_fileDialogs.ShowOwnerError(L"Please open Spine files from the same runtime version together.", m_windowHandle);
			return false;
		}

		bundle.binarySkeleton = item.binarySkeleton;
		bundle.atlasBytes.emplace_back(path_util::ReadBinaryTextFile(item.atlasPath.c_str()));
		bundle.textureDirectoriesUtf8.emplace_back(sl_text::WideToUtf8(item.textureDirectory));
		bundle.skeletonBytes.emplace_back(path_util::ReadBinaryTextFile(item.skeletonPath.c_str()));
		bundle.items.push_back(std::move(item));
	}

	if (!bundle.items.empty())
	{
		bundle.title = sl_text::Utf8ToWide(bundle.items.front().displayName);
	}
	return !bundle.items.empty();
}

bool SpineLoveWindow::AddSpineFromPath(const std::wstring& skelFilePath)
{
	for (size_t i = 0; i < m_loadedSpineEntries.size(); ++i)
	{
		if (m_loadedSpineEntries[i].skelPath == skelFilePath)
		{
			m_showLoadedSpinePanel = true;
			SelectLoadedSpine(i);
			return true;
		}
	}

	SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	if (runtime == nullptr || !runtime->ContainsDrawableContent())
	{
		StartSkeletonOpenFromPaths({ skelFilePath });
		m_showLoadedSpinePanel = !m_loadedSpineEntries.empty();
		SyncLoadedSpineDatum();
		return !m_loadedSpineEntries.empty();
	}

	SkeletonOpenItem item;
	if (!BuildSkeletonOpenItem(skelFilePath, item, true))
		return false;

	const std::string atlasUtf8 = sl_text::WideToUtf8(item.atlasPath);
	const std::string skelUtf8 = sl_text::WideToUtf8(skelFilePath);
	SlLayerFileRequest layerRequest{};
	layerRequest.atlasPath = atlasUtf8.c_str();
	layerRequest.skeletonPath = skelUtf8.c_str();
	layerRequest.binarySkeleton = item.binarySkeleton;
	if (!runtime->AddLayerFromFiles(layerRequest))
		return false;

	size_t insertedRuntimeIndex = runtime->LoadedSkeletonCount();
	if (insertedRuntimeIndex > 0)
	{
		--insertedRuntimeIndex;
		while (insertedRuntimeIndex > 0 && runtime->PromoteSkeleton(insertedRuntimeIndex))
			--insertedRuntimeIndex;
	}

	m_loadedSpineEntries.push_back({ item.skeletonPath, item.atlasPath, item.displayName });
	m_loadedSpineVisibility.push_back(true);
	std::rotate(m_loadedSpineEntries.begin(), m_loadedSpineEntries.end() - 1, m_loadedSpineEntries.end());
	std::rotate(m_loadedSpineVisibility.begin(), m_loadedSpineVisibility.end() - 1, m_loadedSpineVisibility.end());
	m_showLoadedSpinePanel = true;
	SelectLoadedSpine(0);
	const std::vector<std::string>& animationNames = runtime->MotionNames();
	if (!animationNames.empty() &&
		std::find(animationNames.begin(), animationNames.end(), runtime->ActiveMotionName()) == animationNames.end())
	{
		runtime->PlayMotionByIndex(0);
	}
	const std::vector<std::string>& skinNames = runtime->LookNames();
	if (!skinNames.empty() &&
		std::find(skinNames.begin(), skinNames.end(), runtime->ActiveLookName()) == skinNames.end())
	{
		runtime->ApplyLookByIndex(0);
	}
	return true;
}

void SpineLoveWindow::BrowseSkeletonFolder()
{
	SModalGuard guard(m_isModalOpen);
	std::wstring pickedFolder = m_fileDialogs.PickFolder(m_windowHandle);
	if (!pickedFolder.empty())
	{
		RefreshFolderBrowser(pickedFolder);
		if (StartSkeletonOpenFromFolder(pickedFolder))
		{
			RememberFolderNeighborhood(pickedFolder);
		}
	}
}

bool SpineLoveWindow::StartSkeletonOpenFromFolder(const std::wstring& folderPath)
{
	if (QueueLoadedSpineReplaceAction([this, folderPath]() { StartSkeletonOpenFromFolder(folderPath); })) return false;

	std::vector<std::wstring> skeletonPaths;
	path_util::ScanSkeletonFilesRecursive(folderPath, skeletonPaths);
	if (skeletonPaths.empty())
	{
		m_fileDialogs.ShowOwnerError(L"No supported Spine skeleton files were found in this folder.", m_windowHandle);
		return false;
	}

	StartSkeletonOpenFromPaths(skeletonPaths);
	return true;
}

bool SpineLoveWindow::OpenSkeletonAssetBundle(const SkeletonAssetBundle& bundle)
{
	if (m_isLoading) return false;
	if (bundle.items.empty() || bundle.skeletonBytes.empty())return false;

	SlRuntimeHub::RuntimeLane versionIndex = m_runtimeHub.LaneForVersionText(bundle.items.front().version.c_str());
	if (versionIndex == SlRuntimeHub::RuntimeLane::Unknown)
	{
		m_fileDialogs.ShowOwnerError(L"The runtime for this version is not implemented.", m_windowHandle);
		return false;
	}
	if (!m_runtimeHub.LaneIsReady(versionIndex))
	{
		m_fileDialogs.ShowOwnerError(L"The dll for this version is not loaded.", m_windowHandle);
		return false;
	}
	m_runtimeHub.ActivateLane(versionIndex);

	SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	const SPlaybackSnapshot snapshot = CapturePlaybackSnapshot(runtime);

	if (snapshot.hadLoaded)
	{
		m_lastAnimationName = runtime->ActiveMotionName();
		m_lastSkinName      = runtime->ActiveLookName();
	}

	m_isLoading = true;
	SlMemoryBundleRequest loadRequest{};
	loadRequest.atlasText = bundle.atlasBytes;
	loadRequest.textureRoots = bundle.textureDirectoriesUtf8;
	loadRequest.skeletonBytes = bundle.skeletonBytes;
	loadRequest.binarySkeleton = bundle.binarySkeleton;
	bool hasLoaded = runtime->LoadBundleFromMemory(loadRequest);
	m_isLoading = false;
	RestorePlaybackSnapshot(runtime, snapshot, hasLoaded);
	ApplySkeletonOpenResult(snapshot.hadLoaded, hasLoaded, bundle.title.c_str());
	return hasLoaded;
}

void SpineLoveWindow::ApplySkeletonOpenResult(bool hadLoaded, bool hasLoaded, const wchar_t* windowName)
{
	if (hasLoaded)
	{
		ClearAnimationQueue();

		if (m_runtimeHub.CurrentRuntime()->IsMirroredHorizontally())
			m_runtimeHub.CurrentRuntime()->ToggleMirrorX();
		while (m_runtimeHub.CurrentRuntime()->QuarterTurns() != 0)
			m_runtimeHub.CurrentRuntime()->RotateClockwise();

		if (m_userPmaSet)
		{
			bool currentPma = m_runtimeHub.CurrentRuntime()->UsesPremultipliedAlpha();
			if (currentPma != m_userPma)
				m_runtimeHub.CurrentRuntime()->SetPremultipliedAlpha(m_userPma);
		}

		if (!m_lastAnimationName.empty())
		{
			const auto& animNames = m_runtimeHub.CurrentRuntime()->MotionNames();
			auto animIt = std::find(animNames.begin(), animNames.end(), m_lastAnimationName);
			if (animIt != animNames.end())
				m_runtimeHub.CurrentRuntime()->PlayMotionByName(m_lastAnimationName.c_str());
		}

		m_runtimeHub.CurrentRuntime()->TickPlayback(0.0f);

		if (m_runtimeHub.CurrentRuntime()->ResetsViewOnLoad())
			m_runtimeHub.CurrentRuntime()->SetViewOffset(0.0f, 0.0f);
		else
			m_runtimeHub.CurrentRuntime()->CenterOpeningPoseInView();

		ResizeToContentBounds();
		SetViewerTitle(windowName);
		++m_panelState.loadGeneration;

		if (spine_panel::HasSlotNameQueryFilter())
		{
			m_runtimeHub.CurrentRuntime()->SetSlotVisibilityRule(spine_panel::GetSlotNameQueryExcludeCallback());
			m_runtimeHub.CurrentRuntime()->TickPlayback(0.0f);
		}

		m_frameClock.Reset();
	}
	else
	{
		std::wstring message = L"Failed to load Spine(s)";
		const std::string lastError = m_runtimeHub.CurrentRuntime()->LastRuntimeIssue();
		if (!lastError.empty())
		{
			message += L"\n\n";
			message += sl_text::Utf8ToWide(lastError);
		}
		m_fileDialogs.ShowOwnerError(message.c_str(), m_windowHandle);
		SetViewerTitle(nullptr);
	}
	if (hadLoaded != hasLoaded)RefreshPanelCommandState();
}

std::string SpineLoveWindow::ExtractStemUtf8(const std::wstring& path)
{
	size_t pos = path.find_last_of(L"\\/");
	std::wstring filename = (pos != std::wstring::npos) ? path.substr(pos + 1) : path;
	size_t dotPos = filename.find_last_of(L'.');
	if (dotPos != std::wstring::npos) filename = filename.substr(0, dotPos);
	return sl_text::WideToUtf8(filename);
}
