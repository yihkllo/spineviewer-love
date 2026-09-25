
#ifndef Spine_Skin_h
#define Spine_Skin_h

#include <spine/Vector.h>
#include <spine/SpineString.h>

namespace spine {
class Attachment;

class Skeleton;
class BoneData;
class ConstraintData;

class SP_API Skin : public SpineObject {
	friend class Skeleton;

public:
	class SP_API AttachmentMap : public SpineObject {
		friend class Skin;

	public:
		struct SP_API Entry {
			size_t _slotIndex;
			String _name;
			Attachment *_attachment;

			Entry(size_t slotIndex, const String &name, Attachment *attachment) :
					_slotIndex(slotIndex),
					_name(name),
					_attachment(attachment) {
			}
		};

		class SP_API Entries {
			friend class AttachmentMap;

		public:
			bool hasNext() {
				while(true) {
					if (_slotIndex >= _buckets.size()) return false;
					if (_bucketIndex >= _buckets[_slotIndex].size()) {
						_bucketIndex = 0;
						++_slotIndex;
						continue;
					};
					return true;
				}
			}

			Entry &next() {
				Entry &result = _buckets[_slotIndex][_bucketIndex];
				++_bucketIndex;
				return result;
			}

		protected:
			Entries(Vector< Vector<Entry> > &buckets) : _buckets(buckets), _slotIndex(0), _bucketIndex(0) {
			}

		private:
			Vector< Vector<Entry> > &_buckets;
			size_t _slotIndex;
			size_t _bucketIndex;
		};

		void put(size_t slotIndex, const String &attachmentName, Attachment *attachment);

		Attachment *get(size_t slotIndex, const String &attachmentName);

		void remove(size_t slotIndex, const String &attachmentName);

		Entries getEntries();

	protected:
		AttachmentMap();

	private:

		int findInBucket(Vector <Entry> &, const String &attachmentName);

		Vector <Vector<Entry> > _buckets;
	};

	explicit Skin(const String &name);

	~Skin();

	void setAttachment(size_t slotIndex, const String &name, Attachment *attachment);

	Attachment *getAttachment(size_t slotIndex, const String &name);

	void removeAttachment(size_t slotIndex, const String& name);

	void findNamesForSlot(size_t slotIndex, Vector <String> &names);

	void findAttachmentsForSlot(size_t slotIndex, Vector<Attachment *> &attachments);

	const String &getName();

	void addSkin(Skin* other);

	void copySkin(Skin* other);

	AttachmentMap::Entries getAttachments();

	Vector<BoneData*>& getBones();

	Vector<ConstraintData*>& getConstraints();
private:
	const String _name;
	AttachmentMap _attachments;
	Vector<BoneData*> _bones;
	Vector<ConstraintData*> _constraints;

	void attachAll(Skeleton &skeleton, Skin &oldSkin);
};
}

#endif
