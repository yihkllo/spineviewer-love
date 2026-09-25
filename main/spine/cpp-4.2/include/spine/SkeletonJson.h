
#ifndef Spine_SkeletonJson_h
#define Spine_SkeletonJson_h

#include <spine/Vector.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>

namespace spine {
	class Timeline;

	class CurveTimeline;

	class CurveTimeline1;

	class CurveTimeline2;

	class VertexAttachment;

	class Animation;

	class Json;

	class SkeletonData;

	class Atlas;

	class AttachmentLoader;

	class LinkedMesh;

	class String;

	class Sequence;

	class SP_API SkeletonJson : public SpineObject {
	public:
		explicit SkeletonJson(Atlas *atlas);

		explicit SkeletonJson(AttachmentLoader *attachmentLoader, bool ownsLoader = false);

		~SkeletonJson();

		SkeletonData *readSkeletonDataFile(const String &path);

		SkeletonData *readSkeletonData(const char *json);

		void setScale(float scale) { _scale = scale; }

		String &getError() { return _error; }

	private:
		AttachmentLoader *_attachmentLoader;
		Vector<LinkedMesh *> _linkedMeshes;
		float _scale;
		const bool _ownsLoader;
		String _error;

		static Sequence *readSequence(Json *sequence);

		static void
		setBezier(CurveTimeline *timeline, int frame, int value, int bezier, float time1, float value1, float cx1,
				  float cy1,
				  float cx2, float cy2, float time2, float value2);

		static int
		readCurve(Json *curve, CurveTimeline *timeline, int bezier, int frame, int value, float time1, float time2,
				  float value1, float value2, float scale);

		static Timeline *readTimeline(Json *keyMap, CurveTimeline1 *timeline, float defaultValue, float scale);

		static Timeline *
		readTimeline(Json *keyMap, CurveTimeline2 *timeline, const char *name1, const char *name2, float defaultValue,
					 float scale);

		Animation *readAnimation(Json *root, SkeletonData *skeletonData);

		void readVertices(Json *attachmentMap, VertexAttachment *attachment, size_t verticesLength);

		void setError(Json *root, const String &value1, const String &value2);

		int findSlotIndex(SkeletonData *skeletonData, const String &slotName, Vector<Timeline *> timelines);
	};
}

#endif
