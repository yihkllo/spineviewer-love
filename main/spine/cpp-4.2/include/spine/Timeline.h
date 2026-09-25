
#ifndef Spine_Timeline_h
#define Spine_Timeline_h

#include <spine/RTTI.h>
#include <spine/Vector.h>
#include <spine/MixBlend.h>
#include <spine/MixDirection.h>
#include <spine/SpineObject.h>
#include <spine/Property.h>

namespace spine {
	class Skeleton;

	class Event;

	class SP_API Timeline : public SpineObject {
	RTTI_DECL

	public:
		Timeline(size_t frameCount, size_t frameEntries);

		virtual ~Timeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction) = 0;

		size_t getFrameEntries();

		size_t getFrameCount();

		Vector<float> &getFrames();

		float getDuration();

		virtual Vector <PropertyId> &getPropertyIds();

	protected:
		void setPropertyIds(PropertyId propertyIds[], size_t propertyIdsCount);

        Vector <PropertyId> _propertyIds;
		Vector<float> _frames;
		size_t _frameEntries;
	};
}

#endif
