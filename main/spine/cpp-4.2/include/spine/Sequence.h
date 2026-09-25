
#ifndef Spine_Sequence_h
#define Spine_Sequence_h

#include <spine/Vector.h>
#include <spine/SpineString.h>
#include <spine/TextureRegion.h>

namespace spine {
	class Slot;

	class Attachment;

	class SkeletonBinary;
	class SkeletonJson;

	class SP_API Sequence : public SpineObject {
		friend class SkeletonBinary;
		friend class SkeletonJson;
	public:
		Sequence(int count);

		~Sequence();

		Sequence *copy();

		void apply(Slot *slot, Attachment *attachment);

		String getPath(const String &basePath, int index);

		int getId() { return _id; }

		void setId(int id) { _id = id; }

		int getStart() { return _start; }

		void setStart(int start) { _start = start; }

		int getDigits() { return _digits; }

		void setDigits(int digits) { _digits = digits; }

		int getSetupIndex() { return _setupIndex; }

		void setSetupIndex(int setupIndex) { _setupIndex = setupIndex; }

		Vector<TextureRegion *> &getRegions() { return _regions; }

	private:
		int _id;
		Vector<TextureRegion *> _regions;
		int _start;
		int _digits;
		int _setupIndex;

		int getNextID();
	};

	enum SequenceMode {
		hold = 0,
		once = 1,
		loop = 2,
		pingpong = 3,
		onceReverse = 4,
		loopReverse = 5,
		pingpongReverse = 6
	};
}

#endif
