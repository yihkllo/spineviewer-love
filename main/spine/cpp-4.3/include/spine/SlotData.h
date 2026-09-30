#ifndef Spine_SlotData_h
#define Spine_SlotData_h

#include <spine/BlendMode.h>
#include <spine/PosedData.h>
#include <spine/SpineString.h>
#include <spine/RTTI.h>
#include <spine/SlotPose.h>

namespace spine {
	class BoneData;

	class SP_API SlotData : public PosedDataGeneric<SlotPose> {
		friend class SkeletonBinary;
		friend class SkeletonJson;
		friend class PathConstraint;
		friend class AttachmentTimeline;
		friend class RGBATimeline;
		friend class RGBTimeline;
		friend class AlphaTimeline;
		friend class RGBA2Timeline;
		friend class RGB2Timeline;
		friend class DeformTimeline;
		friend class DrawOrderTimeline;
		friend class EventTimeline;
		friend class IkConstraintTimeline;
		friend class PathConstraintMixTimeline;
		friend class PathConstraintPositionTimeline;
		friend class PathConstraintSpacingTimeline;
		friend class ScaleTimeline;
		friend class ShearTimeline;
		friend class TransformConstraintTimeline;
		friend class TranslateTimeline;
		friend class TwoColorTimeline;
		friend class Slot;

	public:
		SlotData(int index, const String &name, BoneData &boneData);

		int getIndex();

		BoneData &getBoneData();

		void setAttachmentName(const String &attachmentName);

		const String &getAttachmentName();

		BlendMode getBlendMode();
		void setBlendMode(BlendMode blendMode);

		bool getVisible();
		void setVisible(bool visible);

	private:
		const int _index;
		BoneData &_boneData;
		String _attachmentName;
		BlendMode _blendMode;
		bool _visible;
	};
}

#endif
