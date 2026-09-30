#ifndef Spine_Attachment_h
#define Spine_Attachment_h

#include <spine/Array.h>
#include <spine/RTTI.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>

namespace spine {
	class Slot;

	class SP_API Attachment : public SpineObject {
		RTTI_DECL_NOPARENT

	public:
		explicit Attachment(const String &name);

		virtual ~Attachment();

		const String &getName() const;

		virtual Attachment &copy() = 0;

		Attachment *getTimelineAttachment();

		void setTimelineAttachment(Attachment *attachment);

		Array<int> &getTimelineSlots();

		void setTimelineSlots(Array<int> &timelineSlots);

		bool isTimelineActive(Array<Slot *> &slots, int slotIndex, bool appliedPose);

		int getRefCount();

		void reference();

		void dereference();

	private:
		const String _name;
		Attachment *_timelineAttachment;
		Array<int> _timelineSlots;
		int _refCount;
	};
}

#endif
