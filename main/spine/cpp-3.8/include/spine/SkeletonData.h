
#ifndef Spine_SkeletonData_h
#define Spine_SkeletonData_h

#include <spine/Vector.h>
#include <spine/SpineString.h>

namespace spine {
class BoneData;

class SlotData;

class Skin;

class EventData;

class Animation;

class IkConstraintData;

class TransformConstraintData;

class PathConstraintData;

class SP_API SkeletonData : public SpineObject {
	friend class SkeletonBinary;

	friend class SkeletonJson;

	friend class Skeleton;

public:
	SkeletonData();

	~SkeletonData();

	BoneData *findBone(const String &boneName);

	int findBoneIndex(const String &boneName);

	SlotData *findSlot(const String &slotName);

	int findSlotIndex(const String &slotName);

	Skin *findSkin(const String &skinName);

	spine::EventData *findEvent(const String &eventDataName);

	Animation *findAnimation(const String &animationName);

	IkConstraintData *findIkConstraint(const String &constraintName);

	TransformConstraintData *findTransformConstraint(const String &constraintName);

	PathConstraintData *findPathConstraint(const String &constraintName);

	int findPathConstraintIndex(const String &pathConstraintName);

	const String &getName();

	void setName(const String &inValue);

	Vector<BoneData *> &getBones();

	Vector<SlotData *> &getSlots();

	Vector<Skin *> &getSkins();

	Skin *getDefaultSkin();

	void setDefaultSkin(Skin *inValue);

	Vector<spine::EventData *> &getEvents();

	Vector<Animation *> &getAnimations();

	Vector<IkConstraintData *> &getIkConstraints();

	Vector<TransformConstraintData *> &getTransformConstraints();

	Vector<PathConstraintData *> &getPathConstraints();

	float getX();

	void setX(float inValue);

	float getY();

	void setY(float inValue);

	float getWidth();

	void setWidth(float inValue);

	float getHeight();

	void setHeight(float inValue);

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
	Vector<BoneData *> _bones;
	Vector<SlotData *> _slots;
	Vector<Skin *> _skins;
	Skin *_defaultSkin;
	Vector<EventData *> _events;
	Vector<Animation *> _animations;
	Vector<IkConstraintData *> _ikConstraints;
	Vector<TransformConstraintData *> _transformConstraints;
	Vector<PathConstraintData *> _pathConstraints;
	float _x, _y, _width, _height;
	String _version;
	String _hash;
	Vector<char*> _strings;

	float _fps;
	String _imagesPath;
	String _audioPath;
};
}

#endif
