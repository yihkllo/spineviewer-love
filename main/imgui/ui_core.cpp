#include "ui_core.h"

#include "ui_fonts.h"
#include "ui_theme.h"

#include <imgui.h>

#if defined _WIN32
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#endif

CUiCore::CUiCore(const InitParams& params)
{
#if defined _WIN32
	if (params.windowHandle == nullptr ||
		params.d3d11Device == nullptr ||
		params.d3d11Context == nullptr)
	{
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	m_imguiContextReady = true;

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ui_theme::ApplyDefaultTheme();
	ui_fonts::InstallDefaultFonts(params.fontPath, params.fontSize);

	m_win32BackendReady = ImGui_ImplWin32_Init(params.windowHandle);
	m_d3d11BackendReady = ImGui_ImplDX11_Init(params.d3d11Device, params.d3d11Context);
	m_bInitialised = m_win32BackendReady && m_d3d11BackendReady;
#else
	(void)params;
#endif
}

CUiCore::~CUiCore()
{
#if defined _WIN32
	if (m_imguiContextReady && ImGui::GetCurrentContext() != nullptr)
	{
		if (m_d3d11BackendReady)
			ImGui_ImplDX11_Shutdown();
		if (m_win32BackendReady)
			ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
#endif
}

bool CUiCore::BeginFrame()
{
#if defined _WIN32
	if (!m_bInitialised || ImGui::GetCurrentContext() == nullptr)
		return false;
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	return true;
#else
	return false;
#endif
}

void CUiCore::SubmitFrame()
{
#if defined _WIN32
	if (!m_bInitialised || ImGui::GetCurrentContext() == nullptr)
		return;
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif
}
