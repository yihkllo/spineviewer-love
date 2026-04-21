#ifndef SPINELOVE_SL_FONT_PROVIDER_H_
#define SPINELOVE_SL_FONT_PROVIDER_H_

#include <memory>
#include <string>
#include <vector>

class CWinFont
{
public:
	CWinFont();
	~CWinFont();

	CWinFont(const CWinFont&) = delete;
	CWinFont& operator=(const CWinFont&) = delete;


	const wchar_t* const GetLocaleName();


	std::wstring FindLocaleFontName(const wchar_t* pwzFontFamilyName);


	std::vector<std::wstring> GetSystemFontFamilyNames();


	std::vector<std::wstring> FindFontFilePaths(const wchar_t* pwzFontFamilyName, bool bBold, bool bItalic);
private:
	class Impl;
	std::unique_ptr<Impl> m_pImpl;
};

#endif

