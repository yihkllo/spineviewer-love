
#ifndef Spine_DeformTimeline_h
#define Spine_DeformTimeline_h

#include <spine/CurveTimeline.h>

namespace spine {
	class VertexAttachment;

	class SP_API DeformTimeline : public CurveTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit DeformTimeline(size_t frameCount, size_t bezierCount, int slotIndex, VertexAttachment *attachment);

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		void setFrame(int frameIndex, float time, Vector<float> &vertices);

		Vector <Vector<float>> &getVertices();

		VertexAttachment *getAttachment();

		void setAttachment(VertexAttachment *inValue);

		virtual void
		setBezier(size_t bezier, size_t frame, float value, float time1, float value1, float cx1, float cy1, float cx2,
				  float cy2, float time2, float value2);

		float getCurvePercent(float time, int frame);

		int getSlotIndex() { return _slotIndex; }

		void setSlotIndex(int inValue) { _slotIndex = inValue; }

	protected:
		int _slotIndex;

		Vector <Vector<float>> _vertices;

		VertexAttachment *_attachment;
	};
}

#endif
