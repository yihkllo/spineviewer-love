#ifndef SPINELOVE_SL_TEXT_CODEC_H_
#define SPINELOVE_SL_TEXT_CODEC_H_

#include <string>

namespace sl_text
{

    std::wstring Utf8ToWide(const std::string& text);
    std::wstring Utf8ToWide(const char* text, int length);

    std::string  WideToUtf8(const std::wstring& text);
    std::string  WideToUtf8(const wchar_t* text, int length);
}

#endif

