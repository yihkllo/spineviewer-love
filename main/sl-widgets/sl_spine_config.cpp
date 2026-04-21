

#include <Windows.h>
#include <CommCtrl.h>

#include "sl_spine_config.h"

CSpineSettingDialogue::CSpineSettingDialogue()
{
	const int height = static_cast<int>(kFontSize * ::GetDpiForSystem() / 96.f);
	m_hFont = ::CreateFont(height, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
		EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH, L"yumin");
}

CSpineSettingDialogue::~CSpineSettingDialogue()
{
	if (m_hFont)
		::DeleteObject(m_hFont);
}

bool CSpineSettingDialogue::Open(HINSTANCE hInstance, HWND hWnd, const wchar_t* pwzWindowName)
{
	WNDCLASSEXW wc{};
	wc.cbSize        = sizeof(wc);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WindowProc;
	wc.hInstance      = hInstance;
	wc.hCursor        = ::LoadCursorW(nullptr, IDC_ARROW);
	wc.hbrBackground  = ::GetSysColorBrush(COLOR_BTNFACE);
	wc.lpszClassName  = kClassName;

	if (!::RegisterClassExW(&wc)) return false;

	m_hInstance = hInstance;

	const UINT dpi = ::GetDpiForSystem();
	const int w = ::MulDiv(160, dpi, USER_DEFAULT_SCREEN_DPI);
	const int h = ::MulDiv(160, dpi, USER_DEFAULT_SCREEN_DPI);

	RECT rc{};
	::GetClientRect(hWnd, &rc);
	POINT origin{ rc.left, rc.top };
	::ClientToScreen(hWnd, &origin);

	constexpr DWORD style = WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME;
	m_hWnd = ::CreateWindowW(kClassName, pwzWindowName, style,
		origin.x, origin.y, w, h, hWnd, nullptr, hInstance, this);
	if (!m_hWnd) return false;

	RunMessageLoop();
	return true;
}

int CSpineSettingDialogue::RunMessageLoop()
{
	MSG msg;
	for (;;)
	{
		BOOL ret = ::GetMessageW(&msg, 0, 0, 0);
		if (ret <= 0) return (ret == 0) ? static_cast<int>(msg.wParam) : -1;

		if (!::IsDialogMessageW(m_hWnd, &msg))
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}
	}
}

LRESULT CSpineSettingDialogue::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NCCREATE)
	{
		auto* cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
	}

	auto* self = reinterpret_cast<CSpineSettingDialogue*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	return self ? self->HandleMessage(hWnd, uMsg, wParam, lParam) : ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CSpineSettingDialogue::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:  return OnCreate(hWnd);
	case WM_DESTROY: return OnDestroy();
	case WM_CLOSE:   return OnClose();
	case WM_PAINT:   return OnPaint();
	case WM_SIZE:    return OnSize();
	case WM_COMMAND: return OnCommand(wParam, lParam);
	}
	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CSpineSettingDialogue::OnCreate(HWND hWnd)
{
	m_hWnd = hWnd;
	::ShowWindow(hWnd, SW_NORMAL);
	::EnableWindow(::GetWindow(m_hWnd, GW_OWNER), FALSE);

	m_atlasLabel.Create(L"Atlas", m_hWnd);
	m_atlasEdit.Create(m_atlasExt.c_str(), m_hWnd);
	m_skelLabel.Create(L"Skeleton", m_hWnd);
	m_skelEdit.Create(m_skelExt.c_str(), m_hWnd);

	::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));
	return 0;
}

LRESULT CSpineSettingDialogue::OnDestroy()
{
	::PostQuitMessage(0);
	return 0;
}

LRESULT CSpineSettingDialogue::OnClose()
{
	CollectInputs();

	HWND hOwner = ::GetWindow(m_hWnd, GW_OWNER);
	::EnableWindow(hOwner, TRUE);
	::BringWindowToTop(hOwner);

	::DestroyWindow(m_hWnd);
	::UnregisterClassW(kClassName, m_hInstance);
	return 0;
}

LRESULT CSpineSettingDialogue::OnPaint()
{
	PAINTSTRUCT ps;
	::BeginPaint(m_hWnd, &ps);
	::EndPaint(m_hWnd, &ps);
	return 0;
}

LRESULT CSpineSettingDialogue::OnSize()
{
	RECT rc;
	::GetClientRect(m_hWnd, &rc);

	const long cw = rc.right - rc.left;
	const long ch = rc.bottom - rc.top;
	const long gapX = cw / 12;
	const long gapY = ch / 48;
	const long fh = static_cast<long>(kFontSize * ::GetDpiForWindow(m_hWnd) / 96.f);

	const long x = gapX;
	const long w = cw * 3 / 4;
	const long h = fh + gapY;
	long y = gapY * 2;

	::MoveWindow(m_atlasLabel.GetHwnd(), x, y, w, h, TRUE);  y += h;
	::MoveWindow(m_atlasEdit.GetHwnd(),  x, y, w, h, TRUE);  y += h + gapY * 4;
	::MoveWindow(m_skelLabel.GetHwnd(),  x, y, w, h, TRUE);  y += h;
	::MoveWindow(m_skelEdit.GetHwnd(),   x, y, w, h, TRUE);

	return 0;
}

LRESULT CSpineSettingDialogue::OnCommand(WPARAM  , LPARAM  )
{
	return 0;
}

BOOL CSpineSettingDialogue::SetFontCallback(HWND hWnd, LPARAM lParam)
{
	::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
	return TRUE;
}

void CSpineSettingDialogue::CollectInputs()
{
	m_atlasExt = m_atlasEdit.GetText();
	m_skelExt  = m_skelEdit.GetText();
}
