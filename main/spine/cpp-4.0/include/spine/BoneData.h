
#ifndef Spine_BoneData_h
#define Spine_BoneData_h

#include <spine/TransformMode.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/Color.h>

namespace spine {
	class SP_API BoneData : public SpineObject {
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

	public:
		BoneData(int index, const String &name, BoneData *parent = NULL);

		int getIndex();

		const String &getName();

		BoneData *getParent();

		float getLength();

		void setLength(float inValue);

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

		TransformMode getTransformMode();

		void setTransformMode(TransformMode inValue);

		bool isSkinRequired();

		void setSkinRequired(bool inValue);

		Color &getColor();

	private:
		const int _index;
		const String _name;
		BoneData *_parent;
		float _length;
		float _x, _y, _rotation, _scaleX, _scaleY, _shearX, _shearY;
		TransformMode _transformMode;
		bool _skinRequired;
		Color _color;
	};
}

#endif
