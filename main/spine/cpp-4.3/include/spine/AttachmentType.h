#ifndef Spine_AttachmentType_h
#define Spine_AttachmentType_h

#include <string.h>

namespace spine {
	enum AttachmentType {
		AttachmentType_Region,
		AttachmentType_Boundingbox,
		AttachmentType_Mesh,
		AttachmentType_Linkedmesh,
		AttachmentType_Path,
		AttachmentType_Point,
		AttachmentType_Clipping
	};

	inline AttachmentType AttachmentType_valueOf(const char *value) {
		if (strcmp(value, "region") == 0)
			return AttachmentType_Region;
		else if (strcmp(value, "mesh") == 0)
			return AttachmentType_Mesh;
		else if (strcmp(value, "linkedmesh") == 0)
			return AttachmentType_Linkedmesh;
		else if (strcmp(value, "boundingbox") == 0)
			return AttachmentType_Boundingbox;
		else if (strcmp(value, "path") == 0)
			return AttachmentType_Path;
		else if (strcmp(value, "clipping") == 0)
			return AttachmentType_Clipping;
		else if (strcmp(value, "point") == 0)
			return AttachmentType_Point;
		else
			return AttachmentType_Region;
	}
}

#endif
