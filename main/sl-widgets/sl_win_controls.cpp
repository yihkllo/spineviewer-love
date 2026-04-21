
#include "sl_win_controls.h"

namespace
{
	HINSTANCE AppInstance() { return ::GetModuleHandle(nullptr); }
}


bool CListView::Create(HWND hParentWnd, const wchar_t** columnNames, size_t columnCount, bool hasCheckBox)
{
	m_hWnd = ::CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
		WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_ALIGNLEFT | WS_TABSTOP | LVS_SINGLESEL,
		0, 0, 0, 0, hParentWnd, nullptr, AppInstance(), nullptr);
	if (!m_hWnd) return false;

	DWORD exStyle = LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP;
	if (hasCheckBox) exStyle |= LVS_EX_CHECKBOXES;
	::SendMessageW(m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, exStyle);

	LVCOLUMNW col{};
	col.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT | LVCF_WIDTH;
	col.fmt  = LVCFMT_LEFT;
	for (size_t i = 0; i < columnCount; ++i)
	{
		col.iSubItem = static_cast<int>(i);
		col.pszText  = const_cast<LPWSTR>(columnNames[i]);
		::SendMessageW(m_hWnd, LVM_INSERTCOLUMN, i, reinterpret_cast<LPARAM>(&col));
	}
	return true;
}

void CListView::AdjustWidth()
{
	if (!m_hWnd) return;

	const int colCount = GetColumnCount();
	if (colCount <= 0) return;

	RECT rc;
	::GetClientRect(m_hWnd, &rc);

	LVCOLUMNW col{};
	col.mask = LVCF_WIDTH;
	col.cx   = (rc.right - rc.left) / colCount;
	for (int i = 0; i < colCount; ++i)
		::SendMessageW(m_hWnd, LVM_SETCOLUMN, i, reinterpret_cast<LPARAM>(&col));
}

bool CListView::Add(const wchar_t** columns, size_t columnCount, bool toBottom)
{
	if (!m_hWnd) return false;

	int iItem = GetItemCount();
	if (iItem < 0) return false;

	for (size_t i = 0; i < columnCount; ++i)
	{
		LVITEMW item{};
		item.mask     = LVIF_TEXT | LVIF_PARAM;
		item.iItem    = toBottom ? iItem : 0;
		item.iSubItem = static_cast<int>(i);
		item.pszText  = const_cast<wchar_t*>(columns[i]);

		if (i == 0)
		{
			LRESULT pos = ::SendMessageW(m_hWnd, LVM_INSERTITEM, 0, reinterpret_cast<LPARAM>(&item));
			if (pos == -1) return false;
			iItem = static_cast<int>(pos);
		}
		else
		{
			if (::SendMessageW(m_hWnd, LVM_SETITEMTEXT, iItem, reinterpret_cast<LPARAM>(&item)) == -1)
				return false;
		}
	}
	return true;
}

bool CListView::Add(const std::vector<std::wstring>& columns, bool toBottom)
{
	if (columns.empty()) return false;

	std::vector<const wchar_t*> ptrs(columns.size());
	for (size_t i = 0; i < columns.size(); ++i)
		ptrs[i] = columns[i].c_str();
	return Add(ptrs.data(), ptrs.size(), toBottom);
}

void CListView::Clear() const
{
	if (m_hWnd)
		::SendMessageW(m_hWnd, LVM_DELETEALLITEMS, 0, 0);
}

void CListView::CreateSingleList(const std::vector<std::wstring>& items)
{
	if (!m_hWnd) return;
	Clear();
	for (const auto& s : items)
	{
		const wchar_t* p = s.c_str();
		Add(&p, 1);
	}
}

void CListView::CreateSingleList(const wchar_t** items, size_t itemCount)
{
	if (!m_hWnd) return;
	Clear();
	for (size_t i = 0; i < itemCount; ++i)
		Add(&items[i], 1);
}

std::vector<std::wstring> CListView::PickupCheckedItems()
{
	std::vector<std::wstring> out;
	if (!m_hWnd) return out;

	const int count = GetItemCount();
	if (count <= 0) return out;

	out.reserve(count);
	for (int i = 0; i < count; ++i)
	{
		if (ListView_GetCheckState(m_hWnd, i) != 0)
		{
			std::wstring text = GetItemText(i, 0);
			if (!text.empty())
				out.push_back(std::move(text));
		}
	}
	return out;
}

int CListView::GetColumnCount() const
{
	if (!m_hWnd) return -1;

	HWND hHeader = reinterpret_cast<HWND>(::SendMessageW(m_hWnd, LVM_GETHEADER, 0, 0));
	if (!hHeader) return -1;
	return static_cast<int>(::SendMessageW(hHeader, HDM_GETITEMCOUNT, 0, 0));
}

int CListView::GetItemCount() const
{
	if (!m_hWnd) return -1;
	return static_cast<int>(::SendMessageW(m_hWnd, LVM_GETITEMCOUNT, 0, 0));
}

std::wstring CListView::GetItemText(int iRow, int iColumn) const
{
	if (!m_hWnd) return {};

	LV_ITEMW item{};
	item.iSubItem = iColumn;

	for (int bufSize = 256; bufSize <= 1024; bufSize *= 2)
	{
		std::wstring buf(bufSize, L'\0');
		item.cchTextMax = bufSize;
		item.pszText    = &buf[0];

		int len = static_cast<int>(::SendMessageW(m_hWnd, LVM_GETITEMTEXT, iRow, reinterpret_cast<LPARAM>(&item)));
		if (len < bufSize - 1)
		{
			buf.resize(len);
			return buf;
		}
	}
	return {};
}


bool CListBox::Create(HWND hParentWnd)
{
	m_hWnd = ::CreateWindowExW(0, WC_LISTBOX, L"ListBox",
		WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_SORT | LBS_NOINTEGRALHEIGHT | WS_VSCROLL,
		0, 0, 0, 0, hParentWnd, nullptr, AppInstance(), nullptr);
	return m_hWnd != nullptr;
}

void CListBox::Add(const wchar_t* szText, bool toBottom) const
{
	if (!m_hWnd) return;
	const UINT msg = toBottom ? LB_ADDSTRING : LB_INSERTSTRING;
	::SendMessageW(m_hWnd, msg, 0, reinterpret_cast<LPARAM>(szText));
}

void CListBox::Clear() const
{
	if (m_hWnd)
		::SendMessageW(m_hWnd, LB_RESETCONTENT, 0, 0);
}

std::wstring CListBox::GetSelectedItemName()
{
	if (!m_hWnd) return {};

	const long long sel = GetSelectedItemIndex();
	if (sel == LB_ERR) return {};

	LRESULT len = ::SendMessageW(m_hWnd, LB_GETTEXTLEN, static_cast<WPARAM>(sel), 0);
	if (len == LB_ERR) return {};

	std::wstring result(static_cast<size_t>(len) + 1, L'\0');
	LRESULT written = ::SendMessageW(m_hWnd, LB_GETTEXT, static_cast<WPARAM>(sel), reinterpret_cast<LPARAM>(&result[0]));
	if (written == LB_ERR) return {};

	result.resize(written);
	return result;
}

long long CListBox::GetSelectedItemIndex() const
{
	return m_hWnd ? ::SendMessageW(m_hWnd, LB_GETCURSEL, 0, 0) : LB_ERR;
}


bool CComboBox::Create(HWND hParentWnd)
{
	m_hWnd = ::CreateWindowExW(0, WC_COMBOBOXW, L"",
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_SORT,
		0, 0, 0, 0, hParentWnd, nullptr, AppInstance(), nullptr);
	return m_hWnd != nullptr;
}

void CComboBox::Setup(const std::vector<std::wstring>& itemTexts)
{
	Clear();
	if (!m_hWnd) return;
	for (const auto& t : itemTexts)
		::SendMessageW(m_hWnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(t.c_str()));
	SetSelectedItem(0);
}

void CComboBox::Setup(const wchar_t** itemTexts, size_t itemCount)
{
	Clear();
	if (!m_hWnd) return;
	for (size_t i = 0; i < itemCount; ++i)
		::SendMessageW(m_hWnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(itemTexts[i]));
	SetSelectedItem(0);
}

int CComboBox::GetSelectedItemIndex() const
{
	return m_hWnd ? static_cast<int>(::SendMessageW(m_hWnd, CB_GETCURSEL, 0, 0)) : CB_ERR;
}

std::wstring CComboBox::GetSelectedItemText() const
{
	if (!m_hWnd) return {};

	const int sel = GetSelectedItemIndex();
	if (sel == CB_ERR) return {};

	LRESULT len = ::SendMessageW(m_hWnd, CB_GETLBTEXTLEN, static_cast<WPARAM>(sel), 0);
	if (len == CB_ERR) return {};

	std::wstring result(len + 1, L'\0');
	LRESULT written = ::SendMessageW(m_hWnd, CB_GETLBTEXT, static_cast<WPARAM>(sel), reinterpret_cast<LPARAM>(&result[0]));
	if (written == CB_ERR) return {};

	result.resize(written);
	return result;
}

int CComboBox::FindIndex(const wchar_t* szName) const
{
	return m_hWnd ? static_cast<int>(::SendMessageW(m_hWnd, CB_FINDSTRING, -1, reinterpret_cast<LPARAM>(szName))) : CB_ERR;
}

bool CComboBox::SetSelectedItem(int iIndex) const
{
	if (!m_hWnd) return false;
	LRESULT r = ::SendMessageW(m_hWnd, CB_SETCURSEL, iIndex, 0);
	return (iIndex == -1) ? (r == CB_ERR) : (r == iIndex);
}

void CComboBox::Clear() const
{
	if (m_hWnd)
		::SendMessageW(m_hWnd, CB_RESETCONTENT, 0, 0);
}


bool CButton::Create(const wchar_t* szText, HWND hParentWnd, HMENU hMenu, bool hasCheckBox)
{
	DWORD style = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	if (hasCheckBox) style |= BS_CHECKBOX;
	m_hWnd = ::CreateWindowExW(0, WC_BUTTON, szText, style,
		0, 0, 0, 0, hParentWnd, hMenu, AppInstance(), nullptr);
	return m_hWnd != nullptr;
}

void CButton::SetCheckBox(bool checked) const
{
	if (m_hWnd)
		::SendMessageW(m_hWnd, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
}

bool CButton::IsChecked() const
{
	return m_hWnd && (::SendMessageW(m_hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED);
}


bool CSlider::Create(const wchar_t* szText, HWND hParentWnd, HMENU hMenu, unsigned short usMin, unsigned short usMax, unsigned int uiRange, bool bVertical)
{
	DWORD style = WS_VISIBLE | WS_CHILD | WS_TABSTOP | TBS_TOOLTIPS | TBS_BOTH;
	if (bVertical) style |= TBS_VERT;

	m_hWnd = ::CreateWindowExW(0, TRACKBAR_CLASS, szText, style,
		0, 0, 0, 0, hParentWnd, hMenu, AppInstance(), nullptr);

	if (m_hWnd)
	{
		::SendMessageW(m_hWnd, TBM_SETRANGE, TRUE, MAKELONG(usMin, usMax));
		::SendMessageW(m_hWnd, TBM_SETPAGESIZE, TRUE, uiRange);
	}
	return m_hWnd != nullptr;
}

long long CSlider::GetPosition() const
{
	return ::SendMessageW(m_hWnd, TBM_GETPOS, 0, 0);
}

void CSlider::SetPosition(long long llPos) const
{
	::SendMessageW(m_hWnd, TBM_SETPOS, TRUE, llPos);
}

HWND CSlider::GetToolTipHandle() const
{
	return reinterpret_cast<HWND>(::SendMessageW(m_hWnd, TBM_GETTOOLTIPS, 0, 0));
}


bool CFloatSlider::Create(const wchar_t* szText, HWND hParentWnd, HMENU hMenu, float fMin, float fMax, float fRange, unsigned int uiRatio, bool bVertical)
{
	DWORD style = WS_VISIBLE | WS_CHILD | WS_TABSTOP | TBS_TOOLTIPS | TBS_BOTH;
	if (bVertical) style |= TBS_VERT;

	m_hWnd = ::CreateWindowExW(0, TRACKBAR_CLASS, szText, style,
		0, 0, 0, 0, hParentWnd, hMenu, AppInstance(), nullptr);

	if (uiRatio > 0) m_uiRatio = uiRatio;

	if (m_hWnd)
	{
		const unsigned int scaledMin   = static_cast<unsigned int>(fMin   * m_uiRatio);
		const unsigned int scaledMax   = static_cast<unsigned int>(fMax   * m_uiRatio);
		const unsigned int scaledRange = static_cast<unsigned int>(fRange * m_uiRatio);

		::SendMessageW(m_hWnd, TBM_SETRANGE, TRUE, MAKELONG(scaledMin, scaledMax));
		::SendMessageW(m_hWnd, TBM_SETPAGESIZE, TRUE, scaledRange);
	}
	return m_hWnd != nullptr;
}

float CFloatSlider::GetPosition() const
{
	return ::SendMessageW(m_hWnd, TBM_GETPOS, 0, 0) / static_cast<float>(m_uiRatio);
}

void CFloatSlider::SetPosition(float fPos) const
{
	::SendMessageW(m_hWnd, TBM_SETPOS, TRUE, static_cast<LPARAM>(fPos * m_uiRatio));
}

HWND CFloatSlider::GetToolTipHandle() const
{
	return reinterpret_cast<HWND>(::SendMessageW(m_hWnd, TBM_GETTOOLTIPS, 0, 0));
}

void CFloatSlider::OnToolTipNeedText(LPNMTTDISPINFOW pInfo) const
{
	if (!pInfo) return;

	int decimals = 0;
	for (unsigned int v = m_uiRatio; v > 1; v /= kDefaultRatio)
		++decimals;
	swprintf_s(pInfo->szText, L"%0.*f", decimals, GetPosition());
}


bool CStatic::Create(const wchar_t* szText, HWND hParentWnd, bool hasEdge)
{
	DWORD style = WS_VISIBLE | WS_CHILD;
	if (hasEdge) style |= SS_ETCHEDHORZ;
	m_hWnd = ::CreateWindowExW(0, WC_STATIC, szText, style,
		0, 0, 0, 0, hParentWnd, nullptr, AppInstance(), nullptr);
	return m_hWnd != nullptr;
}


bool CEdit::Create(const wchar_t* initialText, HWND hParentWnd, bool bReadOnly, bool bBorder, bool bNumber, bool bPassword)
{
	DWORD style = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	if (bReadOnly) style |= ES_READONLY;
	if (bBorder)   style |= WS_BORDER;
	if (bNumber)   style |= ES_NUMBER;
	if (bPassword) style |= ES_PASSWORD;

	m_hWnd = ::CreateWindowExW(0, WC_EDIT, initialText, style,
		0, 0, 0, 0, hParentWnd, nullptr, AppInstance(), nullptr);
	return m_hWnd != nullptr;
}

std::wstring CEdit::GetText() const
{
	const int len = ::GetWindowTextLengthW(m_hWnd);
	if (len == 0) return {};

	std::wstring result(len + 1, L'\0');
	LRESULT written = ::SendMessageW(m_hWnd, WM_GETTEXT, static_cast<WPARAM>(result.size()), reinterpret_cast<LPARAM>(&result[0]));
	result.resize(written);
	return result;
}

bool CEdit::SetText(size_t textLength, const wchar_t* text) const
{
	return ::SendMessageW(m_hWnd, WM_SETTEXT, textLength, reinterpret_cast<LPARAM>(text)) == TRUE;
}

bool CEdit::SetHint(const wchar_t* text, bool bToBeHidden) const
{
	return ::SendMessageW(m_hWnd, EM_SETCUEBANNER, bToBeHidden ? TRUE : FALSE, reinterpret_cast<LPARAM>(text)) == TRUE;
}


bool CSpin::Create(HWND hParentWnd, unsigned short usMin, unsigned short usMax)
{
	m_buddy.Create(L"", hParentWnd, false, true, true, false);

	m_hWnd = ::CreateWindowExW(0, UPDOWN_CLASSW, L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | UDS_AUTOBUDDY | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK,
		0, 0, 0, 0, hParentWnd, nullptr, AppInstance(), nullptr);

	if (m_hWnd)
	{
		::SendMessageW(m_hWnd, UDM_SETRANGE, TRUE, MAKELONG(usMax, usMin));
		::SendMessageW(m_hWnd, UDM_SETPOS, 0, usMax);
	}
	return m_hWnd != nullptr && m_buddy.GetHwnd() != nullptr;
}

long CSpin::GetValue() const
{
	return static_cast<long>(::SendMessageW(m_hWnd, UDM_GETPOS32, 0, 0));
}

void CSpin::SetValue(long value) const
{
	::SendMessageW(m_hWnd, UDM_SETPOS32, 0, value);
}

HWND CSpin::GetBuddyHandle() const
{
	return m_buddy.GetHwnd();
}

void CSpin::AdjustPosition(int x, int y, int width, int height)
{
	::MoveWindow(m_buddy.GetHwnd(), x, y, width, height, TRUE);
	::MoveWindow(m_hWnd, x + width, y, width / 2, height, TRUE);
}


bool CTab::Create(HWND hParentWnd)
{
	m_hWnd = ::CreateWindowExW(0, WC_TABCONTROLW, L"",
		WS_VISIBLE | WS_CHILD | WS_TABSTOP,
		0, 0, 0, 0, hParentWnd, nullptr, AppInstance(), nullptr);
	return m_hWnd != nullptr;
}

bool CTab::Add(const wchar_t* name)
{
	if (!m_hWnd) return false;

	TCITEMW item{};
	item.mask    = TCIF_TEXT;
	item.pszText = const_cast<wchar_t*>(name);
	return ::SendMessageW(m_hWnd, TCM_INSERTITEM, static_cast<WPARAM>(GetTabCount()), reinterpret_cast<LPARAM>(&item)) != -1;
}

int CTab::GetTabCount() const
{
	return m_hWnd ? static_cast<int>(::SendMessageW(m_hWnd, TCM_GETITEMCOUNT, 0, 0)) : 0;
}

int CTab::GetSelectedTabIndex() const
{
	return m_hWnd ? static_cast<int>(::SendMessageW(m_hWnd, TCM_GETCURSEL, 0, 0)) : -1;
}

void CTab::Adjust() const
{
	if (!m_hWnd) return;
	RECT rc{};
	::GetClientRect(m_hWnd, &rc);
	::SendMessageW(m_hWnd, TCM_ADJUSTRECT, TRUE, reinterpret_cast<LPARAM>(&rc));
}

int CTab::GetItemHeight() const
{
	if (!m_hWnd) return 0;
	RECT rc{};
	::SendMessageW(m_hWnd, TCM_GETITEMRECT, GetSelectedTabIndex(), reinterpret_cast<LPARAM>(&rc));
	return rc.bottom - rc.top;
}
