
#include "sl_font_config.h"
#include "sl_dlg_builder.h"
#include "../sl_font_provider.h"
#include "../sl_text_codec.h"

#include <imgui.h>

CFontSettingDialogue::CFontSettingDialogue()
{
	const int fh = static_cast<int>(kFontSize * ::GetDpiForSystem() / 96.f);
	m_hFont = ::CreateFont(fh, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
		EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH, L"yumin");
}

CFontSettingDialogue::~CFontSettingDialogue()
{
	if (m_hFont)
		::DeleteObject(m_hFont);
}

HWND CFontSettingDialogue::Open(HINSTANCE hInstance, HWND hWndParent, const wchar_t* pwzWindowName)
{
	CDialogueTemplate tpl;
	tpl.SetWindowSize(160, 100);
	return ::CreateDialogIndirectParam(hInstance,
		reinterpret_cast<LPCDLGTEMPLATE>(tpl.Generate(pwzWindowName)),
		hWndParent, reinterpret_cast<DLGPROC>(DialogProc), reinterpret_cast<LPARAM>(this));
}

LRESULT CFontSettingDialogue::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
		::SetWindowLongPtr(hWnd, DWLP_USER, lParam);

	auto* self = reinterpret_cast<CFontSettingDialogue*>(::GetWindowLongPtr(hWnd, DWLP_USER));
	return self ? self->HandleMessage(hWnd, uMsg, wParam, lParam) : FALSE;
}

LRESULT CFontSettingDialogue::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG: return OnInit(hWnd);
	case WM_SIZE:       return OnSize();
	case WM_CLOSE:      return OnClose();
	case WM_NOTIFY:     return OnNotify(wParam, lParam);
	case WM_COMMAND:    return OnCommand(wParam, lParam);
	case WM_VSCROLL:    return OnVScroll(wParam, lParam);
	default:            return FALSE;
	}
}

LRESULT CFontSettingDialogue::OnInit(HWND hWnd)
{
	m_hWnd = hWnd;

	m_nameLabel.Create(L"Font name", m_hWnd);
	m_nameCombo.Create(m_hWnd);
	m_sizeLabel.Create(L"Size", m_hWnd);
	m_sizeSlider.Create(L"", m_hWnd, reinterpret_cast<HMENU>(kFontSizeSlider), 8, 64, 1);
	m_applyBtn.Create(L"Apply", m_hWnd, reinterpret_cast<HMENU>(kApplyButton));

	CWinFont fontProvider;
	m_nameCombo.Setup(fontProvider.GetSystemFontFamilyNames());

	if (m_lastNameIndex == -1)
	{
		std::wstring locale = fontProvider.FindLocaleFontName(L"\u6E38\u660E\u671D");
		if (!locale.empty())
		{
			int idx = m_nameCombo.FindIndex(locale.c_str());
			if (idx != -1)
			{
				m_nameCombo.SetSelectedItem(idx);
				m_lastNameIndex = idx;
			}
		}
	}
	else
	{
		m_nameCombo.SetSelectedItem(m_lastNameIndex);
	}

	LayoutControls();
	SyncSliderToCurrentSize();
	::EnumChildWindows(m_hWnd, SetFontCallback, reinterpret_cast<LPARAM>(m_hFont));
	return TRUE;
}

LRESULT CFontSettingDialogue::OnClose()
{
	::DestroyWindow(m_hWnd);
	m_hWnd = nullptr;
	return 0;
}

LRESULT CFontSettingDialogue::OnSize()
{
	LayoutControls();
	return 0;
}

LRESULT CFontSettingDialogue::OnNotify(WPARAM, LPARAM) { return 0; }
LRESULT CFontSettingDialogue::OnVScroll(WPARAM, LPARAM) { return 0; }

LRESULT CFontSettingDialogue::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(lParam) != 0 && HIWORD(wParam) != CBN_SELCHANGE)
	{
		if (LOWORD(wParam) == kApplyButton)
			ApplySelectedFont();
	}
	return 0;
}

BOOL CFontSettingDialogue::SetFontCallback(HWND hWnd, LPARAM lParam)
{
	::SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
	return TRUE;
}

void CFontSettingDialogue::LayoutControls()
{
	RECT rc;
	::GetClientRect(m_hWnd, &rc);

	const long cw = rc.right - rc.left;
	const long ch = rc.bottom - rc.top;
	const long gx = cw / 24;
	const long gy = ch / 96;
	const int  fh = static_cast<int>(kFontSize * ::GetDpiForSystem() / 96.f);

	long x = gx, y = gy * 2;
	long w = cw - gx * 2;
	long h = ch * 8 / 10;

	if (m_nameLabel.GetHwnd())  ::MoveWindow(m_nameLabel.GetHwnd(), x, y, w, h, TRUE);
	y += fh;
	if (m_nameCombo.GetHwnd())  ::MoveWindow(m_nameCombo.GetHwnd(), x, y, w, h, TRUE);

	y += ch / 6;
	h = ch / 6;
	::MoveWindow(m_sizeLabel.GetHwnd(),  x, y, w, h, TRUE);
	y += fh;
	::MoveWindow(m_sizeSlider.GetHwnd(), x, y, w, h, TRUE);

	w = cw / 4;
	h = static_cast<int>(fh * 1.5);
	x = cw - w - gx * 2;
	y = ch - h - gy * 2;
	if (m_applyBtn.GetHwnd()) ::MoveWindow(m_applyBtn.GetHwnd(), x, y, w, h, TRUE);
}

void CFontSettingDialogue::ApplySelectedFont()
{
	std::wstring name = m_nameCombo.GetSelectedItemText();
	if (name.empty()) return;

	CWinFont provider;
	auto paths = provider.FindFontFilePaths(name.c_str(), false, false);
	if (paths.empty()) return;

	const float size = static_cast<float>(m_sizeSlider.GetPosition());

	ImFontAtlas* atlas = ImGui::GetIO().Fonts;
	atlas->Clear();
	std::string utf8Path = win_text::NarrowUtf8(paths[0]);
	atlas->AddFontFromFileTTF(utf8Path.c_str(), size, nullptr, atlas->GetGlyphRangesChineseFull());

	ImGui::GetStyle()._NextFrameFontSizeBase = size;
	m_lastNameIndex = m_nameCombo.GetSelectedItemIndex();

	if (onFontApplied)
		onFontApplied(nullptr);
}

void CFontSettingDialogue::SyncSliderToCurrentSize()
{
	auto fontSize = static_cast<long long>(ImGui::GetStyle().FontSizeBase);
	m_sizeSlider.SetPosition(fontSize == 0 ? 20 : fontSize);
}
