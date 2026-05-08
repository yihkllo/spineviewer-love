#include "viewer_window_common.h"

using namespace viewer_window_detail;

void SpineLoveWindow::ToggleToolPanelVisibility()
{
	m_showControlPanel = true;
}

void SpineLoveWindow::ToggleClickThroughViewer()
{
	m_colorKeyTransparency ^= true;
	ApplyWindowShellState(true);
}

void SpineLoveWindow::ToggleDragResizeMode()
{
	m_userResizeEnabled = !m_userResizeEnabled;
	ApplyWindowShellState(true);
}

void SpineLoveWindow::FlipWheelZoomDirection()
{
	m_wheelZoomInverted = !m_wheelZoomInverted;
}

void SpineLoveWindow::MatchWindowToCurrentCanvas()
{
	int viewportWidth = 0;
	int viewportHeight = 0;
	if (!QuerySpineSurfaceSize(viewportWidth, viewportHeight)) return;

	SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	if (runtime == nullptr) return;

	SlVec2 baseSize;
	if (!TryResolveCanvasBaseSize(viewportWidth, viewportHeight, runtime->SkeletonScale(), baseSize))
		return;

	runtime->SetBaseSize(baseSize.x, baseSize.y);
	ResizeToContentBounds();
}

void SpineLoveWindow::RestoreDefaultCanvasSize()
{
	m_runtimeHub.CurrentRuntime()->ClearBaseSize();
	ResizeToContentBounds();
}

void SpineLoveWindow::SetViewerTitle(const wchar_t* pwzTitle)
{
	const std::wstring titleText = NormalizeViewerTitle(pwzTitle);
	::SendMessageW(m_windowHandle, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(titleText.c_str()));
}

std::wstring SpineLoveWindow::ReadViewerTitle() const
{
	const LRESULT textLength = ::SendMessageW(m_windowHandle, WM_GETTEXTLENGTH, 0, 0);
	if (textLength <= 0)
		return {};

	std::wstring text(static_cast<size_t>(textLength) + 1, L'\0');
	const LRESULT copied = ::SendMessageW(
		m_windowHandle,
		WM_GETTEXT,
		static_cast<WPARAM>(text.size()),
		reinterpret_cast<LPARAM>(text.data()));
	text.resize(copied > 0 ? static_cast<size_t>(copied) : 0);
	return text;
}

void SpineLoveWindow::SwitchFrameChrome()
{
	if (!SkeletonIsReady())return;
	m_borderlessChrome ^= true;
	ApplyWindowShellState(true);
	ResizeToContentBounds();
}

void SpineLoveWindow::SwitchFullscreenMode()
{
	if (!m_isFullscreen)
	{
		m_fullscreenReturnState.style = ::GetWindowLongPtrW(m_windowHandle, GWL_STYLE);
		m_fullscreenReturnState.exStyle = ::GetWindowLongPtrW(m_windowHandle, GWL_EXSTYLE);
		::GetWindowRect(m_windowHandle, &m_fullscreenReturnState.bounds);
		m_fullscreenReturnState.valid = true;

		HMONITOR hMon = ::MonitorFromWindow(m_windowHandle, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi{ sizeof(mi) };
		::GetMonitorInfoW(hMon, &mi);
		const RECT& rc = mi.rcMonitor;

		const LONG_PTR fullscreenStyle =
			(m_fullscreenReturnState.style & ~(WS_CAPTION | WS_THICKFRAME | WS_SYSMENU)) | WS_POPUP;
		::SetWindowLongPtrW(m_windowHandle, GWL_STYLE, fullscreenStyle);
		::SetWindowPos(m_windowHandle, m_colorKeyTransparency ? HWND_TOPMOST : HWND_TOP,
			rc.left, rc.top,
			rc.right - rc.left, rc.bottom - rc.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		m_isFullscreen = true;
		m_panelState.isFullscreen = true;
	}
	else
	{
		const SWindowSnapshot returnState = m_fullscreenReturnState;
		if (!returnState.valid)
			return;

		::SetWindowLongPtrW(m_windowHandle, GWL_STYLE, returnState.style);
		::SetWindowLongPtrW(m_windowHandle, GWL_EXSTYLE, returnState.exStyle);
		::SetWindowPos(m_windowHandle, nullptr,
			returnState.bounds.left,
			returnState.bounds.top,
			returnState.bounds.right - returnState.bounds.left,
			returnState.bounds.bottom - returnState.bounds.top,
			SWP_FRAMECHANGED | SWP_NOZORDER);

		m_isFullscreen = false;
		m_panelState.isFullscreen = false;
		m_fullscreenReturnState.valid = false;
	}
}

void SpineLoveWindow::RefreshPanelCommandState()
{
}

bool SpineLoveWindow::SkeletonIsReady() const noexcept
{
	SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	return runtime != nullptr && runtime->ContainsDrawableContent();
}

LONG_PTR SpineLoveWindow::ComposeWindowStyle() const noexcept
{
	LONG_PTR style = WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	if (!m_borderlessChrome)
		style |= WS_CAPTION;
	if (m_userResizeEnabled && !m_isFullscreen)
		style |= WS_THICKFRAME;
	return style;
}

LONG_PTR SpineLoveWindow::ComposeWindowExStyle() const noexcept
{
	LONG_PTR style = WS_EX_ACCEPTFILES;
	if (m_colorKeyTransparency)
		style |= WS_EX_LAYERED;
	return style;
}

void SpineLoveWindow::ApplyWindowShellState(bool keepOuterBounds)
{
	if (m_windowHandle == nullptr)
		return;

	RECT bounds{};
	::GetWindowRect(m_windowHandle, &bounds);
	const int width = bounds.right - bounds.left;
	const int height = bounds.bottom - bounds.top;

	::SetWindowLongPtrW(m_windowHandle, GWL_STYLE, ComposeWindowStyle());
	::SetWindowLongPtrW(m_windowHandle, GWL_EXSTYLE, ComposeWindowExStyle());
	if (m_colorKeyTransparency)
	{
		::SetLayeredWindowAttributes(m_windowHandle, RGB(0, 0, 0), 255, LWA_COLORKEY);
	}

	UINT flags = SWP_FRAMECHANGED | SWP_NOACTIVATE;
	if (!keepOuterBounds)
		flags |= SWP_NOMOVE | SWP_NOSIZE;

	::SetWindowPos(
		m_windowHandle,
		m_colorKeyTransparency ? HWND_TOPMOST : HWND_NOTOPMOST,
		bounds.left,
		bounds.top,
		width,
		height,
		flags);
	::InvalidateRect(m_windowHandle, nullptr, FALSE);
}

void SpineLoveWindow::ResizeWindowClientArea(int clientWidth, int clientHeight)
{
	if (clientWidth <= 0 || clientHeight <= 0 || m_windowHandle == nullptr)
		return;

	RECT client{ 0, 0, clientWidth, clientHeight };
	::AdjustWindowRectEx(&client, static_cast<DWORD>(ComposeWindowStyle()), FALSE, static_cast<DWORD>(ComposeWindowExStyle()));

	RECT current{};
	::GetWindowRect(m_windowHandle, &current);
	const int outerWidth = client.right - client.left;
	const int outerHeight = client.bottom - client.top;
	::SetWindowPos(
		m_windowHandle,
		nullptr,
		current.left,
		current.top,
		outerWidth,
		outerHeight,
		SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

std::wstring SpineLoveWindow::NormalizeViewerTitle(const wchar_t* title) const
{
	if (title == nullptr || *title == L'\0')
		return m_defaultWindowTitle;

	const wchar_t* leaf = ::PathFindFileNameW(title);
	if (leaf == nullptr || *leaf == L'\0')
		return m_defaultWindowTitle;
	return leaf;
}

void SpineLoveWindow::ResizeToContentBounds()
{
	if (m_isFullscreen || !SkeletonIsReady())
		return;

	SlPlaybackRuntime* runtime = m_runtimeHub.CurrentRuntime();
	const SlVec2 baseSize = runtime->BaseSize();
	const float canvasScale = runtime->CanvasScale();
	if (baseSize.x <= 0.0f || baseSize.y <= 0.0f || !std::isfinite(canvasScale) || canvasScale <= 0.0f)
		return;

	const int panelWidth = static_cast<int>(std::lround(m_panelState.leftPanelEndX));
	const int safePanelWidth = panelWidth > 0 ? panelWidth : 0;
	const int scaledWidth = static_cast<int>(std::ceil(baseSize.x * canvasScale));
	const int scaledHeight = static_cast<int>(std::ceil(baseSize.y * canvasScale));
	const int desiredClientWidth = safePanelWidth + (scaledWidth > 1 ? scaledWidth : 1);
	const int desiredClientHeight = scaledHeight > 1 ? scaledHeight : 1;

	HMONITOR monitor = ::MonitorFromWindow(m_windowHandle, MONITOR_DEFAULTTONEAREST);
	MONITORINFO info{ sizeof(info) };
	::GetMonitorInfoW(monitor, &info);
	const int workWidth = info.rcWork.right - info.rcWork.left;
	const int workHeight = info.rcWork.bottom - info.rcWork.top;

	const auto clampInt = [](int value, int low, int high) noexcept
	{
		if (value < low) return low;
		if (value > high) return high;
		return value;
	};
	const int maxWidth = workWidth > 320 ? workWidth : 320;
	const int maxHeight = workHeight > 240 ? workHeight : 240;
	const int clampedWidth = clampInt(desiredClientWidth, 320, maxWidth);
	const int clampedHeight = clampInt(desiredClientHeight, 240, maxHeight);
	ResizeWindowClientArea(clampedWidth, clampedHeight);
}

void SpineLoveWindow::StartQueuePlay()
{
	if (m_animQueue.empty()) return;
	if (!m_runtimeHub.CurrentRuntime()->ContainsDrawableContent()) return;
	m_isQueuePlaying = true;
	m_queueIndex = 0;
	m_runtimeHub.CurrentRuntime()->PlayMotionByName(m_animQueue[0].c_str());
	m_runtimeHub.CurrentRuntime()->RestartMotion(false);
}

void SpineLoveWindow::StopQueuePlay()
{
	m_isQueuePlaying = false;
	m_queueIndex = 0;
}

void SpineLoveWindow::ClearAnimationQueue()
{
	m_isQueuePlaying = false;
	m_queueIndex = 0;
	m_animQueue.clear();
}

void SpineLoveWindow::StepQueuePlay()
{
	if (!m_isQueuePlaying) return;
	if (m_animQueue.empty())
	{
		m_isQueuePlaying = false;
		m_queueIndex = 0;
		return;
	}
	if (m_queueIndex >= m_animQueue.size())
		m_queueIndex = 0;

	float fTrack = 0.f, fEnd = 0.f;
	m_runtimeHub.CurrentRuntime()->ReadMotionClock(&fTrack, nullptr, nullptr, &fEnd);

	if (fEnd > 0.f && ::isgreater(fTrack, fEnd))
	{
		++m_queueIndex;
		if (m_queueIndex >= m_animQueue.size())
		{
			m_queueIndex = 0;
		}
		m_runtimeHub.CurrentRuntime()->PlayMotionByName(m_animQueue[m_queueIndex].c_str());
		m_runtimeHub.CurrentRuntime()->RestartMotion(false);
	}
}
