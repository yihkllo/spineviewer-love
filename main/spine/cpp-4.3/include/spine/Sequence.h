#ifndef Spine_Sequence_h
#define Spine_Sequence_h

#include <spine/Array.h>
#include <spine/RTTI.h>
#include <spine/SpineString.h>
#include <spine/TextureRegion.h>

namespace spine {
	class SlotPose;
	class RegionAttachment;
	class MeshAttachment;

	class SkeletonBinary;
	class SkeletonJson;

	class SP_API Sequence : public SpineObject {
		friend class SkeletonBinary;
		friend class SkeletonJson;

	public:

		Sequence(int count, bool pathSuffix);

		Sequence(const Sequence &other);

		~Sequence();

		void update(RegionAttachment &attachment);
		void update(MeshAttachment &attachment);

		Array<TextureRegion *> &getRegions() {
			return _regions;
		}

		int resolveIndex(SlotPose &pose);

		TextureRegion *getRegion(int index);

		Array<float> &getUVs(int index);

		Array<float> &getOffsets(int index);

		int getStart() {
			return _start;
		}

		void setStart(int start) {
			_start = start;
		}

		int getDigits() {
			return _digits;
		}

		void setDigits(int digits) {
			_digits = digits;
		}

		int getSetupIndex() {
			return _setupIndex;
		}

		void setSetupIndex(int setupIndex) {
			_setupIndex = setupIndex;
		}

		bool hasPathSuffix() {
			return _pathSuffix;
		}

		String &getPath(const String &basePath, int index);

		int getId() {
			return _id;
		}

	private:
		static int _nextID;
		int _id;
		Array<TextureRegion *> _regions;
		bool _pathSuffix;
		Array<Array<float>> _uvs;
		Array<Array<float>> _offsets;
		int _start;
		int _digits;
		int _setupIndex;
		String _tmpPath;

		static int nextID();
	};

	enum SequenceMode {
		SequenceMode_hold = 0,
		SequenceMode_once = 1,
		SequenceMode_loop = 2,
		SequenceMode_pingpong = 3,
		SequenceMode_onceReverse = 4,
		SequenceMode_loopReverse = 5,
		SequenceMode_pingpongReverse = 6
	};

	inline SequenceMode SequenceMode_valueOf(const String &value) {
		if (value == "hold") return SequenceMode_hold;
		if (value == "once") return SequenceMode_once;
		if (value == "loop") return SequenceMode_loop;
		if (value == "pingpong") return SequenceMode_pingpong;
		if (value == "onceReverse") return SequenceMode_onceReverse;
		if (value == "loopReverse") return SequenceMode_loopReverse;
		if (value == "pingpongReverse") return SequenceMode_pingpongReverse;
		return SequenceMode_hold;
	}
}

#endif
