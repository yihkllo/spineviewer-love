
#include <clocale>
#include <cwchar>

#include "sl_platform_pch.h"
#include "viewer_window.h"
#include "imgui/ui_core.h"
#include "imgui/custom_titlebar.h"
#include "render_d3d11/d3d11_test_app.h"
#include "sl_crash_dump.h"

#include <d3d11.h>

namespace
{
	constexpr wchar_t kWindowTitle[] = L"spinelove";
	constexpr wchar_t kD3D11SmokeFlag[] = L"--test-d3d11";

	float ComputeUiScale()
	{
		HDC hdc = ::GetDC(nullptr);
		const int physWidth = ::GetDeviceCaps(hdc, DESKTOPHORZRES);
		::ReleaseDC(nullptr, hdc);

		float scale = physWidth / 1920.0f;
		return (scale < 0.5f) ? 0.5f : scale;
	}

	int StartupError(const wchar_t* message)
	{
		::MessageBoxW(nullptr, message, L"spinelove", MB_ICONERROR);
		return 0;
	}

	CUiCore::InitParams BuildUiParams(SpineLoveWindow& window, float uiScale)
	{
		CUiCore::InitParams params{};
		params.windowHandle = window.NativeWindowHandle();
		params.d3d11Device = window.NativeD3DDevice();
		params.d3d11Context = window.NativeD3DContext();
		params.fontSize = 16.0f * uiScale;
		return params;
	}

	int LaunchViewer(HINSTANCE instance, int showCommand)
	{
		SpineLoveWindow mainWindow;
		if (!mainWindow.OpenNativeWindow(instance, kWindowTitle))
			return 0;

		if (!mainWindow.StartD3D11Runtime())
			return StartupError(L"D3D11 startup failed.");

		const float uiScale = ComputeUiScale();
		custom_titlebar::SetScale(uiScale);

		CUiCore ui(BuildUiParams(mainWindow, uiScale));
		if (!ui.Ready())
			return StartupError(L"UI startup failed.");

		::ShowWindow(mainWindow.NativeWindowHandle(), SW_HIDE);
		::ShowWindow(mainWindow.NativeWindowHandle(), showCommand);
		return mainWindow.RunMainLoop(ui);
	}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
	sl_crash_dump::Install();
	std::setlocale(LC_ALL, ".utf8");

	if (::wcsstr(::GetCommandLineW(), kD3D11SmokeFlag) != nullptr)
		return RunD3D11SpriteSmoke(instance, showCommand);

	return LaunchViewer(instance, showCommand);
}
