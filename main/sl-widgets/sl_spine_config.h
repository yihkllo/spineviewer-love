#ifndef SL_SPINE_CONFIG_H_
#define SL_SPINE_CONFIG_H_

#include <Windows.h>
#include <string>

#include "sl_win_controls.h"

class CSpineSettingDialogue
{
public:
	CSpineSettingDialogue();
	~CSpineSettingDialogue();

	CSpineSettingDialogue(const CSpineSettingDialogue&) = delete;
	CSpineSettingDialogue& operator=(const CSpineSettingDialogue&) = delete;

	bool Open(HINSTANCE hInstance, HWND hWnd, const wchar_t* pwzWindowName);
	HWND GetHwnd() const { return m_hWnd; }

	const std::wstring& GetAtlasExtension() const { return m_atlasExt; }
	const std::wstring& GetSkelExtension()  const { return m_skelExt; }

private:
	static constexpr int    kFontSize  = 16;
	static constexpr const wchar_t* kClassName = L"Spine setting dialogue";

	HINSTANCE m_hInstance = nullptr;
	HWND      m_hWnd     = nullptr;
	HFONT     m_hFont    = nullptr;

	CStatic m_atlasLabel;
	CEdit   m_atlasEdit;
	CStatic m_skelLabel;
	CEdit   m_skelEdit;

	std::wstring m_atlasExt = L".atlas";
	std::wstring m_skelExt  = L".skel";

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL    CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);

	int     RunMessageLoop();
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize();
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);

	void CollectInputs();
};

#endif
