#ifndef Spine_PosedData_h
#define Spine_PosedData_h

#include <spine/SpineObject.h>
#include <spine/SpineString.h>

namespace spine {
	template<class P>
	class Pose;

	class SP_API PosedData : public SpineObject {
		friend class SkeletonBinary;
		friend class SkeletonJson;
		friend class BoneTimeline1;
		friend class BoneTimeline2;
		friend class RotateTimeline;
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
		friend class Slot;

	public:
		PosedData(const String &name);
		virtual ~PosedData();

		const String &getName() const {
			return _name;
		};

		bool getSkinRequired() const {
			return _skinRequired;
		};
		void setSkinRequired(bool skinRequired) {
			_skinRequired = skinRequired;
		};

	protected:
		String _name;
		bool _skinRequired;
	};

	inline PosedData::PosedData(const String &name) : _name(name), _skinRequired(false) {
	}

	inline PosedData::~PosedData() {
	}

	template<class P>
	class PosedDataGeneric : public PosedData {
		friend class SkeletonBinary;
		friend class SkeletonJson;
		friend class BoneTimeline1;
		friend class BoneTimeline2;
		friend class RotateTimeline;
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
		friend class PhysicsConstraintTimeline;
		friend class PhysicsConstraintInertiaTimeline;
		friend class PhysicsConstraintStrengthTimeline;
		friend class PhysicsConstraintDampingTimeline;
		friend class PhysicsConstraintMassTimeline;
		friend class PhysicsConstraintWindTimeline;
		friend class PhysicsConstraintGravityTimeline;
		friend class Slot;

	protected:
		P _setupPose;

	public:
		PosedDataGeneric(const String &name) : PosedData(name), _setupPose() {
		}
		virtual ~PosedDataGeneric() {};

		P &getSetupPose() {
			return _setupPose;
		};
		const P &getSetupPose() const {
			return _setupPose;
		};
	};
}

#endif
