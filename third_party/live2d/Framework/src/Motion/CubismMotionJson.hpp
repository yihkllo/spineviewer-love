


#pragma once

#include "CubismJsonHolder.hpp"
#include "Utils/CubismJson.hpp"
#include "Id/CubismId.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

enum EvaluationOptionFlag
{
    EvaluationOptionFlag_AreBeziersRestricted = 0,
};

class CubismMotionJson : public CubismJsonHolder
{
public:
    CubismMotionJson(const csmByte* buffer, csmSizeInt size);

    virtual ~CubismMotionJson();

    csmFloat32 GetMotionDuration() const;

    csmBool IsMotionLoop() const;

    csmBool GetEvaluationOptionFlag(csmInt32 flagType) const;

    csmInt32 GetMotionCurveCount() const;

    csmInt32 GetActualMotionCurveCount() const;

    csmFloat32 GetMotionFps() const;

    csmInt32 GetMotionTotalSegmentCount() const;

    csmInt32 GetMotionTotalPointCount() const;

    csmBool IsExistMotionFadeInTime() const;

    csmBool IsExistMotionFadeOutTime() const;

    csmFloat32 GetMotionFadeInTime() const;

    csmFloat32 GetMotionFadeOutTime() const;

    const csmChar* GetMotionCurveTarget(csmInt32 curveIndex) const;

    CubismIdHandle GetMotionCurveId(csmInt32 curveIndex) const;

    csmBool IsExistMotionCurveFadeInTime(csmInt32 curveIndex) const;

    csmBool IsExistMotionCurveFadeOutTime(csmInt32 curveIndex) const;

    csmFloat32 GetMotionCurveFadeInTime(csmInt32 curveIndex) const;

    csmFloat32 GetMotionCurveFadeOutTime(csmInt32 curveIndex) const;

    csmInt32 GetMotionCurveSegmentCount(csmInt32 curveIndex) const;


    csmFloat32 GetMotionCurveSegment(csmInt32 curveIndex, csmInt32 segmentIndex) const;

    csmInt32 GetEventCount() const;

    csmInt32 GetActualEventCount() const;

    csmInt32 GetTotalEventValueSize() const;

    csmFloat32 GetEventTime(csmInt32 userDataIndex) const;

    const csmChar* GetEventValue(csmInt32 userDataIndex) const;
};

}}}
