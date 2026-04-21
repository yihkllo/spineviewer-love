
#include <locale.h>

#include "sl_platform_pch.h"
#include "viewer_window.h"
#include "render_setup.h"
#include "imgui/ui_core.h"
#include "imgui/custom_titlebar.h"

namespace {

float ComputeUiScale()
{
	HDC hdc = ::GetDC(nullptr);
	const int physWidth = ::GetDeviceCaps(hdc, DESKTOPHORZRES);
	::ReleaseDC(nullptr, hdc);

	float scale = physWidth / 1920.0f;
	return (scale < 0.5f) ? 0.5f : scale;
}

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE  ,
	_In_ LPWSTR     ,
	_In_ int       nCmdShow)
{
	::setlocale(LC_ALL, ".utf8");

	CMainWindow mainWindow;
	if (!mainWindow.Create(hInstance, L"spinelove"))
		return 0;

	CDxLibContext dxLibContext(mainWindow.GetHwnd());
	if (!dxLibContext.IsReady())
	{
		::MessageBoxW(nullptr, L"Failed to setup DxLib.", L"Error", MB_ICONERROR);
		return 0;
	}

	const float uiScale = ComputeUiScale();
	custom_titlebar::SetScale(uiScale);

	CUiCore dxLibImgui(nullptr, 16.0f * uiScale);
	if (!dxLibImgui.HasBeenInitialised())
	{
		::MessageBoxW(nullptr, L"Failed to setup ImGui.", L"Error", MB_ICONERROR);
		return 0;
	}

	::ShowWindow(mainWindow.GetHwnd(), SW_HIDE);
	::ShowWindow(mainWindow.GetHwnd(), nCmdShow);
	return mainWindow.MessageLoop();
}
