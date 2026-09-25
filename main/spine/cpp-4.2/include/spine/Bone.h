
#ifndef Spine_Bone_h
#define Spine_Bone_h

#include <spine/Updatable.h>
#include <spine/SpineObject.h>
#include <spine/Vector.h>
#include <spine/Inherit.h>

namespace spine {
	class BoneData;

	class Skeleton;

	class SP_API Bone : public Updatable {
		friend class AnimationState;

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

	RTTI_DECL

	public:
		static void setYDown(bool inValue);

		static bool isYDown();

		Bone(BoneData &data, Skeleton &skeleton, Bone *parent = NULL);

		virtual void update(Physics physics);

		void updateWorldTransform();

		void
		updateWorldTransform(float x, float y, float rotation, float scaleX, float scaleY, float shearX, float shearY);

		void updateAppliedTransform();

		void setToSetupPose();

		void worldToLocal(float worldX, float worldY, float &outLocalX, float &outLocalY);

        void worldToParent(float worldX, float worldY, float &outParentX, float &outParentY);

		void localToWorld(float localX, float localY, float &outWorldX, float &outWorldY);

        void parentToWorld(float worldX, float worldY, float &outX, float &outY);

		float worldToLocalRotation(float worldRotation);

		float localToWorldRotation(float localRotation);

		void rotateWorld(float degrees);

		float getWorldToLocalRotationX();

		float getWorldToLocalRotationY();

		BoneData &getData();

		Skeleton &getSkeleton();

		Bone *getParent();

		Vector<Bone *> &getChildren();

		float getX();

		void setX(float inValue);

		float getY();

		void setY(float inValue);

		float getRotation();

		void setRotation(float inValue);

		float getScaleX();

		void setScaleX(float inValue);

		float getScaleY();

		void setScaleY(float inValue);

		float getShearX();

		void setShearX(float inValue);

		float getShearY();

		void setShearY(float inValue);

		float getAppliedRotation();

		void setAppliedRotation(float inValue);

		float getAX();

		void setAX(float inValue);

		float getAY();

		void setAY(float inValue);

		float getAScaleX();

		void setAScaleX(float inValue);

		float getAScaleY();

		void setAScaleY(float inValue);

		float getAShearX();

		void setAShearX(float inValue);

		float getAShearY();

		void setAShearY(float inValue);

		float getA();

		void setA(float inValue);

		float getB();

		void setB(float inValue);

		float getC();

		void setC(float inValue);

		float getD();

		void setD(float inValue);

		float getWorldX();

		void setWorldX(float inValue);

		float getWorldY();

		void setWorldY(float inValue);

		float getWorldRotationX();

		float getWorldRotationY();

		float getWorldScaleX();

		float getWorldScaleY();

		bool isActive();

		void setActive(bool inValue);

        Inherit getInherit() { return _inherit; }

        void setInherit(Inherit inValue) { _inherit = inValue; }

	private:
		static bool yDown;

		BoneData &_data;
		Skeleton &_skeleton;
		Bone *_parent;
		Vector<Bone *> _children;
		float _x, _y, _rotation, _scaleX, _scaleY, _shearX, _shearY;
		float _ax, _ay, _arotation, _ascaleX, _ascaleY, _ashearX, _ashearY;
		float _a, _b, _worldX;
		float _c, _d, _worldY;
		bool _sorted;
		bool _active;
        Inherit _inherit;
	};
}

#endif
