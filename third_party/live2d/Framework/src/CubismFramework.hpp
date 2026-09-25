

#pragma once


#include "Live2DCubismCore.hpp"


#include "CubismFrameworkConfig.hpp"


#include <new>
#include "ICubismAllocator.hpp"

#ifdef __linux__
#include <cstdlib>
#endif


namespace Live2D { namespace Cubism { namespace Framework {

class CubismAllocationTag
{ };

class CubismAllocationAlignedTag
{ };

static CubismAllocationTag GlobalTag;
static CubismAllocationAlignedTag GloabalAlignedTag;

}}}

#ifdef CSM_DEBUG_MEMORY_LEAKING


void* operator new (Live2D::Cubism::Framework::csmSizeType size, Live2D::Cubism::Framework::CubismAllocationTag tag, const Live2D::Cubism::Framework::csmChar* fileName, Live2D::Cubism::Framework::csmInt32 lineNumber);
void* operator new (Live2D::Cubism::Framework::csmSizeType size, Live2D::Cubism::Framework::csmUint32 alignment, Live2D::Cubism::Framework::CubismAllocationAlignedTag tag, const Live2D::Cubism::Framework::csmChar* fileName, Live2D::Cubism::Framework::csmInt32 lineNumber);
void  operator delete(void* address, Live2D::Cubism::Framework::CubismAllocationTag tag, const Live2D::Cubism::Framework::csmChar* fileName, Live2D::Cubism::Framework::csmInt32 lineNumber);
void  operator delete(void* address, Live2D::Cubism::Framework::CubismAllocationAlignedTag tag, const Live2D::Cubism::Framework::csmChar* fileName, Live2D::Cubism::Framework::csmInt32 lineNumber);

template<typename T>
void CsmDelete(T* address, const Live2D::Cubism::Framework::csmChar* fileName, Live2D::Cubism::Framework::csmInt32 lineNumber)
{
    if (!address)
    {
        return;
    }

    address->~T();

    operator delete(reinterpret_cast<void*>(address), Live2D::Cubism::Framework::GlobalTag, fileName, lineNumber);
}

#define CSM_NEW                          new(Live2D::Cubism::Framework::GlobalTag, __FILE__, __LINE__)
#define CSM_DELETE_SELF(type, obj)       do { if (!obj){ break; } obj->~type(); operator delete(obj, Live2D::Cubism::Framework::GlobalTag, __FILE__, __LINE__); } while(0)
#define CSM_DELETE(obj)                  CsmDelete(obj, __FILE__, __LINE__)
#define CSM_MALLOC(size)                 Live2D::Cubism::Framework::CubismFramework::Allocate(size, __FILE__, __LINE__)
#define CSM_MALLOC_ALLIGNED(size, align) Live2D::Cubism::Framework::CubismFramework::AllocateAligned(size, align, __FILE__, __LINE__)
#define CSM_FREE(ptr)                    Live2D::Cubism::Framework::CubismFramework::Deallocate(ptr, __FILE__, __LINE__)
#define CSM_FREE_ALLIGNED(ptr)           Live2D::Cubism::Framework::CubismFramework::DeallocateAligned(ptr, __FILE__, __LINE__)

#else


void* operator new (Live2D::Cubism::Framework::csmSizeType size, Live2D::Cubism::Framework::CubismAllocationTag tag);
void* operator new (Live2D::Cubism::Framework::csmSizeType size, Live2D::Cubism::Framework::csmUint32 alignment, Live2D::Cubism::Framework::CubismAllocationAlignedTag tag);
void  operator delete(void* address, Live2D::Cubism::Framework::CubismAllocationTag tag);
void  operator delete(void* address, Live2D::Cubism::Framework::CubismAllocationAlignedTag tag);

template<typename T>
void CsmDelete(T* address)
{
    if (!address)
    {
        return;
    }

    address->~T();

    operator delete(reinterpret_cast<void*>(address), Live2D::Cubism::Framework::GlobalTag);
}

#define CSM_NEW                          new(Live2D::Cubism::Framework::GlobalTag)
#define CSM_DELETE_SELF(type, obj)       do { if (!obj){ break; } obj->~type(); operator delete(obj, Live2D::Cubism::Framework::GlobalTag); } while(0)
#define CSM_DELETE(obj)                  CsmDelete(obj)
#define CSM_MALLOC(size)                 Live2D::Cubism::Framework::CubismFramework::Allocate(size)
#define CSM_MALLOC_ALLIGNED(size, align) Live2D::Cubism::Framework::CubismFramework::AllocateAligned(size, align)
#define CSM_FREE(ptr)                    Live2D::Cubism::Framework::CubismFramework::Deallocate(ptr)
#define CSM_FREE_ALLIGNED(ptr)           Live2D::Cubism::Framework::CubismFramework::DeallocateAligned(ptr)

#endif

#define CSM_PLACEMENT_NEW(addrs)         new((addrs))


#include "Type/CubismBasicType.hpp"


namespace Live2D { namespace Cubism { namespace Framework {

class CubismIdManager;

}}}


#ifdef _MSC_VER
#pragma warning (disable : 4100)
#endif

#ifndef NULL
#define NULL  0
#endif


#define CubismEnsure(expression, message, body)       \
do                                              \
{                                               \
  if (!(expression))                            \
  {                                             \
    CubismFramework::CoreLogFunction("[Cubism Framework] " message); \
    body;                                       \
  }                                             \
}                                               \
while (0);



namespace Csm = Live2D::Cubism::Framework;


namespace Live2D { namespace Cubism { namespace Framework {

namespace Constant {
extern const csmInt32 VertexOffset;

extern const csmInt32 VertexStep;
}

class CubismFramework
{
public:
    class Option
    {
    public:
        enum LogLevel
        {
            LogLevel_Verbose = 0,

            LogLevel_Debug,

            LogLevel_Info,

            LogLevel_Warning,

            LogLevel_Error,

            LogLevel_Off
        };

        Core::csmLogFunction LogFunction;

        LogLevel LoggingLevel;
    };

    static csmBool StartUp(ICubismAllocator* allocator, const Option* option = NULL);

    static void CleanUp();

    static csmBool IsStarted();

    static void Initialize();

    static void Dispose();

    static csmBool IsInitialized();

    static void CoreLogFunction(const csmChar* message);

    static Option::LogLevel GetLoggingLevel();

    static CubismIdManager* GetIdManager();

#ifdef CSM_DEBUG_MEMORY_LEAKING

    static void* Allocate(csmSizeType size, const csmChar* fileName, csmInt32 lineNumber);

    static void* AllocateAligned(csmSizeType size, csmUint32 alignment, const csmChar* fileName, csmInt32 lineNumber);

    static void  Deallocate(void* address, const csmChar* fileName, csmInt32 lineNumber);

    static void  DeallocateAligned(void* address, const csmChar* fileName, csmInt32 lineNumber);

#else

    static void* Allocate(csmSizeType size);

    static void* AllocateAligned(csmSizeType size, csmUint32 alignment);

    static void  Deallocate(void* address);

    static void  DeallocateAligned(void* address);

#endif

private:
    CubismFramework(){}

};

}}}
