#ifndef Spine_Skin_h
#define Spine_Skin_h

#include <spine/Array.h>
#include <spine/SpineString.h>
#include <spine/Color.h>

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
				String _placeholder;
				Attachment *_attachment;

				Entry(size_t slotIndex, const String &placeholder, Attachment *attachment)
					: _slotIndex(slotIndex), _placeholder(placeholder), _attachment(attachment) {
				}
			};

			class SP_API Entries {
				friend class AttachmentMap;

			public:
				bool hasNext() {
					while (true) {
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
				Entries(Array<Array<Entry>> &buckets) : _buckets(buckets), _slotIndex(0), _bucketIndex(0) {
				}

			private:
				Array<Array<Entry>> &_buckets;
				size_t _slotIndex;
				size_t _bucketIndex;
			};

			void put(size_t slotIndex, const String &placeholder, Attachment *attachment);

			Attachment *get(size_t slotIndex, const String &placeholder);

			void remove(size_t slotIndex, const String &placeholder);

			Entries getEntries();

		protected:
			AttachmentMap();

		private:
			int findInBucket(Array<Entry> &, const String &placeholder);

			Array<Array<Entry>> _buckets;
		};

		explicit Skin(const String &name);

		~Skin();

		void setAttachment(size_t slotIndex, const String &placeholder, Attachment *attachment);

		Attachment *getAttachment(size_t slotIndex, const String &placeholder);

		void removeAttachment(size_t slotIndex, const String &placeholder);

		void findNamesForSlot(size_t slotIndex, Array<String> &names);

		void findAttachmentsForSlot(size_t slotIndex, Array<Attachment *> &attachments);

		const String &getName();

		void addSkin(Skin &other);

		void copySkin(Skin &other);

		AttachmentMap::Entries getAttachments();

		Array<BoneData *> &getBones();

		Array<ConstraintData *> &getConstraints();

		Color &getColor() {
			return _color;
		}

	private:
		const String _name;
		AttachmentMap _attachments;
		Array<BoneData *> _bones;
		Array<ConstraintData *> _constraints;
		Color _color;

		void attachAll(Skeleton &skeleton, Skin &oldSkin);
	};
}

#endif
