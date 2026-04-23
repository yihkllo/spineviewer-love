
#include <Windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <dwmapi.h>
#include <Shlwapi.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Shlwapi.lib")

#include <algorithm>
#include <future>
#include <chrono>
#include <cstdio>

#include "viewer_window.h"

#include "sl_path_util.h"
#include "shell_dialog_service.h"
#include "sl_text_codec.h"
#include "frame_export_service.h"
#include "json_extract.h"
#include "sl_string_ops.h"
#include "sl-widgets/sl_app_menu.h"

#include "dxlib/shared/file_checker.h"

#include "imgui/ui_core.h"
#include "imgui/custom_titlebar.h"
#include "imgui/i18n.h"
#include <DxLib.h>
#include <imgui.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


struct SModalGuard
{
	bool& flag;
	SModalGuard(bool& f) : flag(f) { flag = true; }
	~SModalGuard() { flag = false; }
};

struct SDxLibRenderTarget
{
	SDxLibRenderTarget(int iGraphicHandle, bool toClear = true)
	{
		DxLib::SetDrawScreen(iGraphicHandle);
		if (toClear)DxLib::ClearDrawScreen();
	};
	~SDxLibRenderTarget()
	{
		DxLib::SetDrawScreen(DX_SCREEN_BACK);
	}
};

namespace
{
	std::wstring GetExecutableDirectory()
	{
		wchar_t exePath[MAX_PATH] = {};
		::GetModuleFileNameW(nullptr, exePath, MAX_PATH);
		wchar_t* pSlash = ::wcsrchr(exePath, L'\\');
		if (pSlash) *pSlash = L'\0';
		return exePath;
	}

	bool FileExistsW(const std::wstring& path)
	{
		DWORD attr = ::GetFileAttributesW(path.c_str());
		return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
	}

	struct SPlaybackSnapshot
	{
		bool hadLoaded = false;
		float skeletonScale = 1.f;
		float timeScale = 1.f;
	};

	SPlaybackSnapshot CapturePlaybackSnapshot(ISpinePlayer* player)
	{
		SPlaybackSnapshot snapshot{};
		if (player == nullptr)
			return snapshot;

		snapshot.hadLoaded = player->hasSpineBeenLoaded();
		if (snapshot.hadLoaded)
		{
			snapshot.skeletonScale = player->getSkeletonScale();
			snapshot.timeScale = player->getTimeScale();
		}
		return snapshot;
	}

	void RestorePlaybackSnapshot(ISpinePlayer* player, const SPlaybackSnapshot& snapshot, bool hasLoaded)
	{
		if (player == nullptr || !hasLoaded)
			return;

		if (snapshot.hadLoaded)
		{
			player->setSkeletonScale(snapshot.skeletonScale);
			player->setTimeScale(snapshot.timeScale);
		}
		else
		{
			player->setSkeletonScale(1.f);
		}
	}

	std::wstring MakeAnimationTimeTag(float animationTime)
	{
		wchar_t buffer[16]{};
		swprintf_s(buffer, L"_%.3f", animationTime);
		return buffer;
	}

	std::wstring MakeRecordedFrameName(float animationTime, bool isQueueRecording, size_t queueIndex)
	{
		if (!isQueueRecording)
			return MakeAnimationTimeTag(animationTime);

		wchar_t queuePrefix[32]{};
		::swprintf_s(queuePrefix, L"%03zu", queueIndex);
		return std::wstring(queuePrefix) + MakeAnimationTimeTag(animationTime);
	}
}


CMainWindow::CMainWindow()
{

}

CMainWindow::~CMainWindow()
{

}

bool CMainWindow::Create(HINSTANCE hInstance, const wchar_t* pwzWindowName)
{
	WNDCLASSEXW wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = 0;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = nullptr;   
	wcex.lpszClassName = m_swzClassName;

	if (::RegisterClassExW(&wcex))
	{
		m_hInstance = hInstance;
		const wchar_t* windowName = pwzWindowName == nullptr ? m_swzDefaultWindowName : pwzWindowName;

		int iDesktopWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int iDesktopHeight = ::GetSystemMetrics(SM_CYSCREEN);
		int iWindowWidth = iDesktopWidth * 9 / 10;
		int iWindowHeight = iDesktopHeight * 9 / 10;
		int iPosX = (iDesktopWidth - iWindowWidth) / 2;
		int iPosY = (iDesktopHeight - iWindowHeight) / 4;

		DWORD dwStyle = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
		m_hWnd = ::CreateWindowExW(WS_EX_ACCEPTFILES, m_swzClassName, windowName, dwStyle,
			iPosX, iPosY, iWindowWidth, iWindowHeight, nullptr, nullptr, hInstance, this);
		if (m_hWnd != nullptr)
		{
			return true;
		}
	}

	return false;
}

int CMainWindow::MessageLoop()
{
	MSG msg{};

	for (; msg.message != WM_QUIT;)
	{


		if (::IsIconic(m_hWnd))
		{
			BOOL iRet = ::GetMessageW(&msg, nullptr, 0, 0);
			if (!iRet) break;
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
			continue;
		}

		BOOL iRet = ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);
		if (iRet)
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}

		if (m_hasProcessedWmPaint)
		{
			m_hasProcessedWmPaint = false;
			continue;
		}

		Tick();
	}

	return static_cast<int>(msg.wParam);
}

LRESULT CMainWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMainWindow* pThis = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = reinterpret_cast<CMainWindow*>(pCreateStruct->lpCreateParams);
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
	}

	pThis = reinterpret_cast<CMainWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (pThis != nullptr)
	{
		return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CMainWindow::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))return 1;

	switch (uMsg)
	{
	case WM_NCCALCSIZE:
		if (wParam && !::IsZoomed(hWnd)) return 0;
		break;
	case WM_NCHITTEST:
	{
		POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		::ScreenToClient(hWnd, &pt);
		RECT rc{};
		::GetClientRect(hWnd, &rc);
		const int border = ::GetSystemMetrics(SM_CYSIZEFRAME);

		if (pt.y < border && pt.x < border) return HTTOPLEFT;
		if (pt.y < border && pt.x >= rc.right - border) return HTTOPRIGHT;
		if (pt.y >= rc.bottom - border && pt.x < border) return HTBOTTOMLEFT;
		if (pt.y >= rc.bottom - border && pt.x >= rc.right - border) return HTBOTTOMRIGHT;

		if (pt.y < border) return HTTOP;
		if (pt.y >= rc.bottom - border) return HTBOTTOM;
		if (pt.x < border) return HTLEFT;
		if (pt.x >= rc.right - border) return HTRIGHT;

		if (pt.y < (LONG)custom_titlebar::TitleBarHeight() && pt.x < (rc.right - (LONG)custom_titlebar::BtnAreaWidth()))
			return HTCAPTION;
		return HTCLIENT;
	}
	case WM_CREATE:
		return OnCreate(hWnd);
	case WM_DESTROY:
		return OnDestroy();
	case WM_SIZE:
		return OnSize(wParam, lParam);
	case WM_EXITSIZEMOVE:
		ApplyResize();
		return 0;
	case WM_CLOSE:
		return OnClose();
	case WM_PAINT:
		return OnPaint();
	case WM_ERASEBKGND:
		return 1;
	case WM_ACTIVATE:
		
		
		
		if (LOWORD(wParam) != WA_INACTIVE)
			::InvalidateRect(hWnd, nullptr, FALSE);
		break;
	case WM_NCACTIVATE:
		
		
		
		return ::DefWindowProcW(hWnd, uMsg, wParam, -1);
	case WM_KEYDOWN:
	{
		if (ImGui::GetIO().WantCaptureKeyboard) break;

		if (wParam == VK_F11)
		{
			ToggleFullscreen();
			return 0;
		}

		if (wParam == VK_UP || wParam == VK_DOWN || wParam == VK_LEFT || wParam == VK_RIGHT)
			return OnKeyDown(wParam, lParam);
		break;
	}
	case WM_KEYUP:
		return OnKeyUp(wParam, lParam);
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam, lParam);
	case WM_MOUSEWHEEL:
		return OnMouseWheel(wParam, lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam, lParam);
	case WM_RBUTTONUP:
		return OnRButtonUp(wParam, lParam);
	case WM_MBUTTONUP:
		return OnMButtonUp(wParam, lParam);
	case WM_DROPFILES:
		return OnDropFiles(wParam, lParam);
	default:
		break;
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CMainWindow::OnCreate(HWND hWnd)
{
	m_hWnd = hWnd;
	m_spineToolDatum.titleBar.hWnd = hWnd;


	InitialiseMenuBar();
	UpdateMenuItemState();

	m_spineToolDatum.pSpinePlayer = &m_dxLibSpinePlayer;
	m_spineToolDatum.onOpenFiles = [this]() { MenuOnOpenFiles(); };
	m_spineToolDatum.onOpenFolder = [this]() { MenuOnOpenFolder(); };
	m_spineToolDatum.onExtensionSetting = [this]() { MenuOnExtensionSetting(); };
	m_spineToolDatum.onShowTool = [this]() { MenuOnShowToolDialogue(); };
	m_spineToolDatum.onFontSetting = [this]() { MenuOnFont(); };
	m_spineToolDatum.onWindowSettings = [this]() { MenuOnMakeWindowTransparent(); };
	m_spineToolDatum.onAllowDragResize = [this]() { MenuOnAllowDraggedResizing(); };
	m_spineToolDatum.onReverseZoom = [this]() { MenuOnReverseZoomDirection(); };
	m_spineToolDatum.onFitToManualSize = [this]() { MenuOnFiToManualSize(); };
	m_spineToolDatum.onFitToDefaultSize = [this]() { MenuOnFitToDefaultSize(); };
	m_spineToolDatum.onPickFolder = [this]() {
		if (m_showFavoriteSkelFiles)
		{
			m_showFavoriteSkelFiles = false;
			FavoriteSyncDatum();
		}
		SModalGuard guard(m_isModalOpen);
		::LockWindowUpdate(m_hWnd);
		std::wstring folder = m_fileDialogs.PickFolder(m_hWnd);
		::LockWindowUpdate(nullptr);
		::InvalidateRect(m_hWnd, nullptr, FALSE);
		if (folder.empty()) return;
		ScanFolderIntoList(folder);
	};
	m_spineToolDatum.onPlayFile = [this](const std::wstring& path) {
		auto action = [this, path]() {
		m_spineToolDatum.currentSkelFile = path;

		std::string stem = ExtractStemUtf8(path);
		m_spineToolDatum.currentFileName = stem;
		m_spineToolDatum.titleBar.subTitle = stem;
		LoadFilesFromPaths({ path });
		};
		if (QueueLoadedSpineReplaceAction(action)) return;
		action();
	};
	m_spineToolDatum.onAddSpineFromFile = [this](const std::wstring& path) {
		AddSpineFromPath(path);
	};
	m_spineToolDatum.onSelectLoadedSpine = [this](size_t index) {
		SelectLoadedSpine(index);
	};
	m_spineToolDatum.onToggleLoadedSpineVisibility = [this](size_t index) {
		ToggleLoadedSpineVisibility(index);
	};
	m_spineToolDatum.onMoveLoadedSpineUp = [this](size_t index) {
		MoveLoadedSpineUp(index);
	};
	m_spineToolDatum.onMoveLoadedSpineDown = [this](size_t index) {
		MoveLoadedSpineDown(index);
	};
	m_spineToolDatum.onConfirmLoadedSpineReplace = [this]() {
		ConfirmLoadedSpineReplaceAction();
	};
	m_spineToolDatum.onCancelLoadedSpineReplace = [this]() {
		CancelLoadedSpineReplaceAction();
	};
	m_spineToolDatum.onToggleFavoriteView = [this]() {
		m_showFavoriteSkelFiles = !m_showFavoriteSkelFiles;
		FavoriteSyncDatum();
	};
	m_spineToolDatum.onToggleFavoriteFile = [this](const std::wstring& path) {
		FavoriteToggleFile(path);
	};
	m_spineToolDatum.onOpenFileFolder = [this](const std::wstring& path) {
		size_t slash = path.find_last_of(L"\\/");
		if (slash == std::wstring::npos) return;
		std::wstring folderPath = path.substr(0, slash);
		if (folderPath.empty()) return;
		::ShellExecuteW(m_hWnd, L"open", folderPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	};
	m_spineToolDatum.onSnapPng = [this]() { MenuOnSaveAsPng(); };
	m_spineToolDatum.onSnapJpg = [this]() { MenuOnSaveAsJpg(); };
	m_spineToolDatum.onExportGif = [this]() {
		PrepareQueueRecording(PopupMenu::kExportAsGif);
		MenuOnStartRecording(PopupMenu::kExportAsGif);
	};
	m_spineToolDatum.onExportVideo = [this]() {
		PrepareQueueRecording(PopupMenu::kExportAsVideo);
		MenuOnStartRecording(PopupMenu::kExportAsVideo);
	};
	m_spineToolDatum.onExportWebm = [this]() {
		PrepareQueueRecording(PopupMenu::kExportAsWebm);
		MenuOnExportWebm();
	};
	m_spineToolDatum.onExportPngs = [this]() {
		PrepareQueueRecording(PopupMenu::kExportAsPngs);
		MenuOnStartRecording(PopupMenu::kExportAsPngs);
	};
	m_spineToolDatum.onExportJpgs = [this]() {
		PrepareQueueRecording(PopupMenu::kExportAsJpgs);
		MenuOnStartRecording(PopupMenu::kExportAsJpgs);
	};
	m_spineToolDatum.onEndRecording = [this]() { MenuOnEndRecording(); };
	m_spineToolDatum.isRecording = [this]() {
		return m_dxLibRecorder.GetState() == CDxLibRecorder::EState::UnderRecording;
	};


	m_spineToolDatum.pAnimQueue = &m_animQueue;
	m_spineToolDatum.onQueuePlay = [this]() { StartQueuePlay(); };
	m_spineToolDatum.onQueueStop = [this]() { StopQueuePlay(); };
	m_spineToolDatum.isQueuePlaying = [this]() { return m_isQueuePlaying; };
	m_spineToolDatum.getQueueIndex = [this]() { return m_queueIndex; };
	m_spineToolDatum.onPmaChanged = [this](bool pma) { m_userPma = pma; m_userPmaSet = true; };
	m_spineToolDatum.onLoadBackground = [this]() { MenuOnLoadBackground(); };
	m_spineToolDatum.onLoadAudio = [this]() { MenuOnLoadAudio(); };
	m_spineToolDatum.onPlayAudio = [this]() { MenuOnPlayAudio(); };
	m_spineToolDatum.onStopAudio = [this]() { MenuOnStopAudio(); };
	m_spineToolDatum.onToggleLoopAudio = [this]() { MenuOnToggleLoopAudio(); };
	m_spineToolDatum.onAudioPrev = [this]() { MenuOnAudioPrev(); };
	m_spineToolDatum.onAudioNext = [this]() { MenuOnAudioNext(); };
	m_spineToolDatum.onToggleAudioAuto = [this]() { MenuOnToggleAudioAuto(); };

	m_favoriteCachePath = GetExecutableDirectory() + L"\\spine_favorites.txt";
	FavoriteLoadCache();
	FavoriteSyncDatum();
	m_spineToolDatum.onSetAudioVolume = [this](float vol) {
		if (m_pAudioPlayer) m_pAudioPlayer->SetCurrentVolume(static_cast<double>(vol));
	};
	m_spineToolDatum.onProButtonClicked = [this]() {
		::ShellExecuteW(m_hWnd, L"open", L"https://yihkllo.com", nullptr, nullptr, SW_SHOWNORMAL);
	};


	m_spineToolDatum.onSetRenderBgColor = [](int r, int g, int b)
	{
		DxLib::SetBackgroundColor(r, g, b);
	};


	m_spineToolDatum.onLoadTitleBg = [this]()
	{
		SModalGuard guard(m_isModalOpen);
		shell_dialogs::FileDialogRequest titleBgRequest{};
		titleBgRequest.title = L"Select title background image";
		titleBgRequest.primaryFilter = { L"Image", L"*.png;*.jpg;*.jpeg;*.bmp" };
		std::wstring path = m_fileDialogs.PickSingleFile(titleBgRequest, m_hWnd);
		if (!path.empty())
		{
			if (m_spineToolDatum.titleBar.hBgDxLib != -1)
				DxLib::DeleteGraph(m_spineToolDatum.titleBar.hBgDxLib);
			m_spineToolDatum.titleBar.hBgDxLib = DxLib::LoadGraph(path.c_str());
		}
	};


	DragAcceptFiles(m_hWnd, TRUE);


	::EnumChildWindows(m_hWnd, [](HWND hwndChild, LPARAM lParam) -> BOOL {
		DragAcceptFiles(hwndChild, TRUE);
		return TRUE;
	}, 0);


	RECT rc;
	::GetClientRect(m_hWnd, &rc);
	m_pendingClientWidth = rc.right - rc.left;
	m_pendingClientHeight = rc.bottom - rc.top;
	m_hasPendingResize = true;

	return 0;
}

LRESULT CMainWindow::OnDestroy()
{
	::PostQuitMessage(0);

	return 0;
}

LRESULT CMainWindow::OnClose()
{
	::DestroyWindow(m_hWnd);
	::UnregisterClassW(m_swzClassName, m_hInstance);

	return 0;
}

LRESULT CMainWindow::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_hWnd, &ps);

	if (m_isModalOpen)
	{
		::EndPaint(m_hWnd, &ps);
		return 0;
	}

	::EndPaint(m_hWnd, &ps);

	if (m_dxLibSpinePlayer.ActiveRuntime()->hasSpineBeenLoaded())
	{
		Tick();
		m_hasProcessedWmPaint = true;
	}

	return 0;
}

LRESULT CMainWindow::OnSize(WPARAM wParam, LPARAM lParam)
{
	if (wParam == SIZE_MINIMIZED) return 0;

	m_pendingClientWidth = LOWORD(lParam);
	m_pendingClientHeight = HIWORD(lParam);
	m_hasPendingResize = true;


	if (wParam == SIZE_MAXIMIZED)
	{
		ApplyResize();
	}

	
	
	
	DragAcceptFiles(m_hWnd, TRUE);
	ChangeWindowMessageFilterEx(m_hWnd, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
	ChangeWindowMessageFilterEx(m_hWnd, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
	ChangeWindowMessageFilterEx(m_hWnd, 0x0049, MSGFLT_ALLOW, nullptr);

	return 0;
}

void CMainWindow::InitializeGraphics()
{
	RECT rc;
	::GetClientRect(m_hWnd, &rc);
	m_pendingClientWidth = rc.right - rc.left;
	m_pendingClientHeight = rc.bottom - rc.top;
	m_hasPendingResize = true;
	ApplyResize();
}

void CMainWindow::ApplyResize()
{
	if (!m_hasPendingResize) return;
	m_hasPendingResize = false;

	int iDesktopWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int iDesktopHeight = ::GetSystemMetrics(SM_CYSCREEN);

	int iGraphWidth = m_pendingClientWidth < iDesktopWidth ? m_pendingClientWidth : iDesktopWidth;
	int iGraphHeight = m_pendingClientHeight < iDesktopHeight ? m_pendingClientHeight : iDesktopHeight;
	if (iGraphWidth <= 0 || iGraphHeight <= 0) return;

	DxLib::SetGraphMode(iGraphWidth, iGraphHeight, 32);
	m_spineRenderTexture.Reset(DxLib::MakeScreen(iGraphWidth, iGraphHeight, 1));
	::InvalidateRect(m_hWnd, nullptr, FALSE);

	
	
	DragAcceptFiles(m_hWnd, TRUE);
	ChangeWindowMessageFilterEx(m_hWnd, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
	ChangeWindowMessageFilterEx(m_hWnd, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
	ChangeWindowMessageFilterEx(m_hWnd, 0x0049, MSGFLT_ALLOW, nullptr);
	::EnumChildWindows(m_hWnd, [](HWND hwndChild, LPARAM lParam) -> BOOL {
		DragAcceptFiles(hwndChild, TRUE);
		ChangeWindowMessageFilterEx(hwndChild, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
		ChangeWindowMessageFilterEx(hwndChild, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
		ChangeWindowMessageFilterEx(hwndChild, 0x0049, MSGFLT_ALLOW, nullptr);
		return TRUE;
	}, 0);
}

LRESULT CMainWindow::OnKeyDown(WPARAM wParam, LPARAM lParam)
{

	if (wParam == VK_UP)   { KeyUpOnForeFile(-1); return 0; }
	if (wParam == VK_DOWN) { KeyUpOnForeFile(+1); return 0; }


	if (wParam == VK_LEFT)  { m_dxLibSpinePlayer.ActiveRuntime()->shiftAnimationBack(); return 0; }
	if (wParam == VK_RIGHT) { m_dxLibSpinePlayer.ActiveRuntime()->shiftAnimation();     return 0; }

	return 0;
}

LRESULT CMainWindow::OnKeyUp(WPARAM wParam, LPARAM lParam)
{

	if (!ImGui::GetIO().WantCaptureKeyboard)
	{

		if (wParam == VK_UP || wParam == VK_DOWN)
		{
			if (!m_spineToolDatum.currentSkelFile.empty())
				LoadFilesFromPaths({ m_spineToolDatum.currentSkelFile });
			return 0;
		}
	}

	if (ImGui::GetIO().WantCaptureKeyboard)return 0;

	switch (wParam)
	{
	default:
		break;
	}
	return 0;
}

LRESULT CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	int wmKind = LOWORD(lParam);
	if (wmKind == 0)
	{

		switch (wmId)
		{
		case Menu::kOpenFiles:
			MenuOnOpenFiles();
			break;
		case Menu::kOpenFolder:
			MenuOnOpenFolder();
			break;
		case Menu::kExtensionSetting:
			MenuOnExtensionSetting();
			break;
		case Menu::kShowToolDialogue:
			MenuOnShowToolDialogue();
			break;
		case Menu::kFontSetting:
			MenuOnFont();
			break;
		case Menu::kSeeThroughImage:
			MenuOnMakeWindowTransparent();
			break;
		case Menu::kAllowDraggedResizing:
			MenuOnAllowDraggedResizing();
			break;
		case Menu::kReverseZoomDirection:
			MenuOnReverseZoomDirection();
			break;
		case Menu::kFitToManualSize:
			MenuOnFiToManualSize();
			break;
		case Menu::kFitToDefaultSize:
			MenuOnFitToDefaultSize();
			break;
		default: break;
		}
	}
	else
	{

	}

	return 0;
}

LRESULT CMainWindow::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	if (m_spineToolDatum.isSettingWindowOpen) return 0;


	if (ImGui::GetIO().WantCaptureMouse) return 0;
	POINT pt{};
	::GetCursorPos(&pt);
	::ScreenToClient(m_hWnd, &pt);
	if (pt.x < 650) return 0;

	WORD usKey = LOWORD(wParam);
	if ((usKey & MK_LBUTTON) && !(usKey & MK_RBUTTON))
	{
		if (m_wasLeftCombinated)return 0;

		POINT pt{};
		::GetCursorPos(&pt);

		if (m_hasLeftBeenDragged)
		{
			int deltaX = pt.x - m_lastCursorPos.x;
			int deltaY = pt.y - m_lastCursorPos.y;

			if (::GetKeyState(VK_CONTROL) & 0x8000)
			{

				if (!m_bgTexture.Empty())
				{
					m_bgOffsetX += deltaX;
					m_bgOffsetY += deltaY;
				}
			}
			else
			{
				m_dxLibSpinePlayer.ActiveRuntime()->addOffset(-deltaX, -deltaY);
			}
		}

		m_hasLeftBeenDragged = true;
		m_lastCursorPos = pt;
	}
	else if ((usKey & MK_LBUTTON) && (usKey & MK_RBUTTON))
	{
		if (m_wasLeftCombinated || m_wasRightCombinated)return 0;

		POINT pt{};
		::GetCursorPos(&pt);
		if (m_hasRightBeenDragged)
		{
			int deltaX = pt.x - m_lastCursorPos.x;
			int deltaY = pt.y - m_lastCursorPos.y;

			RECT windowRect{};
			::GetWindowRect(m_hWnd, &windowRect);
			::SetWindowPos(m_hWnd, nullptr, windowRect.left + deltaX, windowRect.top + deltaY, 0, 0, SWP_NOSIZE);
		}

		m_lastCursorPos = pt;
		m_hasRightBeenDragged = true;
	}

	return 0;
}

LRESULT CMainWindow::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
	if (m_spineToolDatum.isSettingWindowOpen) return 0;

	POINT pt{};
	::GetCursorPos(&pt);
	::ScreenToClient(m_hWnd, &pt);
	if (pt.x < 650) return 0;

	WORD usKey = LOWORD(wParam);
	const short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	EWheelAction wheelAction = EWheelAction::None;

	if ((usKey & MK_LBUTTON) && !(usKey & MK_CONTROL))
		wheelAction = EWheelAction::TimeScale;
	else if (::GetKeyState(VK_CONTROL) & 0x8000)
		wheelAction = EWheelAction::BackgroundZoom;
	else if (m_dxLibSpinePlayer.ActiveRuntime()->hasSpineBeenLoaded())
		wheelAction = EWheelAction::SkeletonZoom;
	else
		return 0;

	if (m_lastWheelAction != wheelAction)
	{
		m_mouseWheelDeltaRemainder = 0;
		m_lastWheelAction = wheelAction;
	}

	m_mouseWheelDeltaRemainder += wheelDelta;
	const int wheelSteps = m_mouseWheelDeltaRemainder / WHEEL_DELTA;
	m_mouseWheelDeltaRemainder %= WHEEL_DELTA;
	if (wheelSteps == 0) return 0;

	if ((usKey & MK_LBUTTON) && !(usKey & MK_CONTROL))
	{
		static constexpr float kTimeScaleDelta = 0.05f;
		const float scrollSign = wheelSteps > 0 ? -1.f : 1.f;

		float timeScale = m_dxLibSpinePlayer.ActiveRuntime()->getTimeScale()
			+ kTimeScaleDelta * static_cast<float>(std::abs(wheelSteps)) * scrollSign;
		timeScale = (std::max)(0.f, (std::min)(5.f, timeScale));
		m_dxLibSpinePlayer.ActiveRuntime()->setTimeScale(timeScale);

		m_wasLeftCombinated = true;
	}
	else
	{
		if (::GetKeyState(VK_CONTROL) & 0x8000)
		{

			if (!m_bgTexture.Empty())
			{
				const float scrollSign = wheelSteps > 0 ? 1.f : -1.f;

				int bgW = 0, bgH = 0;
				DxLib::GetGraphSize(m_bgTexture.Get(), &bgW, &bgH);


				float centerX = m_bgOffsetX + bgW * m_bgScale * 0.5f;
				float centerY = m_bgOffsetY + bgH * m_bgScale * 0.5f;

				float newScale = m_bgScale;
				for (int i = 0; i < std::abs(wheelSteps); ++i)
					newScale = (std::max)(0.05f, newScale * (1.f + 0.05f * scrollSign));


				m_bgOffsetX = centerX - bgW * newScale * 0.5f;
				m_bgOffsetY = centerY - bgH * newScale * 0.5f;
				m_bgScale = newScale;
			}
		}
		else if (m_dxLibSpinePlayer.ActiveRuntime()->hasSpineBeenLoaded())
		{
			static constexpr float kMinScale = 0.1f;
			static constexpr float kMaxScale = 5.f;
			const bool zoomIn = (wheelSteps > 0) ^ m_isZoomDirectionReversed;
			const float scrollSign = zoomIn ? 1.f : -1.f;
			const bool scaleAllSpines = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;

			const float currentSkeletonScale = m_dxLibSpinePlayer.ActiveRuntime()->getSkeletonScale();
			float skeletonScale = currentSkeletonScale;
			for (int i = 0; i < std::abs(wheelSteps); ++i)
				skeletonScale *= (1.f + 0.05f * scrollSign);
			skeletonScale = (std::max)(kMinScale, (std::min)(kMaxScale, skeletonScale));
			if (scaleAllSpines)
			{
				const float scaleFactor = currentSkeletonScale > 0.f ? (skeletonScale / currentSkeletonScale) : 1.f;
				const size_t spineCount = m_dxLibSpinePlayer.ActiveRuntime()->getNumberOfSpines();
				for (size_t i = 0; i < spineCount; ++i)
				{
					if (!m_dxLibSpinePlayer.ActiveRuntime()->isSpineVisible(i))
						continue;
					float spineScale = m_dxLibSpinePlayer.ActiveRuntime()->getSkeletonScaleAt(i);
					spineScale *= scaleFactor;
					spineScale = (std::max)(kMinScale, (std::min)(kMaxScale, spineScale));
					m_dxLibSpinePlayer.ActiveRuntime()->setSkeletonScaleAt(i, spineScale);
				}
				m_winclock.Restart();
			}
			else
			{
				m_dxLibSpinePlayer.ActiveRuntime()->setSkeletonScale(skeletonScale);
			}
		}
	}

	return 0;
}

LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	POINT pt{};
	::GetCursorPos(&pt);
	::ScreenToClient(m_hWnd, &pt);
	const int kNormalThreshold = 650;
	int threshold = kNormalThreshold;
	if (pt.x < threshold) return 0;

	::GetCursorPos(&m_lastCursorPos);
	m_hasLeftBeenDragged = false;
	m_wasLeftPressed = true;

	if (ImGui::GetIO().WantCaptureMouse)return 0;

	return 0;
}

LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (ImGui::GetIO().WantCaptureMouse)return 0;

	if (m_wasLeftCombinated || m_hasLeftBeenDragged)
	{
		m_hasLeftBeenDragged = false;
		m_wasLeftCombinated = false;
		m_wasLeftPressed = false;

		return 0;
	}

	WORD usKey = LOWORD(wParam);
	if (usKey == 0 && m_wasLeftPressed)
	{
		POINT pt{};
		::GetCursorPos(&pt);
		int iX = m_lastCursorPos.x - pt.x;
		int iY = m_lastCursorPos.y - pt.y;

		if (iX == 0 && iY == 0)
		{
			{
				const auto& recorderState = m_dxLibRecorder.GetState();
				if ((recorderState == CDxLibRecorder::EState::Idle || !m_spineToolDatum.toExportPerAnim)
					&& !m_spineToolDatum.mouseHoverEnabled)
				{
					m_dxLibSpinePlayer.ActiveRuntime()->shiftAnimation();
				}
			}
		}
	}

	m_wasLeftPressed = false;

	return 0;
}

LRESULT CMainWindow::OnRButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (ImGui::GetIO().WantCaptureMouse)return 0;

	if (m_wasRightCombinated || m_hasRightBeenDragged)
	{
		m_wasRightCombinated = false;
		m_hasRightBeenDragged = false;

		return 0;
	}

	return 0;
}

LRESULT CMainWindow::OnMButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (ImGui::GetIO().WantCaptureMouse)return 0;

	WORD usKey = LOWORD(wParam);

	if (usKey == 0)
	{
		if (m_dxLibSpinePlayer.ActiveRuntime()->hasSpineBeenLoaded() && m_dxLibRecorder.GetState() != CDxLibRecorder::EState::UnderRecording)
		{
			m_dxLibSpinePlayer.ActiveRuntime()->resetScale();
			ResizeWindow();
		}
	}

	return 0;
}

LRESULT CMainWindow::OnDropFiles(WPARAM wParam, LPARAM lParam)
{
	HDROP hDrop = (HDROP)wParam;
	UINT fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);

	if (fileCount > 0)
	{
		wchar_t filePath[MAX_PATH];
		DragQueryFileW(hDrop, 0, filePath, MAX_PATH);

		std::wstring path(filePath);


		DWORD attr = ::GetFileAttributesW(path.c_str());
		if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY))
		{
			ScanFolderIntoList(path);
			DragFinish(hDrop);
			return 0;
		}

		std::wstring ext = path.substr(path.find_last_of(L'.'));


		if (ext == L".json" || ext == L".skel" || ext == L".bin")
		{
			std::vector<std::wstring> skelFiles = { path };
			LoadFilesFromPaths(skelFiles);
		}
	}

	DragFinish(hDrop);
	return 0;
}

void CMainWindow::Tick()
{
	if (m_isModalOpen) return;
	if (m_isLoading) return;


	if (m_isWebmEncoding && m_hFfmpegProcess != nullptr)
	{
		DWORD dwExit = STILL_ACTIVE;
		if (::GetExitCodeProcess(m_hFfmpegProcess, &dwExit) && dwExit != STILL_ACTIVE)
		{
			::CloseHandle(m_hFfmpegProcess);
			m_hFfmpegProcess = nullptr;
			m_isWebmEncoding = false;
			m_spineToolDatum.recordFramesCaptured = 0;
			m_spineToolDatum.recordFramesTotal = 0;

		}
	}
	m_spineToolDatum.isWebmEncoding = m_isWebmEncoding;


	if (m_isAudioAuto && m_spineToolDatum.isAudioPlaying && m_pAudioPlayer && m_pAudioPlayer->IsEnded())
	{
		MenuOnAudioNext();
	}

	if (m_hasPendingResize)
	{
		ApplyResize();
	}


	if (!m_hasDragDropEnabled)
	{
		m_hasDragDropEnabled = true;
		DragAcceptFiles(m_hWnd, TRUE);


		if (m_spineToolDatum.titleBar.hIconDxLib == -1)
		{
			wchar_t exePath[MAX_PATH]{};
			::GetModuleFileNameW(nullptr, exePath, MAX_PATH);
			wchar_t* pSlash = ::wcsrchr(exePath, L'\\');
			if (pSlash) { *(pSlash + 1) = L'\0'; }


			{
				int len = ::WideCharToMultiByte(CP_UTF8, 0, exePath, -1, nullptr, 0, nullptr, nullptr);
				std::string exeDir(len > 0 ? len - 1 : 0, '\0');
				if (len > 0) ::WideCharToMultiByte(CP_UTF8, 0, exePath, -1, &exeDir[0], len, nullptr, nullptr);
				i18n::init(exeDir);
			}

			::wcscat_s(exePath, L"app.png");
			int hIcon = DxLib::LoadGraph(exePath);
			if (hIcon != -1) m_spineToolDatum.titleBar.hIconDxLib = hIcon;
		}

		if (m_spineToolDatum.titleBar.customFont == nullptr)
		{
			const float sc = custom_titlebar::GetScale();
			ImGuiIO& io = ImGui::GetIO();

			wchar_t winDir[MAX_PATH]{};
			::GetWindowsDirectoryW(winDir, MAX_PATH);
			std::wstring fontsDir = winDir;
			if (!fontsDir.empty() && fontsDir.back() != L'\\') fontsDir += L'\\';
			fontsDir += L"Fonts\\";
			const std::wstring bundledFontPath = path_util::GetBundledFontPath();
			m_spineToolDatum.titleBar.customFont = io.Fonts->AddFontFromFileTTF(
				win_text::NarrowUtf8(fontsDir + L"segoesc.ttf").c_str(), 40.0f * sc);
			const ImWchar* glyph = io.Fonts->GetGlyphRangesChineseFull();
			const std::wstring subTitleFontPath = bundledFontPath.empty() ? (fontsDir + L"malgun.ttf") : bundledFontPath;
			m_spineToolDatum.titleBar.subTitleFont = io.Fonts->AddFontFromFileTTF(
				win_text::NarrowUtf8(subTitleFontPath).c_str(), 26.7f * sc, nullptr, glyph);
			io.Fonts->Build();
		}

		ChangeWindowMessageFilterEx(m_hWnd, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
		ChangeWindowMessageFilterEx(m_hWnd, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
		ChangeWindowMessageFilterEx(m_hWnd, 0x0049  , MSGFLT_ALLOW, nullptr);
		::EnumChildWindows(m_hWnd, [](HWND hwndChild, LPARAM lParam) -> BOOL {
			DragAcceptFiles(hwndChild, TRUE);
			ChangeWindowMessageFilterEx(hwndChild, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
			ChangeWindowMessageFilterEx(hwndChild, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
			ChangeWindowMessageFilterEx(hwndChild, 0x0049, MSGFLT_ALLOW, nullptr);
			return TRUE;
		}, 0);
	}

	const auto& recorderState = m_dxLibRecorder.GetState();
	if (recorderState != CDxLibRecorder::EState::InitialisingVideoStream)
	{
		CUiCore::NewFrame();


		const int panelX = static_cast<int>(m_spineToolDatum.leftPanelEndX);
		const int playW = m_pendingClientWidth - panelX;
		if (playW > 0)
		{
			int texW = 0, texH = 0;
			if (!m_spineRenderTexture.Empty())
				DxLib::GetGraphSize(m_spineRenderTexture.Get(), &texW, &texH);
			if (texW != playW || texH != m_pendingClientHeight)
			{
				m_spineRenderTexture.Reset(DxLib::MakeScreen(playW, m_pendingClientHeight, 1));
			}
		}

		DxLib::ClearDrawScreen();

		if (!m_spineRenderTexture.Empty())
		{
			{
				SDxLibRenderTarget dxLibRenderTarget(m_spineRenderTexture.Get());
				int texW = 0, texH = 0;
				DxLib::GetGraphSize(m_spineRenderTexture.Get(), &texW, &texH);
				if (!m_bgTexture.Empty())
				{
					int bgW = 0, bgH = 0;
					DxLib::GetGraphSize(m_bgTexture.Get(), &bgW, &bgH);
					float drawW = bgW * m_bgScale;
					float drawH = bgH * m_bgScale;
					DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
					DxLib::DrawExtendGraphF(m_bgOffsetX, m_bgOffsetY, m_bgOffsetX + drawW, m_bgOffsetY + drawH, m_bgTexture.Get(), FALSE);
				}
				if (m_dxLibSpinePlayer.ActiveRuntime()->hasSpineBeenLoaded())
				{
					m_dxLibSpinePlayer.ActiveRuntime()->setRenderScreenSize(texW, texH);
					m_dxLibSpinePlayer.ActiveRuntime()->draw();
				}
			}
			DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
			DxLib::DrawGraph(panelX, 0, m_spineRenderTexture.Get(), TRUE);
		}

		if (m_dxLibSpinePlayer.ActiveRuntime()->hasSpineBeenLoaded())
		{
			if (recorderState == CDxLibRecorder::EState::UnderRecording)
				StepRecording();

			float fDelta = m_winclock.GetElapsedTime();
			if (m_dxLibRecorder.GetState() == CDxLibRecorder::EState::UnderRecording)
				fDelta = 1.f / m_dxLibRecorder.GetFps();

			m_dxLibSpinePlayer.ActiveRuntime()->update(fDelta);
			StepQueuePlay();
			m_winclock.Restart();
		}


		if (m_spineToolDatum.isWritingFrames)
		{
			int written = 0, total = 0;
			m_dxLibRecorder.GetWriteProgress(&written, &total);
			m_spineToolDatum.recordFramesWritten = written;
			if (m_dxLibRecorder.GetState() == CDxLibRecorder::EState::Idle)
			{

				m_spineToolDatum.isWritingFrames     = false;
				m_spineToolDatum.recordFramesCaptured = 0;
				m_spineToolDatum.recordFramesTotal    = 0;
				m_spineToolDatum.recordFramesWritten  = 0;


			}
		}

		ImGuiSpineParameterDialogue();

		CUiCore::Render();
		CUiCore::UpdateAndRenderViewPorts();

		DxLib::ScreenFlip();
	}
}

void CMainWindow::InitialiseMenuBar()
{

}

void CMainWindow::MenuOnOpenFiles()
{
	if (m_dxLibRecorder.GetState() != CDxLibRecorder::EState::Idle)return;

	SModalGuard guard(m_isModalOpen);
	::LockWindowUpdate(m_hWnd);
	shell_dialogs::FileDialogRequest skeletonRequest{};
	skeletonRequest.title = L"Select skeleton files";
	skeletonRequest.primaryFilter = { L"skeleton files", L"*.json;*.skel;*.bin" };
	skeletonRequest.allowAnyFile = true;
	std::vector<std::wstring> skelFilePaths = m_fileDialogs.PickMultipleFiles(skeletonRequest, m_hWnd);
	::LockWindowUpdate(nullptr);
	::InvalidateRect(m_hWnd, nullptr, FALSE);
	if (skelFilePaths.empty())return;

	std::sort(skelFilePaths.begin(), skelFilePaths.end());
	LoadFilesFromPaths(skelFilePaths);
}

void CMainWindow::LoadFilesFromPaths(const std::vector<std::wstring>& skelFilePaths)
{
	if (skelFilePaths.empty())return;
	if (QueueLoadedSpineReplaceAction([this, skelFilePaths]() { LoadFilesFromPaths(skelFilePaths); })) return;

	ClearFolderPathList();

	std::vector<std::wstring> atlasFilePaths;
	std::vector<std::string> atlasData;
	std::vector<std::string> textureDirectories;
	std::vector<std::string> skelData;

	for (const auto& skelPath : skelFilePaths)
	{
		size_t lastSlash = skelPath.find_last_of(L"\\/");
		size_t lastDot = skelPath.find_last_of(L'.');

		std::wstring dir = (lastSlash != std::wstring::npos) ? skelPath.substr(0, lastSlash + 1) : L"";
		std::wstring baseName = skelPath.substr(lastSlash + 1, lastDot - lastSlash - 1);

		std::wstring atlasPath = dir + baseName + L".atlas";
		if (!PathFileExistsW(atlasPath.c_str()))
		{
			atlasPath = dir + baseName + L".atlas.txt";
			if (!PathFileExistsW(atlasPath.c_str()))
			{
				m_fileDialogs.ShowOwnerError((L"Atlas file not found for: " + baseName).c_str(), m_hWnd);
				return;
			}
		}

		atlasFilePaths.push_back(atlasPath);
		atlasData.emplace_back(path_util::LoadFileAsString(atlasPath.c_str()));
		textureDirectories.push_back(win_text::NarrowUtf8(dir));
		skelData.emplace_back(path_util::LoadFileAsString(skelPath.c_str()));
	}

	const std::wstring& selectedAtlasPath = atlasFilePaths[0];
	size_t nPos1 = selectedAtlasPath.find_last_of(L"\\/");
	if (nPos1 == std::wstring::npos)nPos1 = 0;
	else ++nPos1;

	size_t nPos2 = selectedAtlasPath.find(L".", nPos1);
	if (nPos2 == std::wstring::npos)nPos2 = selectedAtlasPath.size();

	std::wstring windowName = selectedAtlasPath.substr(nPos1, nPos2 - nPos1);
	LoadSpinesFromMemory(atlasData, textureDirectories, skelData, windowName.c_str());
	ResetLoadedSpineEntries(skelFilePaths);
	SelectLoadedSpine(0);
}

bool CMainWindow::ConfirmExitLoadedSpineMode()
{
	return !IsLoadedSpineReplaceConfirmationNeeded();
}

bool CMainWindow::IsLoadedSpineReplaceConfirmationNeeded() const
{
	if (m_bypassLoadedSpineReplaceConfirm) return false;
	if (!m_showLoadedSpinePanel) return false;
	return m_loadedSpineEntries.size() > 1;
}

bool CMainWindow::QueueLoadedSpineReplaceAction(std::function<void()> action)
{
	if (!IsLoadedSpineReplaceConfirmationNeeded()) return false;
	m_pendingLoadedSpineReplaceAction = std::move(action);
	m_spineToolDatum.loadedSpineReplaceConfirmMessage =
		"Loading in this way will exit multi-spine mode and replace the currently loaded spines.";
	m_spineToolDatum.showLoadedSpineReplaceConfirm = true;
	return true;
}

void CMainWindow::ConfirmLoadedSpineReplaceAction()
{
	m_spineToolDatum.showLoadedSpineReplaceConfirm = false;
	if (!m_pendingLoadedSpineReplaceAction) return;
	auto action = std::move(m_pendingLoadedSpineReplaceAction);
	m_pendingLoadedSpineReplaceAction = {};
	m_bypassLoadedSpineReplaceConfirm = true;
	action();
	m_bypassLoadedSpineReplaceConfirm = false;
}

void CMainWindow::CancelLoadedSpineReplaceAction()
{
	m_spineToolDatum.showLoadedSpineReplaceConfirm = false;
	m_pendingLoadedSpineReplaceAction = {};
}

void CMainWindow::ScanFolderIntoList(const std::wstring& folderPath)
{
	m_spineToolDatum.skelFileList.clear();
	path_util::ScanSkeletonFilesRecursive(folderPath, m_spineToolDatum.skelFileList);
	std::sort(m_spineToolDatum.skelFileList.begin(), m_spineToolDatum.skelFileList.end());
}

void CMainWindow::FavoriteLoadCache()
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

void CMainWindow::FavoriteSaveCache() const
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

void CMainWindow::FavoriteSyncDatum()
{
	m_spineToolDatum.favoriteSkelFileList.clear();
	for (const std::wstring& path : m_favoriteSkelFiles)
	{
		if (FileExistsW(path))
			m_spineToolDatum.favoriteSkelFileList.push_back(path);
	}
	m_spineToolDatum.showFavoriteSkelFiles = m_showFavoriteSkelFiles;
}

void CMainWindow::FavoriteToggleFile(const std::wstring& path)
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

bool CMainWindow::ResolveAtlasPathForSkeleton(const std::wstring& skelFilePath, std::wstring& atlasPath, bool& isBinarySkel)
{
	size_t lastSlash = skelFilePath.find_last_of(L"\\/");
	size_t lastDot = skelFilePath.find_last_of(L'.');
	std::wstring dir = (lastSlash != std::wstring::npos) ? skelFilePath.substr(0, lastSlash + 1) : L"";
	std::wstring baseName = skelFilePath.substr(lastSlash + 1, lastDot - lastSlash - 1);

	atlasPath = dir + baseName + L".atlas";
	if (!PathFileExistsW(atlasPath.c_str()))
	{
		atlasPath = dir + baseName + L".atlas.txt";
		if (!PathFileExistsW(atlasPath.c_str()))
		{
			m_fileDialogs.ShowOwnerError((L"Atlas file not found for: " + baseName).c_str(), m_hWnd);
			return false;
		}
	}

	std::string skeletonFileDatum = path_util::LoadFileAsString(skelFilePath.c_str());
	using namespace spine_file_verifier;
	SkeletonMetadata skeletonMetaData = VerifySkeletonFileData(reinterpret_cast<const unsigned char*>(skeletonFileDatum.data()), skeletonFileDatum.size());
	if (skeletonMetaData.skeletonFormat == SkeletonFormat::Neither)
	{
		m_fileDialogs.ShowOwnerError(L"This seems not to be valid Spine skeleton file.", m_hWnd);
		return false;
	}
	isBinarySkel = skeletonMetaData.skeletonFormat == SkeletonFormat::Binary;

	if (ISpinePlayer* runtime = m_dxLibSpinePlayer.ActiveRuntime())
	{
		if (runtime->hasSpineBeenLoaded())
		{
			const auto versionIndex = m_dxLibSpinePlayer.ResolveVersion(reinterpret_cast<const char*>(skeletonMetaData.version));
			if (versionIndex != m_dxLibSpinePlayer.ActiveRuntimeSlot())
			{
				m_fileDialogs.ShowOwnerError(L"The file to be added should have the same Spine version as that of being loaded.", m_hWnd);
				return false;
			}
		}
	}

	return true;
}

bool CMainWindow::AddSpineFromPath(const std::wstring& skelFilePath)
{
	if (m_dxLibRecorder.GetState() != CDxLibRecorder::EState::Idle) return false;

	for (size_t i = 0; i < m_loadedSpineEntries.size(); ++i)
	{
		if (m_loadedSpineEntries[i].skelPath == skelFilePath)
		{
			m_showLoadedSpinePanel = true;
			SelectLoadedSpine(i);
			return true;
		}
	}

	ISpinePlayer* runtime = m_dxLibSpinePlayer.ActiveRuntime();
	if (runtime == nullptr || !runtime->hasSpineBeenLoaded())
	{
		LoadFilesFromPaths({ skelFilePath });
		m_showLoadedSpinePanel = !m_loadedSpineEntries.empty();
		SyncLoadedSpineDatum();
		return !m_loadedSpineEntries.empty();
	}

	std::wstring atlasPath;
	bool isBinarySkel = false;
	if (!ResolveAtlasPathForSkeleton(skelFilePath, atlasPath, isBinarySkel))
		return false;

	const std::string atlasUtf8 = win_text::NarrowUtf8(atlasPath);
	const std::string skelUtf8 = win_text::NarrowUtf8(skelFilePath);
	if (!runtime->addSpineFromFile(atlasUtf8.c_str(), skelUtf8.c_str(), isBinarySkel))
		return false;

	m_loadedSpineEntries.push_back({ skelFilePath, atlasPath, ExtractStemUtf8(skelFilePath) });
	m_loadedSpineVisibility.push_back(true);
	std::rotate(m_loadedSpineEntries.begin(), m_loadedSpineEntries.end() - 1, m_loadedSpineEntries.end());
	std::rotate(m_loadedSpineVisibility.begin(), m_loadedSpineVisibility.end() - 1, m_loadedSpineVisibility.end());
	m_showLoadedSpinePanel = true;
	SelectLoadedSpine(0);
	return true;
}

void CMainWindow::ResetLoadedSpineEntries(const std::vector<std::wstring>& skelFilePaths)
{
	m_loadedSpineEntries.clear();
	m_loadedSpineVisibility.clear();
	m_showLoadedSpinePanel = false;
	for (const auto& path : skelFilePaths)
	{
		std::wstring atlasPath;
		bool isBinarySkel = false;
		if (!ResolveAtlasPathForSkeleton(path, atlasPath, isBinarySkel))
			continue;
		m_loadedSpineEntries.push_back({ path, atlasPath, ExtractStemUtf8(path) });
		m_loadedSpineVisibility.push_back(true);
	}
	m_selectedLoadedSpine = 0;
	SyncLoadedSpineDatum();
}

void CMainWindow::SyncLoadedSpineDatum()
{
	m_spineToolDatum.loadedSpineFileList.clear();
	m_spineToolDatum.loadedSpineNames.clear();
	for (const auto& entry : m_loadedSpineEntries)
	{
		m_spineToolDatum.loadedSpineFileList.push_back(entry.skelPath);
		m_spineToolDatum.loadedSpineNames.push_back(entry.displayName);
	}
	m_spineToolDatum.loadedSpineVisibility = m_loadedSpineVisibility;
	m_spineToolDatum.showLoadedSpinePanel = m_showLoadedSpinePanel;
	m_spineToolDatum.selectedLoadedSpine = static_cast<int>(m_selectedLoadedSpine);
}

void CMainWindow::SelectLoadedSpine(size_t index)
{
	if (index >= m_loadedSpineEntries.size()) return;

	ISpinePlayer* runtime = m_dxLibSpinePlayer.ActiveRuntime();
	if (runtime == nullptr) return;

	m_selectedLoadedSpine = index;
	runtime->setSelectedSpineIndex(index);
	m_spineToolDatum.currentSkelFile = m_loadedSpineEntries[index].skelPath;
	m_spineToolDatum.currentFileName = m_loadedSpineEntries[index].displayName;
	m_spineToolDatum.titleBar.subTitle = m_loadedSpineEntries[index].displayName;
	SyncLoadedSpineDatum();
}

void CMainWindow::ToggleLoadedSpineVisibility(size_t index)
{
	if (index >= m_loadedSpineVisibility.size()) return;

	ISpinePlayer* runtime = m_dxLibSpinePlayer.ActiveRuntime();
	if (runtime == nullptr) return;

	const bool newVisible = !m_loadedSpineVisibility[index];
	if (!runtime->setSpineVisible(index, newVisible))
		return;

	m_loadedSpineVisibility[index] = newVisible;
	SyncLoadedSpineDatum();
}

void CMainWindow::MoveLoadedSpineUp(size_t index)
{
	if (index == 0 || index >= m_loadedSpineEntries.size()) return;

	ISpinePlayer* runtime = m_dxLibSpinePlayer.ActiveRuntime();
	if (runtime == nullptr) return;
	if (!runtime->moveSpineUp(index)) return;

	const size_t target = index - 1;
	std::swap(m_loadedSpineEntries[index], m_loadedSpineEntries[target]);
	std::swap(m_loadedSpineVisibility[index], m_loadedSpineVisibility[target]);
	if (m_selectedLoadedSpine == index)
		m_selectedLoadedSpine = target;
	else if (m_selectedLoadedSpine == target)
		m_selectedLoadedSpine = index;
	SyncLoadedSpineDatum();
}

void CMainWindow::MoveLoadedSpineDown(size_t index)
{
	if (index + 1 >= m_loadedSpineEntries.size()) return;

	ISpinePlayer* runtime = m_dxLibSpinePlayer.ActiveRuntime();
	if (runtime == nullptr) return;
	if (!runtime->moveSpineDown(index)) return;

	const size_t target = index + 1;
	std::swap(m_loadedSpineEntries[index], m_loadedSpineEntries[target]);
	std::swap(m_loadedSpineVisibility[index], m_loadedSpineVisibility[target]);
	if (m_selectedLoadedSpine == index)
		m_selectedLoadedSpine = target;
	else if (m_selectedLoadedSpine == target)
		m_selectedLoadedSpine = index;
	SyncLoadedSpineDatum();
}


void CMainWindow::MenuOnOpenFolder()
{
	if (m_dxLibRecorder.GetState() != CDxLibRecorder::EState::Idle)return;

	SModalGuard guard(m_isModalOpen);
	std::wstring wstrPickedupFolderPath = m_fileDialogs.PickFolder(m_hWnd);
	if (!wstrPickedupFolderPath.empty())
	{
		bool bRet = LoadSpineFilesInFolder(wstrPickedupFolderPath);
		if (bRet)
		{
			ClearFolderPathList();
			path_util::GetFilePathListAndIndex(wstrPickedupFolderPath, nullptr, m_folders, &m_nFolderIndex);
		}
	}
}

void CMainWindow::MenuOnExtensionSetting()
{
	m_spineSettingDialogue.Open(::GetModuleHandleA(nullptr), m_hWnd, L"Extensions");
}

void CMainWindow::MenuOnShowToolDialogue()
{
	m_toShowSpineParameter = true;
}

void CMainWindow::MenuOnFont()
{
	if (m_fontSettingDialogue.GetHwnd() == nullptr)
	{
		m_fontSettingDialogue.onFontApplied = [this](void* titleBarFont)
		{
			m_spineToolDatum.titleBar.customFont = titleBarFont;
		};
		HWND hWnd = m_fontSettingDialogue.Open(::GetModuleHandleW(nullptr), m_hWnd, L"Font setting");
		::SendMessage(hWnd, WM_SETICON, ICON_SMALL, ::GetClassLongPtr(m_hWnd, GCLP_HICON));
		::ShowWindow(hWnd, SW_SHOWNORMAL);
	}
	else
	{
		::SetFocus(m_fontSettingDialogue.GetHwnd());
	}
}

void CMainWindow::MenuOnMakeWindowTransparent()
{
	bool bRet = window_menu::SetMenuCheckState(window_menu::GetMenuInBar(m_hWnd, MenuBar::kWindow), Menu::kSeeThroughImage, !m_isTransparentWindow);
	if (bRet)
	{
		m_isTransparentWindow ^= true;
		LONG lStyleEx = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);

		if (m_isTransparentWindow)
		{
			::SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyleEx | WS_EX_LAYERED);
			::SetLayeredWindowAttributes(m_hWnd, RGB(0, 0, 0), 255, LWA_COLORKEY);
			::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		}
		else
		{
			::SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyleEx & ~WS_EX_LAYERED);
			::SetLayeredWindowAttributes(m_hWnd, RGB(0, 0, 0), 255, LWA_COLORKEY);
			::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		}
	}
}

void CMainWindow::MenuOnAllowDraggedResizing()
{
	bool isResizingAllowed = !m_isDraggedResizingAllowed && m_dxLibRecorder.GetState() != CDxLibRecorder::EState::UnderRecording;
	bool bRet = window_menu::SetMenuCheckState(window_menu::GetMenuInBar(m_hWnd, MenuBar::kWindow), Menu::kAllowDraggedResizing, isResizingAllowed);
	if (bRet)
	{
		m_isDraggedResizingAllowed = isResizingAllowed;
		UpdateWindowResizableAttribute();
	}
}

void CMainWindow::MenuOnReverseZoomDirection()
{
	bool bRet = window_menu::SetMenuCheckState(window_menu::GetMenuInBar(m_hWnd, MenuBar::kWindow), Menu::kReverseZoomDirection, !m_isZoomDirectionReversed);
	if (bRet)
	{
		m_isZoomDirectionReversed ^= true;
	}
}

void CMainWindow::MenuOnFiToManualSize()
{
	if (m_spineRenderTexture.Empty())return;

	int iScreenWidth = 0;
	int iScreenHeight = 0;
	DxLib::GetGraphSize(m_spineRenderTexture.Get(), &iScreenWidth, &iScreenHeight);

	const float fSkeletonScale = m_dxLibSpinePlayer.ActiveRuntime()->getSkeletonScale();
	float fBaseWidth = iScreenWidth / fSkeletonScale;
	float fBaseHeight = iScreenHeight / fSkeletonScale;

	m_dxLibSpinePlayer.ActiveRuntime()->setBaseSize(fBaseWidth, fBaseHeight);
	ResizeWindow();
}

void CMainWindow::MenuOnFitToDefaultSize()
{
	m_dxLibSpinePlayer.ActiveRuntime()->resetBaseSize();
	ResizeWindow();
}

void CMainWindow::KeyUpOnNextFolder()
{
	if (m_folders.empty() || m_dxLibRecorder.GetState() != CDxLibRecorder::EState::Idle)return;

	++m_nFolderIndex;
	if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = 0;
	LoadSpineFilesInFolder(m_folders[m_nFolderIndex]);
}

void CMainWindow::KeyUpOnForeFolder()
{
	if (m_folders.empty() || m_dxLibRecorder.GetState() != CDxLibRecorder::EState::Idle)return;

	--m_nFolderIndex;
	if (m_nFolderIndex >= m_folders.size())m_nFolderIndex = m_folders.size() - 1;
	LoadSpineFilesInFolder(m_folders[m_nFolderIndex]);
}

void CMainWindow::KeyUpOnForeFile(int delta)
{
	const auto& fileList = m_spineToolDatum.skelFileList;
	if (fileList.empty()) return;


	int cur = -1;
	for (int i = 0; i < (int)fileList.size(); i++)
	{
		if (fileList[i] == m_spineToolDatum.currentSkelFile) { cur = i; break; }
	}

	int next = cur + delta;
	if (next < 0) next = (int)fileList.size() - 1;
	if (next >= (int)fileList.size()) next = 0;


	m_spineToolDatum.currentSkelFile = fileList[next];
	std::string stem = ExtractStemUtf8(fileList[next]);
	m_spineToolDatum.currentFileName = stem;
	m_spineToolDatum.titleBar.subTitle = stem;
}

void CMainWindow::MenuOnSaveAsJpg()
{
	if (!m_dxLibSpinePlayer.ActiveRuntime()->hasSpineBeenLoaded())return;

	std::wstring wstrFilePath = BuildExportFilePath();
	float fTrackTime = 0.f;
	m_dxLibSpinePlayer.ActiveRuntime()->getCurrentAnimationTime(&fTrackTime, nullptr, nullptr, nullptr);
	wstrFilePath += FormatAnimationTime(fTrackTime).append(L".jpg");


	int flatTex = CreateFlattenedTexture(m_spineRenderTexture.Get(),
		m_spineToolDatum.renderBgR, m_spineToolDatum.renderBgG, m_spineToolDatum.renderBgB);
	m_frameExporter.ExportTextureJpeg(flatTex != -1 ? flatTex : m_spineRenderTexture.Get(), wstrFilePath.c_str());
	if (flatTex != -1) DxLib::DeleteGraph(flatTex);
}

void CMainWindow::MenuOnSaveAsPng()
{
	if (!m_dxLibSpinePlayer.ActiveRuntime()->hasSpineBeenLoaded())return;

	std::wstring wstrFilePath = BuildExportFilePath();
	float fTrackTime = 0.f;
	m_dxLibSpinePlayer.ActiveRuntime()->getCurrentAnimationTime(&fTrackTime, nullptr, nullptr, nullptr);
	wstrFilePath += FormatAnimationTime(fTrackTime).append(L".png");

	if (m_spineToolDatum.exportWithAlpha)
	{
		m_frameExporter.ExportTexturePng(m_spineRenderTexture.Get(), wstrFilePath.c_str());
	}
	else
	{
		int flatTex = CreateFlattenedTexture(m_spineRenderTexture.Get(),
			m_spineToolDatum.renderBgR, m_spineToolDatum.renderBgG, m_spineToolDatum.renderBgB);
		m_frameExporter.ExportTexturePng(flatTex != -1 ? flatTex : m_spineRenderTexture.Get(), wstrFilePath.c_str());
		if (flatTex != -1) DxLib::DeleteGraph(flatTex);
	}
}

void CMainWindow::MenuOnStartRecording(int menuKind)
{
	if (!m_dxLibSpinePlayer.ActiveRuntime()->hasSpineBeenLoaded())return;

	CDxLibRecorder::EOutputType outputType = CDxLibRecorder::EOutputType::Unknown;
	switch (menuKind)
	{
	case PopupMenu::kExportAsGif:
		outputType = CDxLibRecorder::EOutputType::Gif;
		break;
	case PopupMenu::kExportAsVideo:
		outputType = CDxLibRecorder::EOutputType::Video;
		break;
	case PopupMenu::kExportAsWebm:
		outputType = CDxLibRecorder::EOutputType::WebmFrames;
		break;
	case PopupMenu::kExportAsPngs:
		outputType = CDxLibRecorder::EOutputType::Pngs;
		break;
	case PopupMenu::kExportAsJpgs:
		outputType = CDxLibRecorder::EOutputType::Jpgs;
	default:
		break;
	}

	if (outputType == CDxLibRecorder::EOutputType::Unknown)return;

	unsigned short fps = (menuKind == PopupMenu::kExportAsVideo || menuKind == PopupMenu::kExportAsWebm) ?
		m_spineToolDatum.iVideoFps :
		m_spineToolDatum.iImageFps;

	bool bRet = m_dxLibRecorder.Start(outputType, fps);
	if (!bRet)return;


	if (outputType == CDxLibRecorder::EOutputType::Pngs ||
		outputType == CDxLibRecorder::EOutputType::Jpgs ||
		outputType == CDxLibRecorder::EOutputType::WebmFrames)
	{
		float totalDuration = 0.f;
		if (m_isQueueRecording && !m_animQueue.empty())
		{
			for (const auto& animName : m_animQueue)
			{
				float dur = m_dxLibSpinePlayer.ActiveRuntime()->getAnimationDuration(animName.c_str());
				if (dur > 0.f) totalDuration += dur;
			}
		}
		else
		{
			m_dxLibSpinePlayer.ActiveRuntime()->getCurrentAnimationTime(nullptr, nullptr, nullptr, &totalDuration);
		}
		m_spineToolDatum.recordFramesTotal = (totalDuration > 0.f) ? static_cast<int>(totalDuration * fps + 0.5f) : 0;
		m_spineToolDatum.recordFramesCaptured = 0;
	}


	if (outputType == CDxLibRecorder::EOutputType::Video)
	{
		MenuOnAllowDraggedResizing();
	}

	if (m_spineToolDatum.toExportPerAnim && !m_isQueueRecording)
	{
		m_dxLibSpinePlayer.ActiveRuntime()->restartAnimation();
	}
}

void CMainWindow::MenuOnEndRecording()
{
	if (m_dxLibRecorder.GetState() == CDxLibRecorder::EState::UnderRecording)
	{
		const bool isWebm = (m_dxLibRecorder.GetOutputType() == CDxLibRecorder::EOutputType::WebmFrames);
		if (isWebm)
		{


			int frameW = 0, frameH = 0;
			m_dxLibRecorder.GetFirstFrameSize(&frameW, &frameH);


			HANDLE hPipeWrite = INVALID_HANDLE_VALUE;
			if (LaunchFfmpegWebmPipe(m_webmOutputPath, m_spineToolDatum.iVideoFps,
			                         frameW, frameH, hPipeWrite))
			{
				m_dxLibRecorder.SetPipeHandle(hPipeWrite);
			}

			m_dxLibRecorder.End(nullptr);
			m_spineToolDatum.isWritingFrames = true;
			m_spineToolDatum.recordFramesWritten = 0;

		}
		else
		{
			std::wstring wstrFilePath = BuildExportFilePath();
			const auto& outputType = m_dxLibRecorder.GetOutputType();
			if (outputType == CDxLibRecorder::EOutputType::Pngs || outputType == CDxLibRecorder::EOutputType::Jpgs)
			{

				std::wstring wstrFolder = wstrFilePath + L"\\";
				::CreateDirectoryW(wstrFolder.c_str(), nullptr);
				m_dxLibRecorder.End(wstrFolder.c_str());

				m_spineToolDatum.isWritingFrames = true;
				m_spineToolDatum.recordFramesWritten = 0;

			}
			else
			{
				m_dxLibRecorder.End(wstrFilePath.c_str());
				m_spineToolDatum.recordFramesCaptured = 0;
				m_spineToolDatum.recordFramesTotal = 0;
			}
		}
	}
}


void CMainWindow::MenuOnExportWebm()
{
	if (!m_dxLibSpinePlayer.ActiveRuntime()->hasSpineBeenLoaded()) return;
	if (m_isWebmEncoding) return;


	std::wstring ffmpegPath = path_util::GetCurrentProcessPath() + L"\\ffmpeg.exe";
	if (::GetFileAttributesW(ffmpegPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		TASKDIALOGCONFIG tdc{};
		tdc.cbSize             = sizeof(tdc);
		tdc.hwndParent         = m_hWnd;
		tdc.dwFlags            = TDF_ENABLE_HYPERLINKS;
		tdc.dwCommonButtons    = TDCBF_OK_BUTTON;
		tdc.pszWindowTitle     = L"WebM Export";
		tdc.pszMainIcon        = TD_WARNING_ICON;
		tdc.pszMainInstruction = L"ffmpeg.exe not found.";
		tdc.pszContent         =
			L"Please place ffmpeg.exe in the same folder as the application.\r\n\r\n"
			L"Download: <a href=\"https://github.com/BtbN/FFmpeg-Builds/releases\">github.com/BtbN/FFmpeg-Builds/releases</a>";
		tdc.pfCallback = [](HWND, UINT uNotification, WPARAM, LPARAM lParam, LONG_PTR) -> HRESULT {
			if (uNotification == TDN_HYPERLINK_CLICKED)
				::ShellExecuteW(nullptr, L"open", reinterpret_cast<LPCWSTR>(lParam), nullptr, nullptr, SW_SHOWNORMAL);
			return S_OK;
		};
		::TaskDialogIndirect(&tdc, nullptr, nullptr, nullptr);
		return;
	}


	m_webmOutputPath = BuildExportFilePath();


	int fps = m_spineToolDatum.iVideoFps;
	float totalDuration = 0.f;
	if (m_isQueueRecording && !m_animQueue.empty())
	{

		for (const auto& animName : m_animQueue)
		{
			float dur = m_dxLibSpinePlayer.ActiveRuntime()->getAnimationDuration(animName.c_str());
			if (dur > 0.f) totalDuration += dur;
		}
	}
	else
	{
		m_dxLibSpinePlayer.ActiveRuntime()->getCurrentAnimationTime(nullptr, nullptr, nullptr, &totalDuration);
	}
	m_spineToolDatum.recordFramesTotal = (totalDuration > 0.f) ? static_cast<int>(totalDuration * fps + 0.5f) : 0;
	m_spineToolDatum.recordFramesCaptured = 0;

	MenuOnStartRecording(PopupMenu::kExportAsWebm);
}


bool CMainWindow::LaunchFfmpegWebmPipe(const std::wstring& wstrOutputPath, int fps,
                                        int width, int height, HANDLE& hPipeWrite_out)
{
	hPipeWrite_out = INVALID_HANDLE_VALUE;


	std::wstring ffmpegPath = path_util::GetCurrentProcessPath() + L"\\ffmpeg.exe";
	if (::GetFileAttributesW(ffmpegPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		::MessageBoxW(m_hWnd,
			L"ffmpeg.exe not found.\nPlease place ffmpeg.exe in the same folder as the application.",
			L"WebM Export", MB_OK | MB_ICONWARNING);
		return false;
	}

	if (width <= 0 || height <= 0)
	{
		::MessageBoxW(m_hWnd, L"Invalid frame dimensions for WebM export.",
			L"WebM Export", MB_OK | MB_ICONERROR);
		return false;
	}


	std::wstring outputFile = wstrOutputPath + L".webm";

	wchar_t szFps[16]{}, szSize[32]{};
	::swprintf_s(szFps,  L"%d", fps);
	::swprintf_s(szSize, L"%dx%d", width, height);

	std::wstring cmdLine =
		L"\"" + ffmpegPath + L"\" -y"
		L" -f rawvideo -pixel_format rgba"
		L" -video_size " + szSize +
		L" -framerate " + szFps +
		L" -i pipe:0"
		L" -c:v libvpx-vp9 -pix_fmt yuva420p -auto-alt-ref 0 -crf 17"
		L" \"" + outputFile + L"\"";


	SECURITY_ATTRIBUTES sa{};
	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = TRUE;

	HANDLE hReadEnd = INVALID_HANDLE_VALUE, hWriteEnd = INVALID_HANDLE_VALUE;
	if (!::CreatePipe(&hReadEnd, &hWriteEnd, &sa, 0))
	{
		::MessageBoxW(m_hWnd, L"Failed to create pipe for WebM export.",
			L"WebM Export", MB_OK | MB_ICONERROR);
		return false;
	}

	::SetHandleInformation(hWriteEnd, HANDLE_FLAG_INHERIT, 0);

	STARTUPINFOW si{};
	si.cb          = sizeof(si);
	si.dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.hStdInput   = hReadEnd;
	si.hStdOutput  = INVALID_HANDLE_VALUE;
	si.hStdError   = INVALID_HANDLE_VALUE;

	PROCESS_INFORMATION pi{};
	std::vector<wchar_t> cmdBuf(cmdLine.begin(), cmdLine.end());
	cmdBuf.push_back(L'\0');

	BOOL bRet = ::CreateProcessW(nullptr, cmdBuf.data(),
		nullptr, nullptr,
		TRUE,
		CREATE_NO_WINDOW,
		nullptr, nullptr, &si, &pi);


	::CloseHandle(hReadEnd);

	if (!bRet)
	{
		::CloseHandle(hWriteEnd);
		::MessageBoxW(m_hWnd, L"Failed to launch ffmpeg.exe.", L"WebM Export", MB_OK | MB_ICONERROR);
		return false;
	}

	::CloseHandle(pi.hThread);
	m_hFfmpegProcess = pi.hProcess;
	m_isWebmEncoding = true;
	hPipeWrite_out   = hWriteEnd;
	return true;
}

void CMainWindow::ChangeWindowTitle(const wchar_t* pwzTitle)
{
	const wchar_t* pwzName = pwzTitle;
	if (pwzName != nullptr)
	{
		for (;;)
		{
			const wchar_t* pPos = wcspbrk(pwzName, L"\\/");
			if (pPos == nullptr)break;
			pwzName = pPos + 1;
		}
	}

	::SetWindowTextW(m_hWnd, pwzName == nullptr ? m_swzDefaultWindowName : pwzName);
}

std::wstring CMainWindow::GetWindowTitle() const
{
	int iLen = ::GetWindowTextLengthW(m_hWnd);
	if (iLen == 0)return {};

	++iLen;
	std::wstring result(iLen, L'\0');
	int iWritten = ::GetWindowTextW(m_hWnd, &result[0], iLen);
	result.resize(iWritten);

	return result;
}

void CMainWindow::ToggleWindowFrameStyle()
{
	if (!m_dxLibSpinePlayer.ActiveRuntime()->hasSpineBeenLoaded() || m_dxLibRecorder.GetState() == CDxLibRecorder::EState::UnderRecording)return;

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

	m_isFramelessWindow ^= true;

	if (m_isFramelessWindow)
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_CAPTION & ~WS_SYSMENU & ~WS_THICKFRAME);
		::SetWindowPos(m_hWnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		::SetMenu(m_hWnd, nullptr);
	}
	else
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle | WS_CAPTION | WS_SYSMENU | (m_isDraggedResizingAllowed ? WS_THICKFRAME : 0));
		::SetMenu(m_hWnd, m_hMenuBar);
	}

	ResizeWindow();
}

void CMainWindow::ToggleFullscreen()
{
	if (!m_isFullscreen)
	{
		m_preFullscreenStyle   = ::GetWindowLong(m_hWnd, GWL_STYLE);
		m_preFullscreenStyleEx = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
		::GetWindowRect(m_hWnd, &m_preFullscreenRect);

		HMONITOR hMon = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi{ sizeof(mi) };
		::GetMonitorInfoW(hMon, &mi);
		const RECT& rc = mi.rcMonitor;

		::SetWindowLong(m_hWnd, GWL_STYLE,
			m_preFullscreenStyle & ~WS_CAPTION & ~WS_THICKFRAME & ~WS_SYSMENU);
		::SetMenu(m_hWnd, nullptr);
		::SetWindowPos(m_hWnd, HWND_TOP,
			rc.left, rc.top,
			rc.right - rc.left, rc.bottom - rc.top,
			SWP_FRAMECHANGED | SWP_NOOWNERZORDER);

		m_isFullscreen = true;
		m_spineToolDatum.isFullscreen = true;
	}
	else
	{
		::SetWindowLong(m_hWnd, GWL_STYLE,   m_preFullscreenStyle);
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, m_preFullscreenStyleEx);
		::SetMenu(m_hWnd, m_hMenuBar);
		::SetWindowPos(m_hWnd, nullptr,
			m_preFullscreenRect.left,
			m_preFullscreenRect.top,
			m_preFullscreenRect.right  - m_preFullscreenRect.left,
			m_preFullscreenRect.bottom - m_preFullscreenRect.top,
			SWP_FRAMECHANGED | SWP_NOZORDER);

		m_isFullscreen = false;
		m_spineToolDatum.isFullscreen = false;
	}
}

void CMainWindow::UpdateMenuItemState()
{
	constexpr const unsigned int toolMenuIndices[] = { Menu::kShowToolDialogue };
	constexpr const unsigned int windowIndices[] = { Menu::kSeeThroughImage, Menu::kAllowDraggedResizing, Menu::kReverseZoomDirection, Menu::kFitToManualSize, Menu::kFitToDefaultSize };

	bool toEnable = m_dxLibSpinePlayer.ActiveRuntime()->hasSpineBeenLoaded();

	window_menu::EnableMenuItems(window_menu::GetMenuInBar(m_hWnd, MenuBar::kTool), toolMenuIndices, toEnable);
	window_menu::EnableMenuItems(window_menu::GetMenuInBar(m_hWnd, MenuBar::kWindow), windowIndices, toEnable);
}

bool CMainWindow::LoadSpineFilesInFolder(const std::wstring& folderPath)
{
	if (QueueLoadedSpineReplaceAction([this, folderPath]() { LoadSpineFilesInFolder(folderPath); })) return false;

	std::vector<std::string> atlasData;
	std::vector<std::string> skelData;

	const std::wstring& wstrAtlasExt = m_spineSettingDialogue.GetAtlasExtension();
	const std::wstring& wstrSkelExt = m_spineSettingDialogue.GetSkelExtension();

	bool isAtlasLonger = wstrAtlasExt.size() > wstrSkelExt.size();

	const std::wstring& wstrLongerExtesion = isAtlasLonger ? wstrAtlasExt : wstrSkelExt;
	const std::wstring& wstrShorterExtension = isAtlasLonger ? wstrSkelExt : wstrAtlasExt;

	std::vector<std::string>& longerPathData = isAtlasLonger ? atlasData : skelData;
	std::vector<std::string>& shorterPathData = isAtlasLonger ? skelData : atlasData;

	std::vector<std::wstring> wstrFilePaths;
	path_util::CreateFilePathList(folderPath.c_str(), L"*", wstrFilePaths);

	for (const auto& filePath : wstrFilePaths)
	{
		const auto EndsWith = [&filePath](const std::wstring& str)
			-> bool
			{
				if (filePath.size() < str.size()) return false;
				return std::equal(str.rbegin(), str.rend(), filePath.rbegin());
			};

		if (EndsWith(wstrLongerExtesion))
		{
			longerPathData.emplace_back(path_util::LoadFileAsString(filePath.c_str()));
		}
		else if (EndsWith(wstrShorterExtension))
		{
			shorterPathData.emplace_back(path_util::LoadFileAsString(filePath.c_str()));
		}
	}

	std::string textureDirectory = win_text::NarrowUtf8(folderPath);
	std::vector<std::string> textureDirectories(atlasData.size(), textureDirectory);

	return LoadSpinesFromMemory(atlasData, textureDirectories, skelData, folderPath.c_str());
}

bool CMainWindow::LoadSpineFiles(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool isBinarySkel, const wchar_t* windowName)
{
	ISpinePlayer* runtime = m_dxLibSpinePlayer.ActiveRuntime();
	const SPlaybackSnapshot snapshot = CapturePlaybackSnapshot(runtime);

	bool hasLoaded = runtime->loadSpineFromFile(atlasPaths, skelPaths, isBinarySkel);
	RestorePlaybackSnapshot(runtime, snapshot, hasLoaded);
	PostSpineLoading(snapshot.hadLoaded, hasLoaded, windowName);
	return hasLoaded;
}

bool CMainWindow::LoadSpinesFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& textureDirectories, const std::vector<std::string>& skelData, const wchar_t* windowName)
{
	if (m_isLoading) return false;
	if (skelData.empty())return false;

	const auto& skeldatum = skelData[0];
	using namespace spine_file_verifier;
	SkeletonMetadata skeletonMetaData = VerifySkeletonFileData(reinterpret_cast<const unsigned char*>(skeldatum.data()), skeldatum.size());
	if (skeletonMetaData.skeletonFormat == SkeletonFormat::Neither)
	{
		m_fileDialogs.ShowOwnerError(L"The format of skeleton seems not to be valid one.", m_hWnd);
		return false;
	}

	bool isBinarySkel = skeletonMetaData.skeletonFormat == SkeletonFormat::Binary;

	CSpineRuntimeRegistry::ERuntimeSlot versionIndex = m_dxLibSpinePlayer.ResolveVersion(reinterpret_cast<const char*>(skeletonMetaData.version));
	if (versionIndex == CSpineRuntimeRegistry::ERuntimeSlot::Unknown)
	{
		m_fileDialogs.ShowOwnerError(L"The runtime for this version is not implemented.", m_hWnd);
		return false;
	}
	if (!m_dxLibSpinePlayer.HasRuntime(versionIndex))
	{
		m_fileDialogs.ShowOwnerError(L"The dll for this version is not loaded.", m_hWnd);
		return false;
	}
	m_dxLibSpinePlayer.SelectRuntime(versionIndex);

	ISpinePlayer* runtime = m_dxLibSpinePlayer.ActiveRuntime();
	const SPlaybackSnapshot snapshot = CapturePlaybackSnapshot(runtime);

	if (snapshot.hadLoaded)
	{
		m_lastAnimationName = runtime->getCurrentAnimationName();
		m_lastSkinName      = runtime->getCurrentSkinName();
	}

	auto pPromise = std::make_shared<std::promise<bool>>();
	std::future<bool> future = pPromise->get_future();

	std::thread([p = pPromise,
	             atlasCopy = atlasData,
	             texDirCopy = textureDirectories,
	             skelCopy = skelData,
	             isBinarySkel,
	             player = m_dxLibSpinePlayer.ActiveRuntime()]() mutable {
		bool result = player->loadSpineFromMemory(atlasCopy, texDirCopy, skelCopy, isBinarySkel);
		p->set_value(result);
	}).detach();

	m_isLoading = true;
	static constexpr DWORD kTimeoutMs = 15000;
	DWORD elapsed = 0;
	static constexpr DWORD kPollMs = 50;
	bool hasLoaded = false;
	bool timedOut = false;

	while (true)
	{
		auto status = future.wait_for(std::chrono::milliseconds(kPollMs));
		if (status == std::future_status::ready)
		{
			hasLoaded = future.get();
			break;
		}

		MSG msg{};
		while (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}

		elapsed += kPollMs;
		if (elapsed >= kTimeoutMs)
		{
			timedOut = true;
			break;
		}
	}

	if (timedOut)
	{
		m_isLoading = false;
		m_fileDialogs.ShowOwnerError(L"Loading timed out (15 s). The file may be corrupted or unsupported.", m_hWnd);
		return false;
	}

	m_isLoading = false;
	RestorePlaybackSnapshot(runtime, snapshot, hasLoaded);
	PostSpineLoading(snapshot.hadLoaded, hasLoaded, windowName);
	return hasLoaded;
}

void CMainWindow::ClearFolderPathList()
{
	m_folders.clear();
	m_nFolderIndex = 0;
}

void CMainWindow::PostSpineLoading(bool hadLoaded, bool hasLoaded, const wchar_t* windowName)
{
	if (hasLoaded)
	{

		if (m_dxLibSpinePlayer.ActiveRuntime()->isFlipX())
			m_dxLibSpinePlayer.ActiveRuntime()->toggleFlipX();
		while (m_dxLibSpinePlayer.ActiveRuntime()->getRotationSteps() != 0)
			m_dxLibSpinePlayer.ActiveRuntime()->rotate90();
		m_dxLibSpinePlayer.ActiveRuntime()->addOffset(0, 0);

		ResizeWindow();
		ChangeWindowTitle(windowName);
		++m_spineToolDatum.loadGeneration;

		if (spine_panel::HasSlotExclusionFilter())
		{
			m_dxLibSpinePlayer.ActiveRuntime()->setSlotExcludeCallback(spine_panel::GetSlotExcludeCallback());
		}


		CSpineRuntimeRegistry::ERuntimeSlot versionIndex = m_dxLibSpinePlayer.ActiveRuntimeSlot();
		bool isSpine3x = versionIndex < CSpineRuntimeRegistry::ERuntimeSlot::Spine40;
		if (isSpine3x && m_userPmaSet)
		{
			bool currentPma = m_dxLibSpinePlayer.ActiveRuntime()->isAlphaPremultiplied();
			if (currentPma != m_userPma)
				m_dxLibSpinePlayer.ActiveRuntime()->togglePma();
		}

		if (!m_lastAnimationName.empty())
		{
			const auto& animNames = m_dxLibSpinePlayer.ActiveRuntime()->getAnimationNames();
			auto animIt = std::find(animNames.begin(), animNames.end(), m_lastAnimationName);
			if (animIt != animNames.end())
				m_dxLibSpinePlayer.ActiveRuntime()->setAnimationByName(m_lastAnimationName.c_str());
		}

		m_winclock.Restart();
	}
	else
	{
		m_fileDialogs.ShowOwnerError(L"Failed to load Spine(s)", m_hWnd);
		ChangeWindowTitle(nullptr);
	}
	if (hadLoaded != hasLoaded)UpdateMenuItemState();
}

std::wstring CMainWindow::BuildExportFilePath()
{
	std::wstring wstrFilePath = path_util::CreateWorkFolder(GetWindowTitle());


	SYSTEMTIME st{};
	::GetLocalTime(&st);
	wchar_t swTime[32]{};
	swprintf_s(swTime, L"%04d%02d%02d_%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	if (m_isQueueRecording)
	{
		wstrFilePath += L"queue_";
		wstrFilePath += swTime;
	}
	else if (!m_spineToolDatum.toExportPerAnim)
	{
		wstrFilePath += swTime;
	}
	else
	{
		wstrFilePath += win_text::WidenUtf8(m_dxLibSpinePlayer.ActiveRuntime()->getCurrentAnimationName());
		wstrFilePath += L"_";
		wstrFilePath += swTime;
	}

	return wstrFilePath;
}

std::wstring CMainWindow::FormatAnimationTime(float fAnimationTime)
{
	return MakeAnimationTimeTag(fAnimationTime);
}

void CMainWindow::StepRecording()
{
	const auto& recorderState = m_dxLibRecorder.GetState();
	if (recorderState == CDxLibRecorder::EState::UnderRecording)
	{
		float fTrack = 0.f;
		float fEnd = 0.f;
		m_dxLibSpinePlayer.ActiveRuntime()->getCurrentAnimationTime(&fTrack, nullptr, nullptr, &fEnd);

		if (m_spineToolDatum.toExportPerAnim && !m_isQueueRecording)
		{
			if (m_dxLibRecorder.HasFrames() && fEnd > 0.f && !::isless(fTrack, fEnd))
			{
				MenuOnEndRecording();
				return;
			}
		}

		const auto& outputType = m_dxLibRecorder.GetOutputType();
		if (outputType == CDxLibRecorder::EOutputType::Pngs || outputType == CDxLibRecorder::EOutputType::Jpgs)
		{
			std::wstring wstrFrameName = MakeRecordedFrameName(fTrack, m_isQueueRecording, m_queueIndex);
			m_dxLibRecorder.CommitFrame(m_spineRenderTexture.Get(), wstrFrameName.c_str());
			m_spineToolDatum.recordFramesCaptured = m_dxLibRecorder.GetFrameCount();
		}
		else if (outputType == CDxLibRecorder::EOutputType::WebmFrames)
		{

			wchar_t szIdx[16]{};
			::swprintf_s(szIdx, L"%05d", m_dxLibRecorder.GetFrameCount() + 1);
			m_dxLibRecorder.CommitFrame(m_spineRenderTexture.Get(), szIdx);
			m_spineToolDatum.recordFramesCaptured = m_dxLibRecorder.GetFrameCount();
		}
		else
		{
			m_dxLibRecorder.CommitFrame(m_spineRenderTexture.Get());
		}
	}
}

void CMainWindow::UpdateWindowResizableAttribute()
{
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	::SetWindowLong(m_hWnd, GWL_STYLE, (m_dxLibSpinePlayer.ActiveRuntime()->hasSpineBeenLoaded() && m_isDraggedResizingAllowed) ? (lStyle | WS_THICKFRAME) : (lStyle & ~WS_THICKFRAME));
}

void CMainWindow::ResizeWindow()
{

}

void CMainWindow::ImGuiSpineParameterDialogue()
{
	if (!m_spineRenderTexture.Empty())
	{
		DxLib::GetGraphSize(m_spineRenderTexture.Get(), &m_spineToolDatum.iTextureWidth, &m_spineToolDatum.iTextureHeight);

		if (!m_spineToolDatum.isFullscreen)
		{
			m_spineToolDatum.iTextureHeight -= static_cast<int>(custom_titlebar::TitleBarHeight());
			if (m_spineToolDatum.iTextureHeight < 0) m_spineToolDatum.iTextureHeight = 0;
		}
	}

	spine_panel::Display(m_spineToolDatum, &m_toShowSpineParameter);
	if (m_spineToolDatum.isWindowToBeResized)
	{
		ResizeWindow();
		m_spineToolDatum.isWindowToBeResized = false;
	}
}

void CMainWindow::StartQueuePlay()
{
	if (m_animQueue.empty()) return;
	if (!m_dxLibSpinePlayer.ActiveRuntime()->hasSpineBeenLoaded()) return;
	m_isQueuePlaying = true;
	m_queueIndex = 0;
	m_dxLibSpinePlayer.ActiveRuntime()->setAnimationByName(m_animQueue[0].c_str());
	m_dxLibSpinePlayer.ActiveRuntime()->restartAnimation(false);
}

void CMainWindow::StopQueuePlay()
{
	m_isQueuePlaying = false;
}

void CMainWindow::StepQueuePlay()
{
	if (!m_isQueuePlaying) return;
	if (m_animQueue.empty()) { m_isQueuePlaying = false; return; }

	float fTrack = 0.f, fEnd = 0.f;
	m_dxLibSpinePlayer.ActiveRuntime()->getCurrentAnimationTime(&fTrack, nullptr, nullptr, &fEnd);

	if (fEnd > 0.f && ::isgreater(fTrack, fEnd))
	{
		++m_queueIndex;
		if (m_queueIndex >= m_animQueue.size())
		{

			if (m_isQueueRecording)
			{

				MenuOnEndRecording();
				m_isQueueRecording = false;
				m_isQueuePlaying = false;
				return;
			}

			m_queueIndex = 0;
		}
		m_dxLibSpinePlayer.ActiveRuntime()->setAnimationByName(m_animQueue[m_queueIndex].c_str());
		m_dxLibSpinePlayer.ActiveRuntime()->restartAnimation(false);
	}
}

void CMainWindow::MenuOnLoadBackground()
{
	SModalGuard guard(m_isModalOpen);
	shell_dialogs::FileDialogRequest backgroundRequest{};
	backgroundRequest.title = L"Select background image";
	backgroundRequest.primaryFilter = { L"Image file", L"*.png;*.jpg;*.jpeg" };
	std::wstring path = m_fileDialogs.PickSingleFile(backgroundRequest, m_hWnd);
	if (path.empty()) return;


	int handle = DxLib::LoadGraph(path.c_str());
	if (handle == -1) return;

	m_bgTexture = DxLibImageHandle(handle);


	int bgW = 0, bgH = 0;
	DxLib::GetGraphSize(handle, &bgW, &bgH);
	int screenW = 0, screenH = 0;
	DxLib::GetScreenState(&screenW, &screenH, nullptr);
	const int panelX = static_cast<int>(m_spineToolDatum.leftPanelEndX);
	const int playW = screenW - panelX;
	m_bgScale = 1.f;
	m_bgOffsetX = (playW - bgW) * 0.5f;
	m_bgOffsetY = (screenH - bgH) * 0.5f;
}


static const wchar_t* kAudioExts[] = { L".wav", L".mp3", L".ogg", L".flac", L".aac", L".m4a" };
static bool IsAudioExt(const std::wstring& name)
{
	size_t dot = name.rfind(L'.');
	if (dot == std::wstring::npos) return false;
	std::wstring ext = name.substr(dot);
	for (auto& e : kAudioExts)
		if (_wcsicmp(ext.c_str(), e) == 0) return true;
	return false;
}

void CMainWindow::AudioScanFolder(const std::wstring& folderPath)
{
	m_audioFiles.clear();
	WIN32_FIND_DATAW fd{};
	std::wstring pattern = folderPath + L"\\*";
	HANDLE hFind = ::FindFirstFileW(pattern.c_str(), &fd);
	if (hFind == INVALID_HANDLE_VALUE) return;
	do {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
		std::wstring name = fd.cFileName;
		if (IsAudioExt(name))
			m_audioFiles.push_back(folderPath + L"\\" + name);
	} while (::FindNextFileW(hFind, &fd));
	::FindClose(hFind);
	std::sort(m_audioFiles.begin(), m_audioFiles.end());
}

void CMainWindow::AudioSyncDatum()
{
	if (m_audioFiles.empty())
	{
		m_spineToolDatum.audioIndex = 0;
		m_spineToolDatum.audioTotal = 0;
		m_spineToolDatum.audioFileName.clear();
	}
	else
	{
		m_spineToolDatum.audioIndex = static_cast<int>(m_audioFileIndex) + 1;
		m_spineToolDatum.audioTotal = static_cast<int>(m_audioFiles.size());
		size_t sep = m_audioFilePath.find_last_of(L"\\/");
		m_spineToolDatum.audioFileName =
			(sep == std::wstring::npos) ? m_audioFilePath : m_audioFilePath.substr(sep + 1);
	}
}

void CMainWindow::AudioLoadFile(const std::wstring& path)
{
	m_pAudioPlayer = std::make_unique<CMfMediaPlayer>();
	m_audioFilePath = path;
	m_spineToolDatum.isAudioPlaying = false;
}

void CMainWindow::MenuOnLoadAudio()
{
	SModalGuard guard(m_isModalOpen);
	shell_dialogs::FileDialogRequest audioRequest{};
	audioRequest.title = L"Select audio file";
	audioRequest.primaryFilter = { L"Audio file", L"*.wav;*.mp3;*.ogg;*.flac;*.aac;*.m4a" };
	std::wstring path = m_fileDialogs.PickSingleFile(audioRequest, m_hWnd);
	if (path.empty()) return;


	size_t sep = path.find_last_of(L"\\/");
	std::wstring folder = (sep != std::wstring::npos) ? path.substr(0, sep) : L".";
	AudioScanFolder(folder);


	m_audioFileIndex = 0;
	for (size_t i = 0; i < m_audioFiles.size(); ++i)
	{
		if (_wcsicmp(m_audioFiles[i].c_str(), path.c_str()) == 0)
		{
			m_audioFileIndex = i;
			break;
		}
	}

	if (m_audioFiles.empty())
		m_audioFiles.push_back(path);

	AudioLoadFile(m_audioFiles[m_audioFileIndex]);
	AudioSyncDatum();
}

void CMainWindow::MenuOnPlayAudio()
{
	if (!m_pAudioPlayer || m_audioFilePath.empty()) return;
	m_pAudioPlayer->Play(m_audioFilePath.c_str());
	m_pAudioPlayer->SetCurrentVolume(static_cast<double>(m_spineToolDatum.audioVolume));
	if (m_isAudioLooping) m_pAudioPlayer->SwitchLoop();
	m_spineToolDatum.isAudioPlaying = true;
}

void CMainWindow::MenuOnStopAudio()
{
	if (!m_pAudioPlayer) return;
	if (m_spineToolDatum.isAudioPlaying)
		m_pAudioPlayer->SwitchPause();
	m_spineToolDatum.isAudioPlaying = false;
}

void CMainWindow::MenuOnToggleLoopAudio()
{
	if (!m_pAudioPlayer) return;
	BOOL looping = m_pAudioPlayer->SwitchLoop();
	m_isAudioLooping = looping != FALSE;
	m_spineToolDatum.isAudioLooping = m_isAudioLooping;
}

void CMainWindow::MenuOnAudioPrev()
{
	if (m_audioFiles.empty()) return;
	if (m_audioFileIndex > 0)
		m_audioFileIndex--;
	else
		m_audioFileIndex = m_audioFiles.size() - 1;
	bool wasPlaying = m_spineToolDatum.isAudioPlaying;
	AudioLoadFile(m_audioFiles[m_audioFileIndex]);
	AudioSyncDatum();
	if (wasPlaying || m_isAudioAuto) MenuOnPlayAudio();
}

void CMainWindow::MenuOnAudioNext()
{
	if (m_audioFiles.empty()) return;
	if (m_audioFileIndex + 1 < m_audioFiles.size())
		m_audioFileIndex++;
	else
		m_audioFileIndex = 0;
	bool wasPlaying = m_spineToolDatum.isAudioPlaying;
	AudioLoadFile(m_audioFiles[m_audioFileIndex]);
	AudioSyncDatum();
	if (wasPlaying || m_isAudioAuto) MenuOnPlayAudio();
}

void CMainWindow::MenuOnToggleAudioAuto()
{
	m_isAudioAuto = !m_isAudioAuto;
	m_spineToolDatum.isAudioAuto = m_isAudioAuto;
}


std::string CMainWindow::ExtractStemUtf8(const std::wstring& path)
{
	size_t pos = path.find_last_of(L"\\/");
	std::wstring filename = (pos != std::wstring::npos) ? path.substr(pos + 1) : path;
	size_t dotPos = filename.find_last_of(L'.');
	if (dotPos != std::wstring::npos) filename = filename.substr(0, dotPos);
	return win_text::NarrowUtf8(filename);
}


int CMainWindow::CreateFlattenedTexture(int srcTex, int r, int g, int b)
{
	int w = 0, h = 0;
	DxLib::GetGraphSize(srcTex, &w, &h);
	int flatTex = DxLib::MakeScreen(w, h, 0);
	if (flatTex == -1) return -1;

	SDxLibRenderTarget rt(flatTex);
	DxLib::DrawBox(0, 0, w, h, DxLib::GetColor(r, g, b), TRUE);
	DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
	DxLib::DrawGraph(0, 0, srcTex, TRUE);
	DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
	return flatTex;
}


void CMainWindow::PrepareQueueRecording(int menuKind)
{
	if (!m_isQueuePlaying || m_animQueue.empty()) return;

	m_isQueueRecording = true;
	m_queueRecordMenuKind = menuKind;
	m_queueIndex = 0;
	m_dxLibSpinePlayer.ActiveRuntime()->setAnimationByName(m_animQueue[0].c_str());
	m_dxLibSpinePlayer.ActiveRuntime()->restartAnimation(false);
}

