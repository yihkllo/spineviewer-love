
#ifndef SPINE_SKIN_H_
#define SPINE_SKIN_H_

#include <spine/dll.h>
#include <spine/Attachment.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SKIN_ENTRIES_HASH_TABLE_SIZE 100

struct spSkeleton;

typedef struct spSkin {
	const char* const name;

#ifdef __cplusplus
	spSkin() :
		name(0) {
	}
#endif
} spSkin;

typedef struct _Entry _Entry;
struct _Entry {
	int slotIndex;
	const char* name;
	spAttachment* attachment;
	_Entry* next;
};

typedef struct _SkinHashTableEntry _SkinHashTableEntry;
struct _SkinHashTableEntry {
	_Entry* entry;
	_SkinHashTableEntry* next;
};

typedef struct {
	spSkin super;
	_Entry* entries;
	_SkinHashTableEntry* entriesHashTable[SKIN_ENTRIES_HASH_TABLE_SIZE];
} _spSkin;

SP_API spSkin* spSkin_create (const char* name);
SP_API void spSkin_dispose (spSkin* self);

SP_API void spSkin_addAttachment (spSkin* self, int slotIndex, const char* name, spAttachment* attachment);
SP_API spAttachment* spSkin_getAttachment (const spSkin* self, int slotIndex, const char* name);

SP_API const char* spSkin_getAttachmentName (const spSkin* self, int slotIndex, int attachmentIndex);

SP_API void spSkin_attachAll (const spSkin* self, struct spSkeleton* skeleton, const spSkin* oldspSkin);

#ifdef SPINE_SHORT_NAMES
typedef spSkin Skin;
#define Skin_create(...) spSkin_create(__VA_ARGS__)
#define Skin_dispose(...) spSkin_dispose(__VA_ARGS__)
#define Skin_addAttachment(...) spSkin_addAttachment(__VA_ARGS__)
#define Skin_getAttachment(...) spSkin_getAttachment(__VA_ARGS__)
#define Skin_getAttachmentName(...) spSkin_getAttachmentName(__VA_ARGS__)
#define Skin_attachAll(...) spSkin_attachAll(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
