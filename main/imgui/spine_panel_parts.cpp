#include "spine_panel_parts.h"

#include "custom_titlebar.h"
#include "../sl_path_util.h"
#include "../sl_text_codec.h"

#include <imgui.h>

#ifdef _WIN32
#include <Windows.h>
#include <backends/imgui_impl_dx11.h>
#endif

namespace
{
#ifdef _WIN32
	std::string GetWindowsFontsDir()
	{
		wchar_t winDir[MAX_PATH]{};
		::GetWindowsDirectoryW(winDir, MAX_PATH);
		std::wstring wDir = winDir;
		if (!wDir.empty() && wDir.back() != L'\\') wDir += L'\\';
		wDir += L"Fonts\\";
		return sl_text::WideToUtf8(wDir);
	}

	void AddMergedFont(ImFontAtlas* atlas, const std::string& path, float size, const ImWchar* ranges, bool merge)
	{
		if (atlas == nullptr || path.empty()) return;

		ImFontConfig config;
		ImFontConfig* configPtr = nullptr;
		if (merge)
		{
			config.MergeMode = true;
			configPtr = &config;
		}
		atlas->AddFontFromFileTTF(path.c_str(), size, configPtr, ranges);
	}
#endif
}

void spine_panel::RebuildPanelFonts(SpinePanelState& panel, float baseSize, const std::string& languageId)
{
#ifdef _WIN32
	ImFontAtlas* atlas = ImGui::GetIO().Fonts;
	if (atlas == nullptr) return;

	const std::string fontsDir = GetWindowsFontsDir();
	const std::wstring bundledFontPath = path_util::GetBundledFontPath();
	const std::string bundledFont = bundledFontPath.empty() ? std::string{} : sl_text::WideToUtf8(bundledFontPath);
	const std::string koreanFont = fontsDir + "malgun.ttf";
	const std::string cjkFont = bundledFont.empty() ? koreanFont : bundledFont;

	atlas->Clear();
	const ImWchar* glyphCJK = atlas->GetGlyphRangesChineseFull();
	const ImWchar* glyphKR = atlas->GetGlyphRangesKorean();
	const bool preferKorean = (languageId == "ko_KR");

	AddMergedFont(atlas, preferKorean ? koreanFont : cjkFont, baseSize, preferKorean ? glyphKR : glyphCJK, false);
	AddMergedFont(atlas, preferKorean ? cjkFont : koreanFont, baseSize, preferKorean ? glyphCJK : glyphKR, true);

	panel.titleBar.customFont = atlas->AddFontFromFileTTF(
		(fontsDir + "segoesc.ttf").c_str(), 40.0f * custom_titlebar::GetScale());
	panel.titleBar.subTitleFont = atlas->AddFontFromFileTTF(
		cjkFont.c_str(), 26.7f * custom_titlebar::GetScale(), nullptr, glyphCJK);

	ImGuiStyle& style = ImGui::GetStyle();
	style._NextFrameFontSizeBase = baseSize;

	ImGui_ImplDX11_InvalidateDeviceObjects();
	ImGui_ImplDX11_CreateDeviceObjects();
#else
	(void)panel;
	(void)baseSize;
	(void)languageId;
#endif
}

void spine_panel::TrackPickState::Sync(size_t itemCount)
{
	if (selected.size() != itemCount)
		selected.assign(itemCount, 0);
}

void spine_panel::TrackPickState::Draw(const std::vector<std::string>& names, const char* listId)
{
	Sync(names.size());

	const float rowHeight = ImGui::GetFrameHeight();
	const float preferredRows = names.size() < 12 ? static_cast<float>(names.size() + 1) : 12.0f;
	const float listHeight = rowHeight * (preferredRows > 4.0f ? preferredRows : 4.0f);
	const float listWidth = ImGui::GetContentRegionAvail().x;

	if (ImGui::BeginChild(listId, ImVec2(listWidth, listHeight), true, ImGuiWindowFlags_NoNav))
	{
		for (size_t i = 0; i < names.size(); ++i)
		{
			ImGui::PushID(static_cast<int>(i));
			bool checked = selected[i] != 0;
			if (ImGui::Checkbox("##trackPick", &checked))
				selected[i] = checked ? 1 : 0;
			ImGui::SameLine();
			if (ImGui::Selectable(names[i].c_str(), checked, 0, ImVec2(ImGui::GetContentRegionAvail().x, rowHeight)))
				selected[i] = selected[i] ? 0 : 1;
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
}

std::vector<std::string> spine_panel::TrackPickState::Collect(const std::vector<std::string>& names) const
{
	std::vector<std::string> picked;
	const size_t count = selected.size() < names.size() ? selected.size() : names.size();
	picked.reserve(count);

	for (size_t i = 0; i < count; ++i)
	{
		if (selected[i] != 0)
			picked.emplace_back(names[i]);
	}
	return picked;
}

void spine_panel::TrackPickState::Reset(size_t itemCount)
{
	selected.assign(itemCount, 0);
}
