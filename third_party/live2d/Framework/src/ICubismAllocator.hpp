

#pragma once

#include "Type/CubismBasicType.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class ICubismAllocator
{
public:
    virtual ~ICubismAllocator() {}

    virtual void* Allocate(const csmSizeType size) = 0;

    virtual void Deallocate(void* memory) = 0;


    virtual void* AllocateAligned(const csmSizeType size, const csmUint32 alignment) = 0;

    virtual void DeallocateAligned(void* alignedMemory) = 0;

};
}}}
