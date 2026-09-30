#ifndef Spine_BoneData_h
#define Spine_BoneData_h

#include <spine/PosedData.h>
#include <spine/BonePose.h>
#include <spine/SpineString.h>
#include <spine/Color.h>
#include <spine/RTTI.h>

namespace spine {
	class SP_API BoneData : public PosedDataGeneric<BonePose> {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class AnimationState;

		friend class RotateTimeline;

		friend class ScaleTimeline;

		friend class ScaleXTimeline;

		friend class ScaleYTimeline;

		friend class ShearTimeline;

		friend class ShearXTimeline;

		friend class ShearYTimeline;

		friend class TranslateTimeline;

		friend class TranslateXTimeline;

		friend class TranslateYTimeline;

		friend class Slot;

	public:
		BoneData(int index, const String &name, BoneData *parent = NULL);

		int getIndex();

		BoneData *getParent();

		float getLength();

		void setLength(float inValue);

		Color &getColor();

		const String &getIcon();

		void setIcon(const String &icon);

		float getIconSize();

		void setIconSize(float iconSize);

		float getIconRotation();

		void setIconRotation(float iconRotation);

		bool getVisible();

		void setVisible(bool inValue);

	private:
		const int _index;
		BoneData *_parent;
		float _length;
		Color _color;
		String _icon;
		float _iconSize;
		float _iconRotation;
		bool _visible;
	};
}

#endif
