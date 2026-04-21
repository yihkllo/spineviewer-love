#ifndef SL_APP_MENU_H_
#define SL_APP_MENU_H_

#include <Windows.h>

namespace window_menu
{
	struct MenuItem
	{
		UINT_PTR id = 0;
		const wchar_t* name = nullptr;
		HMENU child = nullptr;
	};

	namespace detail
	{
		inline BOOL AppendItem(HMENU hMenu, const MenuItem& mi)
		{
			if (mi.child)
				return ::AppendMenuW(hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(mi.child), mi.name);
			if (!mi.name)
				return ::AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
			return ::AppendMenuW(hMenu, MF_STRING, mi.id, mi.name);
		}
	}

	class MenuBuilder
	{
	public:
		template <size_t N>
		MenuBuilder(const MenuItem(&items)[N])
			: m_hMenu(::CreateMenu())
		{
			for (const auto& mi : items)
			{
				if (!IsValid()) { DestroyOnError(); break; }
				m_ok = detail::AppendItem(m_hMenu, mi);
			}
		}
		~MenuBuilder() { DestroyOnError(); }

		MenuBuilder(const MenuBuilder&) = delete;
		MenuBuilder& operator=(const MenuBuilder&) = delete;

		HMENU Get() const { return m_hMenu; }
	private:
		HMENU m_hMenu;
		BOOL  m_ok = TRUE;

		bool IsValid() const { return m_hMenu && m_ok; }
		void DestroyOnError()
		{
			if (m_hMenu && !m_ok)
			{
				::DestroyMenu(m_hMenu);
				m_hMenu = nullptr;
			}
		}
	};

	HMENU GetMenuInBar(HWND hOwnerWindow, unsigned int index);
	bool SetMenuCheckState(HMENU hMenu, unsigned int index, bool checked);
	void EnableMenuItems(HMENU hMenu, const unsigned int* indices, size_t count, bool toEnable);
	template<size_t N>
	void EnableMenuItems(HMENU hMenu, const unsigned int(&indices)[N], bool toEnable)
	{
		EnableMenuItems(hMenu, indices, N, toEnable);
	}

	class CContextMenu
	{
	public:
		CContextMenu() : m_hMenu(::CreatePopupMenu()) {}
		~CContextMenu() { if (m_hMenu) ::DestroyMenu(m_hMenu); }

		CContextMenu(const CContextMenu&) = delete;
		CContextMenu& operator=(const CContextMenu&) = delete;

		void AddItems(const MenuItem* items, size_t count)
		{
			for (size_t i = 0; i < count && IsValid(); ++i)
				m_ok = detail::AppendItem(m_hMenu, items[i]);
		}

		template <size_t N>
		void AddItems(const MenuItem(&items)[N]) { AddItems(items, N); }


		BOOL Display(HWND hOwner) const
		{
			if (!::IsMenu(m_hMenu) || !::IsWindow(hOwner)) return -1;
			POINT pt{};
			::GetCursorPos(&pt);
			return ::TrackPopupMenu(m_hMenu,
				TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD,
				pt.x, pt.y, 0, hOwner, nullptr);
		}

	private:
		HMENU m_hMenu;
		BOOL  m_ok = TRUE;

		bool IsValid() const { return m_hMenu && m_ok; }
	};
}

#endif
