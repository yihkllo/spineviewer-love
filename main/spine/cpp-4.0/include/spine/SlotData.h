
#ifndef Spine_SlotData_h
#define Spine_SlotData_h

#include <spine/BlendMode.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/Color.h>

namespace spine {
	class BoneData;

	class SP_API SlotData : public SpineObject {
		friend class SkeletonBinary;

		friend class SkeletonJson;

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

	public:
		SlotData(int index, const String &name, BoneData &boneData);

		int getIndex();

		const String &getName();

		BoneData &getBoneData();

		Color &getColor();

		Color &getDarkColor();

		bool hasDarkColor();

		void setHasDarkColor(bool inValue);

		const String &getAttachmentName();

		void setAttachmentName(const String &inValue);

		BlendMode getBlendMode();

		void setBlendMode(BlendMode inValue);

	private:
		const int _index;
		String _name;
		BoneData &_boneData;
		Color _color;
		Color _darkColor;

		bool _hasDarkColor;
		String _attachmentName;
		BlendMode _blendMode;
	};
}

#endif
