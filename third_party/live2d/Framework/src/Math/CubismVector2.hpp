

#pragma once

#include "CubismFramework.hpp"
#include "math.h"

#ifndef NULL
#   define  NULL  0
#endif

namespace Live2D { namespace Cubism { namespace Framework {

struct CubismVector2
{
    csmFloat32 X;
    csmFloat32 Y;

    CubismVector2(): X(0.0f), Y(0.0f)
    {}

    CubismVector2(csmFloat32 x, csmFloat32 y) : X(x), Y(y)
    {}

    friend CubismVector2 operator+(const CubismVector2& a, const CubismVector2& b);

    friend CubismVector2 operator-(const CubismVector2& a, const CubismVector2& b);

    friend CubismVector2 operator*(const CubismVector2& vector, const csmFloat32 scalar);

    friend CubismVector2 operator*(const csmFloat32 scalar, const CubismVector2& vector);

    friend CubismVector2 operator/(const CubismVector2& vector, const csmFloat32 scalar);

    const CubismVector2& operator+=(const CubismVector2& rhs);

    const CubismVector2& operator-=(const CubismVector2& rhs);

    const CubismVector2& operator*=(const CubismVector2& rhs);

    const CubismVector2& operator/=(const CubismVector2& rhs);

    const CubismVector2& operator*=(const csmFloat32 scalar);

    const CubismVector2& operator/=(const csmFloat32 scalar);

    csmBool operator==(const CubismVector2& rhs) const;

    csmBool operator!=(const CubismVector2& rhs) const;

    void Normalize();

    csmFloat32 GetLength() const;

    csmFloat32 GetDistanceWith(CubismVector2 a) const;

    csmFloat32 Dot(const CubismVector2& a) const;
};

CubismVector2 operator+(const CubismVector2& a, const CubismVector2& b);
CubismVector2 operator-(const CubismVector2& a, const CubismVector2& b);
CubismVector2 operator*(const CubismVector2& vector, const csmFloat32 scalar);
CubismVector2 operator*(const csmFloat32 scalar, const CubismVector2& vector);
CubismVector2 operator/(const CubismVector2& vector, const csmFloat32 scalar);

}}}

