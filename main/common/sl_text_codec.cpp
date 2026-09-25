#include "sl_text_codec.h"

#include <Windows.h>

#include <limits>

namespace
{
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
