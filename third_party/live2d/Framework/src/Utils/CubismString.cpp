

#include "CubismString.hpp"
#include "Type/csmVector.hpp"
#include <stdio.h>
#include <stdarg.h>

namespace Live2D { namespace Cubism { namespace Framework { namespace Utils {

csmString CubismString::GetFormatedString(const csmChar* format, ...)
{
    csmInt32 bufferSize = 256;
    csmChar* buffer = static_cast<csmChar*>(CSM_MALLOC(sizeof(csmChar)* bufferSize));

    va_list args;
    va_start(args, format);

    for (;;) {
#ifdef _WINDOWS
        if (vsnprintf_s(buffer, bufferSize, _TRUNCATE, format, args) < bufferSize) {
#else
        if (vsnprintf(buffer, bufferSize, format, args) < bufferSize) {
#endif
            break;
        } else {
            CSM_FREE(buffer);
            bufferSize *= 2;
            buffer = static_cast<csmChar*>(CSM_MALLOC(sizeof(csmChar)* bufferSize));
        }
    }
    va_end(args);

    csmString ret = buffer;
    CSM_FREE(buffer);

    return ret;
}

csmBool CubismString::IsStartsWith(const csmChar* text, const csmChar* startWord)
{
    while (*startWord != '\0')
    {
        if (*text == '\0' || *(text++) != *(startWord++))
        {
            return false;
        }
    }
    return true;
}

csmFloat32 CubismString::StringToFloat(const csmChar* string, csmInt32 length, csmInt32 position, csmInt32* outEndPos)
{
    csmInt32 i = position;
    csmBool minus = false;
    csmBool period = false;
    csmFloat32 v1 = 0;

    csmInt32 c = string[i];
    if (c == '-')
    {
        minus = true;
        i++;
    }

    for (; i < length; i++)
    {
        c = string[i];
        if ('0' <= c && c <= '9')
        {
            v1 = v1 * 10 + (c - '0');
        }
        else if (c == '.')
        {
            period = true;
            i++;
            break;
        }
        else
        {
            break;
        }
    }

    if (period)
    {
        csmFloat32 mul = 0.1f;
        for (; i < length; i++)
        {
            c = string[i] & 0xFF;
            if ('0' <= c && c <= '9')
            {
                v1 += mul * (c - '0');
            }
            else
            {
                break;
            }
            mul *= 0.1f;
            if (!c) break;
        }
    }

    if (i == position)
    {
        *outEndPos = -1;
        return 0;
    }

    if (minus) v1 = -v1;

    *outEndPos = i;
    return v1;
}

}}}}

