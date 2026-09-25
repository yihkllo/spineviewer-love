
#include <spine/Skin.h>
#include <spine/extension.h>

_Entry* _Entry_create (int slotIndex, const char* name, spAttachment* attachment) {
	_Entry* self = NEW(_Entry);
	self->slotIndex = slotIndex;
	MALLOC_STR(self->name, name);
	self->attachment = attachment;
	return self;
}

void _Entry_dispose (_Entry* self) {
	spAttachment_dispose(self->attachment);
	FREE(self->name);
	FREE(self);
}


spSkin* spSkin_create (const char* name) {
	spSkin* self = SUPER(NEW(_spSkin));
	MALLOC_STR(self->name, name);
	return self;
}

void spSkin_dispose (spSkin* self) {
	_Entry* entry = SUB_CAST(_spSkin, self)->entries;
	while (entry) {
		_Entry* nextEntry = entry->next;
		_Entry_dispose(entry);
		entry = nextEntry;
	}

	FREE(self->name);
	FREE(self);
}

void spSkin_addAttachment (spSkin* self, int slotIndex, const char* name, spAttachment* attachment) {
	_Entry* newEntry = _Entry_create(slotIndex, name, attachment);
	newEntry->next = SUB_CAST(_spSkin, self)->entries;
	SUB_CAST(_spSkin, self)->entries = newEntry;
}

spAttachment* spSkin_getAttachment (const spSkin* self, int slotIndex, const char* name) {
	const _Entry* entry = SUB_CAST(_spSkin, self)->entries;
	while (entry) {
		if (entry->slotIndex == slotIndex && strcmp(entry->name, name) == 0) return entry->attachment;
		entry = entry->next;
	}
	return 0;
}

const char* spSkin_getAttachmentName (const spSkin* self, int slotIndex, int attachmentIndex) {
	const _Entry* entry = SUB_CAST(_spSkin, self)->entries;
	int i = 0;
	while (entry) {
		if (entry->slotIndex == slotIndex) {
			if (i == attachmentIndex) return entry->name;
			i++;
		}
		entry = entry->next;
	}
	return 0;
}

void spSkin_attachAll (const spSkin* self, spSkeleton* skeleton, const spSkin* oldSkin) {
	const _Entry *entry = SUB_CAST(_spSkin, oldSkin)->entries;
	while (entry) {
		spSlot *slot = skeleton->slots[entry->slotIndex];
		if (slot->attachment == entry->attachment) {
			spAttachment *attachment = spSkin_getAttachment(self, entry->slotIndex, entry->name);
			if (attachment) spSlot_setAttachment(slot, attachment);
		}
		entry = entry->next;
	}
}
