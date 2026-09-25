
#ifndef Spine_AnimationState_h
#define Spine_AnimationState_h

#include <spine/Vector.h>
#include <spine/Pool.h>
#include <spine/Property.h>
#include <spine/MixBlend.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/HasRendererObject.h>
#include "Slot.h"

#ifdef SPINE_USE_STD_FUNCTION
#include <functional>
#endif

namespace spine {
	enum EventType {
		EventType_Start = 0,
		EventType_Interrupt,
		EventType_End,
		EventType_Complete,
		EventType_Dispose,
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
	typedef std::function<void (AnimationState* state, EventType type, TrackEntry* entry, Event* event)> AnimationStateListener;
#else

	typedef void (*AnimationStateListener)(AnimationState *state, EventType type, TrackEntry *entry, Event *event);

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

		Animation *getAnimation();

		TrackEntry *getPrevious();

		bool getLoop();

		void setLoop(bool inValue);

		bool getHoldPrevious();

		void setHoldPrevious(bool inValue);

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

		MixBlend getMixBlend();

		void setMixBlend(MixBlend blend);

		TrackEntry *getMixingFrom();

		TrackEntry *getMixingTo();

		void resetRotationDirections();

		float getTrackComplete();

		void setListener(AnimationStateListener listener);

		void setListener(AnimationStateListenerObject *listener);

        bool wasApplied();

        bool isNextReady () {
            return _next != NULL && _nextTrackLast - _next->_delay >= 0;
        }

	private:
		Animation *_animation;
		TrackEntry *_previous;
		TrackEntry *_next;
		TrackEntry *_mixingFrom;
		TrackEntry *_mixingTo;
		int _trackIndex;

		bool _loop, _holdPrevious, _reverse, _shortestRotation;
		float _eventThreshold, _mixAttachmentThreshold, _alphaAttachmentThreshold, _mixDrawOrderThreshold;
		float _animationStart, _animationEnd, _animationLast, _nextAnimationLast;
		float _delay, _trackTime, _trackLast, _nextTrackLast, _trackEnd, _timeScale;
		float _alpha, _mixTime, _mixDuration, _interruptAlpha, _totalAlpha;
		MixBlend _mixBlend;
		Vector<int> _timelineMode;
		Vector<TrackEntry *> _timelineHoldMix;
		Vector<float> _timelinesRotation;
		AnimationStateListener _listener;
		AnimationStateListenerObject *_listenerObject;

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
		Vector<EventQueueEntry> _eventQueueEntries;
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
		explicit AnimationState(AnimationStateData *data);

		~AnimationState();

		void update(float delta);

		bool apply(Skeleton &skeleton);

		void clearTracks();

		void clearTrack(size_t trackIndex);

		TrackEntry *setAnimation(size_t trackIndex, const String &animationName, bool loop);

		TrackEntry *setAnimation(size_t trackIndex, Animation *animation, bool loop);

		TrackEntry *addAnimation(size_t trackIndex, const String &animationName, bool loop, float delay);

		TrackEntry *addAnimation(size_t trackIndex, Animation *animation, bool loop, float delay);

		TrackEntry *setEmptyAnimation(size_t trackIndex, float mixDuration);

		TrackEntry *addEmptyAnimation(size_t trackIndex, float mixDuration, float delay);

		void setEmptyAnimations(float mixDuration);

		TrackEntry *getCurrent(size_t trackIndex);

		AnimationStateData *getData();

		Vector<TrackEntry *> &getTracks();

		float getTimeScale();

		void setTimeScale(float inValue);

		void setListener(AnimationStateListener listener);

		void setListener(AnimationStateListenerObject *listener);

		void disableQueue();

		void enableQueue();

		void setManualTrackEntryDisposal(bool inValue);

        bool getManualTrackEntryDisposal();

		void disposeTrackEntry(TrackEntry *entry);

	private:
		static const int Subsequent = 0;
		static const int First = 1;
		static const int HoldSubsequent = 2;
		static const int HoldFirst = 3;
		static const int HoldMix = 4;

		static const int Setup = 1;
		static const int Current = 2;

		AnimationStateData *_data;

		Pool<TrackEntry> _trackEntryPool;
		Vector<TrackEntry *> _tracks;
		Vector<Event *> _events;
		EventQueue *_queue;

		HashMap<PropertyId, bool> _propertyIDs;
		bool _animationsChanged;

		AnimationStateListener _listener;
		AnimationStateListenerObject *_listenerObject;

		int _unkeyedState;

		float _timeScale;

		bool _manualTrackEntryDisposal;

		static Animation *getEmptyAnimation();

		static void
		applyRotateTimeline(RotateTimeline *rotateTimeline, Skeleton &skeleton, float time, float alpha, MixBlend pose,
							Vector<float> &timelinesRotation, size_t i, bool firstFrame);

		void applyAttachmentTimeline(AttachmentTimeline *attachmentTimeline, Skeleton &skeleton, float animationTime,
									 MixBlend pose, bool firstFrame);

		bool updateMixingFrom(TrackEntry *to, float delta);

		float applyMixingFrom(TrackEntry *to, Skeleton &skeleton, MixBlend currentPose);

		void queueEvents(TrackEntry *entry, float animationTime);

		void setCurrent(size_t index, TrackEntry *current, bool interrupt);

		void clearNext(TrackEntry *entry);

		TrackEntry *expandToIndex(size_t index);

		TrackEntry *newTrackEntry(size_t trackIndex, Animation *animation, bool loop, TrackEntry *last);

		void animationsChanged();

		void computeHold(TrackEntry *entry);

		void setAttachment(Skeleton &skeleton, spine::Slot &slot, const String &attachmentName, bool attachments);
	};
}

#endif
