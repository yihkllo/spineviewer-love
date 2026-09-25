

#pragma once


#include <cstddef>

namespace Live2D { namespace Cubism { namespace Framework {
typedef bool csmBool;
typedef char csmChar;
typedef unsigned char csmUchar;

typedef unsigned char csmByte;
typedef signed char csmInt8;
typedef unsigned char csmUint8;

typedef signed short csmInt16;
typedef unsigned short csmUint16;

typedef signed int csmInt32;
typedef unsigned int csmUint32;

typedef signed long long csmInt64;
typedef unsigned long long csmUint64;

typedef float csmFloat32;

typedef unsigned int csmSizeInt;

#ifdef _MSC_VER
typedef ::size_t csmSizeType;
#else
typedef std::size_t csmSizeType;
#endif

}}}
