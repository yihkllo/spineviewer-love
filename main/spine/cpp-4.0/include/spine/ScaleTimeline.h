
#ifndef Spine_ScaleTimeline_h
#define Spine_ScaleTimeline_h

#include <spine/TranslateTimeline.h>

namespace spine {
	class SP_API ScaleTimeline : public CurveTimeline2 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit ScaleTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

		virtual ~ScaleTimeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		int getBoneIndex() { return _boneIndex; }

		void setBoneIndex(int inValue) { _boneIndex = inValue; }

	private:
		int _boneIndex;
	};

	class SP_API ScaleXTimeline : public CurveTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit ScaleXTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

		virtual ~ScaleXTimeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		int getBoneIndex() { return _boneIndex; }

		void setBoneIndex(int inValue) { _boneIndex = inValue; }

	private:
		int _boneIndex;
	};

	class SP_API ScaleYTimeline : public CurveTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit ScaleYTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

		virtual ~ScaleYTimeline();

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
