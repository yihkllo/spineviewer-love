#ifndef Spine_SkeletonData_h
#define Spine_SkeletonData_h

#include <spine/Array.h>
#include <spine/SpineString.h>
#include <spine/ConstraintData.h>

namespace spine {
	class BoneData;

	class SlotData;

	class Skin;

	class EventData;

	class Animation;

	class IkConstraintData;

	class TransformConstraintData;

	class PathConstraintData;

	class PhysicsConstraintData;

	class ConstraintData;

	class SP_API SkeletonData : public SpineObject {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class Skeleton;

	public:
		SkeletonData();

		~SkeletonData();

		BoneData *findBone(const String &boneName);

		SlotData *findSlot(const String &slotName);

		Skin *findSkin(const String &skinName);

		EventData *findEvent(const String &eventDataName);

		Animation *findAnimation(const String &animationName);

		Array<Animation *> &findSliderAnimations(Array<Animation *> &animations);

		const String &getName();

		void setName(const String &inValue);

		Array<BoneData *> &getBones();

		Array<SlotData *> &getSlots();

		Array<Skin *> &getSkins();

		Skin *getDefaultSkin();

		void setDefaultSkin(Skin *inValue);

		Array<EventData *> &getEvents();

		Array<Animation *> &getAnimations();

		Array<ConstraintData *> &getConstraints();

		template<class T>
		T *findConstraint(const String &constraintName) {
			getConstraints();
			for (size_t i = 0, n = _constraints.size(); i < n; i++) {
				ConstraintData *constraint = _constraints[i];
				if (constraint->getName() == constraintName && constraint->getRTTI().instanceOf(T::rtti)) {
					return static_cast<T *>(constraint);
				}
			}
			return NULL;
		}

		float getX();

		void setX(float inValue);

		float getY();

		void setY(float inValue);

		float getWidth();

		void setWidth(float inValue);

		float getHeight();

		void setHeight(float inValue);

		float getReferenceScale();

		void setReferenceScale(float inValue);

		const String &getVersion();

		void setVersion(const String &inValue);

		const String &getHash();

		void setHash(const String &inValue);

		const String &getImagesPath();

		void setImagesPath(const String &inValue);

		const String &getAudioPath();

		void setAudioPath(const String &inValue);

		float getFps();

		void setFps(float inValue);

	private:
		String _name;
		Array<BoneData *> _bones;
		Array<SlotData *> _slots;
		Array<Skin *> _skins;
		Skin *_defaultSkin;
		Array<EventData *> _events;
		Array<Animation *> _animations;
		Array<ConstraintData *> _constraints;
		float _x, _y, _width, _height;
		float _referenceScale;
		String _version;
		String _hash;
		Array<char *> _strings;

		float _fps;
		String _imagesPath;
		String _audioPath;
	};
}

#endif
