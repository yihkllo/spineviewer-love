
#ifndef Spine_ShearTimeline_h
#define Spine_ShearTimeline_h

#include <spine/TranslateTimeline.h>

namespace spine {
	class SP_API ShearTimeline : public CurveTimeline2 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit ShearTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

		virtual ~ShearTimeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		int getBoneIndex() { return _boneIndex; }

		void setBoneIndex(int inValue) { _boneIndex = inValue; }

	private:
		int _boneIndex;
	};

	class SP_API ShearXTimeline : public CurveTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit ShearXTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

		virtual ~ShearXTimeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		int getBoneIndex() { return _boneIndex; }

		void setBoneIndex(int inValue) { _boneIndex = inValue; }

	private:
		int _boneIndex;
	};

	class SP_API ShearYTimeline : public CurveTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit ShearYTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

		virtual ~ShearYTimeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		int getBoneIndex() { return _boneIndex; }

		void setBoneIndex(int inValue) { _boneIndex = inValue; }

	private:
		int _boneIndex;
	};
}

#endif
