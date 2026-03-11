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
typedef uint32_t UINT;
typedef uint32_t UINT32;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef int32_t BOOL;
typedef void* HANDLE;
typedef void* HWND;
typedef const char* LPCSTR;
typedef char* LPSTR;
typedef void* LPVOID;
typedef void* PVOID;
typedef const void* LPCVOID;
typedef int32_t LONG;
typedef intptr_t LONG_PTR;
typedef uint32_t ULONG;
typedef intptr_t INT_PTR;
typedef int32_t HRESULT;
typedef void* HMODULE;
typedef void* FARPROC;

typedef struct {
    long left, top, right, bottom;
} RECT;

typedef long long LRESULT;
typedef unsigned long long WPARAM;
typedef long long LPARAM;

#define S_OK ((HRESULT)0L)
#define S_FALSE ((HRESULT)1L)
#define E_FAIL ((HRESULT)0x80004005L)
#define E_NOTIMPL ((HRESULT)0x80004001L)
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
#define _cdecl
#define __cdecl
#define __stdcall
#define __forceinline __attribute__((always_inline)) inline

#ifndef __try
#define __try if(true)
#endif
#ifndef __catch
#define __catch(...) if(false)
#endif
#define __except(x) if(false)
#define GetExceptionCode() 0
#define EXCEPTION_EXECUTE_HANDLER 1
#define EXCEPTION_CONTINUE_SEARCH 0
#define EXCEPTION_CONTINUE_EXECUTION -1
#define EXCEPTION_STACK_OVERFLOW 0xC00000FD
#define _resetstkoflw()

#define DLL_PROCESS_ATTACH 1
#define DLL_THREAD_ATTACH 2
#define DLL_THREAD_DETACH 3
#define DLL_PROCESS_DETACH 0

#define WM_SYSKEYDOWN 0
#define WM_ACTIVATE 0
#define WM_SETCURSOR 0
#define WM_SYSCOMMAND 0
#define WM_CLOSE 0
#define WM_HOTKEY 0
#define WM_SYSCHAR 0
#define WM_CHAR 0
#define WM_INPUTLANGCHANGE 0
#define WM_DESTROY 0
#define WM_COMMAND 0
#define WM_INITDIALOG 0

#define SC_MOVE 0
#define SC_SIZE 0
#define SC_MAXIMIZE 0
#define SC_MONITORPOWER 0

#define IDCANCEL 0

#define _O_RDONLY O_RDONLY
#define _O_BINARY 0
#define _O_WRONLY O_WRONLY
#define _O_TRUNC O_TRUNC
#define _O_CREAT O_CREAT

#define _S_IREAD S_IRUSR
#define _S_IWRITE S_IWUSR

typedef struct {
    UINT  cbSize;
    DWORD dwFlags;
} STICKYKEYS, FILTERKEYS, TOGGLEKEYS;

#define SPI_GETSCREENSAVEACTIVE 0
#define SPI_SETSCREENSAVEACTIVE 1
#define SPI_GETSTICKYKEYS       2
#define SPI_SETSTICKYKEYS       3
#define SPI_GETFILTERKEYS       4
#define SPI_SETFILTERKEYS       5
#define SPI_GETTOGGLEKEYS       6
#define SPI_SETTOGGLEKEYS       7

#define SKF_AVAILABLE 0x00000002
#define FKF_AVAILABLE 0x00000002
#define TKF_AVAILABLE 0x00000002

#define SH_DENYNO 0
#define SH_DENYWR 0

#define CopyMemory(dest, src, len) memcpy((dest), (src), (len))
#define ZeroMemory(dest, len) memset((dest), 0, (len))
#define FillMemory(dest, len, val) memset((dest), (val), (len))
#define MoveMemory(dest, src, len) memmove((dest), (src), (len))

#define stricmp strcasecmp
#define _copysign copysign
#define _vsprintf vsprintf
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

    HMODULE LoadLibraryA(LPCSTR lpLibFileName);
    HMODULE LoadLibrary(LPCSTR lpLibFileName);
    FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
    BOOL FreeLibrary(HMODULE hLibModule);

    void Sleep(DWORD dwMilliseconds);
    DWORD GetTickCount();
    DWORD GetCurrentProcessId();
    BOOL IsDebuggerPresent();
    HMODULE GetModuleHandle(LPCSTR lpModuleName);
    DWORD GetModuleFileName(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
    LPCSTR GetCommandLineA();

    BOOL SystemParametersInfo(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);

    LONG InterlockedExchange(LONG volatile* Target, LONG Value);
    LONG InterlockedIncrement(LONG volatile* Addend);
    LONG InterlockedDecrement(LONG volatile* Addend);
    LONG InterlockedCompareExchange(LONG volatile* Destination, LONG ExChange, LONG Comperand);
    PVOID InterlockedCompareExchangePointer(PVOID volatile* Destination, PVOID ExChange, PVOID Comperand);

#ifdef __cplusplus
}
#endif

#define GetModuleHandleA GetModuleHandle
#define GetModuleFileNameA GetModuleFileName
#define GetCommandLine GetCommandLineA

#define GetCurrentProcess() ((HANDLE)(intptr_t)-2)
inline void TerminateProcess(HANDLE hProcess, UINT uExitCode) { _exit(uExitCode); }

#define GetCurrentDirectory(s, b) (getcwd(b, s) ? (DWORD)strlen(b) : 0)
#define SetCurrentDirectory(b) (chdir(b) == 0)
#define GetUserName(b, s) (strcpy(b, "linux_user"), *s = 11, TRUE)
#define GetComputerName(b, s) (gethostname(b, *s) == 0 ? (*s = (DWORD)strlen(b), TRUE) : FALSE)
#define GetProcessHeap() (void*)0
#define GetLastError() errno

#define _InterlockedExchange(t, v) InterlockedExchange((LONG volatile*)t, (LONG)v)
#define _InterlockedIncrement(a) InterlockedIncrement((LONG volatile*)a)
#define _InterlockedDecrement(a) InterlockedDecrement((LONG volatile*)a)
#define _InterlockedCompareExchange(d, e, c) InterlockedCompareExchange((LONG volatile*)d, (LONG)e, (LONG)c)

#define RGB(r,g,b)          ((DWORD)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

#define SWP_SHOWWINDOW 0
#define SWP_HIDEWINDOW 0
#define SWP_NOZORDER   0
#define SWP_NOMOVE     0
#define SWP_NOSIZE     0
#define SWP_FRAMECHANGED 0
#define SW_SHOW        0
#define SW_HIDE        0
#define WS_EX_TOPMOST  0
#define WS_BORDER      0
#define WS_DLGFRAME    0
#define WS_VISIBLE     0
#define WS_POPUP       0
#define GWL_STYLE      0
#define GCL_HICON      0
#define GCL_HICONSM    0
#define CW_USEDEFAULT  0
#define IDI_ICON1      0
#define IDC_ARROW      0
#define BLACK_BRUSH    0
#define IDD_STARTUP    0
#define IDC_STATIC_LOGO 0
#define IDC_STOP       0
#define IDC_DEBUG      0
#define IDD_STOP       0
#define IDC_DESC       0
#define IDC_FILE       0
#define IDC_LINE       0
#define MB_OK          0
#define MB_ICONERROR   0
#define MB_SYSTEMMODAL 0
#define MB_YESNO       0
#define MB_ICONQUESTION 0
#define IDYES          1
#define IDNO           0

#define HWND_TOPMOST   ((HWND)1)
#define HWND_NOTOPMOST ((HWND)-1)

typedef struct {
    UINT style;
    void* lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    HANDLE hInstance;
    HANDLE hIcon;
    HANDLE hCursor;
    HANDLE hbrBackground;
    LPCSTR lpszMenuName;
    LPCSTR lpszClassName;
} WNDCLASS;

typedef struct {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
} PROCESS_INFORMATION;

typedef struct {
    DWORD cb;
    LPSTR lpReserved;
    LPSTR lpDesktop;
    LPSTR lpTitle;
    DWORD dwX;
    DWORD dwY;
    DWORD dwXSize;
    DWORD dwYSize;
    DWORD dwXCountChars;
    DWORD dwYCountChars;
    DWORD dwFillAttribute;
    DWORD dwFlags;
    WORD wShowWindow;
    WORD cbReserved2;
    LPVOID lpReserved2;
    HANDLE hStdInput;
    HANDLE hStdOutput;
    HANDLE hStdError;
} STARTUPINFO;

typedef int (CALLBACK* DLGPROC)(HWND, UINT, WPARAM, LPARAM);
typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

inline HWND CreateDialog(HANDLE h, LPCSTR n, HWND p, DLGPROC d) { return NULL; }
inline HANDLE GetDlgItem(HWND h, int i) { return NULL; }
inline BOOL GetWindowRect(HWND h, RECT* r) { return FALSE; }
inline BOOL SetWindowPos(HWND h, HWND a, int x, int y, int w, int h2, UINT f) { return FALSE; }
inline BOOL UpdateWindow(HWND h) { return FALSE; }
inline BOOL DestroyWindow(HWND h) { return FALSE; }
inline BOOL ShowWindow(HWND h, int c) { return FALSE; }
inline BOOL SetWindowText(HANDLE h, LPCSTR t) { return FALSE; }
inline void PostQuitMessage(int c) {}
inline WORD RegisterClass(const WNDCLASS* c) { return 0; }
inline HANDLE LoadIcon(HANDLE h, LPCSTR n) { return NULL; }
inline HANDLE LoadCursor(HANDLE h, LPCSTR n) { return NULL; }
inline HANDLE GetStockObject(int i) { return NULL; }
inline HWND CreateWindowEx(DWORD e, LPCSTR c, LPCSTR t, DWORD s, int x, int y, int w, int h, HWND p, HANDLE m, HANDLE i, LPVOID lp) { return NULL; }
inline LONG GetWindowLong(HWND h, int i) { return 0; }
inline BOOL AdjustWindowRect(RECT* r, DWORD s, BOOL m) { return FALSE; }
inline BOOL SetRect(RECT* r, int l, int t, int ri, int b) { if(r){r->left=l;r->top=t;r->right=ri;r->bottom=b;} return TRUE; }
inline INT_PTR DialogBox(HANDLE h, LPCSTR n, HWND p, DLGPROC d) { return 0; }
inline BOOL EndDialog(HWND h, INT_PTR r) { return FALSE; }
inline LRESULT DefWindowProc(HWND h, UINT m, WPARAM w, LPARAM l) { return 0; }
inline int MessageBox(HWND h, LPCSTR t, LPCSTR c, UINT ty) { printf("MessageBox: %s - %s\n", c, t); return 0; }
inline HANDLE CreateMutex(void* a, BOOL b, LPCSTR n) { return (HANDLE)1; }
inline BOOL CreateProcess(LPCSTR n, LPSTR c, void* p, void* t, BOOL h, DWORD f, void* e, LPCSTR d, STARTUPINFO* s, PROCESS_INFORMATION* pi) { return FALSE; }

#define MAKEINTRESOURCE(i) (LPCSTR)((intptr_t)(i))

inline LONG SetWindowLong(HWND h, int i, LONG l) { return 0; }
inline BOOL GetClientRect(HWND h, RECT* r) { if(r) memset(r, 0, sizeof(RECT)); return TRUE; }
inline BOOL SetForegroundWindow(HWND h) { return TRUE; }
inline HWND GetForegroundWindow() { return NULL; }
inline BOOL IsIconic(HWND h) { return FALSE; }
inline BOOL SetClassLong(HWND h, int i, LONG l) { return 0; }

#endif

#endif
