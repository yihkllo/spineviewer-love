
#ifndef Spine_Skeleton_h
#define Spine_Skeleton_h

#include <spine/Vector.h>
#include <spine/MathUtil.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/Color.h>

namespace spine {
class SkeletonData;

class Bone;

class Updatable;

class Slot;

class IkConstraint;

class PathConstraint;

class TransformConstraint;

class Skin;

class Attachment;

class SP_API Skeleton : public SpineObject {
	friend class AnimationState;

	friend class SkeletonBounds;

	friend class SkeletonClipping;

	friend class AttachmentTimeline;

	friend class ColorTimeline;

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
	explicit Skeleton(SkeletonData *skeletonData);

	~Skeleton();

	void updateCache();

	void printUpdateCache();

	void updateWorldTransform();

	void setToSetupPose();

	void setBonesToSetupPose();

	void setSlotsToSetupPose();

	Bone *findBone(const String &boneName);

	int findBoneIndex(const String &boneName);

	Slot *findSlot(const String &slotName);

	int findSlotIndex(const String &slotName);

	void setSkin(const String &skinName);

	void setSkin(Skin *newSkin);

	Attachment *getAttachment(const String &slotName, const String &attachmentName);

	Attachment *getAttachment(int slotIndex, const String &attachmentName);

	void setAttachment(const String &slotName, const String &attachmentName);

	IkConstraint *findIkConstraint(const String &constraintName);

	TransformConstraint *findTransformConstraint(const String &constraintName);

	PathConstraint *findPathConstraint(const String &constraintName);

	void update(float delta);

	void getBounds(float &outX, float &outY, float &outWidth, float &outHeight, Vector<float> &outVertexBuffer);

	Bone *getRootBone();

	SkeletonData *getData();

	Vector<Bone *> &getBones();

	Vector<Updatable *> &getUpdateCacheList();

	Vector<Slot *> &getSlots();

	Vector<Slot *> &getDrawOrder();

	Vector<IkConstraint *> &getIkConstraints();

	Vector<PathConstraint *> &getPathConstraints();

	Vector<TransformConstraint *> &getTransformConstraints();

	Skin *getSkin();

	Color &getColor();

	float getTime();

	void setTime(float inValue);

	void setPosition(float x, float y);

	float getX();

	void setX(float inValue);

	float getY();

	void setY(float inValue);

	float getScaleX();

	void setScaleX(float inValue);

	float getScaleY();

	void setScaleY(float inValue);

private:
	SkeletonData *_data;
	Vector<Bone *> _bones;
	Vector<Slot *> _slots;
	Vector<Slot *> _drawOrder;
	Vector<IkConstraint *> _ikConstraints;
	Vector<TransformConstraint *> _transformConstraints;
	Vector<PathConstraint *> _pathConstraints;
	Vector<Updatable *> _updateCache;
	Vector<Bone *> _updateCacheReset;
	Skin *_skin;
	Color _color;
	float _time;
	float _scaleX, _scaleY;
	float _x, _y;

	void sortIkConstraint(IkConstraint *constraint);

	void sortPathConstraint(PathConstraint *constraint);

	void sortTransformConstraint(TransformConstraint *constraint);

	void sortPathConstraintAttachment(Skin *skin, size_t slotIndex, Bone &slotBone);

	void sortPathConstraintAttachment(Attachment *attachment, Bone &slotBone);

	void sortBone(Bone *bone);

	static void sortReset(Vector<Bone *> &bones);
};
}

#endif
