

#pragma once

#include "CubismFramework.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismTargetPoint
{
public:
    CubismTargetPoint();

    virtual ~CubismTargetPoint();

    void        Update(csmFloat32 deltaTimeSeconds);

    csmFloat32  GetX() const;

    csmFloat32  GetY() const;

    void        Set(csmFloat32 x, csmFloat32 y);

private:
    csmFloat32  _faceTargetX;
    csmFloat32  _faceTargetY;
    csmFloat32  _faceX;
    csmFloat32  _faceY;
    csmFloat32  _faceVX;
    csmFloat32  _faceVY;
    csmFloat32  _lastTimeSeconds;
    csmFloat32  _userTimeSeconds;

};

}}}
