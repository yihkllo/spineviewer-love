

#include "ui_core.h"
#include "../sl_path_util.h"
#include "../sl_text_codec.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>
#include <imgui.h>

#if defined _WIN32
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

#ifndef DX_NON_DIRECT3D9
#include <backends/imgui_impl_dx9.h>
#endif
#elif defined __ANDROID__

#elif defined __APPLE__

#endif

#if defined _WIN32
struct DxLibImguiWin32
{
	void (*RenderDrawData)(ImDrawData*) = nullptr;
	void (*NewFrame)(void) = nullptr;
	void (*RendererShutdown)(void) = nullptr;
};
static DxLibImguiWin32 g_dxLibImguiWin32;

class CDxLibImguiImplWin32 abstract final
{
public:
	static bool CreateContext()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = nullptr;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;


		ImGui::StyleColorsLight();

		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;


		colors[ImGuiCol_Text]                 = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_WindowBg]             = ImVec4(0.96f, 0.93f, 1.00f, 1.00f);
		colors[ImGuiCol_ChildBg]              = ImVec4(0.96f, 0.93f, 1.00f, 1.00f);
		colors[ImGuiCol_PopupBg]              = ImVec4(0.96f, 0.93f, 1.00f, 0.98f);
		colors[ImGuiCol_TitleBg]              = ImVec4(0.75f, 0.57f, 0.98f, 1.00f);
		colors[ImGuiCol_TitleBgActive]        = ImVec4(0.65f, 0.40f, 0.96f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.87f, 0.78f, 0.98f, 1.00f);
		colors[ImGuiCol_Button]               = ImVec4(0.71f, 0.52f, 0.96f, 1.00f);
		colors[ImGuiCol_ButtonHovered]        = ImVec4(0.64f, 0.37f, 0.98f, 1.00f);
		colors[ImGuiCol_ButtonActive]         = ImVec4(0.54f, 0.23f, 0.92f, 1.00f);
		colors[ImGuiCol_Header]               = ImVec4(0.80f, 0.65f, 0.98f, 1.00f);
		colors[ImGuiCol_HeaderHovered]        = ImVec4(0.71f, 0.49f, 0.98f, 1.00f);
		colors[ImGuiCol_HeaderActive]         = ImVec4(0.59f, 0.32f, 0.94f, 1.00f);
		colors[ImGuiCol_FrameBg]              = ImVec4(0.90f, 0.83f, 0.99f, 1.00f);
		colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.83f, 0.70f, 0.99f, 1.00f);
		colors[ImGuiCol_FrameBgActive]        = ImVec4(0.72f, 0.53f, 0.97f, 1.00f);
		colors[ImGuiCol_Tab]                  = ImVec4(0.80f, 0.65f, 0.98f, 1.00f);
		colors[ImGuiCol_TabHovered]           = ImVec4(0.68f, 0.45f, 0.98f, 1.00f);
		colors[ImGuiCol_TabSelected]          = ImVec4(0.57f, 0.28f, 0.95f, 1.00f);
		colors[ImGuiCol_SliderGrab]           = ImVec4(0.64f, 0.40f, 0.95f, 1.00f);
		colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.52f, 0.23f, 0.90f, 1.00f);
		colors[ImGuiCol_CheckMark]            = ImVec4(0.52f, 0.23f, 0.90f, 1.00f);
		colors[ImGuiCol_Separator]            = ImVec4(0.71f, 0.52f, 0.96f, 1.00f);
		colors[ImGuiCol_MenuBarBg]            = ImVec4(0.90f, 0.83f, 0.99f, 1.00f);
		colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.94f, 0.90f, 1.00f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.71f, 0.52f, 0.96f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.65f, 0.40f, 0.96f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.56f, 0.27f, 0.92f, 1.00f);


		style.WindowRounding   = 10.0f;
		style.FrameRounding    = 6.0f;
		style.GrabRounding     = 6.0f;
		style.TabRounding      = 6.0f;
		style.PopupRounding    = 8.0f;
		style.ScrollbarRounding = 8.0f;
		style.ScrollbarSize     = 18.0f;
		style.ItemSpacing      = ImVec2(8.0f, 6.0f);
		style.FramePadding     = ImVec2(8.0f, 4.0f);

		return true;
	}

	static bool Initialise()
	{
		bool bRet = ImGui_ImplWin32_Init(DxLib::GetMainWindowHandle());
		if (!bRet)return false;

		int d3Version = DxLib::GetUseDirect3DVersion();
		if (d3Version == DX_DIRECT3D_11)
		{
			g_dxLibImguiWin32.NewFrame = &ImGui_ImplDX11_NewFrame;
			g_dxLibImguiWin32.RenderDrawData = &ImGui_ImplDX11_RenderDrawData;
			g_dxLibImguiWin32.RendererShutdown = &ImGui_ImplDX11_Shutdown;

			ID3D11Device* pD3D11Device = static_cast<ID3D11Device*>(const_cast<void*>(DxLib::GetUseDirect3D11Device()));
			ID3D11DeviceContext* pD3D11DeviceContext = static_cast<ID3D11DeviceContext*>(const_cast<void*>(DxLib::GetUseDirect3D11DeviceContext()));
			return ImGui_ImplDX11_Init(pD3D11Device, pD3D11DeviceContext);
		}
#ifndef DX_NON_DIRECT3D9
		else if (d3Version == DX_DIRECT3D_9 || d3Version == DX_DIRECT3D_9EX)
		{
			g_dxLibImguiWin32.NewFrame = &ImGui_ImplDX9_NewFrame;
			g_dxLibImguiWin32.RenderDrawData = &ImGui_ImplDX9_RenderDrawData;
			g_dxLibImguiWin32.RendererShutdown = &ImGui_ImplDX9_Shutdown;

			IDirect3DDevice9* pD3D9Device = static_cast<IDirect3DDevice9*>(const_cast<void*>(DxLib::GetUseDirect3DDevice9()));
			return ImGui_ImplDX9_Init(pD3D9Device);
		}
#endif
		return false;
	}

	static void NewFrame()
	{
		if (g_dxLibImguiWin32.NewFrame != nullptr)
		{
			g_dxLibImguiWin32.NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}
	}

	static void Render()
	{
		if (g_dxLibImguiWin32.NewFrame != nullptr)
		{
			ImGui::Render();
			g_dxLibImguiWin32.RenderDrawData(ImGui::GetDrawData());
		}
	}

	static void Shutdown()
	{
		if (g_dxLibImguiWin32.RendererShutdown != nullptr)
		{
			g_dxLibImguiWin32.RendererShutdown();
		}
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
};

#endif

namespace
{
	void LoadFonts(const char* primaryPath, float size)
	{
		ImFontAtlas* atlas = ImGui::GetIO().Fonts;


		if (primaryPath != nullptr && *primaryPath != '\0')
			atlas->AddFontFromFileTTF(primaryPath, size, nullptr, atlas->GetGlyphRangesChineseFull());


		ImFontConfig merge;
		merge.MergeMode = true;
		atlas->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", size, &merge, atlas->GetGlyphRangesKorean());
	}
}

CUiCore::CUiCore(const char* defaultFontfilePath, float fontSize)
{
#if defined _WIN32
	m_bInitialised = CDxLibImguiImplWin32::CreateContext();
	m_bInitialised &= CDxLibImguiImplWin32::Initialise();

	std::string bundledFontPath = win_text::NarrowUtf8(path_util::GetBundledFontPath());
	if (!bundledFontPath.empty())
		LoadFonts(bundledFontPath.c_str(), fontSize);
	else if (defaultFontfilePath)
		LoadFonts(defaultFontfilePath, fontSize);
#endif
}

CUiCore::~CUiCore()
{
#if defined _WIN32
	CDxLibImguiImplWin32::Shutdown();
#endif
}

void CUiCore::NewFrame()
{
#if defined _WIN32
	CDxLibImguiImplWin32::NewFrame();
#endif
}

void CUiCore::Render()
{
#if defined _WIN32
	DxLib::RenderVertex();
	CDxLibImguiImplWin32::Render();
#endif
}

void CUiCore::UpdateAndRenderViewPorts()
{

}
