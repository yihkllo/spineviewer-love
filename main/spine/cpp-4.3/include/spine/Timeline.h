#ifndef Spine_Timeline_h
#define Spine_Timeline_h

#include <spine/RTTI.h>
#include <spine/Array.h>
#include <spine/SpineObject.h>
#include <spine/Property.h>

namespace spine {
	class Skeleton;

	class Event;

	enum MixFrom {
		MixFrom_Current = 0,
		MixFrom_Setup = 1,
		MixFrom_First = 2
	};

	class SP_API Timeline : public SpineObject {
		RTTI_DECL_NOPARENT

	public:
		Timeline(size_t frameCount, size_t frameEntries);

		virtual ~Timeline();

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) = 0;

		bool getAdditive() {
			return _additive;
		}

		bool getInstant() {
			return _instant;
		}

		size_t getFrameEntries();

		size_t getFrameCount();

		Array<float> &getFrames();

		float getDuration();

		virtual Array<PropertyId> &getPropertyIds();

	protected:
		void setPropertyIds(PropertyId propertyIds[], size_t propertyIdsCount);

		Array<PropertyId> _propertyIds;
		Array<float> _frames;
		size_t _frameEntries;
		bool _additive;
		bool _instant;
	};
}

#endif
