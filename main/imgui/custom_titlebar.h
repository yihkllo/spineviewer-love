#ifndef CUSTOM_TITLEBAR_H_
#define CUSTOM_TITLEBAR_H_

#include <Windows.h>
#include <string>

namespace custom_titlebar
{

	static constexpr float kBaseTitleBarHeight = 37.3f;
	static constexpr float kBaseIconSize       = 32.f;


	float GetScale();
	void  SetScale(float scale);


	inline float TitleBarHeight() { return kBaseTitleBarHeight * GetScale(); }
	inline float BtnWidth()       { return TitleBarHeight() * 1.5f; }
	inline float BtnAreaWidth()   { return BtnWidth() * 3.f; }
	inline float IconSize()       { return kBaseIconSize * GetScale(); }


	static constexpr float kTitleBarHeight = kBaseTitleBarHeight;
	static constexpr float kBtnWidth       = kBaseTitleBarHeight * 1.5f;
	static constexpr float kBtnAreaWidth   = kBtnWidth * 3.f;
	static constexpr float kIconSize       = kBaseIconSize;

	struct STitleBarConfig
	{
		HWND        hWnd      = nullptr;
		std::string title     = "spinelove";
		std::string subTitle;
		float       bgColor[4]= { 1.f, 0.95f, 0.97f, 1.f };
		float       textColor[4]   = { 0.118f, 0.118f, 0.118f, 1.f };
		float       subTextColor[4]= { 0.314f, 0.314f, 0.314f, 0.784f };
		float       iconColor[4]   = { 0.118f, 0.118f, 0.118f, 0.863f };
		float       btnHoverColor[4]  = { 0.863f, 0.784f, 0.824f, 0.706f };
		float       btnActiveColor[4] = { 0.745f, 0.667f, 0.725f, 1.f };
		int hIconDxLib        = -1;
		int hBgDxLib          = -1;
		float fontSize        = 40.f;
		void* customFont      = nullptr;
		void* subTitleFont    = nullptr;
	};

	void Draw(const STitleBarConfig& cfg);
}

#endif
