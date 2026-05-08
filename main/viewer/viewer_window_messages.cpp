#include "viewer_window_common.h"

using namespace viewer_window_detail;

namespace
{
	POINT WheelPointInClient(HWND hwnd, LPARAM lParam) noexcept
	{
		POINT point{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		::ScreenToClient(hwnd, &point);
		return point;
	}

	float ZoomAnchorAfterScale(float anchorPosition, float mousePosition, float scaleRatio) noexcept
	{
		return anchorPosition - (mousePosition - anchorPosition) * (scaleRatio - 1.0f);
	}
}

LRESULT SpineLoveWindow::StaticWindowRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SpineLoveWindow* window = BindWindowInstance(hWnd, uMsg, lParam);
	if (window == nullptr)
		window = WindowInstanceFrom(hWnd);

	if (window != nullptr)
		return window->DispatchWindowEvent(hWnd, uMsg, wParam, lParam);
	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT SpineLoveWindow::DispatchWindowEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))return 1;

	LRESULT routed = 0;
	if (RouteFrameChromeMessage(hWnd, uMsg, wParam, lParam, routed)) return routed;
	if (RouteWindowLifetimeMessage(hWnd, uMsg, wParam, lParam, routed)) return routed;
	if (RouteKeyboardMessage(uMsg, wParam, lParam, routed)) return routed;
	if (RoutePointerMessage(uMsg, wParam, lParam, routed)) return routed;
	if (RouteFileDropMessage(uMsg, wParam, lParam, routed)) return routed;

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

bool SpineLoveWindow::RouteFrameChromeMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
	switch (uMsg)
	{
	case WM_NCCALCSIZE:
		if (m_borderlessChrome && wParam && !::IsZoomed(hWnd))
		{
			result = 0;
			return true;
		}
		return false;
	case WM_NCHITTEST:
	{
		if (!m_borderlessChrome)
			return false;

		POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		::ScreenToClient(hWnd, &pt);
		RECT rc{};
		::GetClientRect(hWnd, &rc);
		const int border = ::GetSystemMetrics(SM_CYSIZEFRAME);
		const bool resizeGripActive = m_userResizeEnabled && !m_isFullscreen;

		if (resizeGripActive && pt.y < border && pt.x < border) result = HTTOPLEFT;
		else if (resizeGripActive && pt.y < border && pt.x >= rc.right - border) result = HTTOPRIGHT;
		else if (resizeGripActive && pt.y >= rc.bottom - border && pt.x < border) result = HTBOTTOMLEFT;
		else if (resizeGripActive && pt.y >= rc.bottom - border && pt.x >= rc.right - border) result = HTBOTTOMRIGHT;
		else if (resizeGripActive && pt.y < border) result = HTTOP;
		else if (resizeGripActive && pt.y >= rc.bottom - border) result = HTBOTTOM;
		else if (resizeGripActive && pt.x < border) result = HTLEFT;
		else if (resizeGripActive && pt.x >= rc.right - border) result = HTRIGHT;
		else if (pt.y < (LONG)custom_titlebar::TitleBarHeight() && pt.x < (rc.right - (LONG)custom_titlebar::BtnAreaWidth())) result = HTCAPTION;
		else result = HTCLIENT;
		return true;
	}
	case WM_NCACTIVATE:
		result = ::DefWindowProcW(hWnd, uMsg, wParam, -1);
		return true;
	case WM_ERASEBKGND:
		result = 1;
		return true;
	default:
		return false;
	}
}

bool SpineLoveWindow::RouteWindowLifetimeMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
	switch (uMsg)
	{
	case WM_CREATE:
		result = AttachWindowState(hWnd);
		return true;
	case WM_DESTROY:
		result = ShutdownWindowState();
		return true;
	case WM_SIZE:
		result = StageClientResize(wParam, lParam);
		return true;
	case WM_EXITSIZEMOVE:
		CommitPendingResize();
		result = 0;
		return true;
	case WM_CLOSE:
		result = RequestWindowClose();
		return true;
	case WM_PAINT:
		result = RenderPaintMessage();
		return true;
	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_INACTIVE)
			::InvalidateRect(hWnd, nullptr, FALSE);
		result = 0;
		return true;
	default:
		return false;
	}
}

bool SpineLoveWindow::RouteKeyboardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
	{
		if (UiOwnsKeyboardInput())
			return false;

		if (wParam == VK_F11)
		{
			SwitchFullscreenMode();
			result = 0;
			return true;
		}

		if (wParam == VK_UP || wParam == VK_DOWN || wParam == VK_LEFT || wParam == VK_RIGHT)
		{
			result = HandleKeyPress(wParam, lParam);
			return true;
		}
		return false;
	}
	case WM_KEYUP:
		result = HandleKeyRelease(wParam, lParam);
		return true;
	case WM_COMMAND:
		result = DispatchPanelCommand(wParam, lParam);
		return true;
	default:
		return false;
	}
}

bool SpineLoveWindow::RoutePointerMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
	switch (uMsg)
	{
	case WM_MOUSEMOVE:
		result = TrackPointerDrag(wParam, lParam);
		return true;
	case WM_MOUSEWHEEL:
		result = ApplyWheelGesture(wParam, lParam);
		return true;
	case WM_LBUTTONDOWN:
		result = BeginPrimaryPointer(wParam, lParam);
		return true;
	case WM_LBUTTONUP:
		result = FinishPrimaryPointer(wParam, lParam);
		return true;
	case WM_RBUTTONUP:
		result = FinishSecondaryPointer(wParam, lParam);
		return true;
	case WM_MBUTTONUP:
		result = FinishMiddlePointer(wParam, lParam);
		return true;
	default:
		return false;
	}
}

bool SpineLoveWindow::RouteFileDropMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
	(void)lParam;
	if (uMsg != WM_DROPFILES)
		return false;

	result = AcceptDroppedSkeletons(wParam, lParam);
	return true;
}

LRESULT SpineLoveWindow::AttachWindowState(HWND hWnd)
{
	m_windowHandle = hWnd;
	m_panelState.titleBar.hWnd = hWnd;
	sl_d3d11::GetOverlayDrawer().SetStateRefreshCallback(nullptr);
	m_panelState.overlayDrawer = &sl_d3d11::GetOverlayDrawer();
	custom_titlebar::SetHandleToSrvProvider(&D3D11TitleBarTextureToSrv);


	RefreshPanelCommandState();

	m_panelState.pSpinePlayer = &m_runtimeHub;
	m_panelState.onOpenFiles = [this]() { BrowseSkeletonFiles(); };
	m_panelState.onOpenFolder = [this]() { BrowseSkeletonFolder(); };
	m_panelState.onShowTool = [this]() { ToggleToolPanelVisibility(); };
	m_panelState.onWindowSettings = [this]() { ToggleClickThroughViewer(); };
	m_panelState.onAllowDragResize = [this]() { ToggleDragResizeMode(); };
	m_panelState.onReverseZoom = [this]() { FlipWheelZoomDirection(); };
	m_panelState.onFitToManualSize = [this]() { MatchWindowToCurrentCanvas(); };
	m_panelState.onFitToDefaultSize = [this]() { RestoreDefaultCanvasSize(); };
	m_panelState.onPickFolder = [this]() {
		if (m_showFavoriteSkelFiles)
		{
			m_showFavoriteSkelFiles = false;
			FavoriteSyncDatum();
		}
		SModalGuard guard(m_isModalOpen);
		::LockWindowUpdate(m_windowHandle);
		std::wstring folder = m_fileDialogs.PickFolder(m_windowHandle);
		::LockWindowUpdate(nullptr);
		::InvalidateRect(m_windowHandle, nullptr, FALSE);
		if (folder.empty()) return;
		RefreshFolderBrowser(folder);
	};
	m_panelState.onPlayFile = [this](const std::wstring& path) {
		auto action = [this, path]() {
		m_panelState.currentSkelFile = path;

		std::string stem = ExtractStemUtf8(path);
		m_panelState.currentFileName = stem;
		m_panelState.titleBar.subTitle = stem;
		StartSkeletonOpenFromPaths({ path });
		};
		if (QueueLoadedSpineReplaceAction(action)) return;
		action();
	};
	m_panelState.onAddSpineFromFile = [this](const std::wstring& path) {
		AddSpineFromPath(path);
	};
	m_panelState.onSelectLoadedSpine = [this](size_t index) {
		SelectLoadedSpine(index);
	};
	m_panelState.onToggleLoadedSpineVisibility = [this](size_t index) {
		ToggleLoadedSpineVisibility(index);
	};
	m_panelState.onMoveLoadedSpineUp = [this](size_t index) {
		MoveLoadedSpineUp(index);
	};
	m_panelState.onMoveLoadedSpineDown = [this](size_t index) {
		MoveLoadedSpineDown(index);
	};
	m_panelState.onConfirmLoadedSpineReplace = [this]() {
		ConfirmLoadedSpineReplaceAction();
	};
	m_panelState.onCancelLoadedSpineReplace = [this]() {
		CancelLoadedSpineReplaceAction();
	};
	m_panelState.onToggleFavoriteView = [this]() {
		m_showFavoriteSkelFiles = !m_showFavoriteSkelFiles;
		FavoriteSyncDatum();
	};
	m_panelState.onToggleFavoriteFile = [this](const std::wstring& path) {
		FavoriteToggleFile(path);
	};
	m_panelState.onOpenFileFolder = [this](const std::wstring& path) {
		size_t slash = path.find_last_of(L"\\/");
		if (slash == std::wstring::npos) return;
		std::wstring folderPath = path.substr(0, slash);
		if (folderPath.empty()) return;
		::ShellExecuteW(m_windowHandle, L"open", folderPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	};
	m_panelState.pAnimQueue = &m_animQueue;
	m_panelState.onQueuePlay = [this]() { StartQueuePlay(); };
	m_panelState.onQueueStop = [this]() { StopQueuePlay(); };
	m_panelState.isQueuePlaying = [this]() { return m_isQueuePlaying; };
	m_panelState.getQueueIndex = [this]() { return m_queueIndex; };
	m_panelState.onPmaChanged = [this](bool pma) { m_userPma = pma; m_userPmaSet = true; };
	m_panelState.onLoadBackground = [this]() { BrowseBackgroundImage(); };
	m_panelState.onExportPng = [this](bool keepAlpha) {
		m_pendingExport = { ExportRequestKind::Snapshot, SlExportImageFormat::Png, SlExportMovieFormat::Mp4, keepAlpha };
	};
	m_panelState.onExportJpeg = [this](bool keepAlpha) {
		m_pendingExport = { ExportRequestKind::Snapshot, SlExportImageFormat::Jpeg, SlExportMovieFormat::Mp4, keepAlpha };
	};
	m_panelState.onExportPngFrames = [this](bool keepAlpha) {
		m_pendingExport = { ExportRequestKind::FrameSequence, SlExportImageFormat::Png, SlExportMovieFormat::Mp4, keepAlpha, m_panelState.exportQueueEnabled };
	};
	m_panelState.onExportJpegFrames = [this](bool keepAlpha) {
		m_pendingExport = { ExportRequestKind::FrameSequence, SlExportImageFormat::Jpeg, SlExportMovieFormat::Mp4, keepAlpha, m_panelState.exportQueueEnabled };
	};
	m_panelState.onExportMp4 = [this](bool keepAlpha) {
		m_pendingExport = { ExportRequestKind::Movie, SlExportImageFormat::Png, SlExportMovieFormat::Mp4, keepAlpha, m_panelState.exportQueueEnabled };
	};
	m_panelState.onExportWebm = [this](bool keepAlpha) {
		m_pendingExport = { ExportRequestKind::Movie, SlExportImageFormat::Png, SlExportMovieFormat::Webm, keepAlpha, m_panelState.exportQueueEnabled };
	};
	m_panelState.onExportGif = [this](bool keepAlpha) {
		m_pendingExport = { ExportRequestKind::Movie, SlExportImageFormat::Png, SlExportMovieFormat::Gif, keepAlpha, m_panelState.exportQueueEnabled };
	};

	m_favoriteCachePath = GetExecutableDirectory() + L"\\spine_favorites.txt";
	FavoriteLoadCache();
	FavoriteSyncDatum();
	m_panelState.onProButtonClicked = [this]() {
		::ShellExecuteW(m_windowHandle, L"open", L"https://yihkllo.com", nullptr, nullptr, SW_SHOWNORMAL);
	};


	m_panelState.onSetRenderBgColor = [this](int r, int g, int b)
	{
		m_d3d11ClearColor = SlVec4(
			static_cast<float>(r) / 255.0f,
			static_cast<float>(g) / 255.0f,
			static_cast<float>(b) / 255.0f,
			1.0f);
	};


	m_panelState.onLoadTitleBg = [this]()
	{
		SModalGuard guard(m_isModalOpen);
		shell_dialogs::FileDialogRequest titleBgRequest{};
		titleBgRequest.title = L"Select title background image";
		titleBgRequest.primaryFilter = { L"Image", L"*.png;*.jpg;*.jpeg;*.bmp" };
		std::wstring path = m_fileDialogs.PickSingleFile(titleBgRequest, m_windowHandle);
		if (!path.empty())
		{
			if (!PrepareD3D11SpineRenderer()) return;
			SlTextureId texture = m_d3d11SpineRenderer.LoadTexture(path.c_str(), false);
			if (texture == 0) return;
			if (m_panelState.titleBar.hBgTexture > 0)
				m_d3d11SpineRenderer.ReleaseTexture(static_cast<SlTextureId>(m_panelState.titleBar.hBgTexture));
			m_panelState.titleBar.hBgTexture = static_cast<int>(texture);
		}
	};


	DragAcceptFiles(m_windowHandle, TRUE);


	::EnumChildWindows(m_windowHandle, [](HWND hwndChild, LPARAM lParam) -> BOOL {
		DragAcceptFiles(hwndChild, TRUE);
		return TRUE;
	}, 0);


	RECT rc;
	::GetClientRect(m_windowHandle, &rc);
	m_pendingClientWidth = rc.right - rc.left;
	m_pendingClientHeight = rc.bottom - rc.top;
	m_hasPendingResize = true;

	return 0;
}

LRESULT SpineLoveWindow::ShutdownWindowState()
{
	::PostQuitMessage(0);

	return 0;
}

LRESULT SpineLoveWindow::RequestWindowClose()
{
	::DestroyWindow(m_windowHandle);
	::UnregisterClassW(m_windowClassName, m_processInstance);

	return 0;
}

LRESULT SpineLoveWindow::RenderPaintMessage()
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_windowHandle, &ps);

	if (m_isModalOpen)
	{
		::EndPaint(m_windowHandle, &ps);
		return 0;
	}

	::EndPaint(m_windowHandle, &ps);

	if (m_runtimeHub.CurrentRuntime()->ContainsDrawableContent())
	{
		RunFrame();
		m_paintTickConsumed = true;
	}

	return 0;
}

LRESULT SpineLoveWindow::StageClientResize(WPARAM wParam, LPARAM lParam)
{
	if (wParam == SIZE_MINIMIZED) return 0;

	m_pendingClientWidth = LOWORD(lParam);
	m_pendingClientHeight = HIWORD(lParam);
	m_hasPendingResize = true;


	if (wParam == SIZE_MAXIMIZED)
	{
		CommitPendingResize();
	}

	
	
	
	DragAcceptFiles(m_windowHandle, TRUE);
	ChangeWindowMessageFilterEx(m_windowHandle, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
	ChangeWindowMessageFilterEx(m_windowHandle, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
	ChangeWindowMessageFilterEx(m_windowHandle, 0x0049, MSGFLT_ALLOW, nullptr);

	return 0;
}

LRESULT SpineLoveWindow::HandleKeyPress(WPARAM wParam, LPARAM lParam)
{

	if (wParam == VK_UP)   { PreviewSkeletonByOffset(-1); return 0; }
	if (wParam == VK_DOWN) { PreviewSkeletonByOffset(+1); return 0; }


	if (wParam == VK_LEFT)  { m_runtimeHub.CurrentRuntime()->StepToPreviousMotion(); return 0; }
	if (wParam == VK_RIGHT) { m_runtimeHub.CurrentRuntime()->StepToNextMotion();     return 0; }

	return 0;
}

LRESULT SpineLoveWindow::HandleKeyRelease(WPARAM wParam, LPARAM lParam)
{

	if (!UiOwnsKeyboardInput())
	{

		if (wParam == VK_UP || wParam == VK_DOWN)
		{
			OpenPreviewedSkeleton();
			return 0;
		}
	}

	if (UiOwnsKeyboardInput())return 0;

	switch (wParam)
	{
	default:
		break;
	}
	return 0;
}

LRESULT SpineLoveWindow::DispatchPanelCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	int wmKind = LOWORD(lParam);
	if (wmKind == 0)
	{

		switch (wmId)
		{
		case PanelCommandId::BrowseFiles:
			BrowseSkeletonFiles();
			break;
		case PanelCommandId::BrowseFolder:
			BrowseSkeletonFolder();
			break;
		case PanelCommandId::ToggleToolPanel:
			ToggleToolPanelVisibility();
			break;
		case PanelCommandId::ClickThroughWindow:
			ToggleClickThroughViewer();
			break;
		case PanelCommandId::DragResizeMode:
			ToggleDragResizeMode();
			break;
		case PanelCommandId::InvertWheelZoom:
			FlipWheelZoomDirection();
			break;
		case PanelCommandId::MatchCanvasSize:
			MatchWindowToCurrentCanvas();
			break;
		case PanelCommandId::RestoreCanvasSize:
			RestoreDefaultCanvasSize();
			break;
		default: break;
		}
	}
	else
	{

	}

	return 0;
}

LRESULT SpineLoveWindow::TrackPointerDrag(WPARAM wParam, LPARAM lParam)
{
	if (m_panelState.isSettingWindowOpen) return 0;
	if (UiOwnsPointerInput()) return 0;
	if (!PointerIsOnStage(m_windowHandle) && m_pointerDragMode == PointerDragMode::Idle) return 0;

	const WORD buttons = LOWORD(wParam);
	const bool leftDown = (buttons & MK_LBUTTON) != 0;
	const bool rightDown = (buttons & MK_RBUTTON) != 0;
	if (!leftDown)
	{
		m_pointerDragMode = PointerDragMode::Idle;
		return 0;
	}

	PointerDragMode nextMode = PointerDragMode::SkeletonPan;
	if (rightDown)
		nextMode = PointerDragMode::WindowMove;
	else if ((::GetKeyState(VK_CONTROL) & 0x8000) && HasBackgroundTexture())
		nextMode = PointerDragMode::BackgroundPan;

	const POINT current = CursorScreenPoint();
	if (m_pointerDragMode != nextMode)
	{
		m_pointerDragMode = nextMode;
		m_lastPointerScreen = current;
		return 0;
	}

	const int deltaX = current.x - m_lastPointerScreen.x;
	const int deltaY = current.y - m_lastPointerScreen.y;
	if (deltaX == 0 && deltaY == 0)
		return 0;

	m_primaryClickCandidate = false;
	switch (m_pointerDragMode)
	{
	case PointerDragMode::SkeletonPan:
		if (m_runtimeHub.CurrentRuntime()->ContainsDrawableContent())
		{
			if ((::GetKeyState(VK_SHIFT) & 0x8000) != 0)
				m_runtimeHub.CurrentRuntime()->PanAllByPixels(deltaX, deltaY);
			else
				m_runtimeHub.CurrentRuntime()->PanByPixels(deltaX, deltaY);
		}
		break;
	case PointerDragMode::BackgroundPan:
		m_bgOffsetX += deltaX;
		m_bgOffsetY += deltaY;
		break;
	case PointerDragMode::WindowMove:
	{
		RECT windowRect{};
		::GetWindowRect(m_windowHandle, &windowRect);
		::SetWindowPos(m_windowHandle, nullptr, windowRect.left + deltaX, windowRect.top + deltaY, 0, 0, SWP_NOSIZE);
		break;
	}
	default:
		break;
	}
	m_lastPointerScreen = current;

	return 0;
}

LRESULT SpineLoveWindow::ApplyWheelGesture(WPARAM wParam, LPARAM lParam)
{
	if (m_panelState.isSettingWindowOpen) return 0;
	if (UiOwnsPointerInput()) return 0;
	if (!PointerIsOnStage(m_windowHandle)) return 0;

	const WORD buttons = LOWORD(wParam);
	if ((buttons & MK_LBUTTON) != 0)
		return 0;

	const short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	WheelTarget target = WheelTarget::None;

	if ((::GetKeyState(VK_CONTROL) & 0x8000) && HasBackgroundTexture())
		target = WheelTarget::BackgroundZoom;
	else if (m_runtimeHub.CurrentRuntime()->ContainsDrawableContent())
		target = WheelTarget::SkeletonZoom;
	else
		return 0;

	if (m_activeWheelTarget != target)
	{
		m_wheelRemainder = 0;
		m_activeWheelTarget = target;
	}

	m_wheelRemainder += wheelDelta;
	const int wheelSteps = m_wheelRemainder / WHEEL_DELTA;
	m_wheelRemainder %= WHEEL_DELTA;
	if (wheelSteps == 0) return 0;

	const int panelX = static_cast<int>(m_panelState.leftPanelEndX);
	const POINT mouseClient = WheelPointInClient(m_windowHandle, lParam);
	const float mouseStageX = static_cast<float>(mouseClient.x - panelX);
	const float mouseStageY = static_cast<float>(mouseClient.y);

	if (target == WheelTarget::BackgroundZoom)
	{
		int bgW = 0, bgH = 0;
		GetBackgroundTextureSize(bgW, bgH);
		if (bgW <= 0 || bgH <= 0) return 0;

		const float oldScale = m_bgScale;
		m_bgScale = ZoomByWheelSteps(m_bgScale, wheelSteps, wheelSteps > 0, 0.05f, 20.f);
		if (oldScale > 0.0f && m_bgScale != oldScale)
		{
			const float scaleRatio = m_bgScale / oldScale;
			m_bgOffsetX = ZoomAnchorAfterScale(m_bgOffsetX, mouseStageX, scaleRatio);
			m_bgOffsetY = ZoomAnchorAfterScale(m_bgOffsetY, mouseStageY, scaleRatio);
		}
	}
	else if (target == WheelTarget::SkeletonZoom)
	{
		static constexpr float kMinScale = 0.1f;
		static constexpr float kMaxScale = 5.f;
		const bool zoomIn = (wheelSteps > 0) ^ m_wheelZoomInverted;
		const bool scaleAllSpines = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;

		const float currentSkeletonScale = m_runtimeHub.CurrentRuntime()->SkeletonScale();
		const float skeletonScale = ZoomByWheelSteps(currentSkeletonScale, wheelSteps, zoomIn, kMinScale, kMaxScale);
		const float scaleRatio = currentSkeletonScale > 0.f ? (skeletonScale / currentSkeletonScale) : 1.f;
		const SlVec2 previousOffset = m_runtimeHub.CurrentRuntime()->ViewOffset();
		const int playW = m_pendingClientWidth - panelX;
		const int playH = m_pendingClientHeight;
		const float previousAnchorX = static_cast<float>(playW) * 0.5f + previousOffset.x;
		const float previousAnchorY = static_cast<float>(playH) * 0.5f + previousOffset.y;

		if (scaleAllSpines)
		{
			const size_t spineCount = m_runtimeHub.CurrentRuntime()->LoadedSkeletonCount();
			for (size_t i = 0; i < spineCount; ++i)
			{
				if (!m_runtimeHub.CurrentRuntime()->SkeletonLayerVisible(i))
					continue;
				const float oldLayerScale = m_runtimeHub.CurrentRuntime()->SkeletonScaleAt(i);
				const float newLayerScale = (std::max)(kMinScale, (std::min)(kMaxScale, oldLayerScale * scaleRatio));
				const float layerScaleRatio = oldLayerScale > 0.0f ? newLayerScale / oldLayerScale : 1.0f;
				const SlVec2 layerOffset = m_runtimeHub.CurrentRuntime()->ViewOffsetAt(i);
				const float layerAnchorX = static_cast<float>(playW) * 0.5f + layerOffset.x;
				const float layerAnchorY = static_cast<float>(playH) * 0.5f + layerOffset.y;
				m_runtimeHub.CurrentRuntime()->SetSkeletonScaleAt(i, newLayerScale);
				if (layerScaleRatio > 0.0f && layerScaleRatio != 1.0f)
				{
					const float newAnchorX = ZoomAnchorAfterScale(layerAnchorX, mouseStageX, layerScaleRatio);
					const float newAnchorY = ZoomAnchorAfterScale(layerAnchorY, mouseStageY, layerScaleRatio);
					m_runtimeHub.CurrentRuntime()->SetViewOffsetAt(
						i,
						newAnchorX - static_cast<float>(playW) * 0.5f,
						newAnchorY - static_cast<float>(playH) * 0.5f);
				}
			}
		}
		else
		{
			m_runtimeHub.CurrentRuntime()->SetSkeletonScale(skeletonScale);

			if (scaleRatio > 0.0f && scaleRatio != 1.0f)
			{
				const float newAnchorX = ZoomAnchorAfterScale(previousAnchorX, mouseStageX, scaleRatio);
				const float newAnchorY = ZoomAnchorAfterScale(previousAnchorY, mouseStageY, scaleRatio);
				m_runtimeHub.CurrentRuntime()->SetViewOffset(
					newAnchorX - static_cast<float>(playW) * 0.5f,
					newAnchorY - static_cast<float>(playH) * 0.5f);
			}
		}
		m_frameClock.Reset();
	}

	return 0;
}

LRESULT SpineLoveWindow::BeginPrimaryPointer(WPARAM wParam, LPARAM lParam)
{
	if (UiOwnsPointerInput())return 0;
	if (!PointerIsOnStage(m_windowHandle)) return 0;

	m_lastPointerScreen = CursorScreenPoint();
	m_pointerDragMode = PointerDragMode::Idle;
	m_primaryClickCandidate = true;

	return 0;
}

LRESULT SpineLoveWindow::FinishPrimaryPointer(WPARAM wParam, LPARAM lParam)
{
	if (UiOwnsPointerInput())return 0;

	if (m_pointerDragMode != PointerDragMode::Idle)
	{
		m_pointerDragMode = PointerDragMode::Idle;
		m_primaryClickCandidate = false;
		return 0;
	}

	WORD usKey = LOWORD(wParam);
	if (usKey == 0 && m_primaryClickCandidate)
	{
		const POINT pt = CursorScreenPoint();
		const int deltaX = m_lastPointerScreen.x - pt.x;
		const int deltaY = m_lastPointerScreen.y - pt.y;

		if (deltaX == 0 && deltaY == 0)
		{
			if (!m_panelState.mouseHoverEnabled && m_runtimeHub.CurrentRuntime()->ContainsDrawableContent())
				m_runtimeHub.CurrentRuntime()->StepToNextMotion();
		}
	}

	m_primaryClickCandidate = false;

	return 0;
}

LRESULT SpineLoveWindow::FinishSecondaryPointer(WPARAM wParam, LPARAM lParam)
{
	if (UiOwnsPointerInput())return 0;

	if (m_pointerDragMode == PointerDragMode::WindowMove)
		m_pointerDragMode = PointerDragMode::Idle;

	return 0;
}

LRESULT SpineLoveWindow::FinishMiddlePointer(WPARAM wParam, LPARAM lParam)
{
	if (UiOwnsPointerInput())return 0;

	WORD usKey = LOWORD(wParam);

	if (usKey == 0)
	{
		if (m_runtimeHub.CurrentRuntime()->ContainsDrawableContent())
		{
			if ((::GetKeyState(VK_SHIFT) & 0x8000) != 0)
				m_runtimeHub.CurrentRuntime()->ResetViewScaleAll();
			else
				m_runtimeHub.CurrentRuntime()->ResetViewScale();
			ResizeToContentBounds();
		}
	}

	return 0;
}

LRESULT SpineLoveWindow::AcceptDroppedSkeletons(WPARAM wParam, LPARAM lParam)
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
			RefreshFolderBrowser(path);
			DragFinish(hDrop);
			return 0;
		}

		if (IsSkeletonFileName(path))
		{
			std::vector<std::wstring> skelFiles = { path };
			StartSkeletonOpenFromPaths(skelFiles);
		}
	}

	DragFinish(hDrop);
	return 0;
}
