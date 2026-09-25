

#include "CubismTargetPoint.hpp"
#include "Math/CubismMath.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

const csmInt32      FrameRate = 30;
const csmFloat32    Epsilon = 0.01f;

CubismTargetPoint::CubismTargetPoint()
                                    : _faceTargetX(0.0f)
                                    , _faceTargetY(0.0f)
                                    , _faceX(0.0f)
                                    , _faceY(0.0f)
                                    , _faceVX(0.0f)
                                    , _faceVY(0.0f)
                                    , _lastTimeSeconds(0.0f)
                                    , _userTimeSeconds(0.0f)
{ }

CubismTargetPoint::~CubismTargetPoint()
{ }

void CubismTargetPoint::Update(csmFloat32 deltaTimeSeconds)
{
    _userTimeSeconds += deltaTimeSeconds;

    const csmFloat32 FaceParamMaxV = 40.0 / 10.0f;
    const csmFloat32 MaxV = FaceParamMaxV * 1.0f / static_cast<csmFloat32>(FrameRate);

    if (_lastTimeSeconds == 0.0f)
    {
        _lastTimeSeconds = _userTimeSeconds;
        return;
    }

    const csmFloat32  deltaTimeWeight = (_userTimeSeconds - _lastTimeSeconds) * static_cast<csmFloat32>(FrameRate);
    _lastTimeSeconds = _userTimeSeconds;

    const csmFloat32 TimeToMaxSpeed = 0.15f;
    const csmFloat32 FrameToMaxSpeed = TimeToMaxSpeed * static_cast<csmFloat32>(FrameRate);
    const csmFloat32 MaxA = deltaTimeWeight * MaxV / FrameToMaxSpeed;

    const csmFloat32 dx = _faceTargetX - _faceX;
    const csmFloat32 dy = _faceTargetY - _faceY;

    if (CubismMath::AbsF(dx) <= Epsilon && CubismMath::AbsF(dy) <= Epsilon)
    {
        return;
    }

    const csmFloat32 d = CubismMath::SqrtF((dx * dx) + (dy * dy));

    const csmFloat32 vx = MaxV * dx / d;
    const csmFloat32 vy = MaxV * dy / d;

    csmFloat32 ax = vx - _faceVX;
    csmFloat32 ay = vy - _faceVY;

    const csmFloat32 a = CubismMath::SqrtF((ax * ax) + (ay * ay));

    if (a < -MaxA || a > MaxA)
    {
        ax *= MaxA / a;
        ay *= MaxA / a;
    }

    _faceVX += ax;
    _faceVY += ay;

    {

        const csmFloat32 maxV = 0.5f * (CubismMath::SqrtF((MaxA * MaxA) + 16.0f * MaxA * d - 8.0f * MaxA * d) - MaxA);
        const csmFloat32 curV = CubismMath::SqrtF((_faceVX * _faceVX) + (_faceVY * _faceVY));

        if (curV > maxV)
        {
            _faceVX *= maxV / curV;
            _faceVY *= maxV / curV;
        }
    }

    _faceX += _faceVX;
    _faceY += _faceVY;
}

void CubismTargetPoint::Set(csmFloat32 x, csmFloat32 y)
{
    this->_faceTargetX = x;
    this->_faceTargetY = y;
}

csmFloat32 CubismTargetPoint::GetX() const
{
    return this->_faceX;
}

csmFloat32 CubismTargetPoint::GetY() const
{
    return this->_faceY;
}

}}}
