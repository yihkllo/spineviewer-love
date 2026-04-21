
#include "sl_app_menu.h"

HMENU window_menu::GetMenuInBar(HWND hOwnerWindow, unsigned int index)
{
	HMENU hBar = ::GetMenu(hOwnerWindow);
	return hBar ? ::GetSubMenu(hBar, index) : nullptr;
}

bool window_menu::SetMenuCheckState(HMENU hMenu, unsigned int index, bool checked)
{
	return ::CheckMenuItem(hMenu, index, checked ? MF_CHECKED : MF_UNCHECKED) != static_cast<DWORD>(-1);
}

void window_menu::EnableMenuItems(HMENU hMenu, const unsigned int* indices, size_t count, bool toEnable)
{
	const UINT flag = toEnable ? MF_ENABLED : MF_GRAYED;
	for (size_t i = 0; i < count; ++i)
		::EnableMenuItem(hMenu, indices[i], flag);
}
