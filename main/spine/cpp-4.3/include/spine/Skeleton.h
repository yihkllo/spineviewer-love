#ifndef Spine_Skeleton_h
#define Spine_Skeleton_h

#include <spine/Array.h>
#include <spine/MathUtil.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/Color.h>
#include <spine/Physics.h>
#include <spine/Update.h>
#include <spine/Posed.h>
#include <spine/Constraint.h>
#include <spine/DrawOrder.h>

namespace spine {
	class SkeletonData;

	class Bone;

	class BonePose;

	class Updatable;

	class Slot;

	class DrawOrder;

	class IkConstraint;

	class PathConstraint;

	class PhysicsConstraint;

	class TransformConstraint;

	class Skin;

	class Attachment;

	class SkeletonClipping;

	class SP_API Skeleton : public SpineObject {
		friend class AnimationState;

		friend class SkeletonBounds;

		friend class SkeletonClipping;

		friend class SlotCurveTimeline;

		friend class AttachmentTimeline;

		friend class RGBATimeline;

		friend class RGBTimeline;

		friend class AlphaTimeline;

		friend class RGBA2Timeline;

		friend class RGB2Timeline;

		friend class DeformTimeline;

		friend class DrawOrderFolderTimeline;

		friend class DrawOrderTimeline;

		friend class EventTimeline;

		friend class IkConstraintTimeline;

		friend class InheritTimeline;

		friend class PathConstraint;

		friend class PathConstraintMixTimeline;

		friend class PathConstraintPositionTimeline;

		friend class PathConstraintSpacingTimeline;

		friend class SliderTimeline;

		friend class SliderMixTimeline;

		friend class ScaleTimeline;

		friend class ScaleXTimeline;

		friend class ScaleYTimeline;

		friend class ShearTimeline;

		friend class ShearXTimeline;

		friend class ShearYTimeline;

		friend class TransformConstraintTimeline;

		friend class BoneTimeline1;

		friend class BoneTimeline2;

		friend class RotateTimeline;

		friend class TranslateTimeline;

		friend class TranslateXTimeline;

		friend class TranslateYTimeline;

		friend class TwoColorTimeline;

		friend class PhysicsConstraint;

		friend class BonePose;

		friend class IkConstraint;

		friend class PathConstraint;

		friend class PhysicsConstraint;

		friend class TransformConstraint;

		friend class Slider;

	public:
		explicit Skeleton(SkeletonData &skeletonData);

		~Skeleton();

		void updateCache();

		void printUpdateCache();

		void constrained(Posed &object);

		void sortBone(Bone *bone);

		static void sortReset(Array<Bone *> &bones);

		void updateWorldTransform(Physics physics);

		void setupPose();

		void setupPoseBones();

		void setupPoseSlots();

		SkeletonData &getData();

		Array<Bone *> &getBones();

		Array<Update *> &getUpdateCache();

		Bone *getRootBone();

		Bone *findBone(const String &boneName);

		Array<Slot *> &getSlots();

		Slot *findSlot(const String &slotName);

		DrawOrder &getDrawOrder();

		Skin *getSkin();

		void setSkin(const String &skinName);

		void setSkin(Skin *newSkin);

		Attachment *getAttachment(const String &slotName, const String &placeholder);

		Attachment *getAttachment(int slotIndex, const String &placeholder);

		void setAttachment(const String &slotName, const String &placeholder);

		Array<Constraint *> &getConstraints();

		Array<PhysicsConstraint *> &getPhysicsConstraints();

		template<class T>
		T *findConstraint(const String &constraintName) {
			if (constraintName.isEmpty()) return NULL;
			for (size_t i = 0; i < _constraints.size(); i++) {
				Constraint *constraint = _constraints[i];
				if (constraint->getRTTI().isExactly(T::rtti)) {
					if (constraint->getData().getName() == constraintName) {
						return (T *) constraint;
					}
				}
			}
			return NULL;
		}

		void getBounds(float &outX, float &outY, float &outWidth, float &outHeight);

		void getBounds(float &outX, float &outY, float &outWidth, float &outHeight, Array<float> &outVertexBuffer, SkeletonClipping *clipping);

		Color &getColor();

		void setColor(Color &color);

		void setColor(float r, float g, float b, float a);

		float getScaleX();

		void setScaleX(float inValue);

		float getScaleY();

		void setScaleY(float inValue);

		void setScale(float scaleX, float scaleY);

		float getX();

		void setX(float inValue);

		float getY();

		void setY(float inValue);

		void setPosition(float x, float y);

		void getPosition(float &x, float &y);

		float getWindX();

		void setWindX(float windX);

		float getWindY();

		void setWindY(float windY);

		float getGravityX();

		void setGravityX(float gravityX);

		float getGravityY();

		void setGravityY(float gravityY);

		void physicsTranslate(float x, float y);

		void physicsRotate(float x, float y, float degrees);

		float getTime();

		void setTime(float time);

		void update(float delta);

	protected:
		SkeletonData &_data;
		Array<Bone *> _bones;
		Array<Slot *> _slots;
		DrawOrder _drawOrder;
		Array<Constraint *> _constraints;
		Array<PhysicsConstraint *> _physics;
		Array<Update *> _updateCache;
		Array<Posed *> _resetCache;
		Skin *_skin;
		Color _color;
		float _x, _y;
		float _scaleX, _scaleY;
		float _windX, _windY, _gravityX, _gravityY;
		float _time;
		int _update;
	};
}

#endif
