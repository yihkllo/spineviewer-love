

#pragma once

#include "CubismMatrix44.hpp"

namespace Live2D { namespace Cubism { namespace Framework {
class CubismViewMatrix : public CubismMatrix44
{
public:
    CubismViewMatrix();

    virtual ~CubismViewMatrix();

    void        AdjustTranslate(csmFloat32 x, csmFloat32 y);

    void        AdjustScale(csmFloat32 cx, csmFloat32 cy, csmFloat32 scale);

    void        SetScreenRect(csmFloat32 left, csmFloat32 right, csmFloat32 bottom, csmFloat32 top);

    void        SetMaxScreenRect(csmFloat32 left, csmFloat32 right, csmFloat32 bottom, csmFloat32 top);

    void        SetMaxScale(csmFloat32 maxScale);

    void        SetMinScale(csmFloat32 minScale);

    csmFloat32  GetMaxScale() const;

    csmFloat32  GetMinScale() const;

    csmBool     IsMaxScale() const;

    csmBool     IsMinScale() const;

    csmFloat32  GetScreenLeft() const;

    csmFloat32  GetScreenRight() const;

    csmFloat32  GetScreenBottom() const;

    csmFloat32  GetScreenTop() const;

    csmFloat32  GetMaxLeft() const;

    csmFloat32  GetMaxRight() const;

    csmFloat32  GetMaxBottom() const;

    csmFloat32  GetMaxTop() const;

private:
    csmFloat32  _screenLeft;
    csmFloat32  _screenRight;
    csmFloat32  _screenTop;
    csmFloat32  _screenBottom;
    csmFloat32  _maxLeft;
    csmFloat32  _maxRight;
    csmFloat32  _maxTop;
    csmFloat32  _maxBottom;
    csmFloat32  _maxScale;
    csmFloat32  _minScale;
};

}}}
