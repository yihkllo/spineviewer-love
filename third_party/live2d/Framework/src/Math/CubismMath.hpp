

#pragma once

#include <cmath>
#include "Type/CubismBasicType.hpp"
#include "Math/CubismVector2.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismMath
{
public:
    static const csmFloat32 Pi;
    static const csmFloat32 Epsilon;

    static csmFloat32 RangeF(csmFloat32 value, csmFloat32 min, csmFloat32 max)
    {
        if (value < min) value = min;
        else if (value > max) value = max;
        return value;
    };

    static csmFloat32 SinF(csmFloat32 x)
    {
        return sinf(x);
    };

    static csmFloat32 CosF(csmFloat32 x)
    {
        return cosf(x);
    };

    static csmFloat32 AbsF(csmFloat32 x)
    {
        return std::fabs(x);
    };

    static csmFloat32 SqrtF(csmFloat32 x)
    {
        return sqrtf(x);
    };

    static csmFloat32 GetEasingSine(csmFloat32 value)
    {
        if (value < 0.0f) return 0.0f;
        else if (value > 1.0f) return 1.0f;

        return static_cast<csmFloat32>(0.5f - 0.5f * CosF(value * Pi));
    }

    static csmFloat32 Max(csmFloat32 l, csmFloat32 r)
    {
        return (l > r) ? l : r;
    }

    static csmFloat32 Min(csmFloat32 l, csmFloat32 r)
    {
        return (l > r) ? r : l;
    }

    static csmInt32 Clamp(csmInt32 val, csmInt32 min, csmInt32 max);

    static csmFloat32 ClampF(csmFloat32 val, csmFloat32 min, csmFloat32 max);

    static csmFloat32 DegreesToRadian(csmFloat32 degrees);

    static csmFloat32 RadianToDegrees(csmFloat32 radian);

    static csmFloat32 DirectionToRadian(CubismVector2 from, CubismVector2 to);

    static csmFloat32 DirectionToDegrees(CubismVector2 from, CubismVector2 to);

    static CubismVector2 RadianToDirection(csmFloat32 totalAngle);

    static csmFloat32 QuadraticEquation(csmFloat32 a, csmFloat32 b, csmFloat32 c);

    static csmFloat32 CardanoAlgorithmForBezier(csmFloat32 a, csmFloat32 b, csmFloat32 c, csmFloat32 d);

    static csmFloat32 ModF(csmFloat32 dividend, csmFloat32 divisor);

private:
    CubismMath();
};

}}}

