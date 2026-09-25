
#ifndef Spine_IkConstraintData_h
#define Spine_IkConstraintData_h

#include <spine/Vector.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/ConstraintData.h>

namespace spine {
	class BoneData;

	class SP_API IkConstraintData : public ConstraintData {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class IkConstraint;

		friend class Skeleton;

		friend class IkConstraintTimeline;

	public:
		RTTI_DECL

		explicit IkConstraintData(const String &name);

		Vector<BoneData *> &getBones();

		BoneData *getTarget();

		void setTarget(BoneData *inValue);

		int getBendDirection();

		void setBendDirection(int inValue);

		bool getCompress();

		void setCompress(bool inValue);

		bool getStretch();

		void setStretch(bool inValue);

		bool getUniform();

		void setUniform(bool inValue);

		float getMix();

		void setMix(float inValue);

		float getSoftness();

		void setSoftness(float inValue);

	private:
		Vector<BoneData *> _bones;
		BoneData *_target;
		int _bendDirection;
		bool _compress;
		bool _stretch;
		bool _uniform;
		float _mix;
		float _softness;
	};
}

#endif
