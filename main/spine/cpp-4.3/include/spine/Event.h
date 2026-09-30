#ifndef Spine_Event_h
#define Spine_Event_h

#include <spine/SpineObject.h>
#include <spine/SpineString.h>

namespace spine {
	class EventData;

	class SP_API Event : public SpineObject {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class AnimationState;

	public:
		Event(float time, const EventData &data);

		const EventData &getData();

		float getTime();

		int getInt();

		void setInt(int inValue);

		float getFloat();

		void setFloat(float inValue);

		const String &getString();

		void setString(const String &inValue);

		float getVolume();

		void setVolume(float inValue);

		float getBalance();

		void setBalance(float inValue);

	private:
		const EventData &_data;
		const float _time;
		int _intValue;
		float _floatValue;
		String _stringValue;
		float _volume;
		float _balance;
	};
}

#endif
