

#pragma once

#include "CubismFramework.hpp"

#ifdef CSM_DEBUG
#include "assert.h"
#define CSM_ASSERT(expr)    assert(expr)
#else
#define CSM_ASSERT(expr)
#endif

#define CubismLogPrint(level, fmt, ...)         Live2D::Cubism::Framework::Utils::CubismDebug::Print(level,  "[CSM]" fmt, ## __VA_ARGS__)
#define CubismLogPrintln(level, fmt, ...)       CubismLogPrint(level, fmt "\n", ## __VA_ARGS__)

#if CSM_LOG_LEVEL <= CSM_LOG_LEVEL_VERBOSE
#define CubismLogVerbose(fmt, ...)    CubismLogPrintln(Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Verbose, "[V]" fmt, ## __VA_ARGS__)
#define CubismLogDebug(fmt, ...)      CubismLogPrintln(Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Debug,   "[D]" fmt, ## __VA_ARGS__)
#define CubismLogInfo(fmt, ...)       CubismLogPrintln(Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Info,    "[I]" fmt, ## __VA_ARGS__)
#define CubismLogWarning(fmt, ...)    CubismLogPrintln(Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Warning, "[W]" fmt, ## __VA_ARGS__)
#define CubismLogError(fmt, ...)      CubismLogPrintln(Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Error,   "[E]" fmt, ## __VA_ARGS__)
#elif CSM_LOG_LEVEL == CSM_LOG_LEVEL_DEBUG
#define CubismLogVerbose(fmt, ...)
#define CubismLogDebug(fmt, ...)      CubismLogPrintln(Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Debug,   "[D]" fmt, ## __VA_ARGS__)
#define CubismLogInfo(fmt, ...)       CubismLogPrintln(Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Info,    "[I]" fmt, ## __VA_ARGS__)
#define CubismLogWarning(fmt, ...)    CubismLogPrintln(Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Warning, "[W]" fmt, ## __VA_ARGS__)
#define CubismLogError(fmt, ...)      CubismLogPrintln(Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Error,   "[E]" fmt, ## __VA_ARGS__)
#elif CSM_LOG_LEVEL == CSM_LOG_LEVEL_INFO
#define CubismLogVerbose(fmt, ...)
#define CubismLogDebug(fmt, ...)
#define CubismLogInfo(fmt, ...)       CubismLogPrintln(Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Info,    "[I]" fmt, ## __VA_ARGS__)
#define CubismLogWarning(fmt, ...)    CubismLogPrintln(Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Warning, "[W]" fmt, ## __VA_ARGS__)
#define CubismLogError(fmt, ...)      CubismLogPrintln(Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Error,   "[E]" fmt, ## __VA_ARGS__)
#elif CSM_LOG_LEVEL == CSM_LOG_LEVEL_WARNING
#define CubismLogVerbose(fmt, ...)
#define CubismLogDebug(fmt, ...)
#define CubismLogInfo(fmt, ...)
#define CubismLogWarning(fmt, ...)    CubismLogPrintln(Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Warning, "[W]" fmt, ## __VA_ARGS__)
#define CubismLogError(fmt, ...)      CubismLogPrintln(Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Error,   "[E]" fmt, ## __VA_ARGS__)
#elif CSM_LOG_LEVEL == CSM_LOG_LEVEL_ERROR
#define CubismLogVerbose(fmt, ...)
#define CubismLogDebug(fmt, ...)
#define CubismLogInfo(fmt, ...)
#define CubismLogWarning(fmt, ...)
#define CubismLogError(fmt, ...)      CubismLogPrintln(Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Error,   "[E]" fmt, ## __VA_ARGS__)
#else
#define CubismLogVerbose(fmt, ...)
#define CubismLogDebug(fmt, ...)
#define CubismLogInfo(fmt, ...)
#define CubismLogWarning(fmt, ...)
#define CubismLogError(fmt, ...)
#endif

namespace Live2D { namespace Cubism { namespace Framework {
template<class T>
class csmVector;
}}}

namespace Live2D { namespace Cubism { namespace Framework { namespace Utils {

class CubismDebug
{
public:
    static void Print(CubismFramework::Option::LogLevel logLevel, const csmChar* format, ...);

    static void DumpBytes(CubismFramework::Option::LogLevel logLevel, const csmUint8* data, csmInt32 length);

private:

    CubismDebug();

};
}}}}

