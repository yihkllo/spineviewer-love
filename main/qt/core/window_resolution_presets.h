#pragma once

#include <cstddef>

namespace window_resolution_presets
{
	struct Preset
	{
		const char* label;
		int width;
		int height;
	};

	inline constexpr Preset kItems[] = {
		{ "Default", 0, 0 },
		{ "1920x1080", 1920, 1080 },
		{ "1920x1200", 1920, 1200 },
		{ "2560x1440", 2560, 1440 },
		{ "2560x1600", 2560, 1600 },
		{ "2880x1620", 2880, 1620 },
		{ "2880x1800", 2880, 1800 },
	};

	inline constexpr int Count()
	{
		return static_cast<int>(sizeof(kItems) / sizeof(kItems[0]));
	}

	inline const Preset* Get(int index)
	{
		return index >= 0 && index < Count() ? &kItems[index] : nullptr;
	}

	inline float UiScale(int index, float defaultScale)
	{
		const Preset* preset = Get(index);
		float scale = preset != nullptr && preset->width > 0
			? static_cast<float>(preset->width) / 1920.0f
			: defaultScale;
		if (scale < 0.5f) scale = 0.5f;
		if (scale > 3.0f) scale = 3.0f;
		return scale;
	}
}
