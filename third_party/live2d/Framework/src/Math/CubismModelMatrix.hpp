

#pragma once

#include "CubismMatrix44.hpp"
#include "Type/csmMap.hpp"

namespace Live2D { namespace Cubism { namespace Framework {
class CubismModelMatrix : public CubismMatrix44
{
public:
    CubismModelMatrix();

    CubismModelMatrix(csmFloat32 w, csmFloat32 h);

    virtual ~CubismModelMatrix();

    void    SetWidth(csmFloat32 w);

    void    SetHeight(csmFloat32 h);

    void    SetPosition(csmFloat32 x, csmFloat32 y);

    void    SetCenterPosition(csmFloat32 x, csmFloat32 y);

    void    Top(csmFloat32 y);

    void    Bottom(csmFloat32 y);

    void    Left(csmFloat32 x);

    void    Right(csmFloat32 x);

    void    CenterX(csmFloat32 x);

    void    SetX(csmFloat32 x);

    void    CenterY(csmFloat32 y);

    void    SetY(csmFloat32 y);

    void    SetupFromLayout(csmMap<csmString, csmFloat32>& layout);

private:
    csmFloat32  _width;
    csmFloat32  _height;
};

}}}
