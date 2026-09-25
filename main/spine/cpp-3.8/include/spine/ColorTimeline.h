
#ifndef Spine_ColorTimeline_h
#define Spine_ColorTimeline_h

#include <spine/CurveTimeline.h>

namespace spine {
class SP_API ColorTimeline : public CurveTimeline {
	friend class SkeletonBinary;

	friend class SkeletonJson;

RTTI_DECL

public:
	static const int ENTRIES;

	explicit ColorTimeline(int frameCount);

	virtual void
	apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
		MixDirection direction);

	virtual int getPropertyId();

	void setFrame(int frameIndex, float time, float r, float g, float b, float a);

	int getSlotIndex();

	void setSlotIndex(int inValue);

	Vector<float> &getFrames();

protected:
	static const int PREV_TIME;
	static const int PREV_R;
	static const int PREV_G;
	static const int PREV_B;
	static const int PREV_A;
	static const int R;
	static const int G;
	static const int B;
	static const int A;

private:
	int _slotIndex;
	Vector<float> _frames;
};
}

#endif
