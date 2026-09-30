#include "sl_text_codec.h"

#if defined(_WIN32)
#include <Windows.h>
#endif

#include <limits>

namespace
{
#if defined(_WIN32)
	int ClampLength(size_t length)
	{
		constexpr size_t kMaxInt = static_cast<size_t>((std::numeric_limits<int>::max)());
		return length > kMaxInt ? 0 : static_cast<int>(length);
	}

	std::wstring Utf8BytesToWide(const char* text, size_t textLength)
	{
		if (text == nullptr || textLength == 0)
			return {};

		const int byteCount = ClampLength(textLength);
		if (byteCount <= 0)
			return {};

		const int charCount = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, byteCount, nullptr, 0);
		if (charCount <= 0)
			return {};

		std::wstring wideText(static_cast<size_t>(charCount), L'\0');
		const int written = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, byteCount, &wideText[0], charCount);
		if (written != charCount)
			return {};

		return wideText;
	}

	std::string WideToUtf8Bytes(const wchar_t* text, size_t textLength)
	{
		if (text == nullptr || textLength == 0)
			return {};

		const int charCount = ClampLength(textLength);
		if (charCount <= 0)
			return {};

		const int byteCount = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text, charCount, nullptr, 0, nullptr, nullptr);
		if (byteCount <= 0)
			return {};

		std::string utf8Text(static_cast<size_t>(byteCount), '\0');
		const int written = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text, charCount, &utf8Text[0], byteCount, nullptr, nullptr);
		if (written != byteCount)
			return {};

		return utf8Text;
	}
#else
	bool Scalar(char32_t c)
	{
		return c <= 0x10FFFF && (c < 0xD800 || c > 0xDFFF);
	}

	std::wstring Utf8BytesToWide(const char* text, size_t textLength)
	{
		if (text == nullptr || textLength == 0)
			return {};
		const auto* bytes = reinterpret_cast<const unsigned char*>(text);
		std::wstring wideText;
		wideText.reserve(textLength);
		for (size_t i = 0; i < textLength;)
		{
			const unsigned char lead = bytes[i];
			size_t extra = 0;
			char32_t c = 0;
			if (lead < 0x80) c = lead;
			else if ((lead & 0xE0) == 0xC0) { extra = 1; c = lead & 0x1F; }
			else if ((lead & 0xF0) == 0xE0) { extra = 2; c = lead & 0x0F; }
			else if ((lead & 0xF8) == 0xF0) { extra = 3; c = lead & 0x07; }
			else return {};
			for (size_t k = 1; k <= extra; ++k)
			{
				if (i + k >= textLength || (bytes[i + k] & 0xC0) != 0x80) return {};
				c = (c << 6) | (bytes[i + k] & 0x3F);
			}
			static constexpr char32_t minimum[] = { 0, 0x80, 0x800, 0x10000 };
			if (c < minimum[extra] || !Scalar(c)) return {};
			if constexpr (sizeof(wchar_t) == 2)
			{
				if (c >= 0x10000)
				{
					c -= 0x10000;
					wideText.push_back(static_cast<wchar_t>(0xD800 + (c >> 10)));
					wideText.push_back(static_cast<wchar_t>(0xDC00 + (c & 0x3FF)));
				}
				else wideText.push_back(static_cast<wchar_t>(c));
			}
			else wideText.push_back(static_cast<wchar_t>(c));
			i += extra + 1;
		}
		return wideText;
	}

	std::string WideToUtf8Bytes(const wchar_t* text, size_t textLength)
	{
		if (text == nullptr || textLength == 0)
			return {};
		std::string utf8Text;
		utf8Text.reserve(textLength);
		for (size_t i = 0; i < textLength; ++i)
		{
			char32_t c = static_cast<char32_t>(text[i]);
			if constexpr (sizeof(wchar_t) == 2)
			{
				if (c >= 0xD800 && c <= 0xDBFF)
				{
					if (i + 1 >= textLength) return {};
					const char32_t low = static_cast<char32_t>(text[i + 1]);
					if (low < 0xDC00 || low > 0xDFFF) return {};
					c = 0x10000 + ((c - 0xD800) << 10) + (low - 0xDC00);
					++i;
				}
			}
			if (!Scalar(c)) return {};
			if (c < 0x80) utf8Text.push_back(static_cast<char>(c));
			else if (c < 0x800) { utf8Text.push_back(static_cast<char>(0xC0 | (c >> 6))); utf8Text.push_back(static_cast<char>(0x80 | (c & 0x3F))); }
			else if (c < 0x10000) { utf8Text.push_back(static_cast<char>(0xE0 | (c >> 12))); utf8Text.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F))); utf8Text.push_back(static_cast<char>(0x80 | (c & 0x3F))); }
			else { utf8Text.push_back(static_cast<char>(0xF0 | (c >> 18))); utf8Text.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F))); utf8Text.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F))); utf8Text.push_back(static_cast<char>(0x80 | (c & 0x3F))); }
		}
		return utf8Text;
	}
#endif
}

std::wstring sl_text::Utf8ToWide(const std::string& text)
{
	return Utf8BytesToWide(text.data(), text.size());
}

std::wstring sl_text::Utf8ToWide(const char* text, int length)
{
	if (text == nullptr || length <= 0)
		return {};

	return Utf8BytesToWide(text, static_cast<size_t>(length));
}

std::string sl_text::WideToUtf8(const std::wstring& text)
{
	return WideToUtf8Bytes(text.data(), text.size());
}

std::string sl_text::WideToUtf8(const wchar_t* text, int length)
{
	if (text == nullptr || length <= 0)
		return {};

	return WideToUtf8Bytes(text, static_cast<size_t>(length));
}
