
#ifndef SPINE_ANIMATION_H_
#define SPINE_ANIMATION_H_

#include <spine/dll.h>
#include <spine/Event.h>
#include <spine/Attachment.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spTimeline spTimeline;
struct spSkeleton;

typedef struct spAnimation {
	const char* const name;
	float duration;

	int timelinesCount;
	spTimeline** timelines;

#ifdef __cplusplus
	spAnimation() :
		name(0),
		duration(0),
		timelinesCount(0),
		timelines(0) {
	}
#endif
} spAnimation;

typedef enum {
	SP_MIX_POSE_SETUP,
	SP_MIX_POSE_CURRENT,
	SP_MIX_POSE_CURRENT_LAYERED
} spMixPose;

typedef enum {
	SP_MIX_DIRECTION_IN,
	SP_MIX_DIRECTION_OUT
} spMixDirection;

SP_API spAnimation* spAnimation_create (const char* name, int timelinesCount);
SP_API void spAnimation_dispose (spAnimation* self);

SP_API void spAnimation_apply (const spAnimation* self, struct spSkeleton* skeleton, float lastTime, float time, int loop,
		spEvent** events, int* eventsCount, float alpha, spMixPose pose, spMixDirection direction);

#ifdef SPINE_SHORT_NAMES
typedef spAnimation Animation;
#define Animation_create(...) spAnimation_create(__VA_ARGS__)
#define Animation_dispose(...) spAnimation_dispose(__VA_ARGS__)
#define Animation_apply(...) spAnimation_apply(__VA_ARGS__)
#endif


typedef enum {
	SP_TIMELINE_ROTATE,
	SP_TIMELINE_TRANSLATE,
	SP_TIMELINE_SCALE,
	SP_TIMELINE_SHEAR,
	SP_TIMELINE_ATTACHMENT,
	SP_TIMELINE_COLOR,
	SP_TIMELINE_DEFORM,
	SP_TIMELINE_EVENT,
	SP_TIMELINE_DRAWORDER,
	SP_TIMELINE_IKCONSTRAINT,
	SP_TIMELINE_TRANSFORMCONSTRAINT,
	SP_TIMELINE_PATHCONSTRAINTPOSITION,
	SP_TIMELINE_PATHCONSTRAINTSPACING,
	SP_TIMELINE_PATHCONSTRAINTMIX,
	SP_TIMELINE_TWOCOLOR
} spTimelineType;

struct spTimeline {
	const spTimelineType type;
	const void* const vtable;

#ifdef __cplusplus
	spTimeline() :
		type(SP_TIMELINE_SCALE),
		vtable(0) {
	}
#endif
};

SP_API void spTimeline_dispose (spTimeline* self);
SP_API void spTimeline_apply (const spTimeline* self, struct spSkeleton* skeleton, float lastTime, float time, spEvent** firedEvents,
		int* eventsCount, float alpha, spMixPose pose, spMixDirection direction);
SP_API int spTimeline_getPropertyId (const spTimeline* self);

#ifdef SPINE_SHORT_NAMES
typedef spTimeline Timeline;
#define TIMELINE_SCALE SP_TIMELINE_SCALE
#define TIMELINE_ROTATE SP_TIMELINE_ROTATE
#define TIMELINE_TRANSLATE SP_TIMELINE_TRANSLATE
#define TIMELINE_COLOR SP_TIMELINE_COLOR
#define TIMELINE_ATTACHMENT SP_TIMELINE_ATTACHMENT
#define TIMELINE_EVENT SP_TIMELINE_EVENT
#define TIMELINE_DRAWORDER SP_TIMELINE_DRAWORDER
#define Timeline_dispose(...) spTimeline_dispose(__VA_ARGS__)
#define Timeline_apply(...) spTimeline_apply(__VA_ARGS__)
#endif


typedef struct spCurveTimeline {
	spTimeline super;
	float* curves;

#ifdef __cplusplus
	spCurveTimeline() :
		super(),
		curves(0) {
	}
#endif
} spCurveTimeline;

SP_API void spCurveTimeline_setLinear (spCurveTimeline* self, int frameIndex);
SP_API void spCurveTimeline_setStepped (spCurveTimeline* self, int frameIndex);

SP_API void spCurveTimeline_setCurve (spCurveTimeline* self, int frameIndex, float cx1, float cy1, float cx2, float cy2);
SP_API float spCurveTimeline_getCurvePercent (const spCurveTimeline* self, int frameIndex, float percent);

#ifdef SPINE_SHORT_NAMES
typedef spCurveTimeline CurveTimeline;
#define CurveTimeline_setLinear(...) spCurveTimeline_setLinear(__VA_ARGS__)
#define CurveTimeline_setStepped(...) spCurveTimeline_setStepped(__VA_ARGS__)
#define CurveTimeline_setCurve(...) spCurveTimeline_setCurve(__VA_ARGS__)
#define CurveTimeline_getCurvePercent(...) spCurveTimeline_getCurvePercent(__VA_ARGS__)
#endif


typedef struct spBaseTimeline {
	spCurveTimeline super;
	int const framesCount;
	float* const frames;
	int boneIndex;

#ifdef __cplusplus
	spBaseTimeline() :
		super(),
		framesCount(0),
		frames(0),
		boneIndex(0) {
	}
#endif
} spBaseTimeline;


static const int ROTATE_PREV_TIME = -2, ROTATE_PREV_ROTATION = -1;
static const int ROTATE_ROTATION = 1;
static const int ROTATE_ENTRIES = 2;

typedef struct spBaseTimeline spRotateTimeline;

SP_API spRotateTimeline* spRotateTimeline_create (int framesCount);

SP_API void spRotateTimeline_setFrame (spRotateTimeline* self, int frameIndex, float time, float angle);

#ifdef SPINE_SHORT_NAMES
typedef spRotateTimeline RotateTimeline;
#define RotateTimeline_create(...) spRotateTimeline_create(__VA_ARGS__)
#define RotateTimeline_setFrame(...) spRotateTimeline_setFrame(__VA_ARGS__)
#endif


static const int TRANSLATE_ENTRIES = 3;

typedef struct spBaseTimeline spTranslateTimeline;

SP_API spTranslateTimeline* spTranslateTimeline_create (int framesCount);

SP_API void spTranslateTimeline_setFrame (spTranslateTimeline* self, int frameIndex, float time, float x, float y);

#ifdef SPINE_SHORT_NAMES
typedef spTranslateTimeline TranslateTimeline;
#define TranslateTimeline_create(...) spTranslateTimeline_create(__VA_ARGS__)
#define TranslateTimeline_setFrame(...) spTranslateTimeline_setFrame(__VA_ARGS__)
#endif


typedef struct spBaseTimeline spScaleTimeline;

SP_API spScaleTimeline* spScaleTimeline_create (int framesCount);

SP_API void spScaleTimeline_setFrame (spScaleTimeline* self, int frameIndex, float time, float x, float y);

#ifdef SPINE_SHORT_NAMES
typedef spScaleTimeline ScaleTimeline;
#define ScaleTimeline_create(...) spScaleTimeline_create(__VA_ARGS__)
#define ScaleTimeline_setFrame(...) spScaleTimeline_setFrame(__VA_ARGS__)
#endif


typedef struct spBaseTimeline spShearTimeline;

SP_API spShearTimeline* spShearTimeline_create (int framesCount);

SP_API void spShearTimeline_setFrame (spShearTimeline* self, int frameIndex, float time, float x, float y);

#ifdef SPINE_SHORT_NAMES
typedef spShearTimeline ShearTimeline;
#define ShearTimeline_create(...) spShearTimeline_create(__VA_ARGS__)
#define ShearTimeline_setFrame(...) spShearTimeline_setFrame(__VA_ARGS__)
#endif


static const int COLOR_ENTRIES = 5;

typedef struct spColorTimeline {
	spCurveTimeline super;
	int const framesCount;
	float* const frames;
	int slotIndex;

#ifdef __cplusplus
	spColorTimeline() :
		super(),
		framesCount(0),
		frames(0),
		slotIndex(0) {
	}
#endif
} spColorTimeline;

SP_API spColorTimeline* spColorTimeline_create (int framesCount);

SP_API void spColorTimeline_setFrame (spColorTimeline* self, int frameIndex, float time, float r, float g, float b, float a);

#ifdef SPINE_SHORT_NAMES
typedef spColorTimeline ColorTimeline;
#define ColorTimeline_create(...) spColorTimeline_create(__VA_ARGS__)
#define ColorTimeline_setFrame(...) spColorTimeline_setFrame(__VA_ARGS__)
#endif


static const int TWOCOLOR_ENTRIES = 8;

typedef struct spTwoColorTimeline {
	spCurveTimeline super;
	int const framesCount;
	float* const frames;
	int slotIndex;

#ifdef __cplusplus
	spTwoColorTimeline() :
		super(),
		framesCount(0),
		frames(0),
		slotIndex(0) {
	}
#endif
} spTwoColorTimeline;

SP_API spTwoColorTimeline* spTwoColorTimeline_create (int framesCount);

SP_API void spTwoColorTimeline_setFrame (spTwoColorTimeline* self, int frameIndex, float time, float r, float g, float b, float a, float r2, float g2, float b2);

#ifdef SPINE_SHORT_NAMES
typedef spTwoColorTimeline TwoColorTimeline;
#define TwoColorTimeline_create(...) spTwoColorTimeline_create(__VA_ARGS__)
#define TwoColorTimeline_setFrame(...) spTwoColorTimeline_setFrame(__VA_ARGS__)
#endif


typedef struct spAttachmentTimeline {
	spTimeline super;
	int const framesCount;
	float* const frames;
	int slotIndex;
	const char** const attachmentNames;

#ifdef __cplusplus
	spAttachmentTimeline() :
		super(),
		framesCount(0),
		frames(0),
		slotIndex(0),
		attachmentNames(0) {
	}
#endif
} spAttachmentTimeline;

SP_API spAttachmentTimeline* spAttachmentTimeline_create (int framesCount);

SP_API void spAttachmentTimeline_setFrame (spAttachmentTimeline* self, int frameIndex, float time, const char* attachmentName);

#ifdef SPINE_SHORT_NAMES
typedef spAttachmentTimeline AttachmentTimeline;
#define AttachmentTimeline_create(...) spAttachmentTimeline_create(__VA_ARGS__)
#define AttachmentTimeline_setFrame(...) spAttachmentTimeline_setFrame(__VA_ARGS__)
#endif


typedef struct spEventTimeline {
	spTimeline super;
	int const framesCount;
	float* const frames;
	spEvent** const events;

#ifdef __cplusplus
	spEventTimeline() :
		super(),
		framesCount(0),
		frames(0),
		events(0) {
	}
#endif
} spEventTimeline;

SP_API spEventTimeline* spEventTimeline_create (int framesCount);

SP_API void spEventTimeline_setFrame (spEventTimeline* self, int frameIndex, spEvent* event);

#ifdef SPINE_SHORT_NAMES
typedef spEventTimeline EventTimeline;
#define EventTimeline_create(...) spEventTimeline_create(__VA_ARGS__)
#define EventTimeline_setFrame(...) spEventTimeline_setFrame(__VA_ARGS__)
#endif


typedef struct spDrawOrderTimeline {
	spTimeline super;
	int const framesCount;
	float* const frames;
	const int** const drawOrders;
	int const slotsCount;

#ifdef __cplusplus
	spDrawOrderTimeline() :
		super(),
		framesCount(0),
		frames(0),
		drawOrders(0),
		slotsCount(0) {
	}
#endif
} spDrawOrderTimeline;

SP_API spDrawOrderTimeline* spDrawOrderTimeline_create (int framesCount, int slotsCount);

SP_API void spDrawOrderTimeline_setFrame (spDrawOrderTimeline* self, int frameIndex, float time, const int* drawOrder);

#ifdef SPINE_SHORT_NAMES
typedef spDrawOrderTimeline DrawOrderTimeline;
#define DrawOrderTimeline_create(...) spDrawOrderTimeline_create(__VA_ARGS__)
#define DrawOrderTimeline_setFrame(...) spDrawOrderTimeline_setFrame(__VA_ARGS__)
#endif


typedef struct spDeformTimeline {
	spCurveTimeline super;
	int const framesCount;
	float* const frames;
	int const frameVerticesCount;
	const float** const frameVertices;
	int slotIndex;
	spAttachment* attachment;

#ifdef __cplusplus
	spDeformTimeline() :
		super(),
		framesCount(0),
		frames(0),
		frameVerticesCount(0),
		frameVertices(0),
		slotIndex(0) {
	}
#endif
} spDeformTimeline;

SP_API spDeformTimeline* spDeformTimeline_create (int framesCount, int frameVerticesCount);

SP_API void spDeformTimeline_setFrame (spDeformTimeline* self, int frameIndex, float time, float* vertices);

#ifdef SPINE_SHORT_NAMES
typedef spDeformTimeline DeformTimeline;
#define DeformTimeline_create(...) spDeformTimeline_create(__VA_ARGS__)
#define DeformTimeline_setFrame(...) spDeformTimeline_setFrame(__VA_ARGS__)
#endif


static const int IKCONSTRAINT_ENTRIES = 3;

typedef struct spIkConstraintTimeline {
	spCurveTimeline super;
	int const framesCount;
	float* const frames;
	int ikConstraintIndex;

#ifdef __cplusplus
	spIkConstraintTimeline() :
		super(),
		framesCount(0),
		frames(0),
		ikConstraintIndex(0) {
	}
#endif
} spIkConstraintTimeline;

SP_API spIkConstraintTimeline* spIkConstraintTimeline_create (int framesCount);

SP_API void spIkConstraintTimeline_setFrame (spIkConstraintTimeline* self, int frameIndex, float time, float mix, int bendDirection);

#ifdef SPINE_SHORT_NAMES
typedef spIkConstraintTimeline IkConstraintTimeline;
#define IkConstraintTimeline_create(...) spIkConstraintTimeline_create(__VA_ARGS__)
#define IkConstraintTimeline_setFrame(...) spIkConstraintTimeline_setFrame(__VA_ARGS__)
#endif


static const int TRANSFORMCONSTRAINT_ENTRIES = 5;

typedef struct spTransformConstraintTimeline {
	spCurveTimeline super;
	int const framesCount;
	float* const frames;
	int transformConstraintIndex;

#ifdef __cplusplus
	spTransformConstraintTimeline() :
		super(),
		framesCount(0),
		frames(0),
		transformConstraintIndex(0) {
	}
#endif
} spTransformConstraintTimeline;

SP_API spTransformConstraintTimeline* spTransformConstraintTimeline_create (int framesCount);

SP_API void spTransformConstraintTimeline_setFrame (spTransformConstraintTimeline* self, int frameIndex, float time, float rotateMix, float translateMix, float scaleMix, float shearMix);

#ifdef SPINE_SHORT_NAMES
typedef spTransformConstraintTimeline TransformConstraintTimeline;
#define TransformConstraintTimeline_create(...) spTransformConstraintTimeline_create(__VA_ARGS__)
#define TransformConstraintTimeline_setFrame(...) spTransformConstraintTimeline_setFrame(__VA_ARGS__)
#endif


static const int PATHCONSTRAINTPOSITION_ENTRIES = 2;

typedef struct spPathConstraintPositionTimeline {
	spCurveTimeline super;
	int const framesCount;
	float* const frames;
	int pathConstraintIndex;

#ifdef __cplusplus
	spPathConstraintPositionTimeline() :
		super(),
		framesCount(0),
		frames(0),
		pathConstraintIndex(0) {
	}
#endif
} spPathConstraintPositionTimeline;

SP_API spPathConstraintPositionTimeline* spPathConstraintPositionTimeline_create (int framesCount);

SP_API void spPathConstraintPositionTimeline_setFrame (spPathConstraintPositionTimeline* self, int frameIndex, float time, float value);

#ifdef SPINE_SHORT_NAMES
typedef spPathConstraintPositionTimeline PathConstraintPositionTimeline;
#define PathConstraintPositionTimeline_create(...) spPathConstraintPositionTimeline_create(__VA_ARGS__)
#define PathConstraintPositionTimeline_setFrame(...) spPathConstraintPositionTimeline_setFrame(__VA_ARGS__)
#endif


static const int PATHCONSTRAINTSPACING_ENTRIES = 2;

typedef struct spPathConstraintSpacingTimeline {
	spCurveTimeline super;
	int const framesCount;
	float* const frames;
	int pathConstraintIndex;

#ifdef __cplusplus
	spPathConstraintSpacingTimeline() :
		super(),
		framesCount(0),
		frames(0),
		pathConstraintIndex(0) {
	}
#endif
} spPathConstraintSpacingTimeline;

SP_API spPathConstraintSpacingTimeline* spPathConstraintSpacingTimeline_create (int framesCount);

SP_API void spPathConstraintSpacingTimeline_setFrame (spPathConstraintSpacingTimeline* self, int frameIndex, float time, float value);

#ifdef SPINE_SHORT_NAMES
typedef spPathConstraintSpacingTimeline PathConstraintSpacingTimeline;
#define PathConstraintSpacingTimeline_create(...) spPathConstraintSpacingTimeline_create(__VA_ARGS__)
#define PathConstraintSpacingTimeline_setFrame(...) spPathConstraintSpacingTimeline_setFrame(__VA_ARGS__)
#endif


static const int PATHCONSTRAINTMIX_ENTRIES = 3;

typedef struct spPathConstraintMixTimeline {
	spCurveTimeline super;
	int const framesCount;
	float* const frames;
	int pathConstraintIndex;

#ifdef __cplusplus
	spPathConstraintMixTimeline() :
		super(),
		framesCount(0),
		frames(0),
		pathConstraintIndex(0) {
	}
#endif
} spPathConstraintMixTimeline;

SP_API spPathConstraintMixTimeline* spPathConstraintMixTimeline_create (int framesCount);

SP_API void spPathConstraintMixTimeline_setFrame (spPathConstraintMixTimeline* self, int frameIndex, float time, float rotateMix, float translateMix);

#ifdef SPINE_SHORT_NAMES
typedef spPathConstraintMixTimeline PathConstraintMixTimeline;
#define PathConstraintMixTimeline_create(...) spPathConstraintMixTimeline_create(__VA_ARGS__)
#define PathConstraintMixTimeline_setFrame(...) spPathConstraintMixTimeline_setFrame(__VA_ARGS__)
#endif


#ifdef __cplusplus
}
#endif

#endif
