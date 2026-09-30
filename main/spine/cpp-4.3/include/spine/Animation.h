#ifndef Spine_Animation_h
#define Spine_Animation_h

#include <spine/Array.h>
#include <spine/Color.h>
#include <spine/Map.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/Property.h>
#include <spine/Timeline.h>

namespace spine {
	class Timeline;
	class BoneTimeline;

	class Skeleton;

	class Event;

	class AnimationState;

	class SP_API Animation : public SpineObject {
		friend class AnimationState;

		friend class TrackEntry;

		friend class AnimationStateData;

		friend class AttachmentTimeline;

		friend class RGBATimeline;

		friend class RGBTimeline;

		friend class AlphaTimeline;

		friend class RGBA2Timeline;

		friend class RGB2Timeline;

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

		friend class TranslateXTimeline;

		friend class TranslateYTimeline;

		friend class TwoColorTimeline;

		friend class Slider;

	public:

		Animation(const String &name);

		~Animation();

		Array<Timeline *> &getTimelines();

		void setTimelines(Array<Timeline *> &timelines, Array<int> &bones);

		bool hasTimeline(Array<PropertyId> &ids);

		float getDuration();

		void setDuration(float inValue);

		void apply(Skeleton &skeleton, float lastTime, float time, bool loop, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
				   bool appliedPose);

		const String &getName();

		const Array<int> &getBones();

		Color &getColor();

		static int search(Array<float> &values, float target);

		static int search(Array<float> &values, float target, int step);

	protected:
		Array<Timeline *> _timelines;
		Map<PropertyId, bool> _timelineIds;
		Array<int> _bones;
		float _duration;
		String _name;
		Color _color;
	};
}

#endif
