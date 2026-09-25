
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
		explicit DeformTimeline(int frameCount);

		virtual void apply(Skeleton& skeleton, float lastTime, float time, Vector<Event*>* pEvents, float alpha, MixBlend blend, MixDirection direction);

		virtual int getPropertyId();

		void setFrame(int frameIndex, float time, Vector<float>& vertices);

		int getSlotIndex();
		void setSlotIndex(int inValue);
		Vector<float>& getFrames();
		Vector< Vector<float> >& getVertices();
		VertexAttachment* getAttachment();
		void setAttachment(VertexAttachment* inValue);

	private:
		int _slotIndex;
		Vector<float> _frames;
		Vector< Vector<float> > _frameVertices;
		VertexAttachment* _attachment;
	};
}

#endif
