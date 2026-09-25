
#ifndef Spine_TranslateTimeline_h
#define Spine_TranslateTimeline_h

#include <spine/CurveTimeline.h>

#include <spine/Animation.h>
#include <spine/Property.h>

namespace spine {

	class SP_API TranslateTimeline : public CurveTimeline2 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit TranslateTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

		virtual ~TranslateTimeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		int getBoneIndex() { return _boneIndex; }

		void setBoneIndex(int inValue) { _boneIndex = inValue; }

	private:
		int _boneIndex;
	};

	class SP_API TranslateXTimeline : public CurveTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit TranslateXTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

		virtual ~TranslateXTimeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		int getBoneIndex() { return _boneIndex; }

		void setBoneIndex(int inValue) { _boneIndex = inValue; }

	private:
		int _boneIndex;
	};

	class SP_API TranslateYTimeline : public CurveTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit TranslateYTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

		virtual ~TranslateYTimeline();

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
