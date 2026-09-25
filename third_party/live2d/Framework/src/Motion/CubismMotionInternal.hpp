

#pragma once

#include "CubismFramework.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

enum CubismMotionCurveTarget
{
    CubismMotionCurveTarget_Model,
    CubismMotionCurveTarget_Parameter,
    CubismMotionCurveTarget_PartOpacity
};


enum CubismMotionSegmentType
{
    CubismMotionSegmentType_Linear = 0,
    CubismMotionSegmentType_Bezier = 1,
    CubismMotionSegmentType_Stepped = 2,
    CubismMotionSegmentType_InverseStepped = 3
};

struct CubismMotionPoint
{
    CubismMotionPoint()
        : Time(0.0f)
        , Value(0.0f)
    { }

    csmFloat32 Time;
    csmFloat32 Value;
};

typedef csmFloat32 (*csmMotionSegmentEvaluationFunction)(const CubismMotionPoint* points, const csmFloat32 time);


struct CubismMotionSegment
{
    CubismMotionSegment()
        : Evaluate(NULL)
        , BasePointIndex(0)
        , SegmentType(0)
    { }

    csmMotionSegmentEvaluationFunction Evaluate;
    csmInt32 BasePointIndex;
    csmInt32 SegmentType;
};

struct CubismMotionCurve
{
    CubismMotionCurve()
        : Type(CubismMotionCurveTarget_Model)
        , SegmentCount(0)
        , BaseSegmentIndex(0)
        , FadeInTime(0.0f)
        , FadeOutTime(0.0f)
    { }

    CubismMotionCurveTarget Type;
    CubismIdHandle Id;
    csmInt32 SegmentCount;
    csmInt32 BaseSegmentIndex;
    csmFloat32 FadeInTime;
    csmFloat32 FadeOutTime;
};

struct CubismMotionEvent
{
    CubismMotionEvent()
        : FireTime(0.0f)
    { }

    csmFloat32  FireTime;
    csmString   Value;
};

struct CubismMotionData
{
    CubismMotionData()
        : Duration(0.0f)
        , Loop(0)
        , CurveCount(0)
        , EventCount(0)
        , Fps(0.0f)
    { }

    csmFloat32 Duration;
    csmInt16 Loop;
    csmInt16 CurveCount;
    csmInt32 EventCount;
    csmFloat32 Fps;
    csmVector<CubismMotionCurve> Curves;
    csmVector<CubismMotionSegment> Segments;
    csmVector<CubismMotionPoint> Points;
    csmVector<CubismMotionEvent> Events;
};

}}}
