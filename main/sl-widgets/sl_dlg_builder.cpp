
#include "sl_dlg_builder.h"
#include <cstring>

void CDialogueTemplate::SetWindowSize(unsigned short usWidth, unsigned short usHeight)
{
	m_usWidth  = usWidth;
	m_usHeight = usHeight;
}

void CDialogueTemplate::MakeWindowResizable(bool bResizable) { m_bResizable = bResizable; }
void CDialogueTemplate::MakeWindowChild(bool bChild)         { m_bChild = bChild; }

const unsigned char* CDialogueTemplate::Generate(const wchar_t* wszWindowTitle)
{


#pragma pack(push, 1)
	struct Header
	{
		WORD  dlgVer     = 0x01;
		WORD  signature  = 0xffff;
		DWORD helpID     = 0;
		DWORD exstyle    = 0;
		DWORD style      = DS_MODALFRAME | DS_SETFONT | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | WS_SYSMENU;
		WORD  cDlgItems  = 0;
		short x  = 0;
		short y  = 0;
		short cx = 0x80;
		short cy = 0x60;
		WORD  menu        = 0;
		WORD  windowClass = 0;
	};

	struct FontInfo
	{
		WORD pointsize    = 0x08;
		WORD weight       = FW_REGULAR;
		BYTE italic       = TRUE;
		BYTE characterset = ANSI_CHARSET;
	};
#pragma pack(pop)

	static constexpr wchar_t kDefaultTitle[]    = L"Dialogue";
	static constexpr wchar_t kDefaultTypeface[] = L"MS Shell Dlg";

	Header hdr{};
	hdr.cx = m_usWidth;
	hdr.cy = m_usHeight;

	if (m_bChild)
	{
		hdr.style &= ~(WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_BORDER);
		hdr.style |= WS_CHILD;
	}
	else if (m_bResizable)
	{
		hdr.style &= ~DS_MODALFRAME;
		hdr.style |= WS_THICKFRAME;
	}

	const wchar_t* title = wszWindowTitle ? wszWindowTitle : kDefaultTitle;
	const size_t titleBytes    = (wcslen(title) + 1) * sizeof(wchar_t);
	const size_t typefaceBytes = sizeof(kDefaultTypeface);


	const size_t total = sizeof(Header) + titleBytes + sizeof(FontInfo) + typefaceBytes;
	m_buffer.resize(total);

	auto* dst = m_buffer.data();
	memcpy(dst, &hdr, sizeof(Header));       dst += sizeof(Header);
	memcpy(dst, title, titleBytes);           dst += titleBytes;

	FontInfo font{};
	memcpy(dst, &font, sizeof(FontInfo));     dst += sizeof(FontInfo);
	memcpy(dst, kDefaultTypeface, typefaceBytes);

	return m_buffer.data();
}
