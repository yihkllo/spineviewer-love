#ifndef Spine_SkeletonJson_h
#define Spine_SkeletonJson_h

#include <spine/Array.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>

namespace spine {
	class Timeline;

	class CurveTimeline;

	class CurveTimeline1;

	class BoneTimeline2;

	class VertexAttachment;

	class Animation;

	class Json;

	class SkeletonData;

	class Atlas;

	class AttachmentLoader;

	class LinkedMesh;

	class String;

	class Sequence;

	class Skin;

	class Attachment;

	class SP_API SkeletonJson : public SpineObject {
	public:
		explicit SkeletonJson(Atlas &atlas);

		explicit SkeletonJson(AttachmentLoader &attachmentLoader, bool ownsLoader = false);

		~SkeletonJson();

		SkeletonData *readSkeletonDataFile(const String &path);

		SkeletonData *readSkeletonData(const char *json);

		void setScale(float scale) {
			_scale = scale;
		}

		const String &getError() const {
			return _error;
		}

	private:
		AttachmentLoader *_attachmentLoader;
		Array<LinkedMesh *> _linkedMeshes;
		float _scale;
		const bool _ownsLoader;
		String _error;

		static Sequence *readSequence(Json *sequence);

		static void setBezier(CurveTimeline *timeline, int frame, int value, int bezier, float time1, float value1, float cx1, float cy1, float cx2,
							  float cy2, float time2, float value2);

		static int readCurve(Json *curve, CurveTimeline *timeline, int bezier, int frame, int value, float time1, float time2, float value1,
							 float value2, float scale);

		static void readTimeline(Array<Timeline *> &timelines, Json *keyMap, CurveTimeline1 *timeline, float defaultValue, float scale);

		static void readTimeline(Array<Timeline *> &timelines, Json *keyMap, BoneTimeline2 *timeline, const char *name1, const char *name2,
								 float defaultValue, float scale);

		Animation *readAnimation(Json *root, SkeletonData *skeletonData);

		Attachment *readAttachment(Json *map, Skin *skin, int slotIndex, const char *placeholder, SkeletonData *skeletonData);

		void readVertices(Json *attachmentMap, VertexAttachment *attachment, size_t verticesLength);

		bool readDrawOrder(SkeletonData *skeletonData, Json *keyMap, int slotCount, const Array<int> *folderSlots, Array<int> &drawOrder);

		void setError(Json *root, const String &value1, const String &value2);

		int findSlotIndex(SkeletonData *skeletonData, const String &slotName, Array<Timeline *> timelines);
	};
}

#endif
