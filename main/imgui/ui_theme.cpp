#include "ui_theme.h"

#include <imgui.h>

namespace ui_theme {

void ApplyDefaultTheme()
{
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

	style.WindowRounding = 10.0f;
	style.FrameRounding = 6.0f;
	style.GrabRounding = 6.0f;
	style.TabRounding = 6.0f;
	style.PopupRounding = 8.0f;
	style.ScrollbarRounding = 8.0f;
	style.ScrollbarSize = 18.0f;
	style.ItemSpacing = ImVec2(8.0f, 6.0f);
	style.FramePadding = ImVec2(8.0f, 4.0f);
}

}
