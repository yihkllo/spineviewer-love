#ifndef Spine_ConstraintData_h
#define Spine_ConstraintData_h

#include <spine/SpineString.h>
#include <spine/SpineObject.h>
#include <spine/PosedData.h>
#include <spine/RTTI.h>
#include <string.h>

namespace spine {
	class Skeleton;
	class Constraint;

	enum ScaleYMode {
		ScaleYMode_None = 0,
		ScaleYMode_Uniform,
		ScaleYMode_Volume
	};

	inline ScaleYMode ScaleYMode_valueOf(const char *value) {
		if (strcmp(value, "uniform") == 0)
			return ScaleYMode_Uniform;
		else if (strcmp(value, "volume") == 0)
			return ScaleYMode_Volume;
		else
			return ScaleYMode_None;
	}

	inline const char *ScaleYMode_toString(ScaleYMode scaleYMode) {
		switch (scaleYMode) {
			case ScaleYMode_Uniform:
				return "uniform";
			case ScaleYMode_Volume:
				return "volume";
			default:
				return "none";
		}
	}

	class SP_API ConstraintData : public SpineObject {
		RTTI_DECL_NOPARENT
		friend class Skeleton;
		friend class Constraint;

	public:
		ConstraintData(const String &name) : SpineObject(name) {
		}
		virtual ~ConstraintData() {
		}

		virtual Constraint &create(Skeleton &skeleton) = 0;

		virtual const String &getName() const = 0;

		virtual bool getSkinRequired() const = 0;
	};

	template<class T, class P>
	class ConstraintDataGeneric : public PosedDataGeneric<P>, public ConstraintData {
	public:
		ConstraintDataGeneric(const String &name) : PosedDataGeneric<P>(name), ConstraintData(name) {
		}
		virtual ~ConstraintDataGeneric() {
		}

		virtual Constraint &create(Skeleton &skeleton) override = 0;

		virtual const String &getName() const override {
			return PosedDataGeneric<P>::getName();
		}
		virtual bool getSkinRequired() const override {
			return PosedDataGeneric<P>::getSkinRequired();
		}
	};
}

#endif
