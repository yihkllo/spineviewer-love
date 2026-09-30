#ifndef Spine_Posed_h
#define Spine_Posed_h

#include <spine/SpineObject.h>

namespace spine {

	class SP_API Posed {
	public:
		Posed() {
		}
		virtual ~Posed() {
		}

		virtual void constrained() = 0;

		virtual void resetConstrained() = 0;

		virtual bool isPoseEqualToApplied() = 0;

	protected:
		virtual void setupPose() = 0;

		virtual void unconstrained() = 0;
	};

	template<class D, class P, class A>
	class PosedGeneric : public Posed, public SpineObject {
		friend class AnimationState;
		friend class BoneTimeline1;
		friend class BoneTimeline2;
		friend class RotateTimeline;
		friend class IkConstraint;
		friend class TransformConstraint;
		friend class VertexAttachment;
		friend class PathConstraint;
		friend class PhysicsConstraint;
		friend class Skeleton;
		friend class RegionAttachment;
		friend class PointAttachment;
		friend class AttachmentTimeline;
		friend class RGBATimeline;
		friend class RGBTimeline;
		friend class AlphaTimeline;
		friend class RGBA2Timeline;
		friend class RGB2Timeline;
		friend class ScaleTimeline;
		friend class ScaleXTimeline;
		friend class ScaleYTimeline;
		friend class ShearTimeline;
		friend class ShearXTimeline;
		friend class ShearYTimeline;
		friend class TranslateTimeline;
		friend class TranslateXTimeline;
		friend class TranslateYTimeline;
		friend class InheritTimeline;
		friend class Skeleton;

	public:
		PosedGeneric(D &data) : _data(data), _pose(), _constrainedPose(), _appliedPose(&_pose) {
			setupPose();
		}

		virtual ~PosedGeneric() {
		}

		D &getData() {
			return _data;
		}

		P &getPose() {
			return _pose;
		}

		A &getAppliedPose() {
			return *_appliedPose;
		}

		virtual void resetConstrained() override {
			_constrainedPose.set(_pose);
		}

		virtual void constrained() override {
			_appliedPose = &_constrainedPose;
		}

		virtual bool isPoseEqualToApplied() override {
			return _appliedPose == &_pose;
		}

	protected:

		virtual void unconstrained() override {
			_appliedPose = &_pose;
		}

		virtual void setupPose() override {
			_pose.set(_data.getSetupPose());
		}

	protected:
		D &_data;
		A _pose;
		A _constrainedPose;
		A *_appliedPose;
	};
}

#endif
