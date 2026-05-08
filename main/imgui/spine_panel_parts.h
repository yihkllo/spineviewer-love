#pragma once

#include <string>
#include <vector>

#include "spine_panel.h"

namespace spine_panel
{
	void RebuildPanelFonts(SpinePanelState& panel, float baseSize, const std::string& languageId);

	struct TrackPickState
	{
		std::vector<unsigned char> selected;

		void Sync(size_t itemCount);
		void Draw(const std::vector<std::string>& names, const char* listId);
		std::vector<std::string> Collect(const std::vector<std::string>& names) const;
		void Reset(size_t itemCount);
	};
}
