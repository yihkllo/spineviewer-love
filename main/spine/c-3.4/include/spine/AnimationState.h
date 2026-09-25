
#ifndef SPINE_ANIMATIONSTATE_H_
#define SPINE_ANIMATIONSTATE_H_

#include <spine/Animation.h>
#include <spine/AnimationStateData.h>
#include <spine/Event.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SP_ANIMATION_START, SP_ANIMATION_END, SP_ANIMATION_COMPLETE, SP_ANIMATION_EVENT
} spEventType;

typedef struct spAnimationState spAnimationState;

typedef void (*spAnimationStateListener) (spAnimationState* state, int trackIndex, spEventType type, spEvent* event,
		int loopCount);

typedef struct spTrackEntry spTrackEntry;
struct spTrackEntry {
	spAnimationState* const state;
	spTrackEntry* next;
	spTrackEntry* previous;
	spAnimation* animation;
	int loop;
	float delay, time, lastTime, endTime, timeScale;
	spAnimationStateListener listener;
	float mixTime, mixDuration, mix;

	void* rendererObject;

#ifdef __cplusplus
	spTrackEntry() :
		state(0),
		next(0),
		previous(0),
		animation(0),
		loop(0),
		delay(0), time(0), lastTime(0), endTime(0), timeScale(0),
		listener(0),
		mixTime(0), mixDuration(0), mix(0),
		rendererObject(0) {
	}
#endif
};

struct spAnimationState {
	spAnimationStateData* const data;
	float timeScale;
	spAnimationStateListener listener;

	int tracksCount;
	spTrackEntry** tracks;

	void* rendererObject;

#ifdef __cplusplus
	spAnimationState() :
		data(0),
		timeScale(0),
		listener(0),
		tracksCount(0),
		tracks(0),
		rendererObject(0) {
	}
#endif
};

spAnimationState* spAnimationState_create (spAnimationStateData* data);
void spAnimationState_dispose (spAnimationState* self);

void spAnimationState_update (spAnimationState* self, float delta);
void spAnimationState_apply (spAnimationState* self, struct spSkeleton* skeleton);

void spAnimationState_clearTracks (spAnimationState* self);
void spAnimationState_clearTrack (spAnimationState* self, int trackIndex);

spTrackEntry* spAnimationState_setAnimationByName (spAnimationState* self, int trackIndex, const char* animationName,
		int loop);
spTrackEntry* spAnimationState_setAnimation (spAnimationState* self, int trackIndex, spAnimation* animation, int loop);

spTrackEntry* spAnimationState_addAnimationByName (spAnimationState* self, int trackIndex, const char* animationName,
		int loop, float delay);
spTrackEntry* spAnimationState_addAnimation (spAnimationState* self, int trackIndex, spAnimation* animation, int loop,
		float delay);

spTrackEntry* spAnimationState_getCurrent (spAnimationState* self, int trackIndex);

#ifdef SPINE_SHORT_NAMES
typedef spEventType EventType;
#define ANIMATION_START SP_ANIMATION_START
#define ANIMATION_END SP_ANIMATION_END
#define ANIMATION_COMPLETE SP_ANIMATION_COMPLETE
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
#define AnimationState_getCurrent(...) spAnimationState_getCurrent(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
