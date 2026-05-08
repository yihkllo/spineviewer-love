#include "sl_crash_dump.h"

#include <windows.h>
#include <dbghelp.h>
#include <cwchar>

#pragma comment(lib, "Dbghelp.lib")

namespace sl_crash_dump {
namespace {

bool CreateDirectories(const wchar_t* path)
{
	if (path == nullptr || path[0] == L'\0')
		return false;

	wchar_t buffer[MAX_PATH] = {};
	::lstrcpynW(buffer, path, MAX_PATH);

	for (wchar_t* p = buffer; *p; ++p)
	{
		if (*p != L'\\' && *p != L'/')
			continue;
		const wchar_t saved = *p;
		*p = L'\0';
		if (buffer[0] != L'\0' && ::GetFileAttributesW(buffer) == INVALID_FILE_ATTRIBUTES)
			::CreateDirectoryW(buffer, nullptr);
		*p = saved;
	}

	if (::GetFileAttributesW(buffer) == INVALID_FILE_ATTRIBUTES)
		return ::CreateDirectoryW(buffer, nullptr) != FALSE;
	return true;
}

LONG WINAPI HandleUnhandledException(EXCEPTION_POINTERS* exceptionPointers)
{
	wchar_t exePath[MAX_PATH] = {};
	::GetModuleFileNameW(nullptr, exePath, MAX_PATH);
	wchar_t* lastSlash = ::wcsrchr(exePath, L'\\');
	if (lastSlash != nullptr)
		*lastSlash = L'\0';

	wchar_t dumpDir[MAX_PATH] = {};
	::wsprintfW(dumpDir, L"%s\\..\\..\\..\\..\\crash_dumps", exePath);
	CreateDirectories(dumpDir);

	SYSTEMTIME time{};
	::GetSystemTime(&time);

	wchar_t dumpPath[MAX_PATH] = {};
	::wsprintfW(
		dumpPath,
		L"%s\\crash_%04u%02u%02u_%02u%02u%02u_%lu.dmp",
		dumpDir,
		time.wYear,
		time.wMonth,
		time.wDay,
		time.wHour,
		time.wMinute,
		time.wSecond,
		::GetCurrentProcessId());

	HANDLE file = ::CreateFileW(dumpPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION exceptionInfo{};
		exceptionInfo.ThreadId = ::GetCurrentThreadId();
		exceptionInfo.ExceptionPointers = exceptionPointers;
		exceptionInfo.ClientPointers = FALSE;

		const MINIDUMP_TYPE dumpType = static_cast<MINIDUMP_TYPE>(
			MiniDumpWithDataSegs |
			MiniDumpWithIndirectlyReferencedMemory |
			MiniDumpWithThreadInfo);
		::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), file, dumpType, &exceptionInfo, nullptr, nullptr);
		::CloseHandle(file);

		wchar_t message[MAX_PATH + 64] = {};
		::wsprintfW(message, L"[crash] dump written to: %s\n", dumpPath);
		::OutputDebugStringW(message);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

}

void Install()
{
	static bool installed = false;
	if (installed)
		return;
	installed = true;
	::SetUnhandledExceptionFilter(&HandleUnhandledException);
	::OutputDebugStringW(L"[crash_dump] handler installed\n");
}

}
