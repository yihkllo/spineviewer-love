
#ifndef Spine_BlockAllocator_h
#define Spine_BlockAllocator_h

#include <cstdint>
#include <spine/SpineObject.h>
#include <spine/Extension.h>
#include <spine/MathUtil.h>
#include <spine/Vector.h>

namespace spine {
    struct Block {
        int size;
        int allocated;
        uint8_t *memory;

        int free() {
            return size - allocated;
        }

        bool canFit(int numBytes) {
            return free() >= numBytes;
        }

        uint8_t *allocate(int numBytes) {
            uint8_t *ptr = memory + allocated;
            allocated += numBytes;
            return ptr;
        }
    };

    class BlockAllocator : public SpineObject {
        int initialBlockSize;
        Vector <Block> blocks;

    public:
        BlockAllocator(int initialBlockSize) : initialBlockSize(initialBlockSize) {
            blocks.add(newBlock(initialBlockSize));
        }

        ~BlockAllocator() {
            for (int i = 0, n = (int) blocks.size(); i < n; i++) {
                SpineExtension::free(blocks[i].memory, __FILE__, __LINE__);
            }
        }

        template<typename T>
        T *allocate(size_t num) {
            return (T *) _allocate((int) (sizeof(T) * num));
        }

        void compress() {
            if (blocks.size() == 1) {
                blocks[0].allocated = 0;
                return;
            }
            int totalSize = 0;
            for (int i = 0, n = (int)blocks.size(); i < n; i++) {
                totalSize += blocks[i].size;
                SpineExtension::free(blocks[i].memory, __FILE__, __LINE__);
            }
            blocks.clear();
            blocks.add(newBlock(totalSize));
        }

    private:
        void *_allocate(int numBytes) {
            int alignedNumBytes = numBytes + (numBytes % 16 != 0 ? 16 - (numBytes % 16) : 0);
            Block *block = &blocks[blocks.size() - 1];
            if (!block->canFit(alignedNumBytes)) {
                blocks.add(newBlock(MathUtil::max(initialBlockSize, alignedNumBytes)));
                block = &blocks[blocks.size() - 1];
            }
            return block->allocate(alignedNumBytes);
        }

        Block newBlock(int numBytes) {
            Block block = {MathUtil::max(initialBlockSize, numBytes), 0, nullptr};
            block.memory = SpineExtension::alloc<uint8_t>(block.size, __FILE__, __LINE__);
            return block;
        }
    };
}

#endif
