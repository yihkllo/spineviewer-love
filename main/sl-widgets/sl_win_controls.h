#ifndef SL_WIN_CONTROLS_H_
#define SL_WIN_CONTROLS_H_

#include <string>
#include <vector>

#include <Windows.h>
#include <CommCtrl.h>

class CListView
{
public:
	CListView() = default;
	~CListView() = default;

	explicit operator bool() const noexcept { return m_hWnd != nullptr; }

	bool Create(HWND hParentWnd, const wchar_t** columnNames, size_t columnCount, bool hasCheckBox = false);
	template <size_t N>
	void Create(HWND hParentWnd, const wchar_t*(&&names)[N], bool hasCheckBox = false)
	{
		Create(hParentWnd, names, N, hasCheckBox);
	}

	HWND GetHwnd() const { return m_hWnd; }

	void AdjustWidth();

	bool Add(const wchar_t** columns, size_t columnCount, bool toBottom = true);
	template<size_t N>
	bool Add(const wchar_t*(&&cols)[N], bool toBottom = true)
	{
		return Add(cols, N, toBottom);
	}
	bool Add(const std::vector<std::wstring>& columns, bool toBottom = true);

	void Clear() const;

	void CreateSingleList(const std::vector<std::wstring>& items);
	void CreateSingleList(const wchar_t** items, size_t itemCount);

	std::vector<std::wstring> PickupCheckedItems();
private:
	HWND m_hWnd = nullptr;

	int GetColumnCount() const;
	int GetItemCount() const;
	std::wstring GetItemText(int iRow, int iColumn) const;
};

class CListBox
{
public:
	CListBox() = default;
	~CListBox() = default;

	explicit operator bool() const noexcept { return m_hWnd != nullptr; }

	bool Create(HWND hParentWnd);
	HWND GetHwnd() const { return m_hWnd; }

	void Add(const wchar_t* szText, bool toBottom = true) const;
	void Clear() const;
	std::wstring GetSelectedItemName();
private:
	HWND m_hWnd = nullptr;

	long long GetSelectedItemIndex() const;
};

class CComboBox
{
public:
	CComboBox() = default;
	~CComboBox() = default;

	explicit operator bool() const noexcept { return m_hWnd != nullptr; }

	bool Create(HWND hParentWnd);
	HWND GetHwnd() const { return m_hWnd; }

	void Setup(const std::vector<std::wstring>& itemTexts);
	void Setup(const wchar_t** itemTexts, size_t itemCount);
	template<size_t N>
	void Setup(const wchar_t* (&&texts)[N])
	{
		Setup(texts, N);
	}

	int GetSelectedItemIndex() const;
	std::wstring GetSelectedItemText() const;

	int FindIndex(const wchar_t* szName) const;
	bool SetSelectedItem(int iIndex) const;
private:
	HWND m_hWnd = nullptr;

	void Clear() const;
};

class CButton
{
public:
	CButton() = default;
	~CButton() = default;

	bool Create(const wchar_t* szText, HWND hParentWnd, HMENU hMenu, bool hasCheckBox = false);
	HWND GetHwnd() const { return m_hWnd; }

	void SetCheckBox(bool checked) const;
	bool IsChecked() const;
private:
	HWND m_hWnd = nullptr;
};

class CSlider
{
public:
	CSlider() = default;
	~CSlider() = default;

	bool Create(const wchar_t* szText, HWND hParentWnd, HMENU hMenu, unsigned short usMin, unsigned short usMax, unsigned int uiRange, bool bVertical = false);
	HWND GetHwnd() const { return m_hWnd; }

	long long GetPosition() const;
	void SetPosition(long long llPos) const;

	HWND GetToolTipHandle() const;
private:
	HWND m_hWnd = nullptr;
};

class CFloatSlider
{
public:
	CFloatSlider() = default;
	~CFloatSlider() = default;

	bool Create(const wchar_t* szText, HWND hParentWnd, HMENU hMenu, float fMin, float fMax, float fRange, unsigned int uiRatio = kDefaultRatio, bool bVertical = false);
	HWND GetHwnd() const { return m_hWnd; }

	float GetPosition() const;
	void SetPosition(float fPos) const;

	HWND GetToolTipHandle() const;
	void OnToolTipNeedText(LPNMTTDISPINFOW pInfo) const;

	unsigned int GetRatio() const { return m_uiRatio; }
private:
	static constexpr unsigned int kDefaultRatio = 10;
	HWND m_hWnd = nullptr;
	unsigned int m_uiRatio = kDefaultRatio;
};

class CStatic
{
public:
	CStatic() = default;
	~CStatic() = default;

	bool Create(const wchar_t* szText, HWND hParentWnd, bool hasEdge = false);
	HWND GetHwnd() const { return m_hWnd; }
private:
	HWND m_hWnd = nullptr;
};

class CEdit
{
public:
	CEdit() = default;
	~CEdit() = default;

	explicit operator bool() const noexcept { return m_hWnd != nullptr; }

	bool Create(const wchar_t* initialText, HWND hParentWnd, bool bReadOnly = false, bool bBorder = true, bool bNumber = false, bool bPassword = false);
	HWND GetHwnd() const { return m_hWnd; }

	std::wstring GetText() const;
	bool SetText(size_t textLength, const wchar_t* text) const;

	bool SetHint(const wchar_t* text, bool bToBeHidden = true) const;
private:
	HWND m_hWnd = nullptr;
};

class CSpin
{
public:
	CSpin() = default;
	~CSpin() = default;

	bool Create(HWND hParentWnd, unsigned short usMin, unsigned short usMax);
	HWND GetHwnd() const { return m_hWnd; }

	long GetValue() const;
	void SetValue(long value) const;

	HWND GetBuddyHandle() const;
	void AdjustPosition(int x, int y, int width, int height);
private:
	HWND m_hWnd = nullptr;
	CEdit m_buddy;
};

class CTab
{
public:
	CTab() = default;
	~CTab() = default;

	bool Create(HWND hParentWnd);
	HWND GetHwnd() const { return m_hWnd; }

	bool Add(const wchar_t* name);
	int GetTabCount() const;
	int GetSelectedTabIndex() const;

	void Adjust() const;
	int GetItemHeight() const;

private:
	HWND m_hWnd = nullptr;
};

#endif
