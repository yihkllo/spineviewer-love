
#ifndef Spine_EventData_h
#define Spine_EventData_h

#include <spine/SpineObject.h>
#include <spine/SpineString.h>

namespace spine {
class SP_API EventData : public SpineObject {
	friend class SkeletonBinary;

	friend class SkeletonJson;

	friend class Event;

public:
	explicit EventData(const String &name);

	const String &getName() const;

	int getIntValue() const;

	void setIntValue(int inValue);

	float getFloatValue()  const;

	void setFloatValue(float inValue);

	const String &getStringValue()  const;

	void setStringValue(const String &inValue);

	const String &getAudioPath()  const;

	void setAudioPath(const String &inValue);

	float getVolume() const;

	void setVolume(float inValue);

	float getBalance() const;

	void setBalance(float inValue);

private:
	const String _name;
	int _intValue;
	float _floatValue;
	String _stringValue;
	String _audioPath;
	float _volume;
	float _balance;
};
}

#endif
