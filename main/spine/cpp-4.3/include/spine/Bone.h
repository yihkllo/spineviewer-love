#ifndef Spine_Bone_h
#define Spine_Bone_h

#include <spine/Posed.h>
#include <spine/PosedActive.h>
#include <spine/BoneData.h>
#include <spine/BonePose.h>
#include <spine/Array.h>

namespace spine {

	class SP_API Bone : public PosedGeneric<BoneData, BonePose, BonePose>, public PosedActive, public Update {
		friend class AnimationState;
		friend class RotateTimeline;
		friend class IkConstraint;
		friend class TransformConstraint;
		friend class VertexAttachment;
		friend class PathConstraint;
		friend class PhysicsConstraint;
		friend class Skeleton;
		friend class Slider;
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

		RTTI_DECL

	public:

		Bone(BoneData &data, Bone *parent);

		Bone(Bone &bone, Bone *parent);

		Bone *getParent();

		Array<Bone *> &getChildren();

		static bool isYDown() {
			return yDown;
		}
		static void setYDown(bool value) {
			yDown = value;
		}

		virtual void update(Skeleton &skeleton, Physics physics) override {

		}

	private:
		static bool yDown;
		Bone *const _parent;
		Array<Bone *> _children;
		bool _sorted;
	};
}

#endif
