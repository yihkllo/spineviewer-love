#ifndef Spine_IkConstraintPose_h
#define Spine_IkConstraintPose_h

#include <spine/Pose.h>
#include <spine/RTTI.h>

namespace spine {

	class SP_API IkConstraintPose : public Pose<IkConstraintPose> {
		friend class IkConstraint;
		friend class IkConstraintTimeline;
		friend class SkeletonJson;
		friend class SkeletonBinary;

	public:
		IkConstraintPose();
		virtual ~IkConstraintPose();

		virtual void set(IkConstraintPose &pose) override;

		float getMix();
		void setMix(float mix);

		float getSoftness();
		void setSoftness(float softness);

		int getBendDirection();
		void setBendDirection(int bendDirection);

		bool getCompress();
		void setCompress(bool compress);

		bool getStretch();
		void setStretch(bool stretch);

	private:
		int _bendDirection;
		bool _compress, _stretch;
		float _mix, _softness;
	};
}

#endif
