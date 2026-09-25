
#ifndef Spine_InheritTimeline_h
#define Spine_InheritTimeline_h

#include <spine/Timeline.h>

#include <spine/Animation.h>
#include <spine/Property.h>
#include <spine/Inherit.h>

namespace spine {

	class SP_API InheritTimeline : public Timeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit InheritTimeline(size_t frameCount, int boneIndex);

		virtual ~InheritTimeline();

        void setFrame(int frame, float time, Inherit inherit);

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		int getBoneIndex() { return _boneIndex; }

		void setBoneIndex(int inValue) { _boneIndex = inValue; }

	private:
		int _boneIndex;

        static const int ENTRIES = 2;
        static const int INHERIT = 1;
	};
}

#endif
