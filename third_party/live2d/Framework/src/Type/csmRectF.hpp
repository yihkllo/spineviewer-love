

#pragma once

#include "CubismFramework.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class csmRectF
{
public:

    csmRectF();

    csmRectF(csmFloat32 x, csmFloat32 y, csmFloat32 w, csmFloat32 h);

    virtual ~csmRectF();

    csmFloat32 GetCenterX() const { return X + 0.5f * Width; }

    csmFloat32 GetCenterY() const { return Y + 0.5f * Height; }

    csmFloat32 GetRight() const { return X + Width; }

    csmFloat32 GetBottom() const { return Y + Height; }

    void SetRect(csmRectF* r);

    void Expand(csmFloat32 w, csmFloat32 h);

    csmFloat32 X;
    csmFloat32 Y;
    csmFloat32 Width;
    csmFloat32 Height;
};
}}}

