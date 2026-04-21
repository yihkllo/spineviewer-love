#ifndef SPINELOVE_SL_TEXT_CODEC_H_
#define SPINELOVE_SL_TEXT_CODEC_H_

#include <string>

namespace win_text
{

    std::wstring WidenUtf8(const std::string& str);
    std::wstring WidenUtf8(const char* str, int length);

    std::string  NarrowUtf8(const std::wstring& wstr);
    std::string  NarrowUtf8(const wchar_t* wstr, int length);

    std::wstring WidenAnsi(const std::string& str);
    std::wstring WidenAnsi(const char* str, int length);

    std::string  NarrowAnsi(const std::wstring& wstr);
    std::string  NarrowAnsi(const wchar_t* wstr, int length);


    int WidenUtf8Static(const char* str, int length, wchar_t* dst, int dstSize);
    int NarrowUtf8Static(const wchar_t* wstr, int length, char* dst, int dstSize);
}

#endif

