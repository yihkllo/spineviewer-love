
#ifndef SPINE_SLOTDATA_H_
#define SPINE_SLOTDATA_H_

#include <spine/dll.h>
#include <spine/BoneData.h>
#include <spine/Color.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SP_BLEND_MODE_NORMAL, SP_BLEND_MODE_ADDITIVE, SP_BLEND_MODE_MULTIPLY, SP_BLEND_MODE_SCREEN
} spBlendMode;

typedef struct spSlotData {
	const int index;
	const char* const name;
	const spBoneData* const boneData;
	const char* attachmentName;
	spColor color;
	spColor* darkColor;
	spBlendMode blendMode;

#ifdef __cplusplus
	spSlotData() :
		index(0),
		name(0),
		boneData(0),
		attachmentName(0),
		color(),
		darkColor(0),
		blendMode(SP_BLEND_MODE_NORMAL) {
	}
#endif
} spSlotData;

SP_API spSlotData* spSlotData_create (const int index, const char* name, spBoneData* boneData);
SP_API void spSlotData_dispose (spSlotData* self);

SP_API void spSlotData_setAttachmentName (spSlotData* self, const char* attachmentName);

#ifdef SPINE_SHORT_NAMES
typedef spBlendMode BlendMode;
#define BLEND_MODE_NORMAL SP_BLEND_MODE_NORMAL
#define BLEND_MODE_ADDITIVE SP_BLEND_MODE_ADDITIVE
#define BLEND_MODE_MULTIPLY SP_BLEND_MODE_MULTIPLY
#define BLEND_MODE_SCREEN SP_BLEND_MODE_SCREEN
typedef spSlotData SlotData;
#define SlotData_create(...) spSlotData_create(__VA_ARGS__)
#define SlotData_dispose(...) spSlotData_dispose(__VA_ARGS__)
#define SlotData_setAttachmentName(...) spSlotData_setAttachmentName(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
