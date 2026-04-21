
#include <string>
#include <vector>
#include <algorithm>
#include <map>

#include "spine_panel.h"
#include "custom_titlebar.h"
#include "../spine_runtime_registry.h"
#include "../sl_font_provider.h"
#include "../sl_path_util.h"
#include "../sl_text_codec.h"

#include <imgui.h>
#include "i18n.h"

#ifdef _WIN32
#include <Windows.h>
#include <backends/imgui_impl_dx11.h>
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

	bool PointInSlotMesh(float px, float py, const SlotMeshData& meshData)
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


	bool BuildAlphaContour(const SlotMeshData& meshData, std::vector<float>& outWorldEdgePoints)
	{
		outWorldEdgePoints.clear();

		if (meshData.worldVertices.size() < 4 || meshData.uvs.size() < 4 || meshData.textureHandle < 0)
			return false;


		float minX = meshData.worldVertices[0], maxX = minX;
		float minY = meshData.worldVertices[1], maxY = minY;
		for (size_t vi = 2; vi < meshData.worldVertices.size(); vi += 2)
		{
			float x = meshData.worldVertices[vi];
			float y = meshData.worldVertices[vi + 1];
			if (x < minX) minX = x;
			if (x > maxX) maxX = x;
			if (y < minY) minY = y;
			if (y > maxY) maxY = y;
		}

		const int padding = 2;
		int bufW = static_cast<int>(maxX - minX) + padding * 2 + 1;
		int bufH = static_cast<int>(maxY - minY) + padding * 2 + 1;
		if (bufW <= 0 || bufH <= 0 || bufW > 4096 || bufH > 4096)
			return false;


		int offscreen = DxLib::MakeScreen(bufW, bufH, TRUE);
		if (offscreen == -1) return false;


		int prevScreen = DxLib::GetDrawScreen();
		DxLib::SetDrawScreen(offscreen);
		DxLib::ClearDrawScreen();


		int vertCount = static_cast<int>(meshData.worldVertices.size()) / 2;
		std::vector<DxLib::VERTEX2D> verts(vertCount);
		for (int k = 0; k < vertCount; ++k)
		{
			DxLib::VERTEX2D& v = verts[k];
			v.pos.x = meshData.worldVertices[k * 2] - minX + padding;
			v.pos.y = meshData.worldVertices[k * 2 + 1] - minY + padding;
			v.pos.z = 0.f;
			v.rhw = 1.f;
			v.dif.r = 255; v.dif.g = 255; v.dif.b = 255; v.dif.a = 255;
			v.u = meshData.uvs[k * 2];
			v.v = meshData.uvs[k * 2 + 1];
		}

		DxLib::SetDrawBlendMode(DX_BLENDMODE_PMA_ALPHA, 255);
		DxLib::DrawPolygonIndexed2D(
			verts.data(),
			vertCount,
			meshData.triangles.data(),
			static_cast<int>(meshData.triangles.size()) / 3,
			meshData.textureHandle,
			TRUE
		);


		int softImg = DxLib::MakeARGB8ColorSoftImage(bufW, bufH);
		if (softImg == -1)
		{
			DxLib::SetDrawScreen(prevScreen);
			DxLib::DeleteGraph(offscreen);
			return false;
		}
		DxLib::GetDrawScreenSoftImage(0, 0, bufW, bufH, softImg);

		DxLib::SetDrawScreen(prevScreen);

		unsigned char* pPixels = static_cast<unsigned char*>(DxLib::GetImageAddressSoftImage(softImg));
		int pitch = DxLib::GetPitchSoftImage(softImg);
		if (pPixels == nullptr || pitch <= 0)
		{
			DxLib::DeleteSoftImage(softImg);
			DxLib::DeleteGraph(offscreen);
			return false;
		}


		const int alphaThreshold = 16;
		auto getAlpha = [&](int x, int y) -> int
		{
			if (x < 0 || x >= bufW || y < 0 || y >= bufH) return 0;
			return pPixels[y * pitch + x * 4 + 3];
		};

		for (int y = 0; y < bufH; ++y)
		{
			for (int x = 0; x < bufW; ++x)
			{
				int a = getAlpha(x, y);
				if (a <= alphaThreshold) continue;

				if (getAlpha(x - 1, y) <= alphaThreshold ||
					getAlpha(x + 1, y) <= alphaThreshold ||
					getAlpha(x, y - 1) <= alphaThreshold ||
					getAlpha(x, y + 1) <= alphaThreshold)
				{

					float wx = static_cast<float>(x) - padding + minX;
					float wy = static_cast<float>(y) - padding + minY;
					outWorldEdgePoints.push_back(wx);
					outWorldEdgePoints.push_back(wy);
				}
			}
		}

		DxLib::DeleteSoftImage(softImg);
		DxLib::DeleteGraph(offscreen);
		return !outWorldEdgePoints.empty();
	}

	void DrawSlotAlphaOutline(const SlotMeshData& meshData,
		unsigned int col, float thickness, DxLib::MATRIX* pMat)
	{
		if (meshData.worldVertices.size() < 4) return;

		std::vector<float> edgePoints;
		if (!BuildAlphaContour(meshData, edgePoints)) return;

		DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
		DxLib::SetTransformTo2D(pMat);

		float halfT = thickness * 0.5f;
		for (size_t i = 0; i < edgePoints.size(); i += 2)
		{
			float wx = edgePoints[i];
			float wy = edgePoints[i + 1];
			if (thickness <= 2.f)
			{
				DxLib::DrawPixel(static_cast<int>(wx), static_cast<int>(wy), col);
			}
			else
			{
				DxLib::DrawBoxAA(wx - halfT, wy - halfT, wx + halfT, wy + halfT, col, TRUE);
			}
		}

		DxLib::ResetTransformTo2D();
	}
}


namespace spine_panel
{
	namespace slot_exclusion
	{
		static constexpr int kFilterSize = 32;
		static char s_szFilter[kFilterSize]{};
		static size_t s_nFilterLength = 0;
		static std::basic_string<bool> s_slotChecks;
		static bool (*s_externalCallback)(const char*, size_t) = nullptr;

		static bool IsSlotToBeExcluded(const char* szSlotName, size_t nSlotNameLength)
		{
			if (s_nFilterLength == 0)return false;

			const char* pNameEnd = szSlotName + nSlotNameLength;
			return std::search(szSlotName, pNameEnd, s_szFilter, s_szFilter + s_nFilterLength) != pNameEnd;
		}
	};

#ifdef _WIN32
	static std::vector<std::string> GetSystemFonts()
	{
		CWinFont winFont;
		std::vector<std::wstring> wFonts = winFont.GetSystemFontFamilyNames();
		std::vector<std::string> fonts;

		for (const auto& wFont : wFonts)
		{
			fonts.push_back(win_text::NarrowUtf8(wFont));
		}

		return fonts;
	}


	static std::string GetWindowsFontsDir()
	{
		wchar_t winDir[MAX_PATH]{};
		::GetWindowsDirectoryW(winDir, MAX_PATH);
		std::wstring wDir = winDir;
		if (!wDir.empty() && wDir.back() != L'\\') wDir += L'\\';
		wDir += L"Fonts\\";
		return win_text::NarrowUtf8(wDir);
	}
#endif

	struct ImGuiComboBox
	{
		unsigned int selectedIndex = 0;

		void update(const std::vector<std::string>& itemNames, const char* comboLabel)
		{
			if (selectedIndex >= itemNames.size())
			{
				selectedIndex = 0;
				return;
			}

			if (ImGui::BeginCombo(comboLabel, itemNames[selectedIndex].c_str(), ImGuiComboFlags_HeightLarge))
			{
				for (size_t i = 0; i < itemNames.size(); ++i)
				{
					bool isSelected = (selectedIndex == i);
					if (ImGui::Selectable(itemNames[i].c_str(), isSelected))
					{
						selectedIndex = static_cast<unsigned int>(i);
					}

					if (isSelected)ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
	};

	struct ImGuiListview
	{
		std::basic_string<bool> checks;
		ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_NoAutoSelect | ImGuiMultiSelectFlags_NoAutoClear | ImGuiMultiSelectFlags_ClearOnEscape;

		void update(const std::vector<std::string>& itemNames, const char* windowLabel)
		{
			if (checks.size() != itemNames.size())
			{
				clear(itemNames);
			}

			float listH = ImGui::GetFontSize() * (float)(checks.size() / 4 + 2);
			float minListH = ImGui::GetFontSize() * 15.f;
			if (listH < minListH) listH = minListH;
			ImVec2 childWindowSize = { ImGui::GetWindowWidth() * 3 / 4.f, listH };
			if (ImGui::BeginChild(windowLabel, childWindowSize, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoNav))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0));
				float rowH = ImGui::GetFrameHeight();
				for (size_t i = 0; i < itemNames.size(); ++i)
				{
					std::string cbId = "##lvcb" + std::to_string(i);
					if (ImGui::Checkbox(cbId.c_str(), &checks[i])){}
					ImGui::SameLine();
					if (ImGui::Selectable(itemNames[i].c_str(), checks[i], 0, ImVec2(ImGui::GetContentRegionAvail().x, rowH)))
						checks[i] = !checks[i];
				}
				ImGui::PopStyleVar();
			}

			ImGui::EndChild();
		}

		void pickupCheckedItems(const std::vector<std::string>& itemNames, std::vector<std::string>& selectedItems)
		{
			if (itemNames.size() != checks.size())return;

			for (size_t i = 0; i < checks.size(); ++i)
			{
				if (checks[i])
				{
					selectedItems.emplace_back(itemNames[i]);
				}
			}
		}

		void clear(const std::vector<std::string>& itemNames)
		{
			checks = std::basic_string<bool>(itemNames.size(), false);
		}
	};

	static void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	static void ScrollableSliderInt(const char* label, int* v, int v_min, int v_max)
	{
		ImGui::InputInt(label, v, 1, 10);
		if (*v < v_min) *v = v_min;
		if (*v > v_max) *v = v_max;
		ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
		if (ImGui::IsItemHovered())
		{
			float wheel = ImGui::GetIO().MouseWheel;
			if (wheel > 0 && *v < v_max)
			{
				++(*v);
			}
			else if (wheel < 0 && *v > v_min)
			{
				--(*v);
			}
		}
	}

	static void ScrollableSliderFloat(const char* label, float* v, float v_min, float v_max)
	{
		ImGui::SliderFloat(label, v, v_min, v_max, "%.1f");
		ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
		if (ImGui::IsItemHovered())
		{
			float wheel = ImGui::GetIO().MouseWheel;
			if (wheel > 0 && *v < v_max)
			{
				++(*v);
			}
			else if (wheel < 0 && *v > v_min)
			{
				--(*v);
			}
		}
	}
}

void spine_panel::Display(SSpineToolDatum& spineToolDatum, bool* pIsOpen)
{
	if (pIsOpen == nullptr)return;

	ImGuiIO& io = ImGui::GetIO();

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
	static bool s_panelsHidden = false;
	static bool s_exportPanelOpen   = false;
	static bool s_exportEverOpened  = false;
	const float uiScale = io.DisplaySize.x > 0 ? io.DisplaySize.x / 1920.0f : 1.0f;
#define S(x) ((x) * uiScale)

	static float s_panelScale = 1.0f;
	const float leftPanelWidth  = S(213.0f * s_panelScale);
	const float rightPanelWidth = S(213.0f * s_panelScale);
	const float exportPanelWidth = S(230.0f);
	const float tbH = spineToolDatum.isFullscreen ? 0.f : custom_titlebar::TitleBarHeight();

	if (!spineToolDatum.isFullscreen)
		custom_titlebar::Draw(spineToolDatum.titleBar);

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
	bool spineLoaded = spineToolDatum.pSpinePlayer != nullptr &&
		static_cast<CSpineRuntimeRegistry*>(spineToolDatum.pSpinePlayer)->ActiveRuntime() != nullptr;

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
		if (spineToolDatum.onOpenFiles) spineToolDatum.onOpenFiles();


	if (ImGui::Button(TR("Setting"), ImVec2(btnWidth, S(30.0f))))
		showSettingWindow = true;


	if (ImGui::Button(TR("Background"), ImVec2(btnWidth, S(30.0f))))
		if (spineToolDatum.onLoadBackground) spineToolDatum.onLoadBackground();


	if (ImGui::Button(TR("Audio"), ImVec2(btnWidth, S(30.0f))))
		if (spineToolDatum.onLoadAudio) spineToolDatum.onLoadAudio();

	if (spineToolDatum.audioTotal > 0)
	{

		ImGui::SetWindowFontScale(1.2f);
		char audioBuf[32];
		snprintf(audioBuf, sizeof(audioBuf), "%d / %d", spineToolDatum.audioIndex, spineToolDatum.audioTotal);
		ImGui::TextUnformatted(audioBuf);
		if (!spineToolDatum.audioFileName.empty())
		{
			ImGui::SameLine(0, S(8.f));
			std::string u8name = win_text::NarrowUtf8(spineToolDatum.audioFileName);
			ImGui::TextUnformatted(u8name.c_str());
		}
		ImGui::SetWindowFontScale(1.5f);

		float halfW = (btnWidth - S(2.6667f)) * 0.5f;

		if (ImGui::Button(spineToolDatum.isAudioPlaying ? TR("Stop##Audio") : TR("Play##Audio"), ImVec2(halfW, 0)))
		{
			if (spineToolDatum.isAudioPlaying)
			{ if (spineToolDatum.onStopAudio) spineToolDatum.onStopAudio(); }
			else
			{ if (spineToolDatum.onPlayAudio) spineToolDatum.onPlayAudio(); }
		}
		ImGui::SameLine(0, S(2.6667f));
		bool looping = spineToolDatum.isAudioLooping;
		if (looping) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		if (ImGui::Button(TR("Loop##Audio"), ImVec2(halfW, 0)))
			if (spineToolDatum.onToggleLoopAudio) spineToolDatum.onToggleLoopAudio();
		if (looping) ImGui::PopStyleColor();


		ImGui::SetWindowFontScale(1.2f);
		float vol = spineToolDatum.audioVolume;
		ImGui::SetNextItemWidth(btnWidth);
		if (ImGui::SliderFloat("##AudioVol", &vol, 0.f, 1.f, TR("Vol %.2f")))
		{
			spineToolDatum.audioVolume = vol;
			if (spineToolDatum.onSetAudioVolume) spineToolDatum.onSetAudioVolume(vol);
		}
		ImGui::SetWindowFontScale(1.5f);


		float thirdW = (btnWidth - S(5.3333f)) / 3.f;
		bool autoOn = spineToolDatum.isAudioAuto;
		if (autoOn) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		if (ImGui::Button(TR("Auto##Audio"), ImVec2(thirdW, 0)))
			if (spineToolDatum.onToggleAudioAuto) spineToolDatum.onToggleAudioAuto();
		if (autoOn) ImGui::PopStyleColor();
		ImGui::SameLine(0, S(2.6667f));
		if (ImGui::Button("<##AudioPrev", ImVec2(thirdW, 0)))
			if (spineToolDatum.onAudioPrev) spineToolDatum.onAudioPrev();
		ImGui::SameLine(0, S(2.6667f));
		if (ImGui::Button(">##AudioNext", ImVec2(thirdW, 0)))
			if (spineToolDatum.onAudioNext) spineToolDatum.onAudioNext();
	}

	if (ImGui::Button(TR("Pro"), ImVec2(btnWidth, S(30.0f))))
	{
		showProWindow = true;
	}


	ImGui::Separator();

	if (!spineLoaded) ImGui::BeginDisabled();

	if (spineLoaded)
	{
		CSpineRuntimeRegistry* pDxLibSpinePlayerDynamic = static_cast<CSpineRuntimeRegistry*>(spineToolDatum.pSpinePlayer);
		ISpinePlayer* pDxLibSpinePlayer = pDxLibSpinePlayerDynamic->ActiveRuntime();

		ImGui::SetWindowFontScale(1.2f);

		bool pma = pDxLibSpinePlayer->isAlphaPremultiplied();
		bool pmaChecked = pma;
		CSpineRuntimeRegistry::ERuntimeSlot versionIndex = pDxLibSpinePlayerDynamic->ActiveRuntimeSlot();
		if (versionIndex >= CSpineRuntimeRegistry::ERuntimeSlot::Spine40)
		{
			ImGui::BeginDisabled();
			ImGui::Checkbox(TR("Alpha premultiplied"), &pmaChecked);
			ImGui::EndDisabled();
		}
		else
		{
			if (ImGui::Checkbox(TR("Alpha premultiplied"), &pmaChecked))
			{
				pDxLibSpinePlayer->togglePma();
				if (spineToolDatum.onPmaChanged) spineToolDatum.onPmaChanged(pmaChecked);
			}
		}

		{
			bool bResetOffset = pDxLibSpinePlayer->isResetOffsetOnLoad();
			if (ImGui::Checkbox(TR("Offset (0,0) Load##resetoffset2"), &bResetOffset))
				pDxLibSpinePlayer->setResetOffsetOnLoad(bResetOffset);
		}

		ImGui::SeparatorText(TR("Scale"));
		{
			float scale = pDxLibSpinePlayer->getSkeletonScale();
			int scalePercent = static_cast<int>(scale * 100.f);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(TR("Reset")).x - ImGui::GetStyle().FramePadding.x * 2 - ImGui::GetStyle().ItemSpacing.x);
			if (ImGui::SliderInt("##scale", &scalePercent, 10, 500, "%d%%"))
				pDxLibSpinePlayer->setSkeletonScale(scalePercent / 100.f);
			ImGui::SameLine();
			if (ImGui::Button(TR("Reset##scale")))
				pDxLibSpinePlayer->setSkeletonScale(1.f);
		}

		ImGui::SeparatorText(TR("Speed"));
		{
			float timeScale = pDxLibSpinePlayer->getTimeScale();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(TR("Reset")).x - ImGui::GetStyle().FramePadding.x * 2 - ImGui::GetStyle().ItemSpacing.x);
			if (ImGui::SliderFloat("##timescale", &timeScale, 0.f, 5.f, "%.2fx"))
				pDxLibSpinePlayer->setTimeScale(timeScale);
			ImGui::SameLine();
			if (ImGui::Button(TR("Reset##timescale")))
				pDxLibSpinePlayer->setTimeScale(1.f);
		}

		ImGui::SeparatorText(TR("Mix"));
		{
			static float s_defaultMix = 0.f;
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(TR("Reset")).x - ImGui::GetStyle().FramePadding.x * 2 - ImGui::GetStyle().ItemSpacing.x);
			if (ImGui::SliderFloat("##defaultmix", &s_defaultMix, 0.f, 1.f, "%.1fs"))
				pDxLibSpinePlayer->setDefaultMix(s_defaultMix);
			ImGui::SameLine();
			if (ImGui::Button(TR("Reset##mix")))
			{
				s_defaultMix = 0.f;
				pDxLibSpinePlayer->setDefaultMix(0.f);
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
		CSpineRuntimeRegistry* pDxLibSpinePlayerDynamic = static_cast<CSpineRuntimeRegistry*>(spineToolDatum.pSpinePlayer);
		ISpinePlayer* pDxLibSpinePlayer = pDxLibSpinePlayerDynamic->ActiveRuntime();
		const std::vector<std::string>& animationNames = pDxLibSpinePlayer->getAnimationNames();

		const std::string& currentAnimName = pDxLibSpinePlayer->getCurrentAnimationName();

		ImGui::BeginChild("AnimationList", ImVec2(0, S(213.0f)), false, ImGuiWindowFlags_NoNav);
		ImGui::SetWindowFontScale(1.5f);
		for (size_t i = 0; i < animationNames.size(); i++) {
			float dur = pDxLibSpinePlayer->getAnimationDuration(animationNames[i].c_str());
			char durStr[32];
			if (dur > 0.f) snprintf(durStr, sizeof(durStr), "%.1fs", dur);
			else durStr[0] = '\0';

			bool isCurrent = (animationNames[i] == currentAnimName);
			float rowWidth = ImGui::GetContentRegionAvail().x;
			float durTextWidth = dur > 0.f ? ImGui::CalcTextSize(durStr).x : 0.f;

			ImGui::PushID(static_cast<int>(i));
			if (ImGui::Selectable("##anim", isCurrent, 0, ImVec2(rowWidth, 0))) {
				if (spineToolDatum.isQueuePlaying && spineToolDatum.isQueuePlaying())
					spineToolDatum.onQueueStop();
				pDxLibSpinePlayer->setAnimationByIndex(i);
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
		
		static std::vector<std::string> cachedMixSkinNames;
		static std::string cachedSingleSkinName;
		static bool cachedIsMixMode = false;

		ImGui::Text("%s", TR("Skin"));
		ImGui::SameLine(leftPanelWidth - S(80.0f));
		ImGui::Checkbox(TR("Mix"), &mixMode);
		ImGui::Separator();

		const std::vector<std::string>& skinNames = pDxLibSpinePlayer->getSkinNames();

		static unsigned int prevLoadGeneration = 0;
		bool spineChanged = (spineToolDatum.loadGeneration != prevLoadGeneration);
		if (spineChanged) {
			prevLoadGeneration = spineToolDatum.loadGeneration;
			selectedSkins.assign(skinNames.size(), 0);
			currentSkin = -1;

			if (cachedIsMixMode && mixMode) {
				
				std::vector<std::string> skinsToMix;
				for (size_t i = 0; i < skinNames.size(); ++i) {
					for (const auto& cached : cachedMixSkinNames) {
						if (skinNames[i] == cached) {
							selectedSkins[i] = 1;
							skinsToMix.push_back(skinNames[i]);
							break;
						}
					}
				}
				if (skinsToMix.empty()) {
					pDxLibSpinePlayer->setSkinByIndex(0);
				} else if (skinsToMix.size() == 1) {
					for (size_t k = 0; k < skinNames.size(); k++) {
						if (skinNames[k] == skinsToMix[0]) { pDxLibSpinePlayer->setSkinByIndex(k); break; }
					}
				} else {
					pDxLibSpinePlayer->mixSkins(skinsToMix);
				}
				prevMixMode = mixMode; 
			} else if (!cachedIsMixMode && !mixMode && !cachedSingleSkinName.empty()) {
				
				for (int i = 0; i < (int)skinNames.size(); ++i) {
					if (skinNames[i] == cachedSingleSkinName) {
						currentSkin = i;
						pDxLibSpinePlayer->setSkinByIndex(i);
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
					pDxLibSpinePlayer->setSkinByIndex(0);
				}
			} else {

				for (size_t i = 0; i < selectedSkins.size(); i++) {
					selectedSkins[i] = 0;
				}

				if (!skinNames.empty()) {
					pDxLibSpinePlayer->setSkinByIndex(0);
				}
			}
		}

		
		cachedIsMixMode = mixMode;
		if (mixMode) {
			cachedMixSkinNames.clear();
			for (size_t i = 0; i < skinNames.size(); ++i) {
				if (selectedSkins[i]) cachedMixSkinNames.push_back(skinNames[i]);
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
						pDxLibSpinePlayer->setSkinByIndex(0);
					} else if (skinsToMix.size() == 1) {
						for (size_t k = 0; k < skinNames.size(); k++) {
							if (skinNames[k] == skinsToMix[0]) { pDxLibSpinePlayer->setSkinByIndex(k); break; }
						}
					} else {
						pDxLibSpinePlayer->mixSkins(skinsToMix);
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
							pDxLibSpinePlayer->setSkinByIndex(0);
						}
					} else {

						currentSkin = static_cast<int>(i);
						pDxLibSpinePlayer->setSkinByIndex(i);
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
		CSpineRuntimeRegistry* pRP = static_cast<CSpineRuntimeRegistry*>(spineToolDatum.pSpinePlayer);
		ISpinePlayer* pSP = pRP->ActiveRuntime();

		ImGui::SetWindowFontScale(1.5f);

		if (ImGui::CollapsingHeader(TR("Size/Flip")))
		{
			ImGui::SetWindowFontScale(1.2f);
			ImGui::Text(TR("Window size: (%d, %d)"), spineToolDatum.iTextureWidth, spineToolDatum.iTextureHeight);
				const auto& baseSize = pSP->getBaseSize();
				const auto& offset = pSP->getOffset();
				ImGui::Text(TR("Skeleton size: (%.2f, %.2f)"), baseSize.u, baseSize.v);
				ImGui::Text(TR("Offset: (%.2f, %.2f)"), offset.u, offset.v);

			{
				ImGui::SetWindowFontScale(1.35f);
				const float availW = ImGui::GetContentRegionAvail().x;
				const float gapW = ImGui::GetStyle().ItemSpacing.x;
				const float btnW = (availW - gapW) * 0.5f;
				if (ImGui::Button(TR("Mirror##flip"), ImVec2(btnW, 0)))
					pSP->toggleFlipX();
				ImGui::SameLine();
				if (ImGui::Button(TR("Rotate##flip"), ImVec2(btnW, 0)))
					pSP->rotate90();
			}

			ImGui::SetWindowFontScale(1.5f);
			}
			if (ImGui::CollapsingHeader(TR("Track Mix")))
			{
				const std::vector<std::string>& an = pSP->getAnimationNames();
				static ImGuiListview trackLV2;
				trackLV2.update(an, "Tracks##Tracks2");
				const float availW = ImGui::GetContentRegionAvail().x;
				const float gapW = ImGui::GetStyle().ItemSpacing.x;
				const float totalBtnW = availW * 0.8f;
				const float trackBtnW = (totalBtnW - gapW) * 0.5f;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availW - totalBtnW) * 0.5f);
				if (ImGui::Button(TR("Add##AddTracks2"), ImVec2(trackBtnW, 0))) { std::vector<std::string> c; trackLV2.pickupCheckedItems(an, c); pSP->addAnimationTracks(c, true); }
				ImGui::SameLine();
				if (ImGui::Button(TR("Clear##ClearTracks2"), ImVec2(trackBtnW, 0))) { trackLV2.clear(an); pSP->addAnimationTracks({}); }
			}
			if (ImGui::CollapsingHeader(TR("Slot")))
			{
				const std::vector<std::string>& sn = pSP->getSlotNames();

				static std::string mouseHoverSlotName;
				static bool mouseHoverClicked = false;
				static bool mouseHoverJustClicked = false;
				std::string listHoveredSlot;
				if (ImGui::TreeNodeEx(TR("Exclude slot by items"), ImGuiTreeNodeFlags_DefaultOpen))
				{
										auto& slotChecks2 = slot_exclusion::s_slotChecks;
					static unsigned int s_slotChecksGeneration = 0;
					bool slotChecksNeedReinit = (slotChecks2.size() != sn.size()) || (s_slotChecksGeneration != spineToolDatum.loadGeneration);
					if (slotChecksNeedReinit)
					{
						s_slotChecksGeneration = spineToolDatum.loadGeneration;
						slotChecks2.resize(sn.size());
						auto* cb = HasSlotExclusionFilter() ? GetSlotExcludeCallback() : nullptr;
						auto* extCb = slot_exclusion::s_externalCallback;
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
						pSP->setSlotsToExclude(excluded);
						pSP->setSlotExcludeCallback(nullptr);
							slot_exclusion::s_nFilterLength = 0;
							slot_exclusion::s_szFilter[0] = '\0';
					}

					if (ImGui::Button(TR("Clear##ClearExcSlots2"), ImVec2(S(106.7f), 0)))
					{
						slotChecks2.assign(sn.size(), true);
						pSP->setSlotsToExclude({});
							pSP->setSlotExcludeCallback(nullptr);
							slot_exclusion::s_nFilterLength = 0;
							slot_exclusion::s_szFilter[0] = '\0';
					}
					ImGui::TreePop();
				}
				if (ImGui::TreeNode(TR("Exclude slot by filter")))
				{
					using namespace slot_exclusion;
					ImGui::InputText(TR("Filter"), s_szFilter, sizeof(s_szFilter));
					if (ImGui::Button(TR("Apply##FilterSlot2"), ImVec2(S(106.7f), 0)))
						{
							s_nFilterLength = strlen(s_szFilter);
								if (s_slotChecks.size() != sn.size())
									s_slotChecks.assign(sn.size(), true);
							for (size_t i = 0; i < sn.size(); ++i)
							{
								s_slotChecks[i] = !(s_nFilterLength != 0 && IsSlotToBeExcluded(sn[i].c_str(), sn[i].size()));
							}
							std::vector<std::string> excluded;
							for (size_t i = 0; i < sn.size(); ++i)
								if (!s_slotChecks[i]) excluded.push_back(sn[i]);
							pSP->setSlotExcludeCallback(nullptr);
							pSP->setSlotsToExclude(excluded);
						}
					ImGui::TreePop();
				}
				if (ImGui::TreeNode(TR("Mouse slot hover")))
				{
					bool& mouseHoverEnabled = spineToolDatum.mouseHoverEnabled;
					static float mouseHoverThick = 3.2f;
					static ImVec4 mouseHoverColor = ImVec4(0.f, 1.f, 0.f, 1.f);


					DxLib::MATRIX mat = pSP->calculateTransformMatrix();
					{
						DxLib::MATRIX panelOffsetMat = DxLib::MGetTranslate(DxLib::VGet(spineToolDatum.leftPanelEndX, 0.f, 0.f));
						mat = DxLib::MMult(mat, panelOffsetMat);
					}

					ImGui::Checkbox(TR("Enable##MouseHover"), &mouseHoverEnabled);
					if (mouseHoverEnabled && !sn.empty())
					{
						DxLib::MATRIX invMat = DxLib::MInverse(mat);
						ImVec2 mp = ImGui::GetIO().MousePos;
						DxLib::VECTOR mLocal = DxLib::VTransform(DxLib::VGet(mp.x, mp.y, 0.f), invMat);


						const auto& slotChecksRef = slot_exclusion::s_slotChecks;
						std::string hitSlot;
						for (size_t si = 0; si < sn.size(); ++si)
						{

							if (si < slotChecksRef.size() && !slotChecksRef[si]) continue;
							const auto& slotName = sn[si];
							SlotMeshData meshData;
							if (pSP->getSlotMeshData(slotName, meshData) && meshData.worldVertices.size() >= 2)
							{

								if (PointInSlotMesh(mLocal.x, mLocal.y, meshData))
									hitSlot = slotName;
							}
							else
							{

								const auto& b = pSP->getCurrentBoundingOfSlot(slotName);
								if (b.z == 0.f) continue;
								float x0 = b.x, y0 = b.y, x1 = b.x + b.z, y1 = b.y + b.w;
								if (x0 > x1) std::swap(x0, x1);
								if (y0 > y1) std::swap(y0, y1);
								if (mLocal.x >= x0 && mLocal.x <= x1 && mLocal.y >= y0 && mLocal.y <= y1)
									hitSlot = slotName;
							}
						}


						bool isMouseOnCanvas = mp.x > spineToolDatum.leftPanelEndX && mp.x < spineToolDatum.rightPanelStartX;
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
							SlotMeshData lMeshData;
							if (pSP->getSlotMeshData(listHoveredSlot, lMeshData) && lMeshData.worldVertices.size() >= 2)
							{
								DrawSlotAlphaOutline(lMeshData, lcol, mouseHoverThick, &mat);
							}
							else
							{
								const auto& lb = pSP->getCurrentBoundingOfSlot(listHoveredSlot);
								if (lb.z != 0.f)
								{
									DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
									DxLib::SetTransformTo2D(&mat);
									DxLib::DrawBoxAA(lb.x, lb.y, lb.x+lb.z, lb.y+lb.w, lcol, 0, mouseHoverThick);
									DxLib::ResetTransformTo2D();
								}
							}
						}


						if (!hitSlot.empty())
						{
							unsigned int col = ImGui::ColorConvertFloat4ToU32(mouseHoverColor);
							col = ((col&0xff)<<16)|((col&0xff0000)>>16)|(col&0xff00ff00);
							SlotMeshData hMeshData;
							if (pSP->getSlotMeshData(hitSlot, hMeshData) && hMeshData.worldVertices.size() >= 2)
							{
								DrawSlotAlphaOutline(hMeshData, col, mouseHoverThick, &mat);
							}
							else
							{
								const auto& b = pSP->getCurrentBoundingOfSlot(hitSlot);
								if (b.z != 0.f)
								{
									DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
									DxLib::SetTransformTo2D(&mat);
									DxLib::DrawBoxAA(b.x, b.y, b.x+b.z, b.y+b.w, col, 0, mouseHoverThick);
									DxLib::ResetTransformTo2D();
								}
							}
						}

						if (mouseHoverClicked && !mouseHoverSlotName.empty())
						{
							const auto& sb = pSP->getCurrentBoundingOfSlot(mouseHoverSlotName);
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
					ImGui::TreePop();
				}
			}

			if (spineToolDatum.pAnimQueue)
			{
				auto* queue = spineToolDatum.pAnimQueue;
				const auto& an = pSP->getAnimationNames();
				bool queuePlaying = spineToolDatum.isQueuePlaying && spineToolDatum.isQueuePlaying();
				size_t queueIdx = spineToolDatum.getQueueIndex ? spineToolDatum.getQueueIndex() : 0;

				if (ImGui::CollapsingHeader(TR("Queue")))
				{
					static int s_queueSelectIdx = 0;
					if (s_queueSelectIdx >= (int)an.size()) s_queueSelectIdx = 0;
					float addBtnW = 70.f;
					float comboW = ImGui::GetContentRegionAvail().x - addBtnW - 4;
					ImGui::SetNextItemWidth(comboW);
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

					float hw = (ImGui::GetContentRegionAvail().x - 4) * 0.5f;
					if (queuePlaying) ImGui::BeginDisabled();
					if (ImGui::Button(TR("Play"), ImVec2(hw, 0)))
						if (spineToolDatum.onQueuePlay) spineToolDatum.onQueuePlay();
					if (queuePlaying) ImGui::EndDisabled();
					ImGui::SameLine(0, S(2.6667f));
					if (!queuePlaying) ImGui::BeginDisabled();
					if (ImGui::Button(TR("Stop"), ImVec2(hw, 0)))
						if (spineToolDatum.onQueueStop) spineToolDatum.onQueueStop();
					if (!queuePlaying) ImGui::EndDisabled();

					float delBtnW = 28.f;
					for (int i = 0; i < (int)queue->size(); i++)
					{
						bool isCurrent = queuePlaying && ((size_t)i == queueIdx);
						if (isCurrent)
							ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

						float dur = pSP->getAnimationDuration((*queue)[i].c_str());
						char durStr[16] = "";
						if (dur > 0.f) snprintf(durStr, sizeof(durStr), "%.2fs", dur);
						float durW = dur > 0.f ? ImGui::CalcTextSize(durStr).x + S(2.6667f) : 0.f;
						float availW = ImGui::GetContentRegionAvail().x;

						ImGui::Text("%d  %s", i, (*queue)[i].c_str());
						if (dur > 0.f)
						{
							ImGui::SameLine(availW - durW - delBtnW - S(2.6667f));
							ImGui::TextUnformatted(durStr);
						}
						ImGui::SameLine(availW - delBtnW);
						ImGui::PushID(i);
						if (ImGui::Button("X##q", ImVec2(delBtnW, 0)))
						{
							queue->erase(queue->begin() + i);
							--i;
						}
						ImGui::PopID();
						if (isCurrent) ImGui::PopStyleColor();
					}

					if (ImGui::Button(TR("Clear##ClearQueue"), ImVec2(S(106.7f), 0)))
					{
						queue->clear();
						if (spineToolDatum.onQueueStop) spineToolDatum.onQueueStop();
					}
				}
			}

			{
				bool wasOpen = s_exportPanelOpen;
				if (wasOpen)
					ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				if (ImGui::Button(TR("Export"), ImVec2(rightBtnWidth, S(26.7f))))
				{
					s_exportPanelOpen = !s_exportPanelOpen;
					if (s_exportPanelOpen) s_exportEverOpened = true;
				}
				if (wasOpen)
					ImGui::PopStyleColor();
			}

		}


		ImGui::Separator();
		ImGui::SetWindowFontScale(1.5f);
		if (ImGui::Button(TR("Select Folder"), ImVec2(rightBtnWidth, S(30.0f))))
			if (spineToolDatum.onPickFolder) spineToolDatum.onPickFolder();
		ImGui::Separator();
		float remainingHeight = ImGui::GetContentRegionAvail().y;
		float minHeight = S(266.7f);
		float fileListHeight = remainingHeight > minHeight ? remainingHeight : minHeight;
		ImGui::BeginChild("FileList", ImVec2(-S(13.3333f), fileListHeight), false,
			ImGuiWindowFlags_NoNav);
			const auto& fileList = spineToolDatum.skelFileList;
			static std::wstring s_lastScrolledFile;
		for (size_t fi = 0; fi < fileList.size(); fi++)
		{
			const std::wstring& wpath = fileList[fi];
			size_t slash = wpath.find_last_of(L"/\\");
			std::wstring wname = (slash != std::wstring::npos) ? wpath.substr(slash + 1) : wpath;
			std::string name = win_text::NarrowUtf8(wname);

			bool isCurrent = (wpath == spineToolDatum.currentSkelFile);
			std::string label = name + "##f" + std::to_string(fi);
			if (ImGui::Selectable(label.c_str(), isCurrent))
				if (spineToolDatum.onPlayFile) spineToolDatum.onPlayFile(wpath);


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
				std::string folder = win_text::NarrowUtf8(wfolder);
				if (!folder.empty()) ImGui::SetTooltip("%s", folder.c_str());
			}
		}
		ImGui::EndChild();

		ImGui::EndChild();
	ImGui::EndChild();

		if (!spineLoaded)
			s_exportPanelOpen = false;
	}

	ImGui::PopStyleVar();
	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();


	if (!s_panelsHidden)
	{
		static bool s_splitterDragging = false;
		const float splitterW = S(10.0f);
		const float tbH2 = custom_titlebar::TitleBarHeight();

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


	spineToolDatum.leftPanelEndX = s_panelsHidden ? 0.f : (leftPanelWidth + rightPanelWidth + S(5.3333f));


	if (s_exportPanelOpen && spineLoaded)
	{
		static float s_exportPanelH  = 0.f;
		static float s_exportSlideX  = 0.f;
		static bool  s_exportVisible = false;

		const float shownX  = io.DisplaySize.x - exportPanelWidth;
		const float hiddenX = io.DisplaySize.x;


		if (!s_exportVisible)
		{
			s_exportSlideX  = hiddenX;
			s_exportVisible = true;
		}


		float t = 10.f * io.DeltaTime;
		if (t > 1.f) t = 1.f;
		s_exportSlideX += (shownX - s_exportSlideX) * t;

		const float availH  = io.DisplaySize.y - tbH;
		const float exportY = tbH + (s_exportPanelH > 0.f ? (availH - s_exportPanelH) * 0.5f : availH * 0.25f);
		ImGui::SetNextWindowPos(ImVec2(s_exportSlideX, exportY), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(exportPanelWidth, 0), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.95f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(6.6667f), S(6.6667f)));
		ImGui::Begin("##ExportFloat", nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize   |
			ImGuiWindowFlags_NoMove     |
			ImGuiWindowFlags_NoCollapse);
		ImGui::SetWindowFontScale(1.5f);


		float btnCloseW = exportPanelWidth - 16.f;
		if (ImGui::Button(">>", ImVec2(btnCloseW, S(30.0f))))
		{
			s_exportPanelOpen = false;
			s_exportVisible   = false;
		}
		ImGui::Spacing();

		ImVec4 headerColor   = ImGui::GetStyleColorVec4(ImGuiCol_Header);
		ImVec4 headerHovered = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);
		ImVec4 headerActive  = ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive);
		ImGui::PushStyleColor(ImGuiCol_Button, headerColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, headerHovered);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, headerActive);


		constexpr int minFps = 15, maxImgFps = 90, maxVidFps = 240;
		bool recording = spineToolDatum.isRecording && spineToolDatum.isRecording();

		CSpineRuntimeRegistry* pEP = static_cast<CSpineRuntimeRegistry*>(spineToolDatum.pSpinePlayer);
		ISpinePlayer* pSPE = pEP->ActiveRuntime();

		if (spineToolDatum.isWebmEncoding)
		{
			ImGui::TextUnformatted(TR("Encoding WebM..."));
			ImGui::ProgressBar(1.0f, ImVec2(-1, S(20.0f)));
		}
		else if (spineToolDatum.isWritingFrames)
		{

			ImGui::TextUnformatted(TR("Writing frames..."));
			const int w = spineToolDatum.recordFramesWritten;
			const int t = spineToolDatum.recordFramesTotal;
			float progress = (t > 0) ? ((float)w / (float)t) : 0.f;
			if (progress > 1.f) progress = 1.f;
			char szBuf[32]{};
			snprintf(szBuf, sizeof(szBuf), "%d / %d", w, t);
			ImGui::ProgressBar(progress, ImVec2(-1, S(20.0f)), szBuf);
		}
		else if (recording)
		{

			if (spineToolDatum.recordFramesTotal > 0)
			{
				float progress = (float)spineToolDatum.recordFramesCaptured / (float)spineToolDatum.recordFramesTotal;
				if (progress > 1.f) progress = 1.f;
				char szBuf[32]{};
				snprintf(szBuf, sizeof(szBuf), "%d / %d", spineToolDatum.recordFramesCaptured, spineToolDatum.recordFramesTotal);
				ImGui::ProgressBar(progress, ImVec2(-1, S(20.0f)), szBuf);
			}
			if (!spineToolDatum.toExportPerAnim)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.f));
				if (ImGui::Button(TR("Stop Recording"), ImVec2(-1, S(40.0f))))
					if (spineToolDatum.onEndRecording) spineToolDatum.onEndRecording();
				ImGui::PopStyleColor();
			}
		}
		else
		{
			ImGui::SeparatorText(TR("Snapshot"));
			float halfW = (ImGui::GetContentRegionAvail().x - 4) * 0.5f;
			if (ImGui::Button("PNG", ImVec2(halfW, S(28.0f))))
				if (spineToolDatum.onSnapPng) spineToolDatum.onSnapPng();
			ImGui::SameLine(0, S(2.6667f));
			if (ImGui::Button("JPG", ImVec2(halfW, S(28.0f))))
				if (spineToolDatum.onSnapJpg) spineToolDatum.onSnapJpg();
			{
				bool alphaOn = spineToolDatum.exportWithAlpha;
				if (alphaOn)
					ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				if (ImGui::Button(alphaOn ? TR("Alpha ON") : TR("Alpha OFF"), ImVec2(-1, S(28.0f))))
					spineToolDatum.exportWithAlpha = !alphaOn;
				if (alphaOn)
					ImGui::PopStyleColor();
			}

			ImGui::SeparatorText(TR("Record"));
			if (ImGui::Button(TR("Export GIF"), ImVec2(-1, S(28.0f))))
				if (spineToolDatum.onExportGif) spineToolDatum.onExportGif();
			if (ImGui::Button(TR("Export mp4"), ImVec2(-1, S(28.0f))))
				if (spineToolDatum.onExportVideo) spineToolDatum.onExportVideo();
			if (spineToolDatum.isWebmEncoding)
			{
				ImGui::BeginDisabled();
				ImGui::Button(TR("WebM Encoding..."), ImVec2(-1, S(28.0f)));
				ImGui::EndDisabled();
			}
			else
			{
				if (ImGui::Button(TR("Export WebM"), ImVec2(-1, S(28.0f))))
					if (spineToolDatum.onExportWebm) spineToolDatum.onExportWebm();
			}
			if (spineToolDatum.toExportPerAnim)
			{
				if (ImGui::Button(TR("Export PNGs"), ImVec2(-1, S(28.0f))))
					if (spineToolDatum.onExportPngs) spineToolDatum.onExportPngs();
				if (ImGui::Button(TR("Export JPGs"), ImVec2(-1, S(28.0f))))
					if (spineToolDatum.onExportJpgs) spineToolDatum.onExportJpgs();
			}

			ImGui::SeparatorText(TR("Export FPS"));
			ScrollableSliderInt(TR("Image"), &spineToolDatum.iImageFps, minFps, maxImgFps);
			ScrollableSliderInt(TR("Video"), &spineToolDatum.iVideoFps, minFps, maxVidFps);
			{
				bool freeRecord = !spineToolDatum.toExportPerAnim;
				if (freeRecord)
					ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				if (ImGui::Button(freeRecord ? (TR("Free recording ON")) : (TR("Free recording OFF")), ImVec2(-1, S(28.0f))))
					spineToolDatum.toExportPerAnim = freeRecord;
				if (freeRecord)
					ImGui::PopStyleColor();
			}
		}

		ImGui::PopStyleColor(3);
		s_exportPanelH = ImGui::GetWindowSize().y;
		ImGui::PopStyleVar();
		ImGui::End();
	}
	else if (!s_exportPanelOpen && spineLoaded && s_exportEverOpened)
	{

		static float s_exportBtnSlideX = 99999.f;
		const float btnW = S(133.3f);
		const float btnH = S(33.3f);
		const float hiddenX = io.DisplaySize.x;
		const float shownX  = io.DisplaySize.x - btnW - S(2.6667f);
		const float triggerZone = S(133.3f);

		float mouseX = io.MousePos.x;
		float mouseY = io.MousePos.y;
		bool inTrigger = (mouseX >= io.DisplaySize.x - triggerZone &&
		                  mouseY >= tbH && mouseY < io.DisplaySize.y);


		if (s_exportBtnSlideX > hiddenX) s_exportBtnSlideX = hiddenX;

		float target = inTrigger ? shownX : hiddenX;
		float t = 8.f * io.DeltaTime;
		if (t > 1.f) t = 1.f;
		s_exportBtnSlideX += (target - s_exportBtnSlideX) * t;

		if (s_exportBtnSlideX < hiddenX - 1.f)
		{
			ImGuiWindowFlags extraFlags = (s_exportBtnSlideX > shownX + btnW * 0.8f)
				? ImGuiWindowFlags_NoInputs : 0;

			const float availH = io.DisplaySize.y - tbH;
			const float btnY   = tbH + (availH - btnH) * 0.5f;
			ImGui::SetNextWindowPos(ImVec2(s_exportBtnSlideX, btnY), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(btnW + S(2.6667f), btnH + S(2.6667f)));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(1.3333f), S(1.3333f)));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::Begin("##ExportToggleBtn", nullptr,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoBackground | extraFlags);
			if (ImGui::Button("<<", ImVec2(btnW, btnH)))
			{
				s_exportPanelOpen = true;
				s_exportBtnSlideX = hiddenX;
			}
			ImGui::End();
			ImGui::PopStyleVar(2);
		}
	}


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


	static int settingPage = 0;
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	if (showSettingWindow)
	{
		ImGui::OpenPopup("SettingWindow");
		showSettingWindow = false;
		settingPage = 0;
	}

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
			if (spineToolDatum.onProButtonClicked) spineToolDatum.onProButtonClicked();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(3);

	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(S(800.f), S(480.f)));
	if (ImGui::BeginPopupModal("SettingWindow", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse))
	{
		spineToolDatum.isSettingWindowOpen = true;
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

			ImGui::Text("Language Settings");
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
			i18n::load(i18n::detail::exeDir(), id);
#ifdef _WIN32

					{
						const std::string fontsDir = GetWindowsFontsDir();
						ImGuiIO& io = ImGui::GetIO();
						const auto& fontAtlas = io.Fonts;
						float curFontSize = s_currentFontSize;

						const std::wstring bundledFontPath = path_util::GetBundledFontPath();
						const bool hasBundledFont = !bundledFontPath.empty();
						const char* mainFontName = nullptr;
						if (id == "ko_KR")
						{

							mainFontName = "malgun.ttf";
						}
						else
						{

							mainFontName = hasBundledFont ? nullptr : "malgun.ttf";
						}
						std::string mainFontPath = (mainFontName != nullptr)
							? (fontsDir + mainFontName)
							: win_text::NarrowUtf8(bundledFontPath);

						fontAtlas->Clear();
						const ImWchar* glyphCJK = fontAtlas->GetGlyphRangesChineseFull();
						const ImWchar* glyphKR = fontAtlas->GetGlyphRangesKorean();
						if (id == "ko_KR")
						{
							fontAtlas->AddFontFromFileTTF(mainFontPath.c_str(), curFontSize, nullptr, glyphKR);

							ImFontConfig cfgMerge;
							cfgMerge.MergeMode = true;
							const std::string cjkMergePath = hasBundledFont
								? win_text::NarrowUtf8(bundledFontPath)
								: (fontsDir + "malgun.ttf");
							fontAtlas->AddFontFromFileTTF(cjkMergePath.c_str(), curFontSize, &cfgMerge, glyphCJK);
						}
						else
						{
							fontAtlas->AddFontFromFileTTF(mainFontPath.c_str(), curFontSize, nullptr, glyphCJK);

							ImFontConfig cfgMerge;
							cfgMerge.MergeMode = true;
							fontAtlas->AddFontFromFileTTF((fontsDir + "malgun.ttf").c_str(), curFontSize, &cfgMerge, glyphKR);
						}


						ImFont* titleBarFont = fontAtlas->AddFontFromFileTTF(
							(fontsDir + "segoesc.ttf").c_str(), 40.0f * custom_titlebar::GetScale());
						spineToolDatum.titleBar.customFont = titleBarFont;
						const std::string subTitleFontPath = bundledFontPath.empty()
							? (fontsDir + "malgun.ttf")
							: win_text::NarrowUtf8(bundledFontPath);
						ImFont* subTitleFont = fontAtlas->AddFontFromFileTTF(
							subTitleFontPath.c_str(), 26.7f * custom_titlebar::GetScale(), nullptr, glyphCJK);
						spineToolDatum.titleBar.subTitleFont = subTitleFont;

						ImGuiStyle& style = ImGui::GetStyle();
						style._NextFrameFontSizeBase = curFontSize;
						s_currentFontSize = curFontSize;

						ImGui_ImplDX11_InvalidateDeviceObjects();
						ImGui_ImplDX11_CreateDeviceObjects();
					}
#endif
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
				if (spineToolDatum.onLoadTitleBg) spineToolDatum.onLoadTitleBg();

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

				auto& tb = spineToolDatum.titleBar;
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

			// Font size slider + Apply
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
					const std::string fontsDir = GetWindowsFontsDir();
					const std::wstring bundledFontPath = path_util::GetBundledFontPath();
					const bool hasBundledFont = !bundledFontPath.empty();
					const std::string mainFontPath = hasBundledFont
						? win_text::NarrowUtf8(bundledFontPath)
						: (fontsDir + "malgun.ttf");
					const auto& fontAtlas = io.Fonts;
					float applySize = s_fontSizeSlider;

					fontAtlas->Clear();
					const ImWchar* glyphCJK = fontAtlas->GetGlyphRangesChineseFull();
					const ImWchar* glyphKR = fontAtlas->GetGlyphRangesKorean();

					if (i18n::currentLang() == "ko_KR")
					{
						fontAtlas->AddFontFromFileTTF((fontsDir + "malgun.ttf").c_str(), applySize, nullptr, glyphKR);
						ImFontConfig cfgMerge;
						cfgMerge.MergeMode = true;
						fontAtlas->AddFontFromFileTTF(mainFontPath.c_str(), applySize, &cfgMerge, glyphCJK);
					}
					else
					{
						fontAtlas->AddFontFromFileTTF(mainFontPath.c_str(), applySize, nullptr, glyphCJK);
						ImFontConfig cfgMerge;
						cfgMerge.MergeMode = true;
						fontAtlas->AddFontFromFileTTF((fontsDir + "malgun.ttf").c_str(), applySize, &cfgMerge, glyphKR);
					}

					ImFont* titleBarFont = fontAtlas->AddFontFromFileTTF(
						(fontsDir + "segoesc.ttf").c_str(), 40.0f * custom_titlebar::GetScale());
					spineToolDatum.titleBar.customFont = titleBarFont;
					ImFont* subTitleFont = fontAtlas->AddFontFromFileTTF(
						mainFontPath.c_str(), 26.7f * custom_titlebar::GetScale(), nullptr, glyphCJK);
					spineToolDatum.titleBar.subTitleFont = subTitleFont;

					ImGuiStyle& styleRef = ImGui::GetStyle();
					styleRef._NextFrameFontSizeBase = applySize;
					s_currentFontSize = applySize;

					ImGui_ImplDX11_InvalidateDeviceObjects();
					ImGui_ImplDX11_CreateDeviceObjects();
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
				s_bgCol[0] = spineToolDatum.renderBgR / 255.f;
				s_bgCol[1] = spineToolDatum.renderBgG / 255.f;
				s_bgCol[2] = spineToolDatum.renderBgB / 255.f;
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
				spineToolDatum.renderBgR = r;
				spineToolDatum.renderBgG = g;
				spineToolDatum.renderBgB = b;
				if (spineToolDatum.onSetRenderBgColor)
					spineToolDatum.onSetRenderBgColor(r, g, b);
			}

			if (ImGui::Button(TR("Reset to Default##renderbg"), ImVec2(-1, S(40))))
			{
				s_bgCol[0] = 0.f; s_bgCol[1] = 0.f; s_bgCol[2] = 0.f;
				spineToolDatum.renderBgR = 0;
				spineToolDatum.renderBgG = 0;
				spineToolDatum.renderBgB = 0;
				if (spineToolDatum.onSetRenderBgColor)
					spineToolDatum.onSetRenderBgColor(0, 0, 0);
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
		spineToolDatum.isSettingWindowOpen = false;
	}



}
#undef S

bool spine_panel::HasSlotExclusionFilter()
{
	return slot_exclusion::s_nFilterLength > 0;
}

bool(*spine_panel::GetSlotExcludeCallback())(const char*, size_t)
{
	return &slot_exclusion::IsSlotToBeExcluded;
}

void spine_panel::SetExternalSlotExcludeCallback(bool (*pFunc)(const char*, size_t))
{
	slot_exclusion::s_externalCallback = pFunc;
}

void spine_panel::ClearExternalSlotExcludeCallback()
{
	slot_exclusion::s_externalCallback = nullptr;
}
