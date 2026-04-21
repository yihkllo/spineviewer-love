#ifndef SL_FONT_CONFIG_H_
#define SL_FONT_CONFIG_H_

#include <Windows.h>
#include <functional>
#include <string>

#include "sl_win_controls.h"

class CFontSettingDialogue
{
public:
	CFontSettingDialogue();
	~CFontSettingDialogue();

	CFontSettingDialogue(const CFontSettingDialogue&) = delete;
	CFontSettingDialogue& operator=(const CFontSettingDialogue&) = delete;

	HWND Open(HINSTANCE hInstance, HWND hWndParent, const wchar_t* pwzWindowName);
	HWND GetHwnd() const { return m_hWnd; }


	std::function<void(void*)> onFontApplied;
private:
	static constexpr int kFontSize = 16;

	enum ControlId { kApplyButton = 1, kFontSizeSlider };

	HWND  m_hWnd  = nullptr;
	HFONT m_hFont = nullptr;

	CStatic   m_nameLabel;
	CComboBox m_nameCombo;
	int       m_lastNameIndex = -1;

	CStatic m_sizeLabel;
	CSlider m_sizeSlider;
	CButton m_applyBtn;

	static LRESULT CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
	static BOOL    CALLBACK SetFontCallback(HWND, LPARAM);

	LRESULT HandleMessage(HWND, UINT, WPARAM, LPARAM);
	LRESULT OnInit(HWND hWnd);
	LRESULT OnClose();
	LRESULT OnSize();
	LRESULT OnNotify(WPARAM, LPARAM);
	LRESULT OnCommand(WPARAM, LPARAM);
	LRESULT OnVScroll(WPARAM, LPARAM);

	void LayoutControls();
	void ApplySelectedFont();
	void SyncSliderToCurrentSize();
};

#endif
