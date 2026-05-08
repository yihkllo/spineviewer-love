#include "ui_fonts.h"

#include "../sl_path_util.h"
#include "../sl_text_codec.h"

#include <imgui.h>
#include <string>

namespace ui_fonts {
namespace {

std::string BundledFontPath()
{
	return sl_text::WideToUtf8(path_util::GetBundledFontPath());
}

const char* ChoosePrimaryFont(const char* fallbackPath, std::string& bundledPath)
{
	bundledPath = BundledFontPath();
	if (!bundledPath.empty())
		return bundledPath.c_str();
	return fallbackPath;
}

void AddChineseGlyphFont(ImFontAtlas& atlas, const char* path, float size)
{
	if (path != nullptr && path[0] != '\0')
		atlas.AddFontFromFileTTF(path, size, nullptr, atlas.GetGlyphRangesChineseFull());
}

void MergeKoreanGlyphs(ImFontAtlas& atlas, float size)
{
	ImFontConfig config;
	config.MergeMode = true;
	atlas.AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", size, &config, atlas.GetGlyphRangesKorean());
}

}

void InstallDefaultFonts(const char* fallbackPath, float size)
{
	ImFontAtlas* atlas = ImGui::GetIO().Fonts;
	if (atlas == nullptr)
		return;

	std::string bundledPath;
	AddChineseGlyphFont(*atlas, ChoosePrimaryFont(fallbackPath, bundledPath), size);
	MergeKoreanGlyphs(*atlas, size);
}

}
