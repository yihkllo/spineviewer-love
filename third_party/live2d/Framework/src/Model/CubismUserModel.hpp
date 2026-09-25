

#pragma once

#include "Effect/CubismPose.hpp"
#include "Effect/CubismEyeBlink.hpp"
#include "Effect/CubismBreath.hpp"
#include "Math/CubismModelMatrix.hpp"
#include "Math/CubismTargetPoint.hpp"
#include "Model/CubismMoc.hpp"
#include "Model/CubismModel.hpp"
#include "Motion/CubismMotionManager.hpp"
#include "Motion/CubismExpressionMotion.hpp"
#include "Physics/CubismPhysics.hpp"
#include "Rendering/CubismRenderer.hpp"
#include "Model/CubismModelUserData.hpp"
#include "Motion/CubismExpressionMotionManager.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismUserModel
{
public:
    CubismUserModel();

    virtual ~CubismUserModel();

    virtual csmBool         IsInitialized();

    virtual void            IsInitialized(csmBool v);

    virtual csmBool         IsUpdating();

    virtual void            IsUpdating(csmBool v);

    virtual void            SetDragging(csmFloat32 x, csmFloat32 y);

    virtual void            SetAcceleration(csmFloat32 x, csmFloat32 y, csmFloat32 z);

    CubismModelMatrix*      GetModelMatrix() const;

    virtual void            SetOpacity(csmFloat32 a);

    virtual csmFloat32      GetOpacity();

    virtual void            LoadModel(const csmByte* buffer, csmSizeInt size, csmBool shouldCheckMocConsistency = false);

    virtual ACubismMotion*  LoadMotion(const csmByte* buffer, csmSizeInt size, const csmChar* name, ACubismMotion::FinishedMotionCallback onFinishedMotionHandler = NULL);

    virtual ACubismMotion*   LoadExpression(const csmByte* buffer, csmSizeInt size, const csmChar* name);

    virtual void            LoadPose(const csmByte* buffer, csmSizeInt size);

    virtual void            LoadPhysics(const csmByte* buffer, csmSizeInt size);

    virtual void            LoadUserData(const csmByte* buffer, csmSizeInt size);

    virtual csmBool         IsHit(CubismIdHandle drawableId, csmFloat32 pointX, csmFloat32 pointY);

    CubismModel*            GetModel() const;

    template<class T> T*    GetRenderer() { return dynamic_cast<T*>(_renderer); }

    void CreateRenderer(csmInt32 maskBufferCount = 1);

    void DeleteRenderer();

    virtual void   MotionEventFired(const csmString& eventValue);

    static void   CubismDefaultMotionEventCallback(const CubismMotionQueueManager* caller, const csmString& eventValue, void* customData);
protected:
    CubismMoc*              _moc;
    CubismModel*            _model;

    CubismMotionManager*    _motionManager;
    CubismExpressionMotionManager*    _expressionManager;
    CubismEyeBlink*         _eyeBlink;
    CubismBreath*           _breath;
    CubismModelMatrix*      _modelMatrix;
    CubismPose*             _pose;
    CubismTargetPoint*      _dragManager;
    CubismPhysics*          _physics;
    CubismModelUserData*    _modelUserData;

    csmBool     _initialized;
    csmBool     _updating;
    csmFloat32  _opacity;
    csmBool     _lipSync;
    csmFloat32  _lastLipSyncValue;
    csmFloat32  _dragX;
    csmFloat32  _dragY;
    csmFloat32  _accelerationX;
    csmFloat32  _accelerationY;
    csmFloat32  _accelerationZ;
    csmBool     _mocConsistency;
    csmBool     _debugMode;

private:
    Rendering::CubismRenderer* _renderer;
};

}}}
