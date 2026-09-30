#ifndef Spine_AnimationState_h
#define Spine_AnimationState_h

#include <spine/Array.h>
#include <spine/Map.h>
#include <spine/Pool.h>
#include <spine/Property.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/Timeline.h>
#include <spine/HasRendererObject.h>
#include <spine/Interpolation.h>
#include "Slot.h"

#ifdef SPINE_USE_STD_FUNCTION
#include <functional>
#endif

namespace spine {
	enum EventType {
		EventType_Start = 0,
		EventType_Interrupt,
		EventType_End,
		EventType_Dispose,
		EventType_Complete,
		EventType_Event
	};

	class AnimationState;

	class TrackEntry;

	class Animation;

	class Event;

	class AnimationStateData;

	class Skeleton;

	class RotateTimeline;

	class AttachmentTimeline;

#ifdef SPINE_USE_STD_FUNCTION
	typedef std::function<void(AnimationState *state, EventType type, TrackEntry *entry, Event *event)> AnimationStateListener;
#else

	typedef void (*AnimationStateListener)(AnimationState *state, EventType type, TrackEntry *entry, Event *event, void *userData);

#endif

	class SP_API AnimationStateListenerObject {
	public:
		AnimationStateListenerObject() {};

		virtual ~AnimationStateListenerObject() {};

	public:

		virtual void callback(AnimationState *state, EventType type, TrackEntry *entry, Event *event) = 0;
	};

	class SP_API TrackEntry : public SpineObject, public HasRendererObject {
		friend class EventQueue;

		friend class AnimationState;

	public:
		TrackEntry();

		virtual ~TrackEntry();

		int getTrackIndex();

		Animation &getAnimation();

		void setAnimation(Animation &animation);

		TrackEntry *getPrevious();

		bool getLoop();

		void setLoop(bool inValue);

		bool getAdditive();

		void setAdditive(bool inValue);

		bool getReverse();

		void setReverse(bool inValue);

		bool getShortestRotation();

		void setShortestRotation(bool inValue);

		float getDelay();

		void setDelay(float inValue);

		float getTrackTime();

		void setTrackTime(float inValue);

		float getTrackEnd();

		void setTrackEnd(float inValue);

		float getAnimationStart();

		void setAnimationStart(float inValue);

		float getAnimationEnd();

		void setAnimationEnd(float inValue);

		float getAnimationLast();

		void setAnimationLast(float inValue);

		float getAnimationTime();

		float getTimeScale();

		void setTimeScale(float inValue);

		float getAlpha();

		void setAlpha(float inValue);

		float getEventThreshold();

		void setEventThreshold(float inValue);

		float getMixAttachmentThreshold();

		void setMixAttachmentThreshold(float inValue);

		float getAlphaAttachmentThreshold();

		void setAlphaAttachmentThreshold(float inValue);

		float getMixDrawOrderThreshold();

		void setMixDrawOrderThreshold(float inValue);

		TrackEntry *getNext();

		bool isComplete();

		float getMixTime();

		void setMixTime(float inValue);

		float getMixDuration();

		void setMixDuration(float inValue);

		void setMixDuration(float mixDuration, float delay);

		Interpolation &getMixInterpolation();

		void setMixInterpolation(Interpolation &mixInterpolation);

		TrackEntry *getMixingFrom();

		TrackEntry *getMixingTo();

		void resetRotationDirections();

		float getTrackComplete();

#ifdef SPINE_USE_STD_FUNCTION
		void setListener(AnimationStateListener listener);
#else
		void setListener(AnimationStateListener listener, void *userData = NULL);
#endif

		void setListener(AnimationStateListenerObject *listener);

		bool isEmptyAnimation();

		bool wasApplied();

		bool isNextReady() {
			return _next != NULL && _nextTrackLast - _next->_delay >= 0;
		}

		AnimationState *getAnimationState() {
			return _state;
		}

		void setAnimationState(AnimationState *state) {
			_state = state;
		}

	private:
		Animation *_animation;
		TrackEntry *_previous;
		TrackEntry *_next;
		TrackEntry *_mixingFrom;
		TrackEntry *_mixingTo;
		int _trackIndex;

		bool _loop, _additive, _reverse, _shortestRotation, _keepHold;
		float _eventThreshold, _mixAttachmentThreshold, _alphaAttachmentThreshold, _mixDrawOrderThreshold;
		float _animationStart, _animationEnd, _animationLast, _nextAnimationLast;
		float _delay, _trackTime, _trackLast, _nextTrackLast, _trackEnd, _timeScale;
		float _alpha, _mixTime, _mixDuration, _totalAlpha;
		Interpolation *_mixInterpolation;
		Array<int> _timelineMode;
		Array<TrackEntry *> _timelineHoldMix;
		Array<float> _timelinesRotation;
		AnimationStateListener _listener;
#ifndef SPINE_USE_STD_FUNCTION
		void *_listenerUserData;
#endif
		AnimationStateListenerObject *_listenerObject;
		AnimationState *_state;

		float mix();

		void reset();
	};

	class SP_API EventQueueEntry : public SpineObject {
		friend class EventQueue;

	public:
		EventType _type;
		TrackEntry *_entry;
		Event *_event;

		EventQueueEntry(EventType eventType, TrackEntry *trackEntry, Event *event = NULL);
	};

	class SP_API EventQueue : public SpineObject {
		friend class AnimationState;

	private:
		Array<EventQueueEntry> _eventQueueEntries;
		AnimationState &_state;
		bool _drainDisabled;

		static EventQueue *newEventQueue(AnimationState &state);

		static EventQueueEntry newEventQueueEntry(EventType eventType, TrackEntry *entry, Event *event = NULL);

		EventQueue(AnimationState &state);

		~EventQueue();

		void start(TrackEntry *entry);

		void interrupt(TrackEntry *entry);

		void end(TrackEntry *entry);

		void dispose(TrackEntry *entry);

		void complete(TrackEntry *entry);

		void event(TrackEntry *entry, Event *event);

		void drain();
	};

	class SP_API AnimationState : public SpineObject, public HasRendererObject {
		friend class TrackEntry;

		friend class EventQueue;

	public:
		explicit AnimationState(AnimationStateData &data);

		~AnimationState();

		void update(float delta);

		bool apply(Skeleton &skeleton);

		void clearTracks();

		void clearTrack(size_t trackIndex);

		TrackEntry &setAnimation(size_t trackIndex, const String &animationName, bool loop);

		TrackEntry &setAnimation(size_t trackIndex, Animation &animation, bool loop);

		TrackEntry &addAnimation(size_t trackIndex, const String &animationName, bool loop, float delay);

		TrackEntry &addAnimation(size_t trackIndex, Animation &animation, bool loop, float delay);

		TrackEntry &setEmptyAnimation(size_t trackIndex, float mixDuration);

		TrackEntry &addEmptyAnimation(size_t trackIndex, float mixDuration, float delay);

		void setEmptyAnimations(float mixDuration);

		TrackEntry *getTrack(size_t trackIndex);

		AnimationStateData &getData();

		Array<TrackEntry *> &getTracks();

		float getTimeScale();

		void setTimeScale(float inValue);

#ifdef SPINE_USE_STD_FUNCTION
		void setListener(AnimationStateListener listener);
#else
		void setListener(AnimationStateListener listener, void *userData = NULL);
#endif

		void setListener(AnimationStateListenerObject *listener);

		void disableQueue();

		void enableQueue();

		void setManualTrackEntryDisposal(bool inValue);

		bool getManualTrackEntryDisposal();

		void disposeTrackEntry(TrackEntry *entry);

	private:
		static const int Current = 0;
		static const int Setup = 1;
		static const int First = 2;
		static const int Mode = 3;
		static const int Hold = 4;

		static const int AttachSetup = 1;
		static const int AttachRetain = 2;

		AnimationStateData *_data;

		Pool<TrackEntry> _trackEntryPool;
		Array<TrackEntry *> _tracks;
		Array<Event *> _events;
		EventQueue *_queue;

		Map<PropertyId, TrackEntry *> _propertyIDs;
		bool _animationsChanged;

		AnimationStateListener _listener;
#ifndef SPINE_USE_STD_FUNCTION
		void *_listenerUserData;
#endif
		AnimationStateListenerObject *_listenerObject;

		int _unkeyedState;

		float _timeScale;

		bool _manualTrackEntryDisposal;

		static Animation *getEmptyAnimation();

		static void applyRotateTimeline(RotateTimeline *rotateTimeline, Skeleton &skeleton, float time, float alpha, MixFrom from,
										Array<float> &timelinesRotation, size_t i, bool firstFrame);

		void applyAttachmentTimeline(AttachmentTimeline *attachmentTimeline, Skeleton &skeleton, float animationTime, MixFrom from, bool retain);

		bool updateMixingFrom(TrackEntry *to, float delta);

		float applyMixingFrom(TrackEntry *to, Skeleton &skeleton);

		void queueEvents(TrackEntry *entry, float animationTime);

		void eventsReverse(TrackEntry *entry, float animationLast, float animationTime);

		void setTrack(size_t index, TrackEntry *current, bool interrupt);

		void clearNext(TrackEntry *entry);

		TrackEntry *expandToIndex(size_t index);

		TrackEntry *newTrackEntry(size_t trackIndex, Animation *animation, bool loop, TrackEntry *last);

		void animationsChanged();

		void computeHold(TrackEntry *entry, TrackEntry *track);

		int from(TrackEntry *track, Timeline *timeline, Array<PropertyId> &ids);
	};
}

#endif
