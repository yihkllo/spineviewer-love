#ifndef Spine_TransformConstraintData_h
#define Spine_TransformConstraintData_h

#include <spine/ConstraintData.h>
#include <spine/PosedData.h>
#include <spine/Array.h>
#include <spine/TransformConstraintPose.h>

namespace spine {
	class BoneData;
	class TransformConstraint;
	class BonePose;
	class TransformConstraintPose;

	class FromProperty;
	class ToProperty;

	class SP_API FromProperty : public SpineObject {
		friend class SkeletonBinary;

	public:
		RTTI_DECL_NOPARENT

		float _offset;

		Array<ToProperty *> _to;

		FromProperty();
		virtual ~FromProperty();

		virtual float value(Skeleton &skeleton, BonePose &source, bool local, float *offsets) = 0;
	};

	class SP_API ToProperty : public SpineObject {
		friend class SkeletonBinary;

	public:
		RTTI_DECL_NOPARENT

		float _offset;

		float _max;

		float _scale;

		ToProperty();
		virtual ~ToProperty();

		virtual float mix(TransformConstraintPose &pose) = 0;

		virtual void apply(Skeleton &skeleton, TransformConstraintPose &pose, BonePose &bone, float value, bool local, bool additive) = 0;
	};

	class SP_API FromRotate : public FromProperty {
	public:
		RTTI_DECL

		FromRotate() : FromProperty() {
		}
		~FromRotate() {
		}

		float value(Skeleton &skeleton, BonePose &source, bool local, float *offsets) override;
	};

	class SP_API ToRotate : public ToProperty {
	public:
		RTTI_DECL

		ToRotate() : ToProperty() {
		}
		~ToRotate() {
		}

		float mix(TransformConstraintPose &pose) override;
		void apply(Skeleton &skeleton, TransformConstraintPose &pose, BonePose &bone, float value, bool local, bool additive) override;
	};

	class SP_API FromX : public FromProperty {
	public:
		RTTI_DECL

		FromX() : FromProperty() {
		}
		~FromX() {
		}

		float value(Skeleton &skeleton, BonePose &source, bool local, float *offsets) override;
	};

	class SP_API ToX : public ToProperty {
	public:
		RTTI_DECL

		ToX() : ToProperty() {
		}
		~ToX() {
		}

		float mix(TransformConstraintPose &pose) override;
		void apply(Skeleton &skeleton, TransformConstraintPose &pose, BonePose &bone, float value, bool local, bool additive) override;
	};

	class SP_API FromY : public FromProperty {
	public:
		RTTI_DECL

		FromY() : FromProperty() {
		}
		~FromY() {
		}

		float value(Skeleton &skeleton, BonePose &source, bool local, float *offsets) override;
	};

	class SP_API ToY : public ToProperty {
	public:
		RTTI_DECL

		ToY() : ToProperty() {
		}
		~ToY() {
		}

		float mix(TransformConstraintPose &pose) override;
		void apply(Skeleton &skeleton, TransformConstraintPose &pose, BonePose &bone, float value, bool local, bool additive) override;
	};

	class SP_API FromScaleX : public FromProperty {
	public:
		RTTI_DECL

		FromScaleX() : FromProperty() {
		}
		~FromScaleX() {
		}

		float value(Skeleton &skeleton, BonePose &source, bool local, float *offsets) override;
	};

	class SP_API ToScaleX : public ToProperty {
	public:
		RTTI_DECL

		ToScaleX() : ToProperty() {
		}
		~ToScaleX() {
		}

		float mix(TransformConstraintPose &pose) override;
		void apply(Skeleton &skeleton, TransformConstraintPose &pose, BonePose &bone, float value, bool local, bool additive) override;
	};

	class SP_API FromScaleY : public FromProperty {
	public:
		RTTI_DECL

		FromScaleY() : FromProperty() {
		}
		~FromScaleY() {
		}

		float value(Skeleton &skeleton, BonePose &source, bool local, float *offsets) override;
	};

	class SP_API ToScaleY : public ToProperty {
	public:
		RTTI_DECL

		ToScaleY() : ToProperty() {
		}
		~ToScaleY() {
		}

		float mix(TransformConstraintPose &pose) override;
		void apply(Skeleton &skeleton, TransformConstraintPose &pose, BonePose &bone, float value, bool local, bool additive) override;
	};

	class SP_API FromShearY : public FromProperty {
	public:
		RTTI_DECL

		FromShearY() : FromProperty() {
		}
		~FromShearY() {
		}

		float value(Skeleton &skeleton, BonePose &source, bool local, float *offsets) override;
	};

	class SP_API ToShearY : public ToProperty {
	public:
		RTTI_DECL

		ToShearY() : ToProperty() {
		}
		~ToShearY() {
		}

		float mix(TransformConstraintPose &pose) override;
		void apply(Skeleton &skeleton, TransformConstraintPose &pose, BonePose &bone, float value, bool local, bool additive) override;
	};

	class SP_API TransformConstraintData : public ConstraintDataGeneric<TransformConstraint, TransformConstraintPose> {
	public:
		RTTI_DECL
		static const int ROTATION;
		static const int X;
		static const int Y;
		static const int SCALEX;
		static const int SCALEY;
		static const int SHEARY;
		friend class SkeletonBinary;
		friend class SkeletonJson;
		friend class TransformConstraint;
		friend class Skeleton;
		friend class TransformConstraintTimeline;

	public:
		explicit TransformConstraintData(const String &name);
		~TransformConstraintData();

		virtual Constraint &create(Skeleton &skeleton) override;

		Array<BoneData *> &getBones();

		BoneData &getSource();
		void setSource(BoneData &source);

		float getOffsetRotation();
		void setOffsetRotation(float offsetRotation);

		float getOffsetX();
		void setOffsetX(float offsetX);

		float getOffsetY();
		void setOffsetY(float offsetY);

		float getOffsetScaleX();
		void setOffsetScaleX(float offsetScaleX);

		float getOffsetScaleY();
		void setOffsetScaleY(float offsetScaleY);

		float getOffsetShearY();
		void setOffsetShearY(float offsetShearY);

		bool getLocalSource();
		void setLocalSource(bool localSource);

		bool getLocalTarget();
		void setLocalTarget(bool localTarget);

		bool getAdditive();
		void setAdditive(bool additive);

		bool getClamp();
		void setClamp(bool clamp);

		Array<FromProperty *> &getProperties();

	private:
		Array<BoneData *> _bones;
		BoneData *_source;
		float _offsets[6];
		bool _localSource, _localTarget, _additive, _clamp;
		Array<FromProperty *> _properties;
	};
}

#endif
