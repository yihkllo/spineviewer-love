#include "viewer_window_common.h"

using namespace viewer_window_detail;

namespace
{
	struct InitialWindowRect
	{
		int x = CW_USEDEFAULT;
		int y = CW_USEDEFAULT;
		int width = 1280;
		int height = 720;
	};

	InitialWindowRect ChooseInitialWindowRect()
	{
		const int desktopWidth = (std::max)(1, ::GetSystemMetrics(SM_CXSCREEN));
		const int desktopHeight = (std::max)(1, ::GetSystemMetrics(SM_CYSCREEN));

		InitialWindowRect rect{};
		rect.width = (std::max)(640, desktopWidth * 9 / 10);
		rect.height = (std::max)(480, desktopHeight * 9 / 10);
		rect.x = (desktopWidth - rect.width) / 2;
		rect.y = (desktopHeight - rect.height) / 4;
		return rect;
	}

	bool RegisterSpineLoveWindowClass(HINSTANCE instance, const wchar_t* className, WNDPROC router)
	{
		WNDCLASSW windowClass{};
		windowClass.lpfnWndProc = router;
		windowClass.hInstance = instance;
		windowClass.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
		windowClass.hbrBackground = nullptr;
		windowClass.lpszClassName = className;

		if (::RegisterClassW(&windowClass) != 0)
			return true;
		return ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
	}
}

SpineLoveWindow::SpineLoveWindow()
{

}

SpineLoveWindow::~SpineLoveWindow()
{

}

bool SpineLoveWindow::OpenNativeWindow(HINSTANCE hInstance, const wchar_t* pwzWindowName)
{
	if (hInstance == nullptr)
		return false;
	if (!RegisterSpineLoveWindowClass(hInstance, m_windowClassName, StaticWindowRouter))
		return false;

	m_processInstance = hInstance;
	const wchar_t* windowName = pwzWindowName == nullptr ? m_defaultWindowTitle : pwzWindowName;
	const InitialWindowRect placement = ChooseInitialWindowRect();
	const DWORD windowStyle = static_cast<DWORD>(ComposeWindowStyle());
	const DWORD windowStyleEx = static_cast<DWORD>(ComposeWindowExStyle());

	m_windowHandle = ::CreateWindowExW(
		windowStyleEx,
		m_windowClassName,
		windowName,
		windowStyle,
		placement.x,
		placement.y,
		placement.width,
		placement.height,
		nullptr,
		nullptr,
		hInstance,
		this);
	return m_windowHandle != nullptr;
}

int SpineLoveWindow::RunMainLoop(CUiCore& uiCore)
{
	m_uiCore = &uiCore;

	MSG msg{};
	while (msg.message != WM_QUIT)
	{
		if (!DispatchNextOsMessage(msg))
			break;
		if (msg.message == WM_QUIT)
			break;
		if (::IsIconic(m_windowHandle))
			continue;

		if (m_paintTickConsumed)
		{
			m_paintTickConsumed = false;
			continue;
		}

		RunFrame();
	}

	m_uiCore = nullptr;
	return static_cast<int>(msg.wParam);
}

bool SpineLoveWindow::DispatchNextOsMessage(MSG& msg)
{
	if (::IsIconic(m_windowHandle) != FALSE)
	{
		const BOOL result = ::GetMessageW(&msg, nullptr, 0, 0);
		if (result <= 0)
			return result == 0;

		::TranslateMessage(&msg);
		::DispatchMessageW(&msg);
		return true;
	}

	if (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE) == FALSE)
		return true;
	if (msg.message == WM_QUIT)
		return false;

	::TranslateMessage(&msg);
	::DispatchMessageW(&msg);
	return true;
}
