
#ifndef SPINE_ANIMATIONSTATE_H_
#define SPINE_ANIMATIONSTATE_H_

#include <spine/dll.h>
#include <spine/Animation.h>
#include <spine/AnimationStateData.h>
#include <spine/Event.h>
#include <spine/Array.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SP_ANIMATION_START, SP_ANIMATION_INTERRUPT, SP_ANIMATION_END, SP_ANIMATION_COMPLETE, SP_ANIMATION_DISPOSE, SP_ANIMATION_EVENT
} spEventType;

typedef struct spAnimationState spAnimationState;
typedef struct spTrackEntry spTrackEntry;

typedef void (*spAnimationStateListener) (spAnimationState* state, spEventType type, spTrackEntry* entry, spEvent* event);

_SP_ARRAY_DECLARE_TYPE(spTrackEntryArray, spTrackEntry*)

struct spTrackEntry {
	spAnimation* animation;
	spTrackEntry* next;
	spTrackEntry* mixingFrom;
	spAnimationStateListener listener;
	int trackIndex;
	int   loop;
	float eventThreshold, attachmentThreshold, drawOrderThreshold;
	float animationStart, animationEnd, animationLast, nextAnimationLast;
	float delay, trackTime, trackLast, nextTrackLast, trackEnd, timeScale;
	float alpha, mixTime, mixDuration, interruptAlpha, totalAlpha;
	spIntArray* timelineData;
	spTrackEntryArray* timelineDipMix;
	float* timelinesRotation;
	int timelinesRotationCount;
	void* rendererObject;
	void* userData;

#ifdef __cplusplus
	spTrackEntry() :
		animation(0),
		next(0), mixingFrom(0),
		listener(0),
		trackIndex(0),
		loop(0),
		eventThreshold(0), attachmentThreshold(0), drawOrderThreshold(0),
		animationStart(0), animationEnd(0), animationLast(0), nextAnimationLast(0),
		delay(0), trackTime(0), trackLast(0), nextTrackLast(0), trackEnd(0), timeScale(0),
		alpha(0), mixTime(0), mixDuration(0), interruptAlpha(0), totalAlpha(0),
		timelineData(0),
		timelineDipMix(0),
		timelinesRotation(0),
		timelinesRotationCount(0),
		rendererObject(0), userData(0) {
	}
#endif
};

struct spAnimationState {
	spAnimationStateData* const data;

	int tracksCount;
	spTrackEntry** tracks;

	spAnimationStateListener listener;

	float timeScale;

	spTrackEntryArray* mixingTo;

	void* rendererObject;
	void* userData;

#ifdef __cplusplus
	spAnimationState() :
		data(0),
		tracksCount(0),
		tracks(0),
		listener(0),
		timeScale(0),
		mixingTo(0),
		rendererObject(0),
		userData(0) {
	}
#endif
};

SP_API spAnimationState* spAnimationState_create (spAnimationStateData* data);
SP_API void spAnimationState_dispose (spAnimationState* self);

SP_API void spAnimationState_update (spAnimationState* self, float delta);
SP_API int   spAnimationState_apply (spAnimationState* self, struct spSkeleton* skeleton);

SP_API void spAnimationState_clearTracks (spAnimationState* self);
SP_API void spAnimationState_clearTrack (spAnimationState* self, int trackIndex);

SP_API spTrackEntry* spAnimationState_setAnimationByName (spAnimationState* self, int trackIndex, const char* animationName,
		int loop);
SP_API spTrackEntry* spAnimationState_setAnimation (spAnimationState* self, int trackIndex, spAnimation* animation, int loop);

SP_API spTrackEntry* spAnimationState_addAnimationByName (spAnimationState* self, int trackIndex, const char* animationName,
		int loop, float delay);
SP_API spTrackEntry* spAnimationState_addAnimation (spAnimationState* self, int trackIndex, spAnimation* animation, int loop,
		float delay);
SP_API spTrackEntry* spAnimationState_setEmptyAnimation(spAnimationState* self, int trackIndex, float mixDuration);
SP_API spTrackEntry* spAnimationState_addEmptyAnimation(spAnimationState* self, int trackIndex, float mixDuration, float delay);
SP_API void spAnimationState_setEmptyAnimations(spAnimationState* self, float mixDuration);

SP_API spTrackEntry* spAnimationState_getCurrent (spAnimationState* self, int trackIndex);

SP_API void spAnimationState_clearListenerNotifications(spAnimationState* self);

SP_API float spTrackEntry_getAnimationTime (spTrackEntry* entry);

SP_API void spAnimationState_disposeStatics ();

#ifdef SPINE_SHORT_NAMES
typedef spEventType EventType;
#define ANIMATION_START SP_ANIMATION_START
#define ANIMATION_INTERRUPT SP_ANIMATION_INTERRUPT
#define ANIMATION_END SP_ANIMATION_END
#define ANIMATION_COMPLETE SP_ANIMATION_COMPLETE
#define ANIMATION_DISPOSE SP_ANIMATION_DISPOSE
#define ANIMATION_EVENT SP_ANIMATION_EVENT
typedef spAnimationStateListener AnimationStateListener;
typedef spTrackEntry TrackEntry;
typedef spAnimationState AnimationState;
#define AnimationState_create(...) spAnimationState_create(__VA_ARGS__)
#define AnimationState_dispose(...) spAnimationState_dispose(__VA_ARGS__)
#define AnimationState_update(...) spAnimationState_update(__VA_ARGS__)
#define AnimationState_apply(...) spAnimationState_apply(__VA_ARGS__)
#define AnimationState_clearTracks(...) spAnimationState_clearTracks(__VA_ARGS__)
#define AnimationState_clearTrack(...) spAnimationState_clearTrack(__VA_ARGS__)
#define AnimationState_setAnimationByName(...) spAnimationState_setAnimationByName(__VA_ARGS__)
#define AnimationState_setAnimation(...) spAnimationState_setAnimation(__VA_ARGS__)
#define AnimationState_addAnimationByName(...) spAnimationState_addAnimationByName(__VA_ARGS__)
#define AnimationState_addAnimation(...) spAnimationState_addAnimation(__VA_ARGS__)
#define AnimationState_setEmptyAnimation(...) spAnimatinState_setEmptyAnimation(__VA_ARGS__)
#define AnimationState_addEmptyAnimation(...) spAnimatinState_addEmptyAnimation(__VA_ARGS__)
#define AnimationState_setEmptyAnimations(...) spAnimatinState_setEmptyAnimations(__VA_ARGS__)
#define AnimationState_getCurrent(...) spAnimationState_getCurrent(__VA_ARGS__)
#define AnimationState_clearListenerNotifications(...) spAnimatinState_clearListenerNotifications(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
