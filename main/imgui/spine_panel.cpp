
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <map>

#include "spine_panel.h"
#include "spine_panel_parts.h"
#include "custom_titlebar.h"
#include "../spine_runtime_registry.h"
#include "../sl_overlay.h"
#include "../sl_text_codec.h"


#include <imgui.h>
#include "i18n.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace
{
	inline float CrossSign(float px, float py, float ax, float ay, float bx, float by)
	{
		return (px - bx) * (ay - by) - (ax - bx) * (py - by);
	}

	inline bool PointInTriangle(float px, float py,
		float ax, float ay, float bx, float by, float cx, float cy)
	{
		float d1 = CrossSign(px, py, ax, ay, bx, by);
		float d2 = CrossSign(px, py, bx, by, cx, cy);
		float d3 = CrossSign(px, py, cx, cy, ax, ay);

		bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
		bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

		return !(hasNeg && hasPos);
	}

	bool PointInReadSlotMesh(float px, float py, const ReadSlotMeshData& meshData)
	{
		for (size_t ti = 0; ti < meshData.triangles.size(); ti += 3)
		{
			int i0 = meshData.triangles[ti] * 2;
			int i1 = meshData.triangles[ti + 1] * 2;
			int i2 = meshData.triangles[ti + 2] * 2;

			if (PointInTriangle(px, py,
				meshData.worldVertices[i0], meshData.worldVertices[i0 + 1],
				meshData.worldVertices[i1], meshData.worldVertices[i1 + 1],
				meshData.worldVertices[i2], meshData.worldVertices[i2 + 1]))
			{
				return true;
			}
		}
		return false;
	}

	float PanelScaleForDisplayWidth(float displayWidth)
	{
		float scale = displayWidth > 0.0f ? displayWidth / 1920.0f : 1.0f;
		if (scale < 0.5f) scale = 0.5f;
		if (scale > 3.0f) scale = 3.0f;
		return scale;
	}
}


namespace spine_panel
{
	namespace slot_name_query
	{
		static constexpr int kQueryBufferBytes = 256;
		static char s_queryUtf8[kQueryBufferBytes]{};
		static std::string s_foldedQuery;
		static std::basic_string<bool> s_visibleRows;
		static bool (*s_externalCallback)(const char*, size_t) = nullptr;

		static std::string FoldAsciiForMatch(const char* text, size_t length)
		{
			std::string folded;
			if (text == nullptr || length == 0) return folded;
			folded.reserve(length);
			for (size_t i = 0; i < length; ++i)
			{
				const unsigned char ch = static_cast<unsigned char>(text[i]);
				if (ch >= 'A' && ch <= 'Z')
					folded.push_back(static_cast<char>(ch + ('a' - 'A')));
				else
					folded.push_back(static_cast<char>(ch));
			}
			return folded;
		}

		static void ClearQuery()
		{
			s_queryUtf8[0] = '\0';
			s_foldedQuery.clear();
		}

		static bool IsNameQueryHidden(const char* slotName, size_t slotNameLength)
		{
			if (s_foldedQuery.empty()) return false;
			const std::string foldedSlotName = FoldAsciiForMatch(slotName, slotNameLength);
			return foldedSlotName.find(s_foldedQuery) != std::string::npos;
		}

		static void ApplyQueryToRows(const std::vector<std::string>& slotNames)
		{
			s_foldedQuery = FoldAsciiForMatch(s_queryUtf8, std::strlen(s_queryUtf8));
			if (s_visibleRows.size() != slotNames.size())
				s_visibleRows.assign(slotNames.size(), true);

			for (size_t i = 0; i < slotNames.size(); ++i)
			{
				s_visibleRows[i] = !IsNameQueryHidden(slotNames[i].c_str(), slotNames[i].size());
			}
		}
	};

}

void spine_panel::DrawControlPanel(SpinePanelState& panel, bool* open)
{
	if (open == nullptr)return;

	ImGuiIO& io = ImGui::GetIO();
	static std::string s_pendingLanguageId;

	static float s_currentFontSize = 0.f;
	if (s_currentFontSize <= 0.f)
	{
		HDC hdc = ::GetDC(nullptr);
		int physW = ::GetDeviceCaps(hdc, DESKTOPHORZRES);
		::ReleaseDC(nullptr, hdc);
		float initScale = physW > 0 ? physW / 1920.0f : 1.0f;
		if (initScale < 0.5f) initScale = 0.5f;
		s_currentFontSize = 16.0f * initScale;
	}
	if (!s_pendingLanguageId.empty())
	{
		const std::string applyLangId = s_pendingLanguageId;
		s_pendingLanguageId.clear();
		i18n::select(applyLangId);
#ifdef _WIN32
		{
			RebuildPanelFonts(panel, s_currentFontSize, applyLangId);
		}
#endif
	}
	static bool s_panelsHidden = false;
	const float uiScale = PanelScaleForDisplayWidth(io.DisplaySize.x);
#define S(x) ((x) * uiScale)

	static float s_panelScale = 1.0f;
	const float leftPanelWidth  = S(213.0f * s_panelScale);
	const float rightPanelWidth = S(213.0f * s_panelScale);
	const float tbH = panel.isFullscreen ? 0.f : custom_titlebar::TitleBarHeight();

	if (!panel.isFullscreen)
		custom_titlebar::Draw(panel.titleBar);

	ImGui::SetNextWindowPos(ImVec2(0, tbH));
	ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y - tbH));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::Begin("##MainLayout", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoMouseInputs |
		ImGuiWindowFlags_NoBackground);


	static bool showSettingWindow = false;
	static bool showProWindow = false;
	static bool s_exportPanelOpen = false;
	static bool s_exportEverOpened = false;
	static float s_exportSlideX = 99999.0f;
	static float s_exportPanelHeight = 0.0f;
	static float s_exportReturnButtonX = 99999.0f;
	bool spineLoaded = panel.pSpinePlayer != nullptr &&
		static_cast<SlRuntimeHub*>(panel.pSpinePlayer)->CurrentRuntime() != nullptr;

	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

	if (!s_panelsHidden)
	{
	const float gap = S(5.3f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, S(2.6667f)));
	ImGui::BeginChild("OuterPanel", ImVec2(leftPanelWidth + rightPanelWidth + gap, 0), true);
	ImGui::PopStyleVar();
	ImGui::BeginChild("LeftPanel", ImVec2(leftPanelWidth, -1), false);
	ImGui::Indent(gap);
	ImGui::SetWindowFontScale(1.5f);

	float btnWidth = leftPanelWidth - gap;


	if (ImGui::Button(TR("File"), ImVec2(btnWidth, S(30.0f))))
		if (panel.onOpenFiles) panel.onOpenFiles();


	if (ImGui::Button(TR("Setting"), ImVec2(btnWidth, S(30.0f))))
		showSettingWindow = true;


	if (ImGui::Button(TR("Background"), ImVec2(btnWidth, S(30.0f))))
		if (panel.onLoadBackground) panel.onLoadBackground();


	if (ImGui::Button(TR("Pro"), ImVec2(btnWidth, S(30.0f))))
	{
		showProWindow = true;
	}


	ImGui::Separator();

	if (!spineLoaded) ImGui::BeginDisabled();

	if (spineLoaded)
	{
		SlRuntimeHub* pSpineRegistry = static_cast<SlRuntimeHub*>(panel.pSpinePlayer);
		SlPlaybackRuntime* pSpinePlayer = pSpineRegistry->CurrentRuntime();

		ImGui::SetWindowFontScale(1.2f);

		bool pma = pSpinePlayer->UsesPremultipliedAlpha();
		bool pmaChecked = pma;
		if (ImGui::Checkbox(TR("Alpha premultiplied"), &pmaChecked))
		{
			pSpinePlayer->SetPremultipliedAlpha(pmaChecked);
			if (panel.onPmaChanged) panel.onPmaChanged(pmaChecked);
		}

		{
			bool bResetOffset = pSpinePlayer->ResetsViewOnLoad();
			if (ImGui::Checkbox(TR("Load at (0,0)##load-at-zero-pan"), &bResetOffset))
				pSpinePlayer->SetResetViewOnLoad(bResetOffset);
		}

		ImGui::SeparatorText(TR("Scale"));
		{
			float scale = pSpinePlayer->SkeletonScale();
			int scalePercent = static_cast<int>(scale * 100.f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(TR("Reset")).x - ImGui::GetStyle().FramePadding.x * 2 - ImGui::GetStyle().ItemSpacing.x);
			if (ImGui::SliderInt("##scale", &scalePercent, 10, 500, "%d%%"))
				pSpinePlayer->SetSkeletonScale(scalePercent / 100.f);
			ImGui::SameLine();
			if (ImGui::Button(TR("Reset##scale")))
				pSpinePlayer->SetSkeletonScale(1.f);
		}

		ImGui::SeparatorText(TR("Speed"));
		{
			float timeScale = pSpinePlayer->TimeScale();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(TR("Reset")).x - ImGui::GetStyle().FramePadding.x * 2 - ImGui::GetStyle().ItemSpacing.x);
			if (ImGui::SliderFloat("##timescale", &timeScale, 0.f, 5.f, "%.2fx"))
				pSpinePlayer->SetTimeScale(timeScale);
			ImGui::SameLine();
			if (ImGui::Button(TR("Reset##timescale")))
				pSpinePlayer->SetTimeScale(1.f);
		}

		ImGui::SeparatorText(TR("Mix"));
		{
			static float s_defaultMix = 0.f;
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(TR("Reset")).x - ImGui::GetStyle().FramePadding.x * 2 - ImGui::GetStyle().ItemSpacing.x);
			if (ImGui::SliderFloat("##defaultmix", &s_defaultMix, 0.f, 1.f, "%.1fs"))
				pSpinePlayer->SetBlendWindowSeconds(s_defaultMix);
			ImGui::SameLine();
			if (ImGui::Button(TR("Reset##mix")))
			{
				s_defaultMix = 0.f;
				pSpinePlayer->SetBlendWindowSeconds(0.f);
			}
		}

		ImGui::SetWindowFontScale(1.5f);
	}

	if (!spineLoaded) ImGui::EndDisabled();


	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Text("%s", TR("Animations"));
	ImGui::Separator();

	if (spineLoaded)
	{
		SlRuntimeHub* pSpineRegistry = static_cast<SlRuntimeHub*>(panel.pSpinePlayer);
		SlPlaybackRuntime* pSpinePlayer = pSpineRegistry->CurrentRuntime();
		const std::vector<std::string>& animationNames = pSpinePlayer->MotionNames();

		const std::string& currentAnimName = pSpinePlayer->ActiveMotionName();

		ImGui::BeginChild("AnimationList", ImVec2(0, S(213.0f)), false, ImGuiWindowFlags_NoNav);
		ImGui::SetWindowFontScale(1.5f);
		for (size_t i = 0; i < animationNames.size(); i++) {
			float dur = pSpinePlayer->MotionDuration(animationNames[i].c_str());
			char durStr[32];
			if (dur > 0.f) snprintf(durStr, sizeof(durStr), "%.1fs", dur);
			else durStr[0] = '\0';

			bool isCurrent = (animationNames[i] == currentAnimName);
			float rowWidth = ImGui::GetContentRegionAvail().x;
			float durTextWidth = dur > 0.f ? ImGui::CalcTextSize(durStr).x : 0.f;

			ImGui::PushID(static_cast<int>(i));
			if (ImGui::Selectable("##anim", isCurrent, 0, ImVec2(rowWidth, 0))) {
				if (panel.isQueuePlaying && panel.isQueuePlaying())
					panel.onQueueStop();
				pSpinePlayer->PlayMotionByIndex(i);
			}
			ImGui::SameLine(0, gap);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - rowWidth);
			ImGui::Text("%s", animationNames[i].c_str());
			if (dur > 0.f) {
				ImGui::SameLine(rowWidth - durTextWidth);
				ImGui::Text("%s", durStr);
			}
			ImGui::PopID();
		}
		ImGui::EndChild();


		ImGui::Spacing();
		ImGui::Separator();

		static bool mixMode = false;
		static bool prevMixMode = false;
		static std::vector<int> selectedSkins;
		static int currentSkin = -1;
		
		static std::vector<std::string> cachedMixLookNames;
		static std::string cachedSingleSkinName;
		static bool cachedIsMixMode = false;

		ImGui::Text("%s", TR("Skin"));
		ImGui::SameLine(leftPanelWidth - S(80.0f));
		ImGui::Checkbox(TR("Mix"), &mixMode);
		ImGui::Separator();

		const std::vector<std::string>& skinNames = pSpinePlayer->LookNames();

		static unsigned int prevLoadGeneration = 0;
		bool spineChanged = (panel.loadGeneration != prevLoadGeneration);
		if (spineChanged) {
			prevLoadGeneration = panel.loadGeneration;
			selectedSkins.assign(skinNames.size(), 0);
			currentSkin = -1;

			if (cachedIsMixMode && mixMode) {
				
				std::vector<std::string> skinsToMix;
				for (size_t i = 0; i < skinNames.size(); ++i) {
					for (const auto& cached : cachedMixLookNames) {
						if (skinNames[i] == cached) {
							selectedSkins[i] = 1;
							skinsToMix.push_back(skinNames[i]);
							break;
						}
					}
				}
				if (skinsToMix.empty()) {
					pSpinePlayer->ApplyLookByIndex(0);
				} else if (skinsToMix.size() == 1) {
					for (size_t k = 0; k < skinNames.size(); k++) {
						if (skinNames[k] == skinsToMix[0]) { pSpinePlayer->ApplyLookByIndex(k); break; }
					}
				} else {
					pSpinePlayer->ComposeLooks(skinsToMix);
				}
				prevMixMode = mixMode; 
			} else if (!cachedIsMixMode && !mixMode && !cachedSingleSkinName.empty()) {
				
				for (int i = 0; i < (int)skinNames.size(); ++i) {
					if (skinNames[i] == cachedSingleSkinName) {
						currentSkin = i;
						pSpinePlayer->ApplyLookByIndex(i);
						break;
					}
				}
				prevMixMode = mixMode; 
			} else {
				
				prevMixMode = !mixMode;
			}
		} else if (selectedSkins.size() != skinNames.size()) {
			selectedSkins.assign(skinNames.size(), 0);
			currentSkin = -1;
		}


		if (mixMode != prevMixMode) {
			prevMixMode = mixMode;
			if (mixMode) {

				currentSkin = -1;

				if (!skinNames.empty()) {
					pSpinePlayer->ApplyLookByIndex(0);
				}
			} else {

				for (size_t i = 0; i < selectedSkins.size(); i++) {
					selectedSkins[i] = 0;
				}

				if (!skinNames.empty()) {
					pSpinePlayer->ApplyLookByIndex(0);
				}
			}
		}

		
		cachedIsMixMode = mixMode;
		if (mixMode) {
			cachedMixLookNames.clear();
			for (size_t i = 0; i < skinNames.size(); ++i) {
				if (selectedSkins[i]) cachedMixLookNames.push_back(skinNames[i]);
			}
		} else {
			cachedSingleSkinName = (currentSkin >= 0 && currentSkin < (int)skinNames.size())
				? skinNames[currentSkin] : std::string();
		}

		ImGui::BeginChild("SkinList", ImVec2(0, mixMode ? -30 : 0), false, ImGuiWindowFlags_NoNav);
		ImGui::SetWindowFontScale(1.5f);

		if (mixMode) {
			for (size_t i = 0; i < skinNames.size(); i++) {
				bool checked = selectedSkins[i];
				if (checked)
					ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

				ImGui::PushID(static_cast<int>(i));

				auto toggleSkin = [&]() {
					selectedSkins[i] = !checked;
					std::vector<std::string> skinsToMix;
					for (size_t j = 0; j < skinNames.size(); j++) {
						if (selectedSkins[j]) skinsToMix.push_back(skinNames[j]);
					}
					if (skinsToMix.empty()) {
						pSpinePlayer->ApplyLookByIndex(0);
					} else if (skinsToMix.size() == 1) {
						for (size_t k = 0; k < skinNames.size(); k++) {
							if (skinNames[k] == skinsToMix[0]) { pSpinePlayer->ApplyLookByIndex(k); break; }
						}
					} else {
						pSpinePlayer->ComposeLooks(skinsToMix);
					}
				};

				if (ImGui::Selectable("##mixsel", false, 0, ImVec2(0, 0)))
					toggleSkin();
				ImGui::PopID();

				if (checked) ImGui::PopStyleColor();

				ImGui::SameLine();
				bool displayChecked = checked;
				if (ImGui::Checkbox(skinNames[i].c_str(), &displayChecked))
					toggleSkin();
			}
		} else {

			for (size_t i = 0; i < skinNames.size(); i++) {
				bool isSelected = (currentSkin == static_cast<int>(i));
				if (ImGui::Selectable(skinNames[i].c_str(), isSelected)) {
					if (currentSkin == static_cast<int>(i)) {

						currentSkin = -1;
						if (!skinNames.empty()) {
							pSpinePlayer->ApplyLookByIndex(0);
						}
					} else {

						currentSkin = static_cast<int>(i);
						pSpinePlayer->ApplyLookByIndex(i);
					}
				}
			}
		}
		ImGui::EndChild();
	}

	ImGui::EndChild();

	ImGui::SameLine(0, gap);
	ImGui::BeginChild("RightPanel", ImVec2(rightPanelWidth, -1), false);

	float rightBtnWidth = rightPanelWidth - gap;

	ImGui::SetWindowFontScale(1.5f);
	if (ImGui::Button("<<", ImVec2(rightBtnWidth, S(26.7f))))
		s_panelsHidden = true;

	if (spineLoaded)
	{
		SlRuntimeHub* pRP = static_cast<SlRuntimeHub*>(panel.pSpinePlayer);
		SlPlaybackRuntime* pSP = pRP->CurrentRuntime();

		ImGui::SetWindowFontScale(1.5f);

		if (ImGui::CollapsingHeader(TR("Size/Flip")))
		{
			ImGui::SetWindowFontScale(1.2f);
			ImGui::Text(TR("Window size: (%d, %d)"), panel.iTextureWidth, panel.iTextureHeight);
			const auto& skeletonSize = pSP->SkeletonContentSize();
			const auto& offset = pSP->ViewOffset();
			ImGui::Text(TR("Skeleton size: (%.2f, %.2f)"), skeletonSize.x, skeletonSize.y);
			ImGui::Text(TR("Offset: (%.2f, %.2f)"), offset.x, offset.y);

			{
				ImGui::SetWindowFontScale(1.35f);
				const float availW = ImGui::GetContentRegionAvail().x;
				const float gapW = ImGui::GetStyle().ItemSpacing.x;
				const float btnW = (availW - gapW) * 0.5f;
				if (ImGui::Button(TR("Mirror##flip"), ImVec2(btnW, 0)))
					pSP->ToggleMirrorX();
				ImGui::SameLine();
				if (ImGui::Button(TR("Rotate##flip"), ImVec2(btnW, 0)))
					pSP->RotateClockwise();
			}

			ImGui::SetWindowFontScale(1.5f);
			}
			if (ImGui::CollapsingHeader(TR("Track Mix")))
			{
				const std::vector<std::string>& an = pSP->MotionNames();
				static TrackPickState trackPicker;
				static unsigned int trackPickGeneration = 0;
				if (trackPickGeneration != panel.loadGeneration)
				{
					trackPickGeneration = panel.loadGeneration;
					trackPicker.Reset(an.size());
				}
				trackPicker.Draw(an, "TrackPickList##Tracks2");
				const float availW = ImGui::GetContentRegionAvail().x;
				const float gapW = ImGui::GetStyle().ItemSpacing.x;
				const float totalBtnW = availW * 0.8f;
				const float trackBtnW = (totalBtnW - gapW) * 0.5f;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availW - totalBtnW) * 0.5f);
				if (ImGui::Button(TR("Add##AddTracks2"), ImVec2(trackBtnW, 0))) { pSP->SetLayeredMotions(trackPicker.Collect(an), true); }
				ImGui::SameLine();
				if (ImGui::Button(TR("Clear##ClearTracks2"), ImVec2(trackBtnW, 0))) { trackPicker.Reset(an.size()); pSP->SetLayeredMotions({}); }
			}
			if (ImGui::CollapsingHeader(TR("Slot")))
			{
				const std::vector<std::string>& sn = pSP->SlotCatalog();

				static std::string mouseHoverSlotName;
				static bool mouseHoverClicked = false;
				static bool mouseHoverJustClicked = false;
				std::string listHoveredSlot;
				if (ImGui::TreeNodeEx(TR("Exclude slot by items"), ImGuiTreeNodeFlags_DefaultOpen))
				{
					auto& slotChecks2 = slot_name_query::s_visibleRows;
					static unsigned int s_slotChecksGeneration = 0;
					bool slotChecksNeedReinit = (slotChecks2.size() != sn.size()) || (s_slotChecksGeneration != panel.loadGeneration);
					if (slotChecksNeedReinit)
					{
						s_slotChecksGeneration = panel.loadGeneration;
						slotChecks2.resize(sn.size());
						auto* cb = HasSlotNameQueryFilter() ? GetSlotNameQueryExcludeCallback() : nullptr;
						auto* extCb = slot_name_query::s_externalCallback;
						for (size_t i = 0; i < sn.size(); ++i)
						{
							bool excluded = (cb && cb(sn[i].c_str(), sn[i].size()))
							             || (extCb && extCb(sn[i].c_str(), sn[i].size()));
							slotChecks2[i] = !excluded;
						}
					}

					float slotListMaxH = ImGui::GetTextLineHeightWithSpacing() * 15.f;
					ImVec2 childSz = { ImGui::GetWindowWidth() * 3 / 4.f, slotListMaxH };
					bool changed = false;
					if (ImGui::BeginChild("SlotsExclude2", childSz, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoNav))
					{
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0));
						float rowH = ImGui::GetFrameHeight();
						for (size_t i = 0; i < sn.size(); ++i)
						{
							bool isHighlighted = mouseHoverClicked && !mouseHoverSlotName.empty() && (sn[i] == mouseHoverSlotName);
							if (isHighlighted)
							{
								ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(1.f, 0.5f, 0.5f, 1.f));
								ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.f, 0.5f, 0.5f, 1.f));
								ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(1.f, 0.4f, 0.4f, 1.f));
							}
							bool checked = slotChecks2[i];
							ImGui::PushID(static_cast<int>(i));
							ImVec2 rowMin = ImGui::GetCursorScreenPos();
							if (ImGui::Checkbox("##slotchk", &checked))
							{
								slotChecks2[i] = checked;
								changed = true;
							}
							ImGui::SameLine();
							bool selectableSelected = isHighlighted ? true : !slotChecks2[i];
							if (ImGui::Selectable(sn[i].c_str(), selectableSelected, 0, ImVec2(ImGui::GetContentRegionAvail().x, rowH)))
							{
								slotChecks2[i] = !slotChecks2[i];
								changed = true;
							}
							ImGui::PopID();
							ImVec2 rowMax = ImVec2(ImGui::GetItemRectMax().x, rowMin.y + rowH);
							if (ImGui::IsMouseHoveringRect(rowMin, rowMax))
								listHoveredSlot = sn[i];
							if (isHighlighted)
							{
								ImGui::PopStyleColor(3);
								if (mouseHoverJustClicked) ImGui::SetScrollHereY(0.5f);
							}
						}
						mouseHoverJustClicked = false;
						ImGui::PopStyleVar();
					}
					ImGui::EndChild();

					if (changed)
					{
						std::vector<std::string> excluded;
						for (size_t i = 0; i < sn.size(); ++i)
							if (!slotChecks2[i]) excluded.push_back(sn[i]);
						pSP->SetHiddenSlots(excluded);
						pSP->SetSlotVisibilityRule(nullptr);
						slot_name_query::ClearQuery();
					}

					if (ImGui::Button(TR("Clear##ClearExcSlots2"), ImVec2(S(106.7f), 0)))
					{
						slotChecks2.assign(sn.size(), true);
						pSP->SetHiddenSlots({});
						pSP->SetSlotVisibilityRule(nullptr);
						slot_name_query::ClearQuery();
					}
					ImGui::TreePop();
				}
				if (ImGui::TreeNode(TR("Hide slots by name text")))
				{
					ImGui::InputText(TR("Slot name text"), slot_name_query::s_queryUtf8, sizeof(slot_name_query::s_queryUtf8));
					if (ImGui::Button(TR("Apply##SlotNameQuery2"), ImVec2(S(106.7f), 0)))
					{
						slot_name_query::ApplyQueryToRows(sn);
						std::vector<std::string> excluded;
						for (size_t i = 0; i < sn.size(); ++i)
							if (!slot_name_query::s_visibleRows[i]) excluded.push_back(sn[i]);
						pSP->SetSlotVisibilityRule(nullptr);
						pSP->SetHiddenSlots(excluded);
					}
					ImGui::TreePop();
				}
				if (ImGui::TreeNode(TR("Mouse slot hover")))
				{
					bool& mouseHoverEnabled = panel.mouseHoverEnabled;
					static float mouseHoverThick = 3.2f;
					static ImVec4 mouseHoverColor = ImVec4(0.f, 1.f, 0.f, 1.f);


					SlMatrix4 mat = pSP->ViewTransform();
					mat = SlMatrixMul(mat, SlMatrixTranslate(panel.leftPanelEndX, 0.f, 0.f));

					ImGui::Checkbox(TR("Enable##MouseHover"), &mouseHoverEnabled);
					if (mouseHoverEnabled && !sn.empty())
					{
						SlMatrix4 invMat{};
						if (SlMatrixInverse(mat, invMat))
						{
						ImVec2 mp = ImGui::GetIO().MousePos;
						SlVec3 mLocal = SlVec3Transform(SlVec3(mp.x, mp.y, 0.f), invMat);


						const auto& slotChecksRef = slot_name_query::s_visibleRows;
						std::string hitSlot;
						for (size_t si = 0; si < sn.size(); ++si)
						{

							if (si < slotChecksRef.size() && !slotChecksRef[si]) continue;
							const auto& slotName = sn[si];
							ReadSlotMeshData meshData;
							if (pSP->ReadSlotMesh(slotName, meshData) && meshData.worldVertices.size() >= 2)
							{

								if (PointInReadSlotMesh(mLocal.x, mLocal.y, meshData))
									hitSlot = slotName;
							}
							else
							{

								const auto& b = pSP->MeasureSlotBounds(slotName);
								if (b.z == 0.f) continue;
								float x0 = b.x, y0 = b.y, x1 = b.x + b.z, y1 = b.y + b.w;
								if (x0 > x1) std::swap(x0, x1);
								if (y0 > y1) std::swap(y0, y1);
								if (mLocal.x >= x0 && mLocal.x <= x1 && mLocal.y >= y0 && mLocal.y <= y1)
									hitSlot = slotName;
							}
						}


						bool isMouseOnCanvas = mp.x > panel.leftPanelEndX && mp.x < panel.rightPanelStartX;
						if (isMouseOnCanvas && ImGui::GetIO().MouseClicked[0])
						{
							if (!hitSlot.empty())
							{
								mouseHoverSlotName = hitSlot;
								mouseHoverClicked = true;
								mouseHoverJustClicked = true;
							}
							else
							{

								mouseHoverSlotName.clear();
								mouseHoverClicked = false;
							}
						}


						if (!hitSlot.empty())
							ImGui::Text(TR("Hovered slot: %s"), hitSlot.c_str());
						else
							ImGui::TextDisabled(TR("Hovered slot: (none)"));


						if (mouseHoverClicked && !mouseHoverSlotName.empty())
							ImGui::Text(TR("Pinned slot: %s"), mouseHoverSlotName.c_str());

						if (ImGui::ColorButton(TR("Colour##MH"), mouseHoverColor, ImGuiColorEditFlags_NoAlpha, ImVec2(S(26.7f), S(26.7f))))
							ImGui::OpenPopup("##MHColorPicker");
						if (ImGui::BeginPopup("##MHColorPicker"))
						{
							ImGuiColorEditFlags pickerFlags =
								ImGuiColorEditFlags_NoAlpha        |
								ImGuiColorEditFlags_PickerHueBar   |
								ImGuiColorEditFlags_NoSidePreview  |
								ImGuiColorEditFlags_NoSmallPreview |
								ImGuiColorEditFlags_DisplayRGB     |
								ImGuiColorEditFlags_InputRGB;
							ImGui::ColorPicker3("##mhColorPick", (float*)&mouseHoverColor, pickerFlags);
							ImGui::EndPopup();
						}


						if (!listHoveredSlot.empty())
						{
							unsigned int lcol = ImGui::ColorConvertFloat4ToU32(mouseHoverColor);
							lcol = ((lcol&0xff)<<16)|((lcol&0xff0000)>>16)|(lcol&0xff00ff00);
							ReadSlotMeshData lMeshData;
							if (pSP->ReadSlotMesh(listHoveredSlot, lMeshData) && lMeshData.worldVertices.size() >= 2)
							{
								if (panel.overlayDrawer) { SlReadSlotMeshOutlineRequest req{}; req.meshData = &lMeshData; req.transform = mat; req.color = lcol; req.thickness = mouseHoverThick; panel.overlayDrawer->DrawReadSlotMeshOutline(req); }
							}
							else
							{
								const auto& lb = pSP->MeasureSlotBounds(listHoveredSlot);
								if (lb.z != 0.f && panel.overlayDrawer)
								{
									SlOverlayBox box{};
									box.rect = SlVec4(lb.x, lb.y, lb.z, lb.w);
									box.transform = mat;
									box.color = lcol;
									box.thickness = mouseHoverThick;
									panel.overlayDrawer->DrawBox(box);
								}
							}
						}


						if (!hitSlot.empty())
						{
							unsigned int col = ImGui::ColorConvertFloat4ToU32(mouseHoverColor);
							col = ((col&0xff)<<16)|((col&0xff0000)>>16)|(col&0xff00ff00);
							ReadSlotMeshData hMeshData;
							if (pSP->ReadSlotMesh(hitSlot, hMeshData) && hMeshData.worldVertices.size() >= 2)
							{
								if (panel.overlayDrawer) { SlReadSlotMeshOutlineRequest req{}; req.meshData = &hMeshData; req.transform = mat; req.color = col; req.thickness = mouseHoverThick; panel.overlayDrawer->DrawReadSlotMeshOutline(req); }
							}
							else
							{
								const auto& b = pSP->MeasureSlotBounds(hitSlot);
								if (b.z != 0.f && panel.overlayDrawer)
								{
									SlOverlayBox box{};
									box.rect = SlVec4(b.x, b.y, b.z, b.w);
									box.transform = mat;
									box.color = col;
									box.thickness = mouseHoverThick;
									panel.overlayDrawer->DrawBox(box);
								}
							}
						}

						if (mouseHoverClicked && !mouseHoverSlotName.empty())
						{
							const auto& sb = pSP->MeasureSlotBounds(mouseHoverSlotName);
							if (sb.z != 0.f)
							{
								static bool showSlotBounding = false;
								ImGui::Text(TR("Slot bounding:"));
								ImGui::SameLine();
								ImGui::Checkbox("##ShowSlotBounding", &showSlotBounding);
								if (showSlotBounding)
								{
									ImGui::Text("  X: %.2f  Y: %.2f", sb.x, sb.y);
									ImGui::Text("  W: %.2f  H: %.2f", sb.z, sb.w);
								}
							}
						}
						}
					}
					ImGui::TreePop();
				}
			}

			if (panel.pAnimQueue)
			{
				auto* queue = panel.pAnimQueue;
				const auto& an = pSP->MotionNames();
				bool queuePlaying = panel.isQueuePlaying && panel.isQueuePlaying();
				const bool queueExporting = panel.exportQueueActive && panel.exportRunning;
				const bool queueLocked = queuePlaying || queueExporting;
				size_t queueIdx = queueExporting
					? panel.exportQueueIndex
					: (panel.getQueueIndex ? panel.getQueueIndex() : 0);

				if (ImGui::CollapsingHeader(TR("Queue")))
				{
					static int s_queueSelectIdx = 0;
					if (s_queueSelectIdx >= (int)an.size()) s_queueSelectIdx = 0;
					float addBtnW = 70.f;
					float comboW = ImGui::GetContentRegionAvail().x - addBtnW - 4;
					ImGui::SetNextItemWidth(comboW);
					if (queueLocked) ImGui::BeginDisabled();
					if (ImGui::BeginCombo("##QueueSel", an.empty() ? "" : an[s_queueSelectIdx].c_str()))
					{
						ImGui::SetWindowFontScale(1.3f);
						for (int i = 0; i < (int)an.size(); i++)
						{
							bool sel = (s_queueSelectIdx == i);
							if (ImGui::Selectable(an[i].c_str(), sel)) s_queueSelectIdx = i;
							if (sel) ImGui::SetItemDefaultFocus();
						}
						ImGui::SetWindowFontScale(1.0f);
						ImGui::EndCombo();
					}
				ImGui::SameLine(0, S(2.6667f));
					if (ImGui::Button(TR("+Add"), ImVec2(addBtnW, 0)) && !an.empty())
						queue->push_back(an[s_queueSelectIdx]);
					if (queueLocked) ImGui::EndDisabled();

					float hw = (ImGui::GetContentRegionAvail().x - 4) * 0.5f;
					if (queueLocked) ImGui::BeginDisabled();
					if (ImGui::Button(TR("Play"), ImVec2(hw, 0)))
						if (panel.onQueuePlay) panel.onQueuePlay();
					if (queueLocked) ImGui::EndDisabled();
					ImGui::SameLine(0, S(2.6667f));
					if (!queuePlaying) ImGui::BeginDisabled();
					if (ImGui::Button(TR("Stop"), ImVec2(hw, 0)))
						if (panel.onQueueStop) panel.onQueueStop();
					if (!queuePlaying) ImGui::EndDisabled();

					float delBtnW = 28.f;
					for (int i = 0; i < (int)queue->size(); i++)
					{
						bool isCurrent = (queuePlaying || queueExporting) && ((size_t)i == queueIdx);
						if (isCurrent)
							ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

						float dur = pSP->MotionDuration((*queue)[i].c_str());
						char durStr[16] = "";
						if (dur > 0.f) snprintf(durStr, sizeof(durStr), "%.2fs", dur);
						float durW = dur > 0.f ? ImGui::CalcTextSize(durStr).x + S(2.6667f) : 0.f;
						float availW = ImGui::GetContentRegionAvail().x;

						ImGui::Text("%d  %s", i + 1, (*queue)[i].c_str());
						if (dur > 0.f)
						{
							ImGui::SameLine(availW - durW - delBtnW - S(2.6667f));
							ImGui::TextUnformatted(durStr);
						}
						ImGui::SameLine(availW - delBtnW);
						ImGui::PushID(i);
						if (queueLocked) ImGui::BeginDisabled();
						if (ImGui::Button("X##q", ImVec2(delBtnW, 0)))
						{
							queue->erase(queue->begin() + i);
							--i;
						}
						if (queueLocked) ImGui::EndDisabled();
						ImGui::PopID();
						if (isCurrent) ImGui::PopStyleColor();
					}

					if (queueLocked) ImGui::BeginDisabled();
					if (ImGui::Button(TR("Clear##ClearQueue"), ImVec2(S(106.7f), 0)))
					{
						queue->clear();
						if (panel.onQueueStop) panel.onQueueStop();
					}
					if (queueLocked) ImGui::EndDisabled();
				}
			}

		}

		{
			const bool exportWasOpen = s_exportPanelOpen;
			if (exportWasOpen)
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
			if (ImGui::Button(TR("Export"), ImVec2(rightBtnWidth, S(30.0f))))
			{
				s_exportPanelOpen = !s_exportPanelOpen;
				if (s_exportPanelOpen)
				{
					s_exportEverOpened = true;
					s_exportSlideX = io.DisplaySize.x;
				}
				else
				{
					s_exportReturnButtonX = io.DisplaySize.x;
				}
			}
			if (exportWasOpen)
				ImGui::PopStyleColor();
		}

		ImGui::Separator();
		ImGui::SetWindowFontScale(1.5f);
		bool favViewOpen = panel.showFavoriteSkelFiles;
		if (favViewOpen)
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_TabSelected));
		if (ImGui::Button(TR("Favorites"), ImVec2(rightBtnWidth, S(30.0f))))
		{
			if (panel.onToggleFavoriteView) panel.onToggleFavoriteView();
		}
		if (favViewOpen)
			ImGui::PopStyleColor();
		if (ImGui::Button(TR("Select Folder"), ImVec2(rightBtnWidth, S(30.0f))))
			if (panel.onPickFolder) panel.onPickFolder();
		ImGui::Separator();
		float remainingHeight = ImGui::GetContentRegionAvail().y;
		float minHeight = S(266.7f);
		float fileListHeight = remainingHeight > minHeight ? remainingHeight : minHeight;
		ImGui::BeginChild("FileList", ImVec2(-S(13.3333f), fileListHeight), false,
			ImGuiWindowFlags_NoNav);
		ImGui::SetWindowFontScale(1.2f);
			const auto& fileList = panel.showFavoriteSkelFiles ? panel.favoriteSkelFileList : panel.skelFileList;
			static std::wstring s_lastScrolledFile;
		for (size_t fi = 0; fi < fileList.size(); fi++)
		{
			const std::wstring& wpath = fileList[fi];
			size_t slash = wpath.find_last_of(L"/\\");
			std::wstring wname = (slash != std::wstring::npos) ? wpath.substr(slash + 1) : wpath;
			size_t dot = wname.find_last_of(L'.');
			std::wstring wdisplayName = (dot != std::wstring::npos && dot > 0) ? wname.substr(0, dot) : wname;
			std::string name = sl_text::WideToUtf8(wdisplayName);

			bool isCurrent = (wpath == panel.currentSkelFile);
			bool isLoaded = std::find(panel.loadedSpineFileList.begin(),
				panel.loadedSpineFileList.end(), wpath) != panel.loadedSpineFileList.end();
			bool isFavorite = std::find(panel.favoriteSkelFileList.begin(),
				panel.favoriteSkelFileList.end(), wpath) != panel.favoriteSkelFileList.end();
			std::string label = name + "##f" + std::to_string(fi);
			std::string popupId = "##filePopup" + std::to_string(fi);
			bool popupOpen = ImGui::IsPopupOpen(popupId.c_str());
			const bool showLoadedHighlight = isLoaded && !isCurrent;
			if (showLoadedHighlight)
			{
				ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_TabSelected));
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyleColorVec4(ImGuiCol_TabSelected));
			}
			if (ImGui::Selectable(label.c_str(), isCurrent || popupOpen || showLoadedHighlight))
				if (panel.onPlayFile) panel.onPlayFile(wpath);
			if (showLoadedHighlight)
				ImGui::PopStyleColor(3);

			if (ImGui::BeginPopupContextItem(popupId.c_str()))
			{
				ImGui::SetWindowFontScale(1.5f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, S(4.0f)));

				auto drawPopupButton = [&](const char* id, const char* text, bool drawHeart) -> bool
				{
					const float btnW = S(240.0f);
					const float btnH = S(34.0f);
					bool clicked = ImGui::Selectable(id, false, 0, ImVec2(btnW, btnH));

					ImDrawList* dl = ImGui::GetWindowDrawList();
					ImVec2 min = ImGui::GetItemRectMin();
					ImVec2 max = ImGui::GetItemRectMax();
					ImU32 borderCol = ImGui::GetColorU32(ImGuiCol_Border);
					dl->AddRect(min, max, borderCol, 0.0f, 0, 1.0f);

					const ImVec2 textSize = ImGui::CalcTextSize(text);
					const float textX = min.x + S(10.0f);
					const float textY = min.y + (btnH - textSize.y) * 0.5f;
					dl->AddText(ImVec2(textX, textY), ImGui::GetColorU32(ImGuiCol_Text), text);

					if (drawHeart)
					{
						const char* heart = "*";
						const ImVec2 heartSize = ImGui::CalcTextSize(heart);
						const float heartX = max.x - heartSize.x - S(10.0f);
						const float heartY = min.y + (btnH - heartSize.y) * 0.5f;
						dl->AddText(ImVec2(heartX, heartY), IM_COL32(220, 40, 40, 255), heart);
					}

					return clicked;
				};

				const char* favAction = isFavorite ? TR("Unfavorite") : TR("Favorite");
				if (drawPopupButton("##favoriteAction", favAction, false))
				{
					if (panel.onToggleFavoriteFile) panel.onToggleFavoriteFile(wpath);
					ImGui::CloseCurrentPopup();
				}
				{
					ImDrawList* dl = ImGui::GetWindowDrawList();
					ImVec2 min = ImGui::GetItemRectMin();
					ImVec2 max = ImGui::GetItemRectMax();
					const char* heart = isFavorite ? "*" : "o";
					const float heartFontSize = ImGui::GetFontSize() * 1.25f;
					const ImVec2 heartSizeBase = ImGui::CalcTextSize(heart);
					const ImVec2 heartSize(heartSizeBase.x * 1.25f, heartSizeBase.y * 1.25f);
					const float heartX = max.x - heartSize.x - S(10.0f);
					const float heartY = min.y + (S(34.0f) - heartSize.y) * 0.5f;
					const ImU32 heartColor = isFavorite ? IM_COL32(220, 40, 40, 255) : IM_COL32(210, 210, 210, 255);
					dl->AddText(ImGui::GetFont(), heartFontSize, ImVec2(heartX, heartY), heartColor, heart);
				}
				if (drawPopupButton("##openContainingFolder", TR("Open Containing Folder"), false))
				{
					if (panel.onOpenFileFolder) panel.onOpenFileFolder(wpath);
					ImGui::CloseCurrentPopup();
				}
				if (drawPopupButton("##addSpine", TR("Add Spine"), false))
				{
					if (panel.onAddSpineFromFile) panel.onAddSpineFromFile(wpath);
					ImGui::CloseCurrentPopup();
				}

				ImGui::PopStyleVar(3);
				ImGui::EndPopup();
			}


			if (isCurrent && s_lastScrolledFile != wpath)
			{
				s_lastScrolledFile = wpath;
				float itemMinY = ImGui::GetItemRectMin().y;
				float itemMaxY = ImGui::GetItemRectMax().y;
				float winMinY  = ImGui::GetWindowPos().y;
				float winMaxY  = winMinY + ImGui::GetWindowSize().y;
				if (itemMinY < winMinY)
					ImGui::SetScrollHereY(0.0f);
				else if (itemMaxY > winMaxY)
					ImGui::SetScrollHereY(1.0f);
			}


			if (ImGui::IsItemHovered())
			{

				size_t slash2 = (slash != std::wstring::npos && slash > 0) ? wpath.find_last_of(L"/\\", slash - 1) : std::wstring::npos;
				std::wstring wfolder = (slash2 != std::wstring::npos) ? wpath.substr(slash2 + 1, slash - slash2 - 1) : wpath.substr(0, slash);
				std::string folder = sl_text::WideToUtf8(wfolder);
				if (!folder.empty()) ImGui::SetTooltip("%s", folder.c_str());
			}
		}
		ImGui::EndChild();

		ImGui::EndChild();
	ImGui::EndChild();

	}

	ImGui::PopStyleVar();
	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	if (panel.showLoadedSpinePanel && !panel.loadedSpineNames.empty())
	{
		static bool s_loadedSpinesOverlayInit = false;
		static ImVec2 s_loadedSpinesOverlayPos;
		const float sceneWindowW = S(286.0f);
		const float itemHeight = S(32.0f);
		const float itemSpacingY = ImGui::GetStyle().ItemSpacing.y;
		const size_t visibleCount = (std::min)(size_t(8), panel.loadedSpineNames.size());
		const float listHeight =
			static_cast<float>(visibleCount) * itemHeight +
			(static_cast<float>(visibleCount > 0 ? visibleCount - 1 : 0) * itemSpacingY);
		const float sceneWindowH = S(66.0f) + listHeight;

		if (!s_loadedSpinesOverlayInit)
		{
			s_loadedSpinesOverlayPos = ImVec2(io.DisplaySize.x - sceneWindowW - S(10.0f), tbH + S(10.0f));
			s_loadedSpinesOverlayInit = true;
		}

		ImGui::SetNextWindowPos(s_loadedSpinesOverlayPos, ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(sceneWindowW, sceneWindowH), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.95f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
		if (ImGui::Begin("##LoadedSpinesOverlay", nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings))
		{
			s_loadedSpinesOverlayPos = ImGui::GetWindowPos();
			ImGui::SetWindowFontScale(1.5f);

			ImVec4 headerColor = ImGui::GetStyleColorVec4(ImGuiCol_Header);
			ImVec4 headerHovered = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);
			ImVec4 headerActive = ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive);
			ImGui::PushStyleColor(ImGuiCol_Button, headerColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, headerHovered);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, headerActive);

			ImGui::SeparatorText(TR("Spines"));
			ImGui::BeginChild("##LoadedSpinesOverlayList", ImVec2(0.f, listHeight), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			for (size_t i = 0; i < panel.loadedSpineNames.size(); ++i)
			{
				const bool selected = static_cast<int>(i) == panel.selectedLoadedSpine;
				const bool isVisible = i < panel.loadedSpineVisibility.size() ? panel.loadedSpineVisibility[i] : true;
				const float layerWidth = S(46.0f);
				const float toggleWidth = S(32.0f);
				const float selectWidth = ImGui::GetContentRegionAvail().x - layerWidth - toggleWidth - S(12.0f);
				const float arrowWidth = S(20.0f);
				const float arrowHeight = itemHeight;
				const float arrowFontSize = ImGui::GetFontSize() * 1.2f;
				const float rowStartY = ImGui::GetCursorPosY();
				ImGui::BeginGroup();
				ImGui::BeginDisabled(i == 0);
				if (ImGui::Button(("##loaded_spine_up_" + std::to_string(i)).c_str(), ImVec2(arrowWidth, arrowHeight)))
				{
					if (panel.onMoveLoadedSpineUp)
						panel.onMoveLoadedSpineUp(i);
				}
				{
					ImDrawList* dl = ImGui::GetWindowDrawList();
					ImVec2 min = ImGui::GetItemRectMin();
					ImVec2 max = ImGui::GetItemRectMax();
					const char* arrow = "^";
					const ImVec2 textSize = ImGui::CalcTextSize(arrow);
					const float textX = min.x + (max.x - min.x - textSize.x * 1.2f) * 0.5f;
					const float textY = min.y + (max.y - min.y - textSize.y * 1.2f) * 0.5f;
					dl->AddText(ImGui::GetFont(), arrowFontSize, ImVec2(textX, textY), ImGui::GetColorU32(ImGuiCol_Text), arrow);
				}
				ImGui::EndDisabled();
				ImGui::SameLine(0.0f, S(2.0f));
				ImGui::BeginDisabled(i + 1 >= panel.loadedSpineNames.size());
				if (ImGui::Button(("##loaded_spine_down_" + std::to_string(i)).c_str(), ImVec2(arrowWidth, arrowHeight)))
				{
					if (panel.onMoveLoadedSpineDown)
						panel.onMoveLoadedSpineDown(i);
				}
				{
					ImDrawList* dl = ImGui::GetWindowDrawList();
					ImVec2 min = ImGui::GetItemRectMin();
					ImVec2 max = ImGui::GetItemRectMax();
					const char* arrow = "v";
					const ImVec2 textSize = ImGui::CalcTextSize(arrow);
					const float textX = min.x + (max.x - min.x - textSize.x * 1.2f) * 0.5f;
					const float textY = min.y + (max.y - min.y - textSize.y * 1.2f) * 0.5f;
					dl->AddText(ImGui::GetFont(), arrowFontSize, ImVec2(textX, textY), ImGui::GetColorU32(ImGuiCol_Text), arrow);
				}
				ImGui::EndDisabled();
				ImGui::EndGroup();
				ImGui::SameLine(0.0f, S(6.0f));
				ImGui::SetCursorPosY(rowStartY);
				if (selected)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				}
				if (!isVisible)
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.55f);

				const std::string loadedSpineButtonId = "##loaded_spine_" + std::to_string(i);
				if (ImGui::Button(loadedSpineButtonId.c_str(), ImVec2(selectWidth, itemHeight)))
				{
					if (panel.onSelectLoadedSpine)
						panel.onSelectLoadedSpine(i);
				}
				{
					ImDrawList* dl = ImGui::GetWindowDrawList();
					const ImVec2 btnMin = ImGui::GetItemRectMin();
					const ImVec2 btnMax = ImGui::GetItemRectMax();
					const float nameFontSize = ImGui::GetFontSize() * 1.15f;
					const char* label = panel.loadedSpineNames[i].c_str();
					const ImVec2 textSize = ImGui::CalcTextSize(label);
					const float textX = btnMin.x + (selectWidth - textSize.x) * 0.5f;
					const float textY = btnMin.y + (itemHeight - nameFontSize) * 0.5f;
					dl->PushClipRect(btnMin, btnMax, true);
					dl->AddText(ImGui::GetFont(), nameFontSize, ImVec2(textX, textY), ImGui::GetColorU32(ImGuiCol_Text), label);
					dl->PopClipRect();
				}
				if (!isVisible)
					ImGui::PopStyleVar();

				if (selected)
					ImGui::PopStyleColor(3);

				ImGui::SameLine(0.0f, S(6.0f));
				bool visibleState = isVisible;
				ImGui::SetCursorPosY(rowStartY + (itemHeight - ImGui::GetFrameHeight()) * 0.5f);
				if (ImGui::Checkbox(("##loaded_spine_toggle_" + std::to_string(i)).c_str(), &visibleState))
				{
					if (panel.onToggleLoadedSpineVisibility)
						panel.onToggleLoadedSpineVisibility(i);
				}
			}
			ImGui::EndChild();

			ImGui::PopStyleColor(3);
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}


	if (!s_panelsHidden)
	{
		static bool s_splitterDragging = false;
		const float splitterW = S(10.0f);
		const float tbH2 = panel.isFullscreen ? 0.f : custom_titlebar::TitleBarHeight();

		const float splX = leftPanelWidth + rightPanelWidth + S(5.3f);
		const ImVec2 mp = io.MousePos;
		const bool onSplitter = (mp.x >= splX - splitterW * 0.5f &&
		                          mp.x <= splX + splitterW * 0.5f &&
		                          mp.y >= tbH2);
		if (io.MouseClicked[0] && onSplitter)
			s_splitterDragging = true;
		if (!io.MouseDown[0])
			s_splitterDragging = false;
		if (onSplitter || s_splitterDragging)
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		if (s_splitterDragging && io.MouseDelta.x != 0.0f)
		{

			const float totalRaw = (213.0f * s_panelScale * 2.0f) + io.MouseDelta.x / uiScale;
			s_panelScale = totalRaw / (213.0f * 2.0f);
			if (s_panelScale < 0.2f) s_panelScale = 0.2f;
			if (s_panelScale > 3.0f) s_panelScale = 3.0f;
		}
	}


	panel.leftPanelEndX = s_panelsHidden ? 0.f : (leftPanelWidth + rightPanelWidth + S(5.3333f));




	if (s_panelsHidden)
	{
		const float btnW = S(200.0f);
		const float btnH = S(33.3f);
		const float hiddenX = -btnW;
		const float shownX  = S(2.6667f);
		const float triggerZone = S(200.0f);
		const float slideSpeed = 8.f;

		static float s_slideX = hiddenX;

		float mouseX = io.MousePos.x;
		float mouseY = io.MousePos.y;
		bool inTrigger = (mouseX >= 0 && mouseX < triggerZone &&
		                  mouseY >= tbH && mouseY < io.DisplaySize.y);

		float target = inTrigger ? shownX : hiddenX;
		float t = slideSpeed * io.DeltaTime;
		if (t > 1.f) t = 1.f;
		s_slideX += (target - s_slideX) * t;


		if (s_slideX > hiddenX + 1.f)
		{
			ImGuiWindowFlags extraFlags = (s_slideX < shownX - btnW * 0.8f)
				? ImGuiWindowFlags_NoInputs : 0;

			ImGui::SetNextWindowPos(ImVec2(s_slideX, tbH + S(2.6667f)), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(btnW + S(2.6667f), btnH + S(2.6667f)));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(1.3333f), S(1.3333f)));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::Begin("##TogglePanelBtn", nullptr,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoBackground | extraFlags);
			if (ImGui::Button(">>", ImVec2(btnW, btnH)))
			{
				s_panelsHidden = false;
				s_slideX = hiddenX;
			}
			ImGui::End();
			ImGui::PopStyleVar(2);
		}
	}

	if (!spineLoaded)
	{
		s_exportPanelOpen = false;
	}

	const float exportPanelWidth = S(230.0f);
	if (s_exportPanelOpen && spineLoaded)
	{
		const float shownX = io.DisplaySize.x - exportPanelWidth;
		const float hiddenX = io.DisplaySize.x;
		if (s_exportSlideX > hiddenX)
			s_exportSlideX = hiddenX;
		const float slideStep = (std::min)(1.0f, 10.0f * io.DeltaTime);
		s_exportSlideX += (shownX - s_exportSlideX) * slideStep;

		const float availableHeight = io.DisplaySize.y - tbH;
		const float exportY = tbH + (s_exportPanelHeight > 0.0f
			? (availableHeight - s_exportPanelHeight) * 0.5f
			: availableHeight * 0.25f);

		ImGui::SetNextWindowPos(ImVec2(s_exportSlideX, exportY), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(exportPanelWidth, 0), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.95f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(10.0f), S(10.0f)));
		ImGui::Begin("##ExportPanel", nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings);

		ImGui::SetWindowFontScale(1.5f);
		if (ImGui::Button(">>", ImVec2(-1, S(30.0f))))
		{
			s_exportPanelOpen = false;
			s_exportReturnButtonX = io.DisplaySize.x;
		}

		if (panel.exportRunning || !panel.exportStatus.empty())
		{
			ImGui::Spacing();
			ImGui::SetWindowFontScale(1.15f);
			ImGui::TextWrapped("%s", panel.exportStatus.c_str());
			const float progress = panel.exportTotal > 0
				? static_cast<float>(panel.exportDone) / static_cast<float>(panel.exportTotal)
				: 0.0f;
			ImGui::ProgressBar((std::max)(0.0f, (std::min)(1.0f, progress)), ImVec2(-1, S(18.0f)));
			ImGui::SetWindowFontScale(1.5f);
		}

		ImGui::Spacing();
		ImGui::SeparatorText(TR("Snapshot"));
		const float gapW = S(4.0f);
		const float halfW = (ImGui::GetContentRegionAvail().x - gapW) * 0.5f;
		if (panel.exportRunning)
			ImGui::BeginDisabled();
		if (ImGui::Button("PNG", ImVec2(halfW, S(30.0f))))
		{
			if (panel.onExportPng)
				panel.onExportPng(panel.exportAlpha);
		}
		ImGui::SameLine(0.0f, gapW);
		if (ImGui::Button("JPG", ImVec2(halfW, S(30.0f))))
		{
			if (panel.onExportJpeg)
				panel.onExportJpeg(false);
		}
		if (panel.exportRunning)
			ImGui::EndDisabled();

		const bool alphaWasEnabled = panel.exportAlpha;
		if (alphaWasEnabled)
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		if (ImGui::Button(panel.exportAlpha ? TR("Alpha ON") : TR("Alpha OFF"), ImVec2(-1, S(30.0f))))
			panel.exportAlpha = !panel.exportAlpha;
		if (alphaWasEnabled)
			ImGui::PopStyleColor();

		const bool queueWasEnabled = panel.exportQueueEnabled;
		if (queueWasEnabled)
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		if (ImGui::Button(panel.exportQueueEnabled ? TR("Queue ON") : TR("Queue OFF"), ImVec2(-1, S(30.0f))))
			panel.exportQueueEnabled = !panel.exportQueueEnabled;
		if (queueWasEnabled)
			ImGui::PopStyleColor();

		ImGui::Spacing();
		ImGui::SeparatorText(TR("Export FPS"));
		auto fpsRow = [&](const char* id, int& fpsValue, const char* label)
		{
			ImGui::PushID(id);
			fpsValue = (std::max)(1, (std::min)(120, fpsValue));
			ImGui::SetNextItemWidth(S(78.0f));
			if (ImGui::InputInt("##value", &fpsValue, 0, 0))
				fpsValue = (std::max)(1, (std::min)(120, fpsValue));
			ImGui::SameLine(0.0f, gapW);
			if (ImGui::Button("-", ImVec2(S(34.0f), S(30.0f))) && fpsValue > 1)
				--fpsValue;
			ImGui::SameLine(0.0f, gapW);
			if (ImGui::Button("+", ImVec2(S(34.0f), S(30.0f))) && fpsValue < 120)
				++fpsValue;
			ImGui::SameLine(0.0f, gapW);
			ImGui::TextUnformatted(TR(label));
			ImGui::PopID();
		};
		fpsRow("ImageFps", panel.exportImageFps, "Image");
		fpsRow("VideoFps", panel.exportVideoFps, "Video");

		ImGui::SeparatorText(TR("Frames"));

		if (ImGui::Button("PNG##FrameSeq", ImVec2(halfW, S(30.0f))))
		{
			if (panel.onExportPngFrames)
				panel.onExportPngFrames(panel.exportAlpha);
		}
		ImGui::SameLine(0.0f, gapW);
		if (ImGui::Button("JPG##FrameSeq", ImVec2(halfW, S(30.0f))))
		{
			if (panel.onExportJpegFrames)
				panel.onExportJpegFrames(false);
		}

		ImGui::SeparatorText(TR("Video"));
		const float thirdW = (ImGui::GetContentRegionAvail().x - gapW * 2.0f) / 3.0f;
		if (ImGui::Button("MP4", ImVec2(thirdW, S(30.0f))))
		{
			if (panel.onExportMp4)
				panel.onExportMp4(false);
		}
		ImGui::SameLine(0.0f, gapW);
		if (ImGui::Button("WebM", ImVec2(thirdW, S(30.0f))))
		{
			if (panel.onExportWebm)
				panel.onExportWebm(panel.exportAlpha);
		}
		ImGui::SameLine(0.0f, gapW);
		if (ImGui::Button("GIF", ImVec2(thirdW, S(30.0f))))
		{
			if (panel.onExportGif)
				panel.onExportGif(false);
		}

		s_exportPanelHeight = ImGui::GetWindowSize().y;
		ImGui::End();
		ImGui::PopStyleVar();
	}
	else if (!s_exportPanelOpen && spineLoaded && s_exportEverOpened)
	{
		const float btnW = S(133.3f);
		const float btnH = S(33.3f);
		const float hiddenX = io.DisplaySize.x;
		const float shownX = io.DisplaySize.x - btnW - S(4.0f);
		const float triggerZone = S(133.3f);
		const bool pointerAtRightEdge =
			io.MousePos.x >= io.DisplaySize.x - triggerZone &&
			io.MousePos.y >= tbH &&
			io.MousePos.y < io.DisplaySize.y;
		if (s_exportReturnButtonX > hiddenX)
			s_exportReturnButtonX = hiddenX;
		const float targetX = pointerAtRightEdge ? shownX : hiddenX;
		const float slideStep = (std::min)(1.0f, 8.0f * io.DeltaTime);
		s_exportReturnButtonX += (targetX - s_exportReturnButtonX) * slideStep;

		if (s_exportReturnButtonX < hiddenX - 1.0f)
		{
			const float availableHeight = io.DisplaySize.y - tbH;
			const float btnY = tbH + (availableHeight - btnH) * 0.5f;
			ImGui::SetNextWindowPos(ImVec2(s_exportReturnButtonX, btnY), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(btnW + S(4.0f), btnH + S(4.0f)), ImGuiCond_Always);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(2.0f), S(2.0f)));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::Begin("##ExportPanelReturn", nullptr,
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoBackground |
				ImGuiWindowFlags_NoSavedSettings);
			if (ImGui::Button("<<", ImVec2(btnW, btnH)))
			{
				s_exportPanelOpen = true;
				s_exportSlideX = io.DisplaySize.x;
				s_exportReturnButtonX = hiddenX;
			}
			ImGui::End();
			ImGui::PopStyleVar(2);
		}
	}


	static int settingPage = 0;
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	if (showSettingWindow)
	{
		ImGui::OpenPopup("SettingWindow");
		showSettingWindow = false;
		settingPage = 0;
	}
	static bool showLoadedSpineReplaceConfirmPrev = false;
	if (panel.showLoadedSpineReplaceConfirm && !showLoadedSpineReplaceConfirmPrev)
		ImGui::OpenPopup((std::string(TR("Warning")) + "##LoadedSpineReplaceConfirm").c_str());
	showLoadedSpineReplaceConfirmPrev = panel.showLoadedSpineReplaceConfirm;

	if (showProWindow)
	{
		ImGui::OpenPopup("ProFeatureWindow");
		showProWindow = false;
	}

	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(S(620.f), S(300.f)));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, S(18.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(24.f), S(22.f)));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, S(12.f));
	if (ImGui::BeginPopup("ProFeatureWindow",
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoTitleBar))
	{
		ImVec2 windowSize = ImGui::GetWindowSize();

		const char* tipText = TR("To use this feature first, please open yihkllo.com.");
		const char* linkText = "yihkllo.com";
		ImGui::SetWindowFontScale(1.7f);
		float tipWidth = ImGui::CalcTextSize(tipText).x;
		float linkWidth = ImGui::CalcTextSize(linkText).x;
		float textBlockHeight = ImGui::GetTextLineHeight() * 2.0f + S(24.f);
		float startY = (windowSize.y - textBlockHeight) * 0.5f;

		ImGui::SetCursorPos(ImVec2((windowSize.x - tipWidth) * 0.5f, startY));
		ImGui::TextUnformatted(tipText);

		ImGui::SetCursorPos(ImVec2((windowSize.x - linkWidth) * 0.5f, startY + ImGui::GetTextLineHeight() + S(24.f)));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.24f, 0.49f, 0.88f, 1.0f));
		ImGui::TextUnformatted(linkText);
		ImGui::PopStyleColor();
		ImGui::SetWindowFontScale(1.0f);
		if (ImGui::IsItemHovered())
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		if (ImGui::IsItemClicked())
		{
			if (panel.onProButtonClicked) panel.onProButtonClicked();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(3);

	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(S(800.f), S(480.f)));
	if (ImGui::BeginPopupModal("SettingWindow", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse))
	{
		panel.isSettingWindowOpen = true;
		ImGui::SetWindowFontScale(1.5f);

		if (settingPage == 0)
		{

			if (ImGui::Button(TR("Language"), ImVec2(-1, S(53.3f))))
				settingPage = 2;

			if (ImGui::Button(TR("Theme"), ImVec2(-1, S(53.3f))))
				settingPage = 3;

			if (ImGui::Button(TR("Render BG Color"), ImVec2(-1, S(53.3f))))
				settingPage = 4;


			ImGui::SetCursorPosY(ImGui::GetWindowHeight() - S(66.7f));
			if (ImGui::Button(TR("Close"), ImVec2(-1, S(53.3f))))
				ImGui::CloseCurrentPopup();
		}
		else if (settingPage == 2)
		{

			ImGui::Text("%s", TR("Language Settings"));
			ImGui::Separator();
			ImGui::Spacing();

			const float langChildH = ImGui::GetContentRegionAvail().y - S(66.7f);
			ImGui::BeginChild("##LangScroll", ImVec2(0, langChildH), false);
			ImGui::SetWindowFontScale(1.5f);

			float btnW = ImGui::GetContentRegionAvail().x;

			static std::vector<std::string> langs;
			static bool langsLoaded = false;
			if (!langsLoaded)
			{
				langs = i18n::getAvailableLangs();

				if (std::find(langs.begin(), langs.end(), "en") == langs.end())
					langs.insert(langs.begin(), "en");
				langsLoaded = true;
			}

			for (const auto& id : langs)
			{
				bool isCurrent = (i18n::currentLang() == id);
				if (isCurrent)
				{
					ImGui::PushStyleColor(ImGuiCol_Button,        ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				}
			if (ImGui::Button(i18n::displayName(id), ImVec2(btnW, S(53.3f))))
			{
				s_pendingLanguageId = id;
			}
				if (isCurrent) ImGui::PopStyleColor(2);
				ImGui::Spacing();
			}

			ImGui::EndChild();

			if (ImGui::Button(TR("Back"), ImVec2(-1, S(53.3f))))
				settingPage = 0;
		}
		else if (settingPage == 3)
		{

			ImGui::Text("%s", TR("Theme"));
			ImGui::Separator();
			ImGui::Spacing();

			const float themeChildH = ImGui::GetContentRegionAvail().y - S(66.7f);
			ImGui::BeginChild("##ThemeScroll", ImVec2(0, themeChildH), false);
			ImGui::SetWindowFontScale(1.5f);

			if (ImGui::Button(TR("Title BG"), ImVec2(-1, S(40))))
				if (panel.onLoadTitleBg) panel.onLoadTitleBg();

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();


			static constexpr float kDefaultHue = 0.74f;
			static constexpr float kDefaultSat = 0.83f;
			static constexpr float kDefaultVal = 1.00f;
			static float s_hue = kDefaultHue;
			static float s_sat = kDefaultSat;
			static float s_val = kDefaultVal;
			static bool s_darkMode = false;
			auto applyTheme = [&](float h, float s, float v, bool dark) {
				auto hsv = [](float h, float s, float v, float a = 1.f) -> ImVec4 {
					ImVec4 c; ImGui::ColorConvertHSVtoRGB(h, s, v, c.x, c.y, c.z); c.w = a; return c;
				};
				ImGuiStyle& st = ImGui::GetStyle();
				ImVec4* c = st.Colors;
				if (dark)
				{
					c[ImGuiCol_Text]                 = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
					c[ImGuiCol_WindowBg]             = hsv(h, 0.f, 0.13f);
					c[ImGuiCol_ChildBg]              = hsv(h, 0.f, 0.13f);
					c[ImGuiCol_PopupBg]              = hsv(h, 0.f, 0.16f, 0.98f);
					c[ImGuiCol_TitleBg]              = hsv(h, 0.f, 0.20f);
					c[ImGuiCol_TitleBgActive]        = hsv(h, 0.f, 0.28f);
					c[ImGuiCol_TitleBgCollapsed]     = hsv(h, 0.f, 0.16f);
					c[ImGuiCol_Button]               = hsv(h, 0.f, 0.30f);
					c[ImGuiCol_ButtonHovered]        = hsv(h, 0.f, 0.40f);
					c[ImGuiCol_ButtonActive]         = hsv(h, 0.f, 0.50f);
					c[ImGuiCol_Header]               = hsv(h, 0.f, 0.25f);
					c[ImGuiCol_HeaderHovered]        = hsv(h, 0.f, 0.35f);
					c[ImGuiCol_HeaderActive]         = hsv(h, 0.f, 0.45f);
					c[ImGuiCol_FrameBg]              = hsv(h, 0.f, 0.22f);
					c[ImGuiCol_FrameBgHovered]       = hsv(h, 0.f, 0.28f);
					c[ImGuiCol_FrameBgActive]        = hsv(h, 0.f, 0.35f);
					c[ImGuiCol_Tab]                  = hsv(h, 0.f, 0.22f);
					c[ImGuiCol_TabHovered]           = hsv(h, 0.f, 0.35f);
					c[ImGuiCol_TabSelected]          = hsv(h, 0.f, 0.40f);
					c[ImGuiCol_SliderGrab]           = hsv(h, 0.f, 0.50f);
					c[ImGuiCol_SliderGrabActive]     = hsv(h, 0.f, 0.60f);
					c[ImGuiCol_CheckMark]            = hsv(h, 0.f, 0.70f);
					c[ImGuiCol_Separator]            = hsv(h, 0.f, 0.30f);
					c[ImGuiCol_MenuBarBg]            = hsv(h, 0.f, 0.16f);
					c[ImGuiCol_ScrollbarBg]          = hsv(h, 0.f, 0.10f);
					c[ImGuiCol_ScrollbarGrab]        = hsv(h, 0.f, 0.35f);
					c[ImGuiCol_ScrollbarGrabHovered] = hsv(h, 0.f, 0.45f);
					c[ImGuiCol_ScrollbarGrabActive]  = hsv(h, 0.f, 0.55f);
				}
				else
				{
					ImGui::StyleColorsLight();
					c = st.Colors;
					c[ImGuiCol_Text]                 = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
					c[ImGuiCol_WindowBg]             = hsv(h, s * 0.08f, v);
					c[ImGuiCol_ChildBg]              = hsv(h, s * 0.08f, v);
					c[ImGuiCol_PopupBg]              = hsv(h, s * 0.08f, v, 0.98f);
					c[ImGuiCol_TitleBg]              = hsv(h, s * 0.50f, v * 0.98f);
					c[ImGuiCol_TitleBgActive]        = hsv(h, s * 0.70f, v * 0.96f);
					c[ImGuiCol_TitleBgCollapsed]     = hsv(h, s * 0.25f, v * 0.98f);
					c[ImGuiCol_Button]               = hsv(h, s * 0.55f, v * 0.96f);
					c[ImGuiCol_ButtonHovered]        = hsv(h, s * 0.75f, v * 0.98f);
					c[ImGuiCol_ButtonActive]         = hsv(h, s * 0.90f, v * 0.92f);
					c[ImGuiCol_Header]               = hsv(h, s * 0.40f, v * 0.98f);
					c[ImGuiCol_HeaderHovered]        = hsv(h, s * 0.60f, v * 0.98f);
					c[ImGuiCol_HeaderActive]         = hsv(h, s * 0.80f, v * 0.94f);
					c[ImGuiCol_FrameBg]              = hsv(h, s * 0.20f, v * 0.99f);
					c[ImGuiCol_FrameBgHovered]       = hsv(h, s * 0.35f, v * 0.99f);
					c[ImGuiCol_FrameBgActive]        = hsv(h, s * 0.55f, v * 0.97f);
					c[ImGuiCol_Tab]                  = hsv(h, s * 0.40f, v * 0.98f);
					c[ImGuiCol_TabHovered]           = hsv(h, s * 0.65f, v * 0.98f);
					c[ImGuiCol_TabSelected]          = hsv(h, s * 0.85f, v * 0.95f);
					c[ImGuiCol_SliderGrab]           = hsv(h, s * 0.70f, v * 0.95f);
					c[ImGuiCol_SliderGrabActive]     = hsv(h, s * 0.90f, v * 0.90f);
					c[ImGuiCol_CheckMark]            = hsv(h, s * 0.90f, v * 0.90f);
					c[ImGuiCol_Separator]            = hsv(h, s * 0.55f, v * 0.96f);
					c[ImGuiCol_MenuBarBg]            = hsv(h, s * 0.20f, v * 0.99f);
					c[ImGuiCol_ScrollbarBg]          = hsv(h, s * 0.12f, v);
					c[ImGuiCol_ScrollbarGrab]        = hsv(h, s * 0.55f, v * 0.96f);
					c[ImGuiCol_ScrollbarGrabHovered] = hsv(h, s * 0.70f, v * 0.96f);
					c[ImGuiCol_ScrollbarGrabActive]  = hsv(h, s * 0.85f, v * 0.92f);
				}

				auto& tb = panel.titleBar;
				if (dark)
				{
					ImVec4 bg = hsv(h, 0.f, 0.11f);
					tb.bgColor[0] = bg.x; tb.bgColor[1] = bg.y; tb.bgColor[2] = bg.z; tb.bgColor[3] = 1.f;
					tb.textColor[0] = 0.9f;  tb.textColor[1] = 0.9f;  tb.textColor[2] = 0.9f;  tb.textColor[3] = 1.f;
					tb.subTextColor[0] = 0.65f; tb.subTextColor[1] = 0.65f; tb.subTextColor[2] = 0.65f; tb.subTextColor[3] = 0.78f;
					tb.iconColor[0] = 0.85f; tb.iconColor[1] = 0.85f; tb.iconColor[2] = 0.85f; tb.iconColor[3] = 0.86f;
					tb.btnHoverColor[0] = 0.3f;  tb.btnHoverColor[1] = 0.3f;  tb.btnHoverColor[2] = 0.35f; tb.btnHoverColor[3] = 0.7f;
					tb.btnActiveColor[0] = 0.4f; tb.btnActiveColor[1] = 0.4f; tb.btnActiveColor[2] = 0.45f; tb.btnActiveColor[3] = 1.f;
				}
				else
				{
					ImVec4 bg = hsv(h, s * 0.12f, v);
					tb.bgColor[0] = bg.x; tb.bgColor[1] = bg.y; tb.bgColor[2] = bg.z; tb.bgColor[3] = 1.f;
					tb.textColor[0] = 0.118f;  tb.textColor[1] = 0.118f;  tb.textColor[2] = 0.118f;  tb.textColor[3] = 1.f;
					tb.subTextColor[0] = 0.314f; tb.subTextColor[1] = 0.314f; tb.subTextColor[2] = 0.314f; tb.subTextColor[3] = 0.78f;
					tb.iconColor[0] = 0.118f; tb.iconColor[1] = 0.118f; tb.iconColor[2] = 0.118f; tb.iconColor[3] = 0.86f;
					tb.btnHoverColor[0] = 0.863f;  tb.btnHoverColor[1] = 0.784f;  tb.btnHoverColor[2] = 0.824f; tb.btnHoverColor[3] = 0.7f;
					tb.btnActiveColor[0] = 0.745f; tb.btnActiveColor[1] = 0.667f; tb.btnActiveColor[2] = 0.725f; tb.btnActiveColor[3] = 1.f;
				}
			};

			bool changed = false;
			ImGui::SetNextItemWidth(-1);
			changed |= ImGui::SliderFloat(TR("Hue##theme"),               &s_hue, 0.f, 1.f, TR("Hue %.2f"));
			ImGui::SetNextItemWidth(-1);
			changed |= ImGui::SliderFloat(TR("Saturation##theme"),  &s_sat, 0.f, 1.f, TR("Saturation %.2f"));
			ImGui::SetNextItemWidth(-1);
			changed |= ImGui::SliderFloat(TR("Brightness##theme"),        &s_val, 0.f, 1.f, TR("Brightness %.2f"));

			if (changed)
				applyTheme(s_hue, s_sat, s_val, s_darkMode);

			{
				static float s_fontSizeSlider = 0.f;
				if (s_fontSizeSlider <= 0.f)
					s_fontSizeSlider = s_currentFontSize;

				const float fsApplyW = S(100);
				const float fsAvail = ImGui::GetContentRegionAvail().x;
				const float spacing = ImGui::GetStyle().ItemSpacing.x;
				ImGui::SetNextItemWidth(fsAvail - fsApplyW - spacing);
				char fsFmt[64];
				snprintf(fsFmt, sizeof(fsFmt), "%s %%.1f", TR("Font Size"));
				ImGui::SliderFloat("##FontSizeSlider", &s_fontSizeSlider, 10.f, 50.f, fsFmt);
				ImGui::SameLine();
				if (ImGui::Button(TR("Apply##fontsize"), ImVec2(fsApplyW, 0)))
				{
#ifdef _WIN32
					s_currentFontSize = s_fontSizeSlider;
					RebuildPanelFonts(panel, s_currentFontSize, i18n::currentLang());
#endif
				}
			}

			ImGui::Spacing();
			{

				const char* darkLabel = s_darkMode ? TR("Dark Mode: ON##theme") : TR("Dark Mode: OFF##theme");
				if (s_darkMode)
				{
					ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.45f, 0.45f, 0.50f, 1.00f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.55f, 0.55f, 0.60f, 1.00f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.35f, 0.35f, 0.40f, 1.00f));
				}
				bool darkClicked = ImGui::Button(darkLabel, ImVec2(-1, S(40)));
				if (s_darkMode)
					ImGui::PopStyleColor(3);
				if (darkClicked)
				{
					s_darkMode = !s_darkMode;
					applyTheme(s_hue, s_sat, s_val, s_darkMode);
				}
			}
			ImGui::Spacing();
			if (ImGui::Button(TR("Reset to Default##theme"), ImVec2(-1, S(40))))
			{
				s_hue = kDefaultHue;
				s_sat = kDefaultSat;
				s_val = kDefaultVal;
				s_darkMode = false;
				applyTheme(s_hue, s_sat, s_val, s_darkMode);
			}

			ImGui::EndChild();

			if (ImGui::Button(TR("Back"), ImVec2(-1, S(53.3f))))
				settingPage = 0;
		}
		else if (settingPage == 4)
		{

			ImGui::Text("%s", TR("Render BG Color"));
			ImGui::Separator();
			ImGui::Spacing();

			static float s_bgCol[3] = { 0.f, 0.f, 0.f };
			static bool s_bgInited = false;
			if (!s_bgInited)
			{
				s_bgCol[0] = panel.renderBgR / 255.f;
				s_bgCol[1] = panel.renderBgG / 255.f;
				s_bgCol[2] = panel.renderBgB / 255.f;
				s_bgInited = true;
			}

			const float bgChildH = ImGui::GetContentRegionAvail().y - S(66.7f);
			ImGui::BeginChild("##BgScroll", ImVec2(0, bgChildH), false);
			ImGui::SetWindowFontScale(1.5f);

			const float kPickerSize = ImGui::GetContentRegionAvail().y - S(40) - S(5.3f);
			ImGuiColorEditFlags pickerFlags =
				ImGuiColorEditFlags_NoAlpha        |
				ImGuiColorEditFlags_PickerHueBar   |
				ImGuiColorEditFlags_NoSidePreview  |
				ImGuiColorEditFlags_NoSmallPreview |
				ImGuiColorEditFlags_DisplayRGB     |
				ImGuiColorEditFlags_InputRGB;
			ImGui::SetNextItemWidth(kPickerSize);
			if (ImGui::ColorPicker3("##bgColorPicker", s_bgCol, pickerFlags))
			{
				int r = static_cast<int>(s_bgCol[0] * 255.f + 0.5f);
				int g = static_cast<int>(s_bgCol[1] * 255.f + 0.5f);
				int b = static_cast<int>(s_bgCol[2] * 255.f + 0.5f);
				panel.renderBgR = r;
				panel.renderBgG = g;
				panel.renderBgB = b;
				if (panel.onSetRenderBgColor)
					panel.onSetRenderBgColor(r, g, b);
			}

			if (ImGui::Button(TR("Reset to Default##renderbg"), ImVec2(-1, S(40))))
			{
				s_bgCol[0] = 0.f; s_bgCol[1] = 0.f; s_bgCol[2] = 0.f;
				panel.renderBgR = 0;
				panel.renderBgG = 0;
				panel.renderBgB = 0;
				if (panel.onSetRenderBgColor)
					panel.onSetRenderBgColor(0, 0, 0);
			}

			ImGui::EndChild();

			if (ImGui::Button(TR("Back"), ImVec2(-1, S(53.3f))))
			{
				s_bgInited = false;
				settingPage = 0;
			}
		}

		ImGui::EndPopup();
	}
	else
	{
		panel.isSettingWindowOpen = false;
	}

	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(S(760.f), S(250.f)));
	if (ImGui::BeginPopupModal((std::string(TR("Warning")) + "##LoadedSpineReplaceConfirm").c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::SetWindowFontScale(1.5f);
		ImGui::Dummy(ImVec2(0, S(10.f)));
		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
		if (!panel.loadedSpineReplaceConfirmMessage.empty())
			ImGui::TextUnformatted(TR(panel.loadedSpineReplaceConfirmMessage.c_str()));
		ImGui::PopTextWrapPos();

		const float buttonW = (ImGui::GetContentRegionAvail().x - S(12.f)) * 0.5f;
		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - S(68.f));
		if (ImGui::Button(TR("Cancel"), ImVec2(buttonW, S(46.f))))
		{
			if (panel.onCancelLoadedSpineReplace) panel.onCancelLoadedSpineReplace();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine(0, S(12.f));
		if (ImGui::Button(TR("Continue"), ImVec2(buttonW, S(46.f))))
		{
			if (panel.onConfirmLoadedSpineReplace) panel.onConfirmLoadedSpineReplace();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}



}
#undef S

bool spine_panel::HasSlotNameQueryFilter()
{
	return !slot_name_query::s_foldedQuery.empty();
}

bool(*spine_panel::GetSlotNameQueryExcludeCallback())(const char*, size_t)
{
	return &slot_name_query::IsNameQueryHidden;
}

void spine_panel::SetExternalSlotExcludeCallback(bool (*pFunc)(const char*, size_t))
{
	slot_name_query::s_externalCallback = pFunc;
}

void spine_panel::ClearExternalSlotExcludeCallback()
{
	slot_name_query::s_externalCallback = nullptr;
}
