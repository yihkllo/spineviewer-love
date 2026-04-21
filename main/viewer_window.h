#ifndef SPINELOVE_VIEWER_WINDOW_H_
#define SPINELOVE_VIEWER_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>
#include <future>

#include "spine_runtime_registry.h"
#include "shell_dialog_service.h"
#include "frame_export_service.h"
#include "frame_capture.h"
#include "render_handle.h"
#include "sl_timer.h"

#include "sl-widgets/sl_spine_config.h"
#include "sl-widgets/sl_font_config.h"

#include "imgui/spine_panel.h"
#include "mf_media_player.h"

class CMainWindow
{
public:
	CMainWindow();
	~CMainWindow();

	bool Create(HINSTANCE hInstance, const wchar_t* pwzWindowName);
	int MessageLoop();
	void InitializeGraphics();
	void ForceRender();

	HWND GetHwnd()const { return m_hWnd; }
private:
	const wchar_t* m_swzClassName = L"Dxlib-spine window";
	const wchar_t* m_swzDefaultWindowName = L"spinelove";

	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnDropFiles(WPARAM wParam, LPARAM lParam);

	struct Menu
	{
		enum
		{
			kOpenFiles = 1, kOpenFolder, kExtensionSetting,
			kShowToolDialogue, kFontSetting,
			kSeeThroughImage, kAllowDraggedResizing, kReverseZoomDirection,
			kFitToManualSize, kFitToDefaultSize,
		};
	};
	struct MenuBar { enum { kFile, kTool, kWindow }; };
	struct PopupMenu
	{
		enum
		{
			kSnapAsPNG = 1, kSnapAsJPG,
			kExportAsGif, kExportAsVideo, kExportAsWebm, kExportAsPngs, kExportAsJpgs,
			kEndRecording
		};
	};

	POINT m_lastCursorPos{};

	bool m_wasLeftCombinated = false;
	bool m_wasLeftPressed = false;
	bool m_hasLeftBeenDragged = false;
	bool m_wasRightCombinated = false;
	bool m_hasRightBeenDragged = false;

	HMENU m_hMenuBar = nullptr;

	bool m_isFramelessWindow = false;
	bool m_isTransparentWindow = false;
	bool m_isDraggedResizingAllowed = false;
	bool m_isFullscreen = false;
	RECT m_preFullscreenRect{};
	LONG m_preFullscreenStyle = 0;
	LONG m_preFullscreenStyleEx = 0;

	bool m_isZoomDirectionReversed = false;

	std::vector<std::wstring> m_folders;
	size_t m_nFolderIndex = 0;

	bool m_hasProcessedWmPaint = false;
	bool m_hasDragDropEnabled = false;
	CWinClock m_winclock;

	void Tick();

	void InitialiseMenuBar();

	void MenuOnOpenFiles();
	void MenuOnOpenFolder();
	void LoadFilesFromPaths(const std::vector<std::wstring>& skelFilePaths);
	void ScanFolderIntoList(const std::wstring& folderPath);
	void MenuOnExtensionSetting();

	void MenuOnShowToolDialogue();
	void MenuOnFont();

	void MenuOnMakeWindowTransparent();
	void MenuOnAllowDraggedResizing();
	void MenuOnReverseZoomDirection();
	void MenuOnFiToManualSize();
	void MenuOnFitToDefaultSize();

	void KeyUpOnNextFolder();
	void KeyUpOnForeFolder();
	void KeyUpOnForeFile(int delta);

	void MenuOnSaveAsJpg();
	void MenuOnSaveAsPng();

	void MenuOnStartRecording(int menuKind);
	void MenuOnEndRecording();


	void MenuOnExportWebm();
	bool LaunchFfmpegWebmPipe(const std::wstring& wstrOutputPath, int fps, int width, int height, HANDLE& hPipeWrite_out);
	bool m_isWebmEncoding = false;
	std::wstring m_webmOutputPath;
	HANDLE m_hFfmpegProcess = nullptr;

	void ChangeWindowTitle(const wchar_t* pwzTitle);
	std::wstring GetWindowTitle() const;

	void ToggleWindowFrameStyle();
	void ToggleFullscreen();
	void UpdateMenuItemState();

	bool LoadSpineFilesInFolder(const std::wstring& folderPath);
	bool LoadSpineFiles(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel, const wchar_t* windowName);
	bool LoadSpinesFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& textureDirectories, const std::vector<std::string>& skelData, const wchar_t* windowName);
	void ClearFolderPathList();
	void PostSpineLoading(bool hadLoaded, bool hasLoaded, const wchar_t* windowName);

	std::wstring BuildExportFilePath();
	std::wstring FormatAnimationTime(float fAnimationTime);
	void StepRecording();
	void ApplyResize();


	int CreateFlattenedTexture(int srcTex, int r, int g, int b);


	static std::string ExtractStemUtf8(const std::wstring& path);


	void PrepareQueueRecording(int menuKind);

	int m_pendingClientWidth = 0;
	int m_pendingClientHeight = 0;
	bool m_hasPendingResize = false;
	bool m_isModalOpen = false;
	bool m_isLoading = false;

	bool m_userPma = false;
	bool m_userPmaSet = false;

	
	
	std::string m_lastAnimationName;
	std::string m_lastSkinName;

	using DxLibImageHandle = DxLibHandle<&DxLib::DeleteGraph>;
	DxLibImageHandle m_spineRenderTexture = { DxLibImageHandle(-1)};

	CSpineRuntimeRegistry m_dxLibSpinePlayer;
	shell_dialogs::CFileDialogService m_fileDialogs;
	frame_export::CFrameExportService m_frameExporter;
	CSpineSettingDialogue m_spineSettingDialogue;

	CDxLibRecorder m_dxLibRecorder;
	spine_panel::SSpineToolDatum m_spineToolDatum;

	CFontSettingDialogue m_fontSettingDialogue;

	void UpdateWindowResizableAttribute();
	void ResizeWindow();

	bool m_toShowSpineParameter = true;
	void ImGuiSpineParameterDialogue();


	std::vector<std::string> m_animQueue;
	bool m_isQueuePlaying = false;
	size_t m_queueIndex = 0;
	bool m_isQueueRecording = false;
	int m_queueRecordMenuKind = 0;
	void StartQueuePlay();
	void StopQueuePlay();
	void StepQueuePlay();


	void MenuOnLoadBackground();
	DxLibImageHandle m_bgTexture = { DxLibImageHandle(-1) };
	float m_bgOffsetX = 0.f;
	float m_bgOffsetY = 0.f;
	float m_bgScale = 1.f;


	void MenuOnLoadAudio();
	void MenuOnPlayAudio();
	void MenuOnStopAudio();
	void MenuOnToggleLoopAudio();
	void MenuOnAudioPrev();
	void MenuOnAudioNext();
	void MenuOnToggleAudioAuto();
	std::unique_ptr<CMfMediaPlayer> m_pAudioPlayer;
	std::wstring m_audioFilePath;
	bool m_isAudioLooping = false;
	bool m_isAudioAuto = false;
	std::vector<std::wstring> m_audioFiles;
	size_t m_audioFileIndex = 0;
	void AudioLoadFile(const std::wstring& path);
	void AudioScanFolder(const std::wstring& folderPath);
	void AudioSyncDatum();


};

#endif

