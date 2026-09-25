
#include <spine/DeformTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/VertexAttachment.h>

#include <spine/Animation.h>
#include <spine/Bone.h>
#include <spine/Property.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(DeformTimeline, CurveTimeline)

DeformTimeline::DeformTimeline(size_t frameCount, size_t bezierCount, int slotIndex, VertexAttachment *attachment)
	: CurveTimeline(frameCount, 1, bezierCount), _slotIndex(slotIndex), _attachment(attachment) {
	PropertyId ids[] = {((PropertyId) Property_Deform << 32) | ((slotIndex << 16 | attachment->_id) & 0xffffffff)};
	setPropertyIds(ids, 1);

	_vertices.ensureCapacity(frameCount);
	for (size_t i = 0; i < frameCount; ++i) {
		Vector<float> vec;
		_vertices.add(vec);
	}
}

void DeformTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
						   MixBlend blend, MixDirection direction) {
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);
	SP_UNUSED(direction);

	Slot *slotP = skeleton._slots[_slotIndex];
	Slot &slot = *slotP;
	if (!slot._bone.isActive()) return;

	Attachment *slotAttachment = slot.getAttachment();
	if (slotAttachment == NULL || !slotAttachment->getRTTI().instanceOf(VertexAttachment::rtti)) {
		return;
	}

	VertexAttachment *attachment = static_cast<VertexAttachment *>(slotAttachment);
	if (attachment->_timelineAttachment != _attachment) {
		return;
	}

	Vector<float> &deformArray = slot._deform;
	if (deformArray.size() == 0) {
		blend = MixBlend_Setup;
	}

	Vector<Vector<float>> &vertices = _vertices;
	size_t vertexCount = vertices[0].size();

	Vector<float> &frames = _frames;
	if (time < _frames[0]) {
		switch (blend) {
			case MixBlend_Setup:
				deformArray.clear();
				return;
			case MixBlend_First: {
				if (alpha == 1) {
					deformArray.clear();
					return;
				}
				deformArray.setSize(vertexCount, 0);
				Vector<float> &deform = deformArray;
				if (attachment->getBones().size() == 0) {
					Vector<float> &setupVertices = attachment->getVertices();
					for (size_t i = 0; i < vertexCount; i++)
						deform[i] += (setupVertices[i] - deform[i]) * alpha;
				} else {
					alpha = 1 - alpha;
					for (size_t i = 0; i < vertexCount; i++)
						deform[i] *= alpha;
				}
			}
			case MixBlend_Replace:
			case MixBlend_Add: {
			}
		}
		return;
	}

	deformArray.setSize(vertexCount, 0);
	Vector<float> &deform = deformArray;

	if (time >= frames[frames.size() - 1]) {
		Vector<float> &lastVertices = vertices[frames.size() - 1];
		if (alpha == 1) {
			if (blend == MixBlend_Add) {
				VertexAttachment *vertexAttachment = static_cast<VertexAttachment *>(slotAttachment);
				if (vertexAttachment->getBones().size() == 0) {
					Vector<float> &setupVertices = vertexAttachment->getVertices();
					for (size_t i = 0; i < vertexCount; i++)
						deform[i] += lastVertices[i] - setupVertices[i];
				} else {
					for (size_t i = 0; i < vertexCount; i++)
						deform[i] += lastVertices[i];
				}
			} else {
				memcpy(deform.buffer(), lastVertices.buffer(), vertexCount * sizeof(float));
			}
		} else {
			switch (blend) {
				case MixBlend_Setup: {
					VertexAttachment *vertexAttachment = static_cast<VertexAttachment *>(slotAttachment);
					if (vertexAttachment->getBones().size() == 0) {
						Vector<float> &setupVertices = vertexAttachment->getVertices();
						for (size_t i = 0; i < vertexCount; i++) {
							float setup = setupVertices[i];
							deform[i] = setup + (lastVertices[i] - setup) * alpha;
						}
					} else {
						for (size_t i = 0; i < vertexCount; i++)
							deform[i] = lastVertices[i] * alpha;
					}
					break;
				}
				case MixBlend_First:
				case MixBlend_Replace:
					for (size_t i = 0; i < vertexCount; i++)
						deform[i] += (lastVertices[i] - deform[i]) * alpha;
					break;
				case MixBlend_Add:
					VertexAttachment *vertexAttachment = static_cast<VertexAttachment *>(slotAttachment);
					if (vertexAttachment->getBones().size() == 0) {
						Vector<float> &setupVertices = vertexAttachment->getVertices();
						for (size_t i = 0; i < vertexCount; i++)
							deform[i] += (lastVertices[i] - setupVertices[i]) * alpha;
					} else {
						for (size_t i = 0; i < vertexCount; i++)
							deform[i] += lastVertices[i] * alpha;
					}
			}
		}
		return;
	}

	int frame = Animation::search(frames, time);
	float percent = getCurvePercent(time, frame);
	Vector<float> &prevVertices = vertices[frame];
	Vector<float> &nextVertices = vertices[frame + 1];

	if (alpha == 1) {
		if (blend == MixBlend_Add) {
			VertexAttachment *vertexAttachment = static_cast<VertexAttachment *>(slotAttachment);
			if (vertexAttachment->getBones().size() == 0) {
				Vector<float> &setupVertices = vertexAttachment->getVertices();
				for (size_t i = 0; i < vertexCount; i++) {
					float prev = prevVertices[i];
					deform[i] += prev + (nextVertices[i] - prev) * percent - setupVertices[i];
				}
			} else {
				for (size_t i = 0; i < vertexCount; i++) {
					float prev = prevVertices[i];
					deform[i] += prev + (nextVertices[i] - prev) * percent;
				}
			}
		} else {
			for (size_t i = 0; i < vertexCount; i++) {
				float prev = prevVertices[i];
				deform[i] = prev + (nextVertices[i] - prev) * percent;
			}
		}
	} else {
		switch (blend) {
			case MixBlend_Setup: {
				VertexAttachment *vertexAttachment = static_cast<VertexAttachment *>(slotAttachment);
				if (vertexAttachment->getBones().size() == 0) {
					Vector<float> &setupVertices = vertexAttachment->getVertices();
					for (size_t i = 0; i < vertexCount; i++) {
						float prev = prevVertices[i], setup = setupVertices[i];
						deform[i] = setup + (prev + (nextVertices[i] - prev) * percent - setup) * alpha;
					}
				} else {
					for (size_t i = 0; i < vertexCount; i++) {
						float prev = prevVertices[i];
						deform[i] = (prev + (nextVertices[i] - prev) * percent) * alpha;
					}
				}
				break;
			}
			case MixBlend_First:
			case MixBlend_Replace:
				for (size_t i = 0; i < vertexCount; i++) {
					float prev = prevVertices[i];
					deform[i] += (prev + (nextVertices[i] - prev) * percent - deform[i]) * alpha;
				}
				break;
			case MixBlend_Add:
				VertexAttachment *vertexAttachment = static_cast<VertexAttachment *>(slotAttachment);
				if (vertexAttachment->getBones().size() == 0) {
					Vector<float> &setupVertices = vertexAttachment->getVertices();
					for (size_t i = 0; i < vertexCount; i++) {
						float prev = prevVertices[i];
						deform[i] += (prev + (nextVertices[i] - prev) * percent - setupVertices[i]) * alpha;
					}
				} else {
					for (size_t i = 0; i < vertexCount; i++) {
						float prev = prevVertices[i];
						deform[i] += (prev + (nextVertices[i] - prev) * percent) * alpha;
					}
				}
		}
	}
}

void DeformTimeline::setBezier(size_t bezier, size_t frame, float value, float time1, float value1, float cx1, float cy1,
							   float cx2, float cy2, float time2, float value2) {
	SP_UNUSED(value1);
	SP_UNUSED(value2);
	size_t i = getFrameCount() + bezier * DeformTimeline::BEZIER_SIZE;
	if (value == 0) _curves[frame] = DeformTimeline::BEZIER + i;
	float tmpx = (time1 - cx1 * 2 + cx2) * 0.03, tmpy = cy2 * 0.03 - cy1 * 0.06;
	float dddx = ((cx1 - cx2) * 3 - time1 + time2) * 0.006, dddy = (cy1 - cy2 + 0.33333333) * 0.018;
	float ddx = tmpx * 2 + dddx, ddy = tmpy * 2 + dddy;
	float dx = (cx1 - time1) * 0.3 + tmpx + dddx * 0.16666667, dy = cy1 * 0.3 + tmpy + dddy * 0.16666667;
	float x = time1 + dx, y = dy;
	for (size_t n = i + DeformTimeline::BEZIER_SIZE; i < n; i += 2) {
		_curves[i] = x;
		_curves[i + 1] = y;
		dx += ddx;
		dy += ddy;
		ddx += dddx;
		ddy += dddy;
		x += dx;
		y += dy;
	}
}

float DeformTimeline::getCurvePercent(float time, int frame) {
	int i = (int) _curves[frame];
	switch (i) {
		case DeformTimeline::LINEAR: {
			float x = _frames[frame];
			return (time - x) / (_frames[frame + getFrameEntries()] - x);
		}
		case DeformTimeline::STEPPED: {
			return 0;
		}
		default: {
		}
	}
	i -= DeformTimeline::BEZIER;
	if (_curves[i] > time) {
		float x = _frames[frame];
		return _curves[i + 1] * (time - x) / (_curves[i] - x);
	}
	int n = i + DeformTimeline::BEZIER_SIZE;
	for (i += 2; i < n; i += 2) {
		if (_curves[i] >= time) {
			float x = _curves[i - 2], y = _curves[i - 1];
			return y + (time - x) / (_curves[i] - x) * (_curves[i + 1] - y);
		}
	}
	float x = _curves[n - 2], y = _curves[n - 1];
	return y + (1 - y) * (time - x) / (_frames[frame + getFrameEntries()] - x);
}

void DeformTimeline::setFrame(int frame, float time, Vector<float> &vertices) {
	_frames[frame] = time;
	_vertices[frame].clear();
	_vertices[frame].addAll(vertices);
}

Vector<Vector<float>> &DeformTimeline::getVertices() {
	return _vertices;
}

VertexAttachment *DeformTimeline::getAttachment() {
	return _attachment;
}

void DeformTimeline::setAttachment(VertexAttachment *inValue) {
	_attachment = inValue;
}
