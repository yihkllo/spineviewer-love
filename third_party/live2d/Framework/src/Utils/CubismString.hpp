

#pragma once

#include "Type/csmString.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Utils{
class CubismString
{
public:
    static csmString GetFormatedString(const csmChar* format, ...);

    static csmBool IsStartsWith(const csmChar* text, const csmChar* startWord);


    static csmFloat32 StringToFloat(const csmChar* string, csmInt32 length, csmInt32 position, csmInt32* outEndPos);

private:
    CubismString();
};
}}}}

