#ifndef Spine_DrawOrder_h
#define Spine_DrawOrder_h

#include <spine/Array.h>
#include <spine/SpineObject.h>

namespace spine {
	class Slot;

	class SP_API DrawOrder : public SpineObject {
		friend class Skeleton;

		friend class DrawOrderTimeline;

		friend class DrawOrderFolderTimeline;

		friend class Slider;

	public:
		explicit DrawOrder(Array<Slot *> &setupPose);

		void setupPose();

		Array<Slot *> &getPose();

		Array<Slot *> &getAppliedPose();

	private:

		void unconstrained();

		void constrained();

		void reset();

		Array<Slot *> &_setupPose;
		Array<Slot *> _pose;
		Array<Slot *> _constrainedPose;
		Array<Slot *> *_appliedPose;
	};
}

#endif
