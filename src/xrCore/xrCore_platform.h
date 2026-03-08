#ifndef XRCORE_PLATFORM_H
#define XRCORE_PLATFORM_H
#pragma once

#ifdef _WIN32
#define VC_EXTRALEAN // Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#ifndef STRICT
# define STRICT // Enable strict syntax
#endif // STRICT
#define IDIRECTPLAY2_OR_GREATER // ?
#define DIRECTINPUT_VERSION 0x0800 //
#define _CRT_SECURE_NO_DEPRECATE // vc8.0 stuff, don't deprecate several ANSI functions

// windows.h
#undef _WIN32_WINNT

#ifndef _WIN32_WINNT
#ifdef _MSC_VER
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#else // ifdef _MSC_VER
#define _WIN32_WINNT 0x0501
#endif // ifdef _MSC_VER
#endif // ifndef _WIN32_WINNT

#ifdef __BORLANDC__
#include <vcl.h>
#include <mmsystem.h>
#include <stdint.h>
#endif

#define NOGDICAPMASKS
//#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NODRAWTEXT
#define NOMEMMGR
#define NOMETAFILE
#define NOSERVICE
#define NOCOMM
#define NOHELP
#define NOPROFILER
#define NOMCX
#define NOMINMAX
#define DOSWIN32
#define _WIN32_DCOM

#pragma warning(push)
#pragma warning(disable:4005)
#include <windows.h>
#ifndef __BORLANDC__
#include <windowsx.h>
#endif
#pragma warning(pop)

#elif defined(__linux__)
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <dlfcn.h>
#include <limits.h>
#include <sys/time.h>

typedef uint32_t DWORD;
typedef uint32_t UINT;
typedef uint32_t UINT32;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef int32_t BOOL;
typedef void* HANDLE;
typedef void* HWND;
typedef void* HDC;
typedef void* HICON;
typedef void* HCURSOR;
typedef void* HBRUSH;
typedef void* HMENU;
typedef const char* LPCSTR;
typedef char* LPSTR;
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef long LONG;
typedef unsigned long ULONG;
typedef uintptr_t ULONG_PTR;
typedef int32_t HRESULT;
typedef void* HMODULE;
typedef HMODULE HINSTANCE;
typedef void* FARPROC;
typedef intptr_t INT_PTR;

typedef struct {
    long left, top, right, bottom;
} RECT, *PRECT, *LPRECT;

typedef struct {
    long x, y;
} POINT, *PPOINT, *LPPOINT;

typedef long long LRESULT;
typedef unsigned long long WPARAM;
typedef long long LPARAM;

#define S_OK 0
#define E_FAIL ((HRESULT)0x80004005L)
#define E_NOTIMPL ((HRESULT)0x80004001L)
#define S_FALSE 1
typedef uint64_t UINT64;
typedef int64_t INT64;

#define LOWORD(l) ((uint16_t)(((uintptr_t)(l)) & 0xffff))
#define HIWORD(l) ((uint16_t)((((uintptr_t)(l)) >> 16) & 0xffff))

typedef int errno_t;

#define _strtoui64 strtoull
#define _atoi64 atoll
#define _strlwr strlwr

inline void strncpy_s(char* dest, size_t dest_sz, const char* src, size_t count) {
    size_t n = count < dest_sz ? count : dest_sz - 1;
    strncpy(dest, src, n);
    dest[n] = 0;
}

inline void _ui64toa_s(uint64_t val, char* dest, size_t dest_sz, int radix) {
    if (radix == 10) snprintf(dest, dest_sz, "%llu", (unsigned long long)val);
    else if (radix == 16) snprintf(dest, dest_sz, "%llx", (unsigned long long)val);
}

inline void _i64toa_s(int64_t val, char* dest, size_t dest_sz, int radix) {
    if (radix == 10) snprintf(dest, dest_sz, "%lld", (long long)val);
    else if (radix == 16) snprintf(dest, dest_sz, "%llx", (long long)val);
}

typedef pthread_mutex_t CRITICAL_SECTION;
typedef void* SRWLOCK;

#define _WINDOWS_
#define TRUE 1
#define FALSE 0
#ifndef NULL
#define NULL 0
#endif

#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef _MAX_PATH
#define _MAX_PATH MAX_PATH
#endif

#define GENERIC_READ 0x80000000
#define GENERIC_WRITE 0x40000000
#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define OPEN_EXISTING 3
#define CREATE_ALWAYS 2
#define TRUNCATE_EXISTING 5

#define PAGE_READONLY 0x02
#define PAGE_READWRITE 0x04

#define FILE_MAP_READ 0x0004
#define FILE_MAP_WRITE 0x0002
#define FILE_MAP_ALL_ACCESS (FILE_MAP_WRITE | FILE_MAP_READ)

#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2

#define STDMETHODCALLTYPE
#define WINAPI
#define APIENTRY WINAPI
#define CALLBACK
#define _cdecl
#define __cdecl
#define __stdcall
#define __forceinline __attribute__((always_inline)) inline

inline char* itoa(int value, char* str, int base) {
    if (base == 10) sprintf(str, "%d", value);
    else if (base == 16) sprintf(str, "%x", value);
    else if (base == 8) sprintf(str, "%o", value);
    return str;
}
#define _itoa itoa

inline void Sleep(uint32_t ms) {
    usleep(ms * 1000);
}

inline uint32_t GetTickCount() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

inline long InterlockedIncrement(long volatile* addend) {
    return __sync_add_and_fetch(addend, 1);
}

inline long InterlockedDecrement(long volatile* addend) {
    return __sync_sub_and_fetch(addend, 1);
}

inline long InterlockedExchange(long volatile* target, long value) {
    return __sync_lock_test_and_set(target, value);
}

inline void* InterlockedCompareExchangePointer(void* volatile* destination, void* exchange, void* comparand) {
    return __sync_val_compare_and_swap(destination, comparand, exchange);
}

#define DLL_PROCESS_ATTACH 1
#define DLL_THREAD_ATTACH 2
#define DLL_THREAD_DETACH 3
#define DLL_PROCESS_DETACH 0

#define __try try
#define __except(x) catch(...)
#define RaiseException(...)
#define EXCEPTION_EXECUTE_HANDLER 1
#define EXCEPTION_CONTINUE_EXECUTION -1

#define _O_RDONLY O_RDONLY
#define _O_BINARY 0
#define _O_WRONLY O_WRONLY
#define _O_TRUNC O_TRUNC
#define _O_CREAT O_CREAT

#define _S_IREAD S_IRUSR
#define _S_IWRITE S_IWUSR

#define SH_DENYNO 0
#define SH_DENYWR 0

#define CopyMemory(dest, src, len) memcpy((dest), (src), (len))
#define ZeroMemory(dest, len) memset((dest), 0, (len))
#define FillMemory(dest, len, val) memset((dest), (val), (len))

#define stricmp strcasecmp
#define _copysign copysign
#define _vsnprintf vsnprintf

inline char* strlwr(char* s) {
    char* p = s;
    while (*p) {
        *p = (char)tolower(*p);
        p++;
    }
    return s;
}

inline void _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext) {
    if (drive) *drive = 0;
    const char* last_slash = strrchr(path, '/');
    const char* last_backslash = strrchr(path, '\\');
    const char* separator = last_slash > last_backslash ? last_slash : last_backslash;

    if (!separator) {
        if (dir) *dir = 0;
        const char* dot = strrchr(path, '.');
        if (dot) {
            if (fname) {
                strncpy(fname, path, (size_t)(dot - path));
                fname[dot - path] = 0;
            }
            if (ext) strcpy(ext, dot);
        } else {
            if (fname) strcpy(fname, path);
            if (ext) *ext = 0;
        }
    } else {
        if (dir) {
            strncpy(dir, path, (size_t)(separator - path + 1));
            dir[separator - path + 1] = 0;
        }
        const char* rest = separator + 1;
        const char* dot = strrchr(rest, '.');
        if (dot) {
            if (fname) {
                strncpy(fname, rest, (size_t)(dot - rest));
                fname[dot - rest] = 0;
            }
            if (ext) strcpy(ext, dot);
        } else {
            if (fname) strcpy(fname, rest);
            if (ext) *ext = 0;
        }
    }
}

inline void _splitpath_s(const char* path, char* drive, size_t drive_sz, char* dir, size_t dir_sz, char* fname, size_t fname_sz, char* ext, size_t ext_sz) {
    _splitpath(path, drive, dir, fname, ext);
}

// Function wrappers to be implemented in xrCore.cpp
#ifdef __cplusplus
extern "C" {
#endif
    DWORD GetCurrentThreadId();
    HANDLE CreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, void* lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
    BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, DWORD* lpNumberOfBytesRead, void* lpOverlapped);
    BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, DWORD* lpNumberOfBytesWritten, void* lpOverlapped);
    BOOL CloseHandle(HANDLE hObject);
    DWORD GetFileSize(HANDLE hFile, DWORD* lpFileSizeHigh);
    DWORD SetFilePointer(HANDLE hFile, long lDistanceToMove, long* lpDistanceToMoveHigh, DWORD dwMoveMethod);
    HANDLE CreateFileMapping(HANDLE hFile, void* lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName);
    LPVOID MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, size_t dwNumberOfBytesToMap);
    BOOL UnmapViewOfFile(LPCVOID lpBaseAddress);

    void OutputDebugString(LPCSTR lpOutputString);
    void OutputDebugStringA(LPCSTR lpOutputString);
    BOOL IsDebuggerPresent();

    // Dynamic library loading wrappers for Linux
    HMODULE LoadLibraryA(LPCSTR lpLibFileName);
    HMODULE LoadLibrary(LPCSTR lpLibFileName);
    FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
    BOOL FreeLibrary(HMODULE hLibModule);

    // Critical Section and SRW Lock wrappers for Linux
    void InitializeCriticalSection(CRITICAL_SECTION* lpCriticalSection);
    void DeleteCriticalSection(CRITICAL_SECTION* lpCriticalSection);
    void EnterCriticalSection(CRITICAL_SECTION* lpCriticalSection);
    void LeaveCriticalSection(CRITICAL_SECTION* lpCriticalSection);
    BOOL TryEnterCriticalSection(CRITICAL_SECTION* lpCriticalSection);

    void InitializeSRWLock(SRWLOCK* SRWLock);
    void AcquireSRWLockExclusive(SRWLOCK* SRWLock);
    void ReleaseSRWLockExclusive(SRWLOCK* SRWLock);
    void AcquireSRWLockShared(SRWLOCK* SRWLock);
    void ReleaseSRWLockShared(SRWLOCK* SRWLock);
    BOOL TryAcquireSRWLockExclusive(SRWLOCK* SRWLock);
    BOOL TryAcquireSRWLockShared(SRWLOCK* SRWLock);
    void DeleteSRWLock(SRWLOCK* SRWLock);

#ifdef __cplusplus
}
#endif

#define GetModuleHandle(x) (void*)0
#define GetModuleFileName(h, p, s) (p[0]=0, 0)
#define GetModuleFileNameA GetModuleFileName
#define GetCurrentDirectory(s, b) (getcwd(b, s) ? (DWORD)strlen(b) : 0)
#define SetCurrentDirectory(b) (chdir(b) == 0)
#define GetUserName(b, s) (strcpy(b, "linux_user"), *s = 10, TRUE)
#define GetComputerName(b, s) (gethostname(b, *s) == 0 ? (*s = (DWORD)strlen(b), TRUE) : FALSE)
#define GetProcessHeap() (void*)0
#define GetCurrentProcess() ((HANDLE)0)
#define GetCurrentProcessId() ((DWORD)getpid())
#define GetCurrentThread() ((HANDLE)pthread_self())
#define GetLastError() errno

#define TerminateProcess(h, c) _exit(c)

inline void _beginthread(void (*entry)(void*), unsigned stack, void* arglist) {
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (stack > 0) pthread_attr_setstacksize(&attr, stack);
    pthread_create(&thread, &attr, (void* (*)(void*))entry, arglist);
    pthread_attr_destroy(&attr);
    pthread_detach(thread);
}

#define _clear87()
#define _control87(a, b) 0
#define MCW_PC 0
#define MCW_RC 0
#define _PC_64 0
#define _PC_53 0
#define _RC_CHOP 0
#define _RC_NEAR 0

#define GetCommandLine() "" // Will be handled in xrCore.cpp

#define RGB(r,g,b)          ((DWORD)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

#endif

#endif
