
#ifndef Spine_Animation_h
#define Spine_Animation_h

#include <spine/Vector.h>
#include <spine/HashMap.h>
#include <spine/MixBlend.h>
#include <spine/MixDirection.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/Property.h>

namespace spine {
	class Timeline;

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

	public:
		Animation(const String &name, Vector<Timeline *> &timelines, float duration);

		~Animation();

		void apply(Skeleton &skeleton, float lastTime, float time, bool loop, Vector<Event *> *pEvents, float alpha,
				   MixBlend blend, MixDirection direction);

		const String &getName();

		Vector<Timeline *> &getTimelines();

		bool hasTimeline(Vector<PropertyId> &ids);

		float getDuration();

		void setDuration(float inValue);

		static int search(Vector<float> &values, float target);

		static int search(Vector<float> &values, float target, int step);
	private:
		Vector<Timeline *> _timelines;
		HashMap<PropertyId, bool> _timelineIds;
		float _duration;
		String _name;
	};
}

#endif
