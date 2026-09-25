
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

	int getIntValue();

	void setIntValue(int inValue);

	float getFloatValue();

	void setFloatValue(float inValue);

	const String &getStringValue();

	void setStringValue(const String &inValue);

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
