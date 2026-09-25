
#ifndef Spine_SkeletonJson_h
#define Spine_SkeletonJson_h

#include <spine/Vector.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>

namespace spine {
class CurveTimeline;

class VertexAttachment;

class Animation;

class Json;

class SkeletonData;

class Atlas;

class AttachmentLoader;

class LinkedMesh;

class String;

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

	static float toColor(const char *value, size_t index);

	static void readCurve(Json *frame, CurveTimeline *timeline, size_t frameIndex);

	Animation *readAnimation(Json *root, SkeletonData *skeletonData);

	void readVertices(Json *attachmentMap, VertexAttachment *attachment, size_t verticesLength);

	void setError(Json *root, const String &value1, const String &value2);
};
}

#endif
