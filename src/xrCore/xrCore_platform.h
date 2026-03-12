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

typedef uint32_t DWORD;
typedef uint32_t* PDWORD;
typedef uint32_t UINT;
typedef uint32_t UINT32;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef int32_t BOOL;
typedef void* HANDLE;
typedef void* HWND;
typedef void* HINSTANCE;
typedef const char* LPCSTR;
typedef char* LPSTR;
typedef void* LPVOID;
typedef void* PVOID;
typedef const void* LPCVOID;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef uintptr_t ULONG_PTR;
typedef uintptr_t DWORD_PTR;
typedef uintptr_t* PDWORD_PTR;
typedef int32_t HRESULT;
typedef void* HMODULE;

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    long long QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER* PLARGE_INTEGER;

typedef struct {
    long left, top, right, bottom;
} RECT;

typedef long long LRESULT;
typedef unsigned long long WPARAM;
typedef long long LPARAM;

#define S_OK ((HRESULT)0x00000000L)
#define E_FAIL ((HRESULT)0x80004005L)
#define E_NOTIMPL ((HRESULT)0x80004001L)
#define S_FALSE ((HRESULT)0x00000001L)
typedef uint64_t UINT64;
typedef int64_t INT64;

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
#define CALLBACK
#define APIENTRY WINAPI
#define _cdecl
#define __cdecl
#define __stdcall
#define __forceinline __attribute__((always_inline)) inline

#ifdef __cplusplus
#define __try try
#define __except(x) catch(...) if (false) { (void)(x); } else
#define __finally { }
#else
#define __try if(true)
#define __except(x) if(false)
#define __finally
#endif
#define GetExceptionCode() 0

#define DLL_PROCESS_ATTACH 1
#define DLL_THREAD_ATTACH 2
#define DLL_THREAD_DETACH 3
#define DLL_PROCESS_DETACH 0

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

    void Sleep(DWORD dwMilliseconds);
    DWORD GetTickCount();
    BOOL QueryPerformanceCounter(PLARGE_INTEGER lpPerformanceCount);
    BOOL QueryPerformanceFrequency(PLARGE_INTEGER lpFrequency);

    typedef void (*FARPROC)();
    HMODULE LoadLibraryA(LPCSTR lpLibFileName);
    #define LoadLibrary LoadLibraryA
    FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
    BOOL FreeLibrary(HMODULE hLibModule);
    HMODULE GetModuleHandleA(LPCSTR lpModuleName);
    #define GetModuleHandle GetModuleHandleA

    void TerminateProcess(HANDLE hProcess, UINT uExitCode);
    HANDLE GetCurrentProcess();
    void RaiseException(DWORD dwExceptionCode, DWORD dwExceptionFlags, DWORD nNumberOfArguments, const ULONG_PTR* lpArguments);

    typedef struct _SYSTEM_LOGICAL_PROCESSOR_INFORMATION {
        ULONG_PTR ProcessorMask;
        int Relationship;
        union {
            struct {
                BYTE Flags;
            } ProcessorCore;
            struct {
                DWORD NodeNumber;
            } NumaNode;
            struct {
                BYTE Type;
                BYTE Level;
                WORD LineSize;
                DWORD Size;
                int Associativity;
            } Cache;
            uint64_t Reserved[2];
        } DUMMYUNIONNAME;
    } SYSTEM_LOGICAL_PROCESSOR_INFORMATION, *PSYSTEM_LOGICAL_PROCESSOR_INFORMATION;

    BOOL GetLogicalProcessorInformation(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer, PDWORD ReturnedLength);
    BOOL GetProcessAffinityMask(HANDLE hProcess, PDWORD_PTR lpProcessAffinityMask, PDWORD_PTR lpSystemAffinityMask);

    typedef void (__cdecl *thread_start_t)(void *);
    uintptr_t _beginthread(thread_start_t start_address, unsigned stack_size, void *arglist);

    void timeBeginPeriod(UINT uPeriod);
    void timeEndPeriod(UINT uPeriod);

#ifdef __cplusplus
}
#endif

inline DWORD GetModuleFileName(HMODULE h, LPSTR p, DWORD s) {
    ssize_t len = readlink("/proc/self/exe", p, s - 1);
    if (len != -1) {
        p[len] = 0;
        return (DWORD)len;
    }
    p[0] = 0;
    return 0;
}
#define GetCurrentDirectory(s, b) (getcwd(b, s) ? (DWORD)strlen(b) : 0)
#define SetCurrentDirectory(b) (chdir(b) == 0)
#define GetUserName(b, s) (strcpy(b, "linux_user"), *s = 10, TRUE)
#define GetComputerName(b, s) (gethostname(b, *s) == 0 ? (*s = (DWORD)strlen(b), TRUE) : FALSE)
#define GetProcessHeap() ((HANDLE)1)
#define GetLastError() errno

#define GetCommandLine() "" // Will be handled in xrCore.cpp

#define RGB(r,g,b)          ((DWORD)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

#define MB_OK                       0x00000000L
#define MB_ICONERROR                0x00000010L
#define MB_SYSTEMMODAL              0x00001000L

inline int MessageBox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
    fprintf(stderr, "MessageBox: [%s] %s\n", lpCaption, lpText);
    return 1;
}

inline void DebugBreak() { abort(); }
#define IsDebuggerPresent() false

#define EXCEPTION_STACK_OVERFLOW 0xC00000FD
#define EXCEPTION_EXECUTE_HANDLER 1
#define EXCEPTION_CONTINUE_SEARCH 0
#define EXCEPTION_CONTINUE_EXECUTION -1

inline void _resetstkoflw() {}

#define SPI_GETSCREENSAVEACTIVE 0x0010
#define SPI_SETSCREENSAVEACTIVE 0x0011
#define SPI_GETSTICKYKEYS 0x003A
#define SPI_SETSTICKYKEYS 0x003B
#define SPI_GETFILTERKEYS 0x0032
#define SPI_SETFILTERKEYS 0x0033
#define SPI_GETTOGGLEKEYS 0x0034
#define SPI_SETTOGGLEKEYS 0x0035

#define SKF_AVAILABLE 0x00000002
#define FKF_AVAILABLE 0x00000002
#define TKF_AVAILABLE 0x00000002

typedef struct { DWORD cbSize; DWORD dwFlags; } STICKYKEYS, FILTERKEYS, TOGGLEKEYS;

inline BOOL SystemParametersInfo(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) { return TRUE; }

inline void PostQuitMessage(int nExitCode) { exit(nExitCode); }

#define CREATE_MUTEX_INITIAL_OWNER 0x00000001
inline HANDLE CreateMutex(void* lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName) { return (HANDLE)1; }

#define RelationProcessorCore 0

#endif

#endif
