#ifndef Spine_DeformTimeline_h
#define Spine_DeformTimeline_h

#include <spine/SlotCurveTimeline.h>

namespace spine {
	class VertexAttachment;

	class SP_API DeformTimeline : public SlotCurveTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit DeformTimeline(size_t frameCount, size_t bezierCount, int slotIndex, VertexAttachment &attachment);

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;

		void setFrame(int frameIndex, float time, Array<float> &vertices);

		Array<Array<float>> &getVertices();

		VertexAttachment &getAttachment();

		void setAttachment(VertexAttachment &inValue);

		virtual void setBezier(size_t bezier, size_t frame, float value, float time1, float value1, float cx1, float cy1, float cx2, float cy2,
							   float time2, float value2) override;

		float getCurvePercent(float time, int frame);

		size_t getFrameCount() {
			return _frames.size();
		}

	protected:
		void _apply(Slot &slot, SlotPose &pose, float time, float alpha, MixFrom from, bool add) override;

	private:
		void applyBeforeFirst(Slot &slot, bool appliedPose, float alpha, MixFrom from);
		void applyToPose(SlotPose &pose, Array<float> &v1, Array<float> *v2, float percent, size_t vertexCount, float alpha, MixFrom from, bool add);
		void applyToSlot(Slot &slot, bool appliedPose, Array<float> &v1, Array<float> *v2, float percent, size_t vertexCount, float alpha,
						 MixFrom from, bool add);

		Array<Array<float>> _vertices;

		VertexAttachment *_attachment;
	};
}

#endif
