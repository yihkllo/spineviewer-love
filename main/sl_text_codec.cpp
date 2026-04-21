#include <Windows.h>

#include "sl_text_codec.h"

namespace
{
	enum class ECodePage : unsigned
	{
		Utf8 = CP_UTF8,
		Ansi = CP_ACP,
	};

	unsigned ResolveCodePage(ECodePage codePage)
	{
		return static_cast<unsigned>(codePage);
	}

	std::wstring DecodeText(const char* source, int length, ECodePage codePage)
	{
		if (source == nullptr || length == 0)
			return {};

		const unsigned windowsCodePage = ResolveCodePage(codePage);
		const int requiredLength = ::MultiByteToWideChar(windowsCodePage, 0, source, length, nullptr, 0);
		if (requiredLength <= 0)
			return {};

		std::wstring result(static_cast<size_t>(requiredLength), L'\0');
		if (::MultiByteToWideChar(windowsCodePage, 0, source, length, &result[0], requiredLength) <= 0)
			return {};
		return result;
	}

	std::string EncodeText(const wchar_t* source, int length, ECodePage codePage)
	{
		if (source == nullptr || length == 0)
			return {};

		const unsigned windowsCodePage = ResolveCodePage(codePage);
		const int requiredLength = ::WideCharToMultiByte(windowsCodePage, 0, source, length, nullptr, 0, nullptr, nullptr);
		if (requiredLength <= 0)
			return {};

		std::string result(static_cast<size_t>(requiredLength), '\0');
		if (::WideCharToMultiByte(windowsCodePage, 0, source, length, &result[0], requiredLength, nullptr, nullptr) <= 0)
			return {};
		return result;
	}

	int DecodeToBuffer(const char* source, int length, wchar_t* destination, int destinationSize, ECodePage codePage)
	{
		return ::MultiByteToWideChar(ResolveCodePage(codePage), 0, source, length, destination, destinationSize);
	}

	int EncodeToBuffer(const wchar_t* source, int length, char* destination, int destinationSize, ECodePage codePage)
	{
		return ::WideCharToMultiByte(ResolveCodePage(codePage), 0, source, length, destination, destinationSize, nullptr, nullptr);
	}
}

std::wstring win_text::WidenUtf8(const std::string& str)
{
	return DecodeText(str.c_str(), static_cast<int>(str.size()), ECodePage::Utf8);
}

std::wstring win_text::WidenUtf8(const char* str, int length)
{
	return DecodeText(str, length, ECodePage::Utf8);
}

std::string win_text::NarrowUtf8(const std::wstring& wstr)
{
	return EncodeText(wstr.c_str(), static_cast<int>(wstr.size()), ECodePage::Utf8);
}

std::string win_text::NarrowUtf8(const wchar_t* wstr, int length)
{
	return EncodeText(wstr, length, ECodePage::Utf8);
}

std::wstring win_text::WidenAnsi(const std::string& str)
{
	return DecodeText(str.c_str(), static_cast<int>(str.size()), ECodePage::Ansi);
}

std::wstring win_text::WidenAnsi(const char* str, int length)
{
	return DecodeText(str, length, ECodePage::Ansi);
}

std::string win_text::NarrowAnsi(const std::wstring& wstr)
{
	return EncodeText(wstr.c_str(), static_cast<int>(wstr.size()), ECodePage::Ansi);
}

std::string win_text::NarrowAnsi(const wchar_t* wstr, int length)
{
	return EncodeText(wstr, length, ECodePage::Ansi);
}

int win_text::WidenUtf8Static(const char* str, int length, wchar_t* dst, int dstSize)
{
	return DecodeToBuffer(str, length, dst, dstSize, ECodePage::Utf8);
}

int win_text::NarrowUtf8Static(const wchar_t* wstr, int length, char* dst, int dstSize)
{
	return EncodeToBuffer(wstr, length, dst, dstSize, ECodePage::Utf8);
}
