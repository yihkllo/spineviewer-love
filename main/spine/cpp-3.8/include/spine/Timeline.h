
#ifndef Spine_Timeline_h
#define Spine_Timeline_h

#include <spine/RTTI.h>
#include <spine/Vector.h>
#include <spine/MixBlend.h>
#include <spine/MixDirection.h>
#include <spine/SpineObject.h>

namespace spine {
class Skeleton;

class Event;

class SP_API Timeline : public SpineObject {
RTTI_DECL

public:
	Timeline();

	virtual ~Timeline();

	virtual void
	apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
		MixDirection direction) = 0;

	virtual int getPropertyId() = 0;
};
}

#endif
