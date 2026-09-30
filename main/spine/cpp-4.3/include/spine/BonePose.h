#ifndef SPINE_BONEPOSE_H_
#define SPINE_BONEPOSE_H_

#include <spine/BoneLocal.h>
#include <spine/Update.h>
#include <spine/RTTI.h>

namespace spine {
	class Bone;
	class Skeleton;

	class SP_API BonePose : public BoneLocal, public Update {
		friend class IkConstraint;
		friend class PathConstraint;
		friend class PhysicsConstraint;
		friend class TransformConstraint;
		friend class TransformConstraintData;
		friend class FromRotate;
		friend class ToRotate;
		friend class FromX;
		friend class ToX;
		friend class FromY;
		friend class ToY;
		friend class FromScaleX;
		friend class ToScaleX;
		friend class FromScaleY;
		friend class ToScaleY;
		friend class FromShearX;
		friend class ToShearX;
		friend class FromShearY;
		friend class ToShearY;
		friend class Skeleton;
		friend class Bone;

		RTTI_DECL

	public:
		BonePose();
		virtual ~BonePose();

		virtual void update(Skeleton &skeleton, Physics physics) override;

		void updateWorldTransform(Skeleton &skeleton);

		void updateLocalTransform(Skeleton &skeleton);

		void validateLocalTransform(Skeleton &skeleton);

		void modifyLocal(Skeleton &skeleton);
		void modifyWorld(Skeleton &skeleton);

		float getA();
		void setA(float a);

		float getB();
		void setB(float b);

		float getC();
		void setC(float c);

		float getD();
		void setD(float d);

		float getWorldX();
		void setWorldX(float worldX);

		float getWorldY();
		void setWorldY(float worldY);

		float getWorldRotationX();

		float getWorldRotationY();

		float getWorldScaleX();

		float getWorldScaleY();

		void worldToLocal(float worldX, float worldY, float &outLocalX, float &outLocalY);

		void localToWorld(float localX, float localY, float &outWorldX, float &outWorldY);

		void worldToParent(float worldX, float worldY, float &outParentX, float &outParentY);

		void parentToWorld(float parentX, float parentY, float &outWorldX, float &outWorldY);

		float worldToLocalRotation(float worldRotation);

		float localToWorldRotation(float localRotation);

		void rotateWorld(float degrees);

	private:
		void setLocal(float ra, float rb, float rc, float rd);
		void setLocal(float ra, float rb, float rc, float rd, float ro);
		void resetWorld(Skeleton &skeleton, int update);

	protected:
		Bone *_bone;
		float _a, _b, _worldX;
		float _c, _d, _worldY;
		int _world, _local;
	};
}

#endif
