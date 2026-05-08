#ifndef SPINELOVE_VIEWER_WINDOW_H_
#define SPINELOVE_VIEWER_WINDOW_H_

#include <Windows.h>

#include <chrono>
#include <string>
#include <vector>
#include <functional>
#include <utility>

#include "spine_runtime_registry.h"
#include "shell_dialog_service.h"
#include "image_exporter.h"

#include "imgui/spine_panel.h"
#include "render_d3d11/d3d11_context.h"

int DeleteTextureHandle(int handle) noexcept;
class CUiCore;

class SlFrameClock
{
public:
	SlFrameClock() { Reset(); }
	~SlFrameClock() = default;

	float ElapsedSeconds() const
	{
		const auto elapsed = Clock::now() - m_lastReset;
		return std::chrono::duration<float>(elapsed).count();
	}

	void Reset() { m_lastReset = Clock::now(); }

private:
	using Clock = std::chrono::steady_clock;
	Clock::time_point m_lastReset = Clock::now();
};

template <int (*Deleter)(int)>
class SlScopedHandle
{
public:
	SlScopedHandle() noexcept = default;

	explicit SlScopedHandle(int handle) noexcept
		: m_handle(handle)
	{}

	~SlScopedHandle() { Release(); }

	int  Get()   const noexcept { return m_handle; }
	bool Empty() const noexcept { return m_handle == -1; }
	explicit operator bool() const noexcept { return m_handle != -1; }

	void Reset() { Release(); }

	void Reset(int newHandle) noexcept
	{
		Release();
		m_handle = newHandle;
	}

	int Detach() noexcept
	{
		int h = m_handle;
		m_handle = -1;
		return h;
	}

	void Swap(SlScopedHandle& other) noexcept { std::swap(m_handle, other.m_handle); }

	SlScopedHandle(SlScopedHandle&& other) noexcept
		: m_handle(other.Detach())
	{}

	SlScopedHandle& operator=(SlScopedHandle&& other) noexcept
	{
		if (this != &other)
		{
			Release();
			m_handle = other.Detach();
		}
		return *this;
	}

	SlScopedHandle(const SlScopedHandle&) = delete;
	SlScopedHandle& operator=(const SlScopedHandle&) = delete;

private:
	int m_handle = -1;

	void Release() noexcept
	{
		if (m_handle != -1 && Deleter != nullptr)
		{
			Deleter(m_handle);
			m_handle = -1;
		}
	}
};

class SpineLoveWindow
{
public:
	SpineLoveWindow();
	~SpineLoveWindow();

	bool OpenNativeWindow(HINSTANCE hInstance, const wchar_t* pwzWindowName);
	int RunMainLoop(CUiCore& uiCore);
	void StartRenderingStack();
	bool StartD3D11Runtime();
	ID3D11Device* NativeD3DDevice() const noexcept;
	ID3D11DeviceContext* NativeD3DContext() const noexcept;

	HWND NativeWindowHandle()const { return m_windowHandle; }
private:
	const wchar_t* m_windowClassName = L"SpineLove window";
	const wchar_t* m_defaultWindowTitle = L"spinelove";

	HINSTANCE m_processInstance = nullptr;
	HWND m_windowHandle = nullptr;

	static LRESULT CALLBACK StaticWindowRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool DispatchNextOsMessage(MSG& msg);
	LRESULT DispatchWindowEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool RouteFrameChromeMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result);
	bool RouteWindowLifetimeMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result);
	bool RouteKeyboardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result);
	bool RoutePointerMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result);
	bool RouteFileDropMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result);
	LRESULT AttachWindowState(HWND hWnd);
	LRESULT ShutdownWindowState();
	LRESULT RequestWindowClose();
	LRESULT RenderPaintMessage();
	LRESULT StageClientResize(WPARAM wParam, LPARAM lParam);
	LRESULT HandleKeyPress(WPARAM wParam, LPARAM lParam);
	LRESULT HandleKeyRelease(WPARAM wParam, LPARAM lParam);
	LRESULT DispatchPanelCommand(WPARAM wParam, LPARAM lParam);
	LRESULT TrackPointerDrag(WPARAM wParam, LPARAM lParam);
	LRESULT ApplyWheelGesture(WPARAM wParam, LPARAM lParam);
	LRESULT BeginPrimaryPointer(WPARAM wParam, LPARAM lParam);
	LRESULT FinishPrimaryPointer(WPARAM wParam, LPARAM lParam);
	LRESULT FinishSecondaryPointer(WPARAM wParam, LPARAM lParam);
	LRESULT FinishMiddlePointer(WPARAM wParam, LPARAM lParam);
	LRESULT AcceptDroppedSkeletons(WPARAM wParam, LPARAM lParam);

	struct PanelCommandId
	{
		enum
		{
			BrowseFiles = 1,
			BrowseFolder,
			ToggleToolPanel,
			ClickThroughWindow,
			DragResizeMode,
			InvertWheelZoom,
			MatchCanvasSize,
			RestoreCanvasSize,
		};
	};
	enum class PointerDragMode
	{
		Idle,
		SkeletonPan,
		BackgroundPan,
		WindowMove,
	};
	POINT m_lastPointerScreen{};
	PointerDragMode m_pointerDragMode = PointerDragMode::Idle;
	bool m_primaryClickCandidate = false;

	bool m_borderlessChrome = true;
	bool m_colorKeyTransparency = false;
	bool m_userResizeEnabled = true;
	bool m_isFullscreen = false;
	struct SWindowSnapshot
	{
		RECT bounds{};
		LONG_PTR style = 0;
		LONG_PTR exStyle = 0;
		bool valid = false;
	};
	SWindowSnapshot m_fullscreenReturnState;

	enum class WheelTarget
	{
		None,
		BackgroundZoom,
		SkeletonZoom,
	};

	bool m_wheelZoomInverted = false;
	int m_wheelRemainder = 0;
	WheelTarget m_activeWheelTarget = WheelTarget::None;

	struct FolderBrowseSession
	{
		std::wstring activeFolder;
		std::vector<std::wstring> siblingFolders;
		size_t siblingIndex = 0;
		std::vector<std::wstring> skeletonFiles;
		size_t previewIndex = 0;
	};
	FolderBrowseSession m_folderBrowser;
	std::wstring m_favoriteCachePath;
	std::vector<std::wstring> m_favoriteSkelFiles;
	struct SLoadedSpineEntry
	{
		std::wstring skelPath;
		std::wstring atlasPath;
		std::string displayName;
	};
	std::vector<SLoadedSpineEntry> m_loadedSpineEntries;
	std::vector<bool> m_loadedSpineVisibility;
	bool m_showLoadedSpinePanel = false;
	size_t m_selectedLoadedSpine = 0;
	bool m_showFavoriteSkelFiles = false;
	bool m_bypassLoadedSpineReplaceConfirm = false;
	std::function<void()> m_pendingLoadedSpineReplaceAction;

	bool m_paintTickConsumed = false;
	bool m_hasDragDropEnabled = false;
	SlFrameClock m_frameClock;
	CUiCore* m_uiCore = nullptr;

	void RunFrame();

	struct SkeletonOpenItem
	{
		std::wstring skeletonPath;
		std::wstring atlasPath;
		std::wstring textureDirectory;
		std::string displayName;
		std::string version;
		bool binarySkeleton = false;
	};

	struct SkeletonAssetBundle
	{
		std::vector<SkeletonOpenItem> items;
		std::vector<std::string> atlasBytes;
		std::vector<std::string> textureDirectoriesUtf8;
		std::vector<std::string> skeletonBytes;
		std::wstring title;
		bool binarySkeleton = false;
	};

	void BrowseSkeletonFiles();
	void BrowseSkeletonFolder();
	void StartSkeletonOpenFromPaths(const std::vector<std::wstring>& skeletonPaths);
	bool ConfirmExitLoadedSpineMode();
	bool IsLoadedSpineReplaceConfirmationNeeded() const;
	bool QueueLoadedSpineReplaceAction(std::function<void()> action);
	void ConfirmLoadedSpineReplaceAction();
	void CancelLoadedSpineReplaceAction();
	bool AddSpineFromPath(const std::wstring& skelFilePath);
	bool BuildSkeletonOpenItem(const std::wstring& skeletonPath, SkeletonOpenItem& item, bool requireCurrentRuntime);
	bool ReadSkeletonAssetBundle(const std::vector<std::wstring>& skeletonPaths, SkeletonAssetBundle& bundle, bool requireCurrentRuntime);
	bool OpenSkeletonAssetBundle(const SkeletonAssetBundle& bundle);
	void ExportCurrentView(SlExportImageFormat format, bool keepAlpha);
	void ExportFrameSequence(SlExportImageFormat format, bool keepAlpha, bool useAnimationQueue);
	void ExportMovie(SlExportMovieFormat format, bool keepAlpha, bool useAnimationQueue);
	void HandlePendingExportRequest();
	void AdvanceExportJob();
	bool RenderExportPixels(SlExportImageFormat format, bool keepAlpha, std::vector<unsigned char>& pixels, int& width, int& height, int& stride, SlColor& matteColor);
	bool WriteAnimationFrames(const std::wstring& folderPath, SlExportImageFormat format, bool keepAlpha, int fps, std::wstring* errorMessage);
	bool EncodeExportMovie(const std::wstring& frameFolder, const std::wstring& outputPath, SlExportMovieFormat format, bool keepAlpha, int fps, std::wstring* errorMessage);
	void ResetLoadedSpineEntries(const std::vector<SkeletonOpenItem>& items);
	void SyncLoadedSpineDatum();
	void SelectLoadedSpine(size_t index);
	void ToggleLoadedSpineVisibility(size_t index);
	void MoveLoadedSpineUp(size_t index);
	void MoveLoadedSpineDown(size_t index);
	void RefreshFolderBrowser(const std::wstring& folderPath);
	void RememberFolderNeighborhood(const std::wstring& folderPath);
	void ClearFolderNeighborhood();
	void FavoriteLoadCache();
	void FavoriteSaveCache() const;
	void FavoriteSyncDatum();
	void FavoriteToggleFile(const std::wstring& path);

	void ToggleToolPanelVisibility();

	void ToggleClickThroughViewer();
	void ToggleDragResizeMode();
	void FlipWheelZoomDirection();
	void MatchWindowToCurrentCanvas();
	void RestoreDefaultCanvasSize();

	bool MoveFolderNeighborhood(int delta);
	bool PreviewSkeletonByOffset(int delta);
	void OpenPreviewedSkeleton();

	void SetViewerTitle(const wchar_t* pwzTitle);
	std::wstring ReadViewerTitle() const;

	void SwitchFrameChrome();
	void SwitchFullscreenMode();
	void RefreshPanelCommandState();

	bool StartSkeletonOpenFromFolder(const std::wstring& folderPath);
	void ClearFolderBrowser();
	void ApplySkeletonOpenResult(bool hadLoaded, bool hasLoaded, const wchar_t* windowName);

	void CommitPendingResize();

	bool PrepareD3D11SpineRenderer();
	bool PrepareSpineSurface(int width, int height);
	bool QuerySpineSurfaceSize(int& outWidth, int& outHeight) const noexcept;


	static std::string ExtractStemUtf8(const std::wstring& path);


	int m_pendingClientWidth = 0;
	int m_pendingClientHeight = 0;
	bool m_hasPendingResize = false;
	bool m_isModalOpen = false;
	bool m_isLoading = false;

	bool m_userPma = true;
	bool m_userPmaSet = true;

	
	
	std::string m_lastAnimationName;
	std::string m_lastSkinName;

	using TextureHandle = SlScopedHandle<&DeleteTextureHandle>;
	TextureHandle m_renderTextureHandle = { TextureHandle(-1)};

	SlRuntimeHub m_runtimeHub;
	sl_d3d11::D3D11Context m_d3d11Context;
	sl_d3d11::D3D11Renderer m_d3d11SpineRenderer;
	bool m_d3d11SpineRendererReady = false;
	SlTextureId m_d3d11SpineRenderTarget = 0;
	int m_d3d11SpineRenderTargetWidth = 0;
	int m_d3d11SpineRenderTargetHeight = 0;
	SlVec4 m_d3d11ClearColor = SlVec4(0.0f, 0.0f, 0.0f, 1.0f);
	shell_dialogs::CFileDialogService m_fileDialogs;

	spine_panel::SpinePanelState m_panelState;

	enum class ExportJobKind
	{
		None,
		FrameSequence,
		Movie,
	};

	enum class ExportJobPhase
	{
		Idle,
		RenderingFrames,
		ReadyToEncode,
		Encoding,
	};

	struct ExportJobState
	{
		ExportJobKind kind = ExportJobKind::None;
		ExportJobPhase phase = ExportJobPhase::Idle;
		SlExportImageFormat imageFormat = SlExportImageFormat::Png;
		SlExportMovieFormat movieFormat = SlExportMovieFormat::Mp4;
		bool keepAlpha = false;
		int fps = 30;
		int frameIndex = 0;
		int frameCount = 0;
		std::wstring frameFolder;
		std::wstring outputPath;
		bool temporaryFrames = false;
		std::string exportMotion;
		std::string restoreMotion;
		float restoreTimeScale = 1.0f;
		float restoreLast = 0.0f;
		std::vector<std::string> exportMotions;
		std::vector<int> motionFrameCounts;
		size_t exportMotionIndex = 0;
		int frameInMotion = 0;
		bool useAnimationQueue = false;
		std::vector<std::wstring> encodeCommands;
		size_t encodeCommandIndex = 0;
		std::wstring encodeFailureMessage;
		HANDLE encodeProcess = nullptr;
		HANDLE encodeThread = nullptr;
	};
	ExportJobState m_exportJob;

	enum class ExportRequestKind
	{
		None,
		Snapshot,
		FrameSequence,
		Movie,
	};

	struct PendingExportRequest
	{
		ExportRequestKind kind = ExportRequestKind::None;
		SlExportImageFormat imageFormat = SlExportImageFormat::Png;
		SlExportMovieFormat movieFormat = SlExportMovieFormat::Mp4;
		bool keepAlpha = false;
		bool useAnimationQueue = false;
	};
	void StartExportRequest(const PendingExportRequest& request);
	PendingExportRequest m_pendingExport;

	bool SkeletonIsReady() const noexcept;
	LONG_PTR ComposeWindowStyle() const noexcept;
	LONG_PTR ComposeWindowExStyle() const noexcept;
	void ApplyWindowShellState(bool keepOuterBounds);
	void ResizeWindowClientArea(int clientWidth, int clientHeight);
	std::wstring NormalizeViewerTitle(const wchar_t* title) const;
	void ResizeToContentBounds();

	bool m_showControlPanel = true;
	void DrawSpineControlPanel();


	std::vector<std::string> m_animQueue;
	bool m_isQueuePlaying = false;
	size_t m_queueIndex = 0;
	void StartQueuePlay();
	void StopQueuePlay();
	void ClearAnimationQueue();
	void StepQueuePlay();


	void BrowseBackgroundImage();
	bool HasBackgroundTexture() const noexcept;
	void GetBackgroundTextureSize(int& outWidth, int& outHeight) const;
	TextureHandle m_bgTexture = { TextureHandle(-1) };
	SlTextureId m_d3d11BackgroundTexture = 0;
	int m_d3d11BackgroundWidth = 0;
	int m_d3d11BackgroundHeight = 0;
	float m_bgOffsetX = 0.f;
	float m_bgOffsetY = 0.f;
	float m_bgScale = 1.f;


};

#endif
