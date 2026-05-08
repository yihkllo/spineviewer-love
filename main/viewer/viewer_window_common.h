#pragma once

#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <Shlwapi.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Shlwapi.lib")

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "../viewer_window.h"

#include "../sl_path_util.h"
#include "../shell_dialog_service.h"
#include "../image_exporter.h"
#include "../sl_text_codec.h"
#include "../sl_overlay.h"

#include "../render_d3d11/d3d11_overlay_drawer.h"
#include "../runtime_shared/slot_mesh_data.h"
#include "../runtime_shared/sl_skeleton_probe.h"

#include "../imgui/ui_core.h"
#include "../imgui/custom_titlebar.h"
#include "../imgui/i18n.h"
#include <imgui.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void* D3D11TitleBarTextureToSrv(int textureHandle);

namespace viewer_window_detail
{
	struct SModalGuard
	{
		bool& flag;
		SModalGuard(bool& f) : flag(f) { flag = true; }
		~SModalGuard() { flag = false; }
	};

	inline bool EndsWithExtension(const std::wstring& path, const wchar_t* extension)
	{
		const size_t extLength = extension ? ::wcslen(extension) : 0;
		if (extLength == 0 || path.size() < extLength)
			return false;
		return ::_wcsicmp(path.c_str() + path.size() - extLength, extension) == 0;
	}

	inline bool IsSkeletonFileName(const std::wstring& path)
	{
		return EndsWithExtension(path, L".json") ||
			EndsWithExtension(path, L".skel");
	}

	inline bool UiOwnsKeyboardInput() noexcept
	{
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	inline bool UiOwnsPointerInput() noexcept
	{
		return ImGui::GetIO().WantCaptureMouse;
	}

	inline bool TryResolveCanvasBaseSize(int viewportWidth, int viewportHeight, float skeletonScale, SlVec2& outBaseSize) noexcept
	{
		outBaseSize = SlVec2(0.0f, 0.0f);
		if (viewportWidth <= 0 || viewportHeight <= 0)
			return false;
		if (!std::isfinite(skeletonScale) || skeletonScale <= 0.0001f)
			return false;

		const float inverseScale = 1.0f / skeletonScale;
		outBaseSize.x = static_cast<float>(viewportWidth) * inverseScale;
		outBaseSize.y = static_cast<float>(viewportHeight) * inverseScale;
		return std::isfinite(outBaseSize.x) && std::isfinite(outBaseSize.y) &&
			outBaseSize.x > 0.0f && outBaseSize.y > 0.0f;
	}

	inline void DrawD3D11SpritePma(sl_d3d11::D3D11Renderer& renderer, SlTextureId texture, const SlRect& dst)
	{
		SlVertex2D vertices[4];
		vertices[0].pos = SlVec3(dst.x, dst.y, 0.0f);
		vertices[1].pos = SlVec3(dst.x + dst.w, dst.y, 0.0f);
		vertices[2].pos = SlVec3(dst.x + dst.w, dst.y + dst.h, 0.0f);
		vertices[3].pos = SlVec3(dst.x, dst.y + dst.h, 0.0f);
		vertices[0].uv = SlVec2(0.0f, 0.0f);
		vertices[1].uv = SlVec2(1.0f, 0.0f);
		vertices[2].uv = SlVec2(1.0f, 1.0f);
		vertices[3].uv = SlVec2(0.0f, 1.0f);
		for (SlVertex2D& vertex : vertices)
			vertex.color = SlColor(1.0f, 1.0f, 1.0f, 1.0f);
		const unsigned short indices[] = { 0, 1, 2, 2, 3, 0 };
		renderer.DrawTriangles(texture, vertices, 4, indices, 6, SlBlendMode::Normal, true);
	}

	inline std::wstring GetExecutableDirectory()
	{
		wchar_t exePath[MAX_PATH] = {};
		::GetModuleFileNameW(nullptr, exePath, MAX_PATH);
		wchar_t* pSlash = ::wcsrchr(exePath, L'\\');
		if (pSlash) *pSlash = L'\0';
		return exePath;
	}

	inline bool FileExistsW(const std::wstring& path)
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

	inline SPlaybackSnapshot CapturePlaybackSnapshot(SlPlaybackRuntime* player)
	{
		SPlaybackSnapshot snapshot{};
		if (player == nullptr)
			return snapshot;

		snapshot.hadLoaded = player->ContainsDrawableContent();
		if (snapshot.hadLoaded)
		{
			snapshot.skeletonScale = player->SkeletonScale();
			snapshot.timeScale = player->TimeScale();
		}
		return snapshot;
	}

	inline void RestorePlaybackSnapshot(SlPlaybackRuntime* player, const SPlaybackSnapshot& snapshot, bool hasLoaded)
	{
		if (player == nullptr || !hasLoaded)
			return;

		if (snapshot.hadLoaded)
		{
			player->SetSkeletonScale(snapshot.skeletonScale);
			player->SetTimeScale(snapshot.timeScale);
		}
		else
		{
			player->SetSkeletonScale(1.f);
		}
	}

	inline POINT CursorScreenPoint()
	{
		POINT pt{};
		::GetCursorPos(&pt);
		return pt;
	}

	inline POINT CursorClientPoint(HWND hwnd)
	{
		POINT pt = CursorScreenPoint();
		::ScreenToClient(hwnd, &pt);
		return pt;
	}

	inline bool PointerIsOnStage(HWND hwnd)
	{
		const POINT client = CursorClientPoint(hwnd);
		return client.x >= 650;
	}

	inline float ZoomByWheelSteps(float value, int wheelSteps, bool positiveZoom, float minValue, float maxValue)
	{
		if (wheelSteps == 0)
			return value;

		const float factor = std::pow(1.05f, static_cast<float>(std::abs(wheelSteps)));
		const float scaled = positiveZoom ? value * factor : value / factor;
		return (std::max)(minValue, (std::min)(maxValue, scaled));
	}

	inline SpineLoveWindow* BindWindowInstance(HWND hwnd, UINT message, LPARAM payload)
	{
		if (message != WM_NCCREATE)
			return nullptr;

		const auto* createStruct = reinterpret_cast<LPCREATESTRUCTW>(payload);
		if (createStruct == nullptr)
			return nullptr;

		auto* window = static_cast<SpineLoveWindow*>(createStruct->lpCreateParams);
		::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
		return window;
	}

	inline SpineLoveWindow* WindowInstanceFrom(HWND hwnd)
	{
		return reinterpret_cast<SpineLoveWindow*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
	}
}
