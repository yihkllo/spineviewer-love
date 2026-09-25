
#ifndef Spine_Animation_h
#define Spine_Animation_h

#include <spine/Vector.h>
#include <spine/HashMap.h>
#include <spine/MixBlend.h>
#include <spine/MixDirection.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>

namespace spine {
class Timeline;

class Skeleton;

class Event;

class SP_API Animation : public SpineObject {
	friend class AnimationState;

	friend class TrackEntry;

	friend class AnimationStateData;

	friend class AttachmentTimeline;

	friend class ColorTimeline;

	friend class DeformTimeline;

	friend class DrawOrderTimeline;

	friend class EventTimeline;

	friend class IkConstraintTimeline;

	friend class PathConstraintMixTimeline;

	friend class PathConstraintPositionTimeline;

	friend class PathConstraintSpacingTimeline;

	friend class RotateTimeline;

	friend class ScaleTimeline;

	friend class ShearTimeline;

	friend class TransformConstraintTimeline;

	friend class TranslateTimeline;

	friend class TwoColorTimeline;

public:
	Animation(const String &name, Vector<Timeline *> &timelines, float duration);

	~Animation();

	void apply(Skeleton &skeleton, float lastTime, float time, bool loop, Vector<Event *> *pEvents, float alpha,
		MixBlend blend, MixDirection direction);

	const String &getName();

	Vector<Timeline *> &getTimelines();

	bool hasTimeline(int id);

	float getDuration();

	void setDuration(float inValue);



private:
	Vector<Timeline *> _timelines;
	HashMap<int, bool> _timelineIds;
	float _duration;
	String _name;

	static int binarySearch(Vector<float> &values, float target, int step);

	static int binarySearch(Vector<float> &values, float target);

	static int linearSearch(Vector<float> &values, float target, int step);
};
}

#endif
