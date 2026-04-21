
#include "custom_titlebar.h"

#include <unordered_map>

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>
#include <imgui.h>

#if defined(_WIN32)
#include <d3d11.h>
#endif

namespace custom_titlebar
{
	static float s_scale = 1.0f;

	float GetScale() { return s_scale; }
	void  SetScale(float scale) { s_scale = scale < 0.5f ? 0.5f : scale; }


	static ID3D11ShaderResourceView* GetOrCreateSRV(int hDxLib)
	{
		static std::unordered_map<int, ID3D11ShaderResourceView*> s_cache;
		auto it = s_cache.find(hDxLib);
		if (it != s_cache.end()) return it->second;

		auto* tex2D = static_cast<ID3D11Texture2D*>(
			const_cast<void*>(DxLib::GetGraphID3D11Texture2D(hDxLib)));
		if (!tex2D) return nullptr;

		ID3D11Device* dev = static_cast<ID3D11Device*>(
			const_cast<void*>(DxLib::GetUseDirect3D11Device()));
		if (!dev) return nullptr;

		D3D11_TEXTURE2D_DESC desc{};
		tex2D->GetDesc(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;

		ID3D11ShaderResourceView* srv = nullptr;
		dev->CreateShaderResourceView(tex2D, &srvDesc, &srv);
		s_cache[hDxLib] = srv;
		return srv;
	}

	static ImTextureID GetImTexId(int hDxLib)
	{
		if (hDxLib == -1) return (ImTextureID)0;
		auto* srv = GetOrCreateSRV(hDxLib);
		return (ImTextureID)(uintptr_t)srv;
	}

	void Draw(const STitleBarConfig& cfg)
	{
		HWND hWnd = cfg.hWnd;
		ImGuiIO& io = ImGui::GetIO();
		const float W  = io.DisplaySize.x;
		const float sc = s_scale;
		const float H  = kBaseTitleBarHeight * sc;

		ImVec4 bgImCol(cfg.bgColor[0], cfg.bgColor[1], cfg.bgColor[2], cfg.bgColor[3]);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(W, H));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,   0.f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, bgImCol);

		ImGui::Begin("##TitleBar", nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize   |
			ImGuiWindowFlags_NoMove     |
			ImGuiWindowFlags_NoScrollbar|
			ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNav);

		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImVec2 winPos = ImGui::GetWindowPos();

		if (ImTextureID bgTex = GetImTexId(cfg.hBgDxLib))
		{
			dl->AddImage(bgTex,
				ImVec2(winPos.x,     winPos.y),
				ImVec2(winPos.x + W, winPos.y + H));
		}
		else
		{
			ImU32 col = ImGui::ColorConvertFloat4ToU32(bgImCol);
			dl->AddRectFilled(
				ImVec2(winPos.x, winPos.y),
				ImVec2(winPos.x + W, winPos.y + H), col);
		}

		const float iconSize = kBaseIconSize * sc;
		float curX = 8.f * sc;
		if (ImTextureID iconTex = GetImTexId(cfg.hIconDxLib))
		{
			float iconY = (H - iconSize) * 0.5f;
			dl->AddImage(iconTex,
				ImVec2(winPos.x + curX,             winPos.y + iconY),
				ImVec2(winPos.x + curX + iconSize,  winPos.y + iconY + iconSize));
			curX += iconSize + 24.f * sc;
		}

		{
			const float scaledFontSize = cfg.fontSize * sc;
			ImFont* font = cfg.customFont ? static_cast<ImFont*>(cfg.customFont) : ImGui::GetFont();
			ImVec2 textSize = font->CalcTextSizeA(scaledFontSize, FLT_MAX, 0.0f, cfg.title.c_str());
			float textY = (H - textSize.y) * 0.5f;
			dl->AddText(font, scaledFontSize,
				ImVec2(winPos.x + curX, winPos.y + textY),
				IM_COL32(
					(int)(cfg.textColor[0]*255), (int)(cfg.textColor[1]*255),
					(int)(cfg.textColor[2]*255), (int)(cfg.textColor[3]*255)),
				cfg.title.c_str());
			curX += textSize.x + 200.f * sc;
		}

		if (!cfg.subTitle.empty())
		{
			ImFont* font = cfg.subTitleFont ? static_cast<ImFont*>(cfg.subTitleFont) : ImGui::GetFont();
			const float subFontSize = 26.7f * sc;
			ImVec2 subSize = font->CalcTextSizeA(subFontSize, FLT_MAX, 0.0f, cfg.subTitle.c_str());
			float subY = (H - subSize.y) * 0.5f;
			dl->AddText(font, subFontSize,
				ImVec2(winPos.x + curX, winPos.y + subY),
				IM_COL32(
					(int)(cfg.subTextColor[0]*255), (int)(cfg.subTextColor[1]*255),
					(int)(cfg.subTextColor[2]*255), (int)(cfg.subTextColor[3]*255)),
				cfg.subTitle.c_str());
		}

		const float btnW = kBaseTitleBarHeight * 1.5f * sc;
		float btnX = W - btnW * 3.f;
		const float ico   = H * 0.25f;
		const float thick = H * 0.03f;
		const ImU32 iconCol = IM_COL32(
			(int)(cfg.iconColor[0]*255), (int)(cfg.iconColor[1]*255),
			(int)(cfg.iconColor[2]*255), (int)(cfg.iconColor[3]*255));
		const ImU32 btnHover  = IM_COL32(
			(int)(cfg.btnHoverColor[0]*255), (int)(cfg.btnHoverColor[1]*255),
			(int)(cfg.btnHoverColor[2]*255), (int)(cfg.btnHoverColor[3]*255));
		const ImU32 btnActive = IM_COL32(
			(int)(cfg.btnActiveColor[0]*255), (int)(cfg.btnActiveColor[1]*255),
			(int)(cfg.btnActiveColor[2]*255), (int)(cfg.btnActiveColor[3]*255));

		auto drawBtnBg = [&](const char* id, ImU32 hoverCol, ImU32 activeCol) -> bool
		{
			ImGui::SetCursorPos(ImVec2(btnX, 0.f));
			ImGui::InvisibleButton(id, ImVec2(btnW, H));
			bool hovered = ImGui::IsItemHovered();
			bool active  = ImGui::IsItemActive();
			bool clicked = ImGui::IsItemClicked();
			ImU32 bg = hovered ? (active ? activeCol : hoverCol) : IM_COL32(0,0,0,0);
			if (bg)
				dl->AddRectFilled(
					ImVec2(winPos.x + btnX,       winPos.y),
					ImVec2(winPos.x + btnX + btnW, winPos.y + H), bg);
			return clicked;
		};


		btnX = W - btnW * 3.f;
		if (drawBtnBg("##min", btnHover, btnActive))
			::ShowWindow(hWnd, SW_MINIMIZE);
		{
			float mx = winPos.x + btnX + btnW * 0.5f;
			float my = winPos.y + H * 0.5f + 1.f;
			dl->AddLine(ImVec2(mx - ico*0.5f, my), ImVec2(mx + ico*0.5f, my), iconCol, thick + 0.3f);
		}


		bool isMax = ::IsZoomed(hWnd);
		btnX += btnW;
		if (drawBtnBg("##max", btnHover, btnActive))
		{
			if (isMax) ::ShowWindow(hWnd, SW_RESTORE);
			else       ::ShowWindow(hWnd, SW_MAXIMIZE);
		}
		{
			float bx = winPos.x + btnX + btnW * 0.5f - ico * 0.5f;
			float by = winPos.y + H * 0.5f - ico * 0.5f;
			if (isMax)
			{
				float off = 3.f;
				dl->AddRect(ImVec2(bx + off, by),       ImVec2(bx + ico, by + ico - off), iconCol, 0, 0, thick);
				dl->AddRect(ImVec2(bx,       by + off), ImVec2(bx + ico - off, by + ico), iconCol, 0, 0, thick);
			}
			else
			{
				dl->AddRect(ImVec2(bx, by), ImVec2(bx + ico, by + ico), iconCol, 0, 0, thick);
			}
		}


		btnX += btnW;
		if (drawBtnBg("##close", IM_COL32(240,100,120,200), IM_COL32(196,43,28,255)))
			::PostMessageW(hWnd, WM_CLOSE, 0, 0);
		{
			float bx = winPos.x + btnX + btnW * 0.5f;
			float by = winPos.y + H * 0.5f;
			float h2 = ico * 0.5f;
			dl->AddLine(ImVec2(bx - h2, by - h2), ImVec2(bx + h2, by + h2), iconCol, thick);
			dl->AddLine(ImVec2(bx + h2, by - h2), ImVec2(bx - h2, by + h2), iconCol, thick);
		}

		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(3);
	}
}
