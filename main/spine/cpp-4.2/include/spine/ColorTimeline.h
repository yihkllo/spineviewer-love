
#ifndef Spine_ColorTimeline_h
#define Spine_ColorTimeline_h

#include <spine/CurveTimeline.h>

namespace spine {
	class SP_API RGBATimeline : public CurveTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit RGBATimeline(size_t frameCount, size_t bezierCount, int slotIndex);

		virtual ~RGBATimeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		void setFrame(int frame, float time, float r, float g, float b, float a);

		int getSlotIndex() { return _slotIndex; };

		void setSlotIndex(int inValue) { _slotIndex = inValue; }

	protected:
		int _slotIndex;

		static const int ENTRIES = 5;
		static const int R = 1;
		static const int G = 2;
		static const int B = 3;
		static const int A = 4;
	};

	class SP_API RGBTimeline : public CurveTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit RGBTimeline(size_t frameCount, size_t bezierCount, int slotIndex);

		virtual ~RGBTimeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		void setFrame(int frame, float time, float r, float g, float b);

		int getSlotIndex() { return _slotIndex; };

		void setSlotIndex(int inValue) { _slotIndex = inValue; }

	protected:
		int _slotIndex;

		static const int ENTRIES = 4;
		static const int R = 1;
		static const int G = 2;
		static const int B = 3;
	};

	class SP_API AlphaTimeline : public CurveTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit AlphaTimeline(size_t frameCount, size_t bezierCount, int slotIndex);

		virtual ~AlphaTimeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		int getSlotIndex() { return _slotIndex; };

		void setSlotIndex(int inValue) { _slotIndex = inValue; }

	protected:
		int _slotIndex;
	};

	class SP_API RGBA2Timeline : public CurveTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit RGBA2Timeline(size_t frameCount, size_t bezierCount, int slotIndex);

		virtual ~RGBA2Timeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		void setFrame(int frame, float time, float r, float g, float b, float a, float r2, float g2, float b2);

		int getSlotIndex() { return _slotIndex; };

		void setSlotIndex(int inValue) { _slotIndex = inValue; }

	protected:
		int _slotIndex;

		static const int ENTRIES = 8;
		static const int R = 1;
		static const int G = 2;
		static const int B = 3;
		static const int A = 4;
		static const int R2 = 5;
		static const int G2 = 6;
		static const int B2 = 7;
	};

	class SP_API RGB2Timeline : public CurveTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit RGB2Timeline(size_t frameCount, size_t bezierCount, int slotIndex);

		virtual ~RGB2Timeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		void setFrame(int frame, float time, float r, float g, float b, float r2, float g2, float b2);

		int getSlotIndex() { return _slotIndex; };

		void setSlotIndex(int inValue) { _slotIndex = inValue; }

	protected:
		int _slotIndex;

		static const int ENTRIES = 7;
		static const int R = 1;
		static const int G = 2;
		static const int B = 3;
		static const int R2 = 4;
		static const int G2 = 5;
		static const int B2 = 6;
	};
}

#endif
