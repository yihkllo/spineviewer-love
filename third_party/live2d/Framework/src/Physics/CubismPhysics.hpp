

#pragma once

#include "Math/CubismVector2.hpp"
#include "CubismPhysicsInternal.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismModel;
struct CubismPhysicsRig;

class CubismPhysics
{
public:
    struct Options
    {
        CubismVector2 Gravity;
        CubismVector2 Wind;
    };

    struct PhysicsOutput
    {
        csmVector<csmFloat32> outputs;
    };

    static CubismPhysics* Create(const csmByte* buffer, csmSizeInt size);

    static void Delete(CubismPhysics* physics);

    void Reset();

    void Stabilization(CubismModel* model);

    void Evaluate(CubismModel* model, csmFloat32 deltaTimeSeconds);

    void SetOptions(const Options& options);

    const Options& GetOptions() const;

private:
    CubismPhysics();

    virtual ~CubismPhysics();

    CubismPhysics(const CubismPhysics&);
    CubismPhysics& operator=(const CubismPhysics&);

    void Parse(const csmByte* physicsJson, csmSizeInt size);

    void Initialize();

    void Interpolate(CubismModel* model, csmFloat32 weight);

    CubismPhysicsRig* _physicsRig;
    Options _options;

    csmVector<PhysicsOutput> _currentRigOutputs;
    csmVector<PhysicsOutput> _previousRigOutputs;

    csmFloat32 _currentRemainTime;

    csmVector<csmFloat32> _parameterCaches;
    csmVector<csmFloat32> _parameterInputCaches;

    csmBool _isJsonValid;
};

}}}
