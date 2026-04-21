#include <Windows.h>
#include <dwrite_1.h>
#include <atlbase.h>

#include <vector>

#include "sl_font_provider.h"

#pragma comment(lib, "Dwrite.lib")

namespace
{
	struct FontFamilyEntry
	{
		CComPtr<IDWriteFontFamily> family;
		std::wstring displayName;
	};

	class FontCatalog
	{
	public:
		FontCatalog()
		{
			HRESULT result = ::DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(&m_factory));
			if (FAILED(result))
				return;

			if (FAILED(m_factory->GetSystemFontCollection(&m_collection)))
				return;

			::GetUserDefaultLocaleName(m_localeName, LOCALE_NAME_MAX_LENGTH);
		}

		bool Ready() const
		{
			return m_factory != nullptr && m_collection != nullptr;
		}

		const wchar_t* LocaleName() const
		{
			return m_localeName;
		}

		std::wstring ResolveFamilyDisplayName(const wchar_t* familyName) const
		{
			FontFamilyEntry entry = FindFamilyEntry(familyName);
			return entry.displayName;
		}

		std::vector<std::wstring> EnumerateDisplayNames() const
		{
			std::vector<std::wstring> names;
			if (!Ready())
				return names;

			const UINT32 familyCount = m_collection->GetFontFamilyCount();
			names.reserve(familyCount);
			for (UINT32 index = 0; index < familyCount; ++index)
			{
				CComPtr<IDWriteFontFamily> family;
				if (FAILED(m_collection->GetFontFamily(index, &family)))
					continue;

				std::wstring name = ReadFamilyDisplayName(family);
				if (!name.empty())
					names.push_back(name);
			}
			return names;
		}

		std::vector<std::wstring> CollectFaceFiles(const wchar_t* familyName, bool bold, bool italic) const
		{
			std::vector<std::wstring> paths;
			FontFamilyEntry entry = FindFamilyEntry(familyName);
			if (!entry.family)
				return paths;

			CComPtr<IDWriteFont> font;
			const DWRITE_FONT_WEIGHT weight = bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_REGULAR;
			const DWRITE_FONT_STYLE style = italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
			if (FAILED(entry.family->GetFirstMatchingFont(weight, DWRITE_FONT_STRETCH_NORMAL, style, &font)))
				return paths;

			CComPtr<IDWriteFontFace> face;
			if (FAILED(font->CreateFontFace(&face)))
				return paths;

			UINT32 fileCount = 0;
			if (FAILED(face->GetFiles(&fileCount, nullptr)) || fileCount == 0)
				return paths;

			std::vector<IDWriteFontFile*> files(fileCount, nullptr);
			if (FAILED(face->GetFiles(&fileCount, &files[0])))
				return paths;

			paths.reserve(fileCount);
			for (size_t index = 0; index < files.size(); ++index)
			{
				IDWriteFontFile* file = files[index];
				std::wstring path = ReadLocalFilePath(file);
				if (!path.empty())
					paths.push_back(path);
				if (file != nullptr)
					file->Release();
			}
			return paths;
		}

	private:
		FontFamilyEntry FindFamilyEntry(const wchar_t* familyName) const
		{
			if (!Ready() || familyName == nullptr)
				return {};

			UINT32 index = 0;
			BOOL found = FALSE;
			if (FAILED(m_collection->FindFamilyName(familyName, &index, &found)) || !found)
				return {};

			FontFamilyEntry entry;
			if (FAILED(m_collection->GetFontFamily(index, &entry.family)))
				return {};
			entry.displayName = ReadFamilyDisplayName(entry.family);
			return entry;
		}

		std::wstring ReadFamilyDisplayName(IDWriteFontFamily* family) const
		{
			if (family == nullptr)
				return {};

			CComPtr<IDWriteLocalizedStrings> names;
			if (FAILED(family->GetFamilyNames(&names)))
				return {};
			return ReadLocalizedString(names);
		}

		std::wstring ReadLocalizedString(IDWriteLocalizedStrings* strings) const
		{
			if (strings == nullptr)
				return {};

			UINT32 localeIndex = 0;
			BOOL localeFound = FALSE;
			strings->FindLocaleName(m_localeName, &localeIndex, &localeFound);
			if (!localeFound)
				localeIndex = 0;

			UINT32 length = 0;
			if (FAILED(strings->GetStringLength(localeIndex, &length)))
				return {};

			std::wstring value(length + 1ULL, L'\0');
			if (FAILED(strings->GetString(localeIndex, &value[0], static_cast<UINT32>(value.size()))))
				return {};
			return value.c_str();
		}

		std::wstring ReadLocalFilePath(IDWriteFontFile* file) const
		{
			if (file == nullptr)
				return {};

			const void* referenceKey = nullptr;
			UINT32 referenceKeyLength = 0;
			if (FAILED(file->GetReferenceKey(&referenceKey, &referenceKeyLength)))
				return {};

			CComPtr<IDWriteFontFileLoader> loader;
			if (FAILED(file->GetLoader(&loader)))
				return {};

			CComPtr<IDWriteLocalFontFileLoader> localLoader;
			if (FAILED(loader->QueryInterface(&localLoader)))
				return {};

			UINT32 pathLength = 0;
			if (FAILED(localLoader->GetFilePathLengthFromKey(referenceKey, referenceKeyLength, &pathLength)))
				return {};

			std::wstring buffer(pathLength + 1ULL, L'\0');
			if (FAILED(localLoader->GetFilePathFromKey(referenceKey, referenceKeyLength, &buffer[0], static_cast<UINT32>(buffer.size()))))
				return {};
			return buffer.c_str();
		}

		CComPtr<IDWriteFactory> m_factory;
		CComPtr<IDWriteFontCollection> m_collection;
		wchar_t m_localeName[LOCALE_NAME_MAX_LENGTH]{};
	};
}

class CWinFont::Impl
{
public:
	const wchar_t* GetLocaleName() const
	{
		return m_catalog.LocaleName();
	}

	std::wstring FindLocaleFontName(const wchar_t* familyName)
	{
		return m_catalog.ResolveFamilyDisplayName(familyName);
	}

	std::vector<std::wstring> GetSystemFontFamilyNames()
	{
		return m_catalog.EnumerateDisplayNames();
	}

	std::vector<std::wstring> FindFontFilePaths(const wchar_t* familyName, bool bold, bool italic)
	{
		return m_catalog.CollectFaceFiles(familyName, bold, italic);
	}

private:
	FontCatalog m_catalog;
};

CWinFont::CWinFont()
	: m_pImpl(std::make_unique<Impl>())
{
}

CWinFont::~CWinFont() = default;

const wchar_t* const CWinFont::GetLocaleName()
{
	return m_pImpl->GetLocaleName();
}

std::wstring CWinFont::FindLocaleFontName(const wchar_t* pwzFontFamilyName)
{
	return m_pImpl->FindLocaleFontName(pwzFontFamilyName);
}

std::vector<std::wstring> CWinFont::GetSystemFontFamilyNames()
{
	return m_pImpl->GetSystemFontFamilyNames();
}

std::vector<std::wstring> CWinFont::FindFontFilePaths(const wchar_t* pwzFontFamilyName, bool bBold, bool bItalic)
{
	return m_pImpl->FindFontFilePaths(pwzFontFamilyName, bBold, bItalic);
}
