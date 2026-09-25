
#ifndef Spine_IkConstraint_h
#define Spine_IkConstraint_h

#include <spine/ConstraintData.h>

#include <spine/Vector.h>

namespace spine {
class IkConstraintData;

class Skeleton;

class Bone;

class SP_API IkConstraint : public Updatable {
	friend class Skeleton;

	friend class IkConstraintTimeline;

RTTI_DECL

public:
	static void apply(Bone &bone, float targetX, float targetY, bool compress, bool stretch, bool uniform, float alpha);

	static void apply(Bone &parent, Bone &child, float targetX, float targetY, int bendDir, bool stretch, float softness, float alpha);

	IkConstraint(IkConstraintData &data, Skeleton &skeleton);

	void apply();

	virtual void update();

	virtual int getOrder();

	IkConstraintData &getData();

	Vector<Bone *> &getBones();

	Bone *getTarget();

	void setTarget(Bone *inValue);

	int getBendDirection();

	void setBendDirection(int inValue);

	bool getCompress();

	void setCompress(bool inValue);

	bool getStretch();

	void setStretch(bool inValue);

	float getMix();

	void setMix(float inValue);

	float getSoftness();

	void setSoftness(float inValue);

	bool isActive();

	void setActive(bool inValue);

private:
	IkConstraintData &_data;
	Vector<Bone *> _bones;
	int _bendDirection;
	bool _compress;
	bool _stretch;
	float _mix;
	float _softness;
	Bone *_target;
	bool _active;
};
}

#endif
