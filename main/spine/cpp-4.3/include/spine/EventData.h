#ifndef Spine_EventData_h
#define Spine_EventData_h

#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/Event.h>

namespace spine {

	class SP_API EventData : public SpineObject {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class Event;

	public:
		explicit EventData(const String &name);

		const String &getName() const;

		Event &getSetupPose();
		const Event &getSetupPose() const;

		const String &getAudioPath() const;

		void setAudioPath(const String &inValue);

	private:
		const String _name;
		String _audioPath;
		Event _setupPose;
	};
}

#endif
