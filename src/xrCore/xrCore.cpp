// xrCore.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#pragma hdrstop

#ifdef _WIN32
#include <mmsystem.h>
#include <objbase.h>
#endif

#include "xrCore.h"

#ifdef _WIN32
#pragma comment(lib,"winmm.lib")
#endif

#ifdef DEBUG
# include <malloc.h>
#endif // DEBUG

#include<fstream>
#include <iostream>
#include <string>

#ifdef __linux__
#include <map>
#include <mutex>
#include <dlfcn.h>
#include <time.h>

static std::map<const void*, size_t> g_mapping_sizes;
static xrCriticalSection g_mapping_mutex;

extern "C" {
    HANDLE CreateFile(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, void* lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
        char normalized_path[MAX_PATH];
        xr_strcpy(normalized_path, lpFileName);
        for (char* p = normalized_path; *p; p++) if (*p == '\\') *p = '/';

        int flags = 0;
        if ((dwDesiredAccess & GENERIC_READ) && (dwDesiredAccess & GENERIC_WRITE)) flags = O_RDWR;
        else if (dwDesiredAccess & GENERIC_WRITE) flags = O_WRONLY;
        else flags = O_RDONLY;

        if (dwCreationDisposition == CREATE_ALWAYS) flags |= O_CREAT | O_TRUNC;
        else if (dwCreationDisposition == OPEN_EXISTING) {}
        else if (dwCreationDisposition == TRUNCATE_EXISTING) flags |= O_TRUNC;

        int fd = open(normalized_path, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd == -1) return INVALID_HANDLE_VALUE;
        return (HANDLE)(intptr_t)fd;
    }

    BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, DWORD* lpNumberOfBytesRead, void* lpOverlapped) {
        ssize_t res = read((int)(intptr_t)hFile, lpBuffer, nNumberOfBytesToRead);
        if (res == -1) return FALSE;
        if (lpNumberOfBytesRead) *lpNumberOfBytesRead = (DWORD)res;
        return TRUE;
    }

    BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, DWORD* lpNumberOfBytesWritten, void* lpOverlapped) {
        ssize_t res = write((int)(intptr_t)hFile, lpBuffer, nNumberOfBytesToWrite);
        if (res == -1) return FALSE;
        if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = (DWORD)res;
        return TRUE;
    }

    BOOL CloseHandle(HANDLE hObject) {
        return close((int)(intptr_t)hObject) == 0;
    }

    DWORD GetFileSize(HANDLE hFile, DWORD* lpFileSizeHigh) {
        struct stat st;
        if (fstat((int)(intptr_t)hFile, &st) == -1) return (DWORD)-1;
        if (lpFileSizeHigh) *lpFileSizeHigh = (DWORD)((uint64_t)st.st_size >> 32);
        return (DWORD)st.st_size;
    }

    DWORD SetFilePointer(HANDLE hFile, LONG lDistanceToMove, LONG* lpDistanceToMoveHigh, DWORD dwMoveMethod) {
        int64_t offset = (uint32_t)lDistanceToMove;
        if (lpDistanceToMoveHigh) offset |= ((int64_t)*lpDistanceToMoveHigh << 32);
        int whence = SEEK_SET;
        if (dwMoveMethod == 1) whence = SEEK_CUR;
        else if (dwMoveMethod == 2) whence = SEEK_END;
        off_t res = lseek((int)(intptr_t)hFile, (off_t)offset, whence);
        if (res == (off_t)-1) return (DWORD)-1;
        if (lpDistanceToMoveHigh) *lpDistanceToMoveHigh = (LONG)((uint64_t)res >> 32);
        return (DWORD)res;
    }

    HANDLE CreateFileMapping(HANDLE hFile, void* lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName) {
        return hFile; // On POSIX we can just use the fd
    }

    LPVOID MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, size_t dwNumberOfBytesToMap) {
        int prot = PROT_READ;
        if (dwDesiredAccess & FILE_MAP_WRITE) prot |= PROT_WRITE;
        off_t offset = ((off_t)dwFileOffsetHigh << 32) | dwFileOffsetLow;
        if (dwNumberOfBytesToMap == 0) {
            struct stat st;
            fstat((int)(intptr_t)hFileMappingObject, &st);
            dwNumberOfBytesToMap = st.st_size - offset;
        }
        void* addr = mmap(NULL, dwNumberOfBytesToMap, prot, MAP_SHARED, (int)(intptr_t)hFileMappingObject, offset);
        if (addr == MAP_FAILED) return NULL;

        xrCriticalSectionGuard guard(g_mapping_mutex);
        g_mapping_sizes[addr] = dwNumberOfBytesToMap;

        return addr;
    }

    BOOL UnmapViewOfFile(LPCVOID lpBaseAddress) {
        xrCriticalSectionGuard guard(g_mapping_mutex);
        auto it = g_mapping_sizes.find(lpBaseAddress);
        if (it != g_mapping_sizes.end()) {
            size_t size = it->second;
            g_mapping_sizes.erase(it);
            return munmap((void*)lpBaseAddress, size) == 0;
        }
        return FALSE;
    }

    void DeleteSRWLock(SRWLOCK* SRWLock) {
        if (*SRWLock) {
            pthread_rwlock_destroy((pthread_rwlock_t*)*SRWLock);
            xr_free(*SRWLock);
            *SRWLock = NULL;
        }
    }

    DWORD GetCurrentThreadId() {
        return (DWORD)pthread_self();
    }

    void InitializeCriticalSection(CRITICAL_SECTION* lpCriticalSection) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(lpCriticalSection, &attr);
        pthread_mutexattr_destroy(&attr);
    }

    void DeleteCriticalSection(CRITICAL_SECTION* lpCriticalSection) {
        pthread_mutex_destroy(lpCriticalSection);
    }

    void EnterCriticalSection(CRITICAL_SECTION* lpCriticalSection) {
        pthread_mutex_lock(lpCriticalSection);
    }

    void LeaveCriticalSection(CRITICAL_SECTION* lpCriticalSection) {
        pthread_mutex_unlock(lpCriticalSection);
    }

    BOOL TryEnterCriticalSection(CRITICAL_SECTION* lpCriticalSection) {
        return pthread_mutex_trylock(lpCriticalSection) == 0;
    }

    void InitializeSRWLock(SRWLOCK* SRWLock) {
        pthread_rwlock_t* lock = (pthread_rwlock_t*)xr_malloc(sizeof(pthread_rwlock_t));
        pthread_rwlock_init(lock, NULL);
        *SRWLock = lock;
    }

    void AcquireSRWLockExclusive(SRWLOCK* SRWLock) {
        pthread_rwlock_wrlock((pthread_rwlock_t*)*SRWLock);
    }

    void ReleaseSRWLockExclusive(SRWLOCK* SRWLock) {
        pthread_rwlock_unlock((pthread_rwlock_t*)*SRWLock);
    }

    void AcquireSRWLockShared(SRWLOCK* SRWLock) {
        pthread_rwlock_rdlock((pthread_rwlock_t*)*SRWLock);
    }

    void ReleaseSRWLockShared(SRWLOCK* SRWLock) {
        pthread_rwlock_unlock((pthread_rwlock_t*)*SRWLock);
    }

    BOOL TryAcquireSRWLockExclusive(SRWLOCK* SRWLock) {
        return pthread_rwlock_trywrlock((pthread_rwlock_t*)*SRWLock) == 0;
    }

    BOOL TryAcquireSRWLockShared(SRWLOCK* SRWLock) {
        return pthread_rwlock_tryrdlock((pthread_rwlock_t*)*SRWLock) == 0;
    }

    void Sleep(DWORD dwMilliseconds) {
        usleep(dwMilliseconds * 1000);
    }

    DWORD GetTickCount() {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (DWORD)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    }

    BOOL QueryPerformanceCounter(PLARGE_INTEGER lpPerformanceCount) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        lpPerformanceCount->QuadPart = (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
        return TRUE;
    }

    BOOL QueryPerformanceFrequency(PLARGE_INTEGER lpFrequency) {
        lpFrequency->QuadPart = 1000000000LL;
        return TRUE;
    }

    HMODULE LoadLibraryA(LPCSTR lpLibFileName) {
        char normalized_path[MAX_PATH];
        xr_strcpy(normalized_path, lpLibFileName);
        for (char* p = normalized_path; *p; p++) if (*p == '\\') *p = '/';

        // Replace .dll with .so
        char* ext = strstr(normalized_path, ".dll");
        if (ext) strcpy(ext, ".so");

        void* handle = dlopen(normalized_path, RTLD_NOW | RTLD_GLOBAL);
        if (!handle) {
            // Try with lib prefix
            char lib_path[MAX_PATH + 4];
            const char* last_slash = strrchr(normalized_path, '/');
            if (last_slash) {
                int dir_len = last_slash - normalized_path + 1;
                strncpy(lib_path, normalized_path, dir_len);
                strcpy(lib_path + dir_len, "lib");
                strcpy(lib_path + dir_len + 3, last_slash + 1);
            } else {
                strcpy(lib_path, "lib");
                strcpy(lib_path + 3, normalized_path);
            }
            handle = dlopen(lib_path, RTLD_NOW | RTLD_GLOBAL);
        }
        return (HMODULE)handle;
    }

    FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
        return (FARPROC)dlsym(hModule, lpProcName);
    }

    BOOL FreeLibrary(HMODULE hLibModule) {
        return dlclose(hLibModule) == 0;
    }

    HMODULE GetModuleHandleA(LPCSTR lpModuleName) {
        if (!lpModuleName) return dlopen(NULL, RTLD_NOW);

        char normalized_path[MAX_PATH];
        xr_strcpy(normalized_path, lpModuleName);
        for (char* p = normalized_path; *p; p++) if (*p == '\\') *p = '/';
        char* ext = strstr(normalized_path, ".dll");
        if (ext) strcpy(ext, ".so");

        void* handle = dlopen(normalized_path, RTLD_NOLOAD | RTLD_NOW | RTLD_GLOBAL);
        if (!handle) {
            char lib_path[MAX_PATH + 4];
            const char* last_slash = strrchr(normalized_path, '/');
            if (last_slash) {
                int dir_len = last_slash - normalized_path + 1;
                strncpy(lib_path, normalized_path, dir_len);
                strcpy(lib_path + dir_len, "lib");
                strcpy(lib_path + dir_len + 3, last_slash + 1);
            } else {
                strcpy(lib_path, "lib");
                strcpy(lib_path + 3, normalized_path);
            }
            handle = dlopen(lib_path, RTLD_NOLOAD | RTLD_NOW | RTLD_GLOBAL);
        }
        return (HMODULE)handle;
    }

    void TerminateProcess(HANDLE hProcess, UINT uExitCode) {
        exit(uExitCode);
    }

    HANDLE GetCurrentProcess() {
        return (HANDLE)1;
    }

    void RaiseException(DWORD dwExceptionCode, DWORD dwExceptionFlags, DWORD nNumberOfArguments, const ULONG_PTR* lpArguments) {
        fprintf(stderr, "Exception Raised: 0x%08X\n", dwExceptionCode);
    }

    BOOL GetLogicalProcessorInformation(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer, PDWORD ReturnedLength) {
        int n_cores = sysconf(_SC_NPROCESSORS_ONLN);
        if (n_cores < 1) n_cores = 1;

        DWORD needed_size = n_cores * sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

        if (*ReturnedLength < needed_size) {
            *ReturnedLength = needed_size;
            return FALSE;
        }

        memset(Buffer, 0, needed_size);
        for (int i = 0; i < n_cores; i++) {
            Buffer[i].Relationship = RelationProcessorCore;
            Buffer[i].ProcessorMask = (ULONG_PTR)1 << i;
        }

        *ReturnedLength = needed_size;
        return TRUE;
    }

    BOOL GetProcessAffinityMask(HANDLE hProcess, PDWORD_PTR lpProcessAffinityMask, PDWORD_PTR lpSystemAffinityMask) {
        *lpProcessAffinityMask = 0x1;
        *lpSystemAffinityMask = 0x1;
        return TRUE;
    }

    struct thread_data {
        thread_start_t fn;
        void* arg;
    };

    static void* thread_wrapper(void* args) {
        thread_data* data = (thread_data*)args;
        thread_start_t fn = data->fn;
        void* arg = data->arg;
        free(data);
        fn(arg);
        return NULL;
    }

    uintptr_t _beginthread(thread_start_t start_address, unsigned stack_size, void *arglist) {
        pthread_t thread;
        thread_data* data = (thread_data*)malloc(sizeof(thread_data));
        data->fn = start_address;
        data->arg = arglist;
        if (pthread_create(&thread, NULL, thread_wrapper, data) != 0) {
            free(data);
            return (uintptr_t)-1;
        }
        return (uintptr_t)thread;
    }

    void timeBeginPeriod(UINT uPeriod) {}
    void timeEndPeriod(UINT uPeriod) {}
}
#endif

XRCORE_API xrCore Core;
extern XRCORE_API u32 build_id;
extern XRCORE_API LPCSTR build_date;

namespace CPU
{
	extern void Detect();
};

static u32 init_counter = 0;

//extern char g_application_path[256];

//. extern xr_vector<shared_str>* LogFile;

// demonized: print modded exes version
extern int get_modded_exes_version();
extern xr_string get_modded_exes_version_string();
extern LPCSTR get_modded_exes_name();
extern std::string timeInDMYHMSMMM();

void xrCore::_initialize(LPCSTR _ApplicationName, LogCallback cb, BOOL init_fs, LPCSTR fs_fname)
{
	xr_strcpy(ApplicationName, _ApplicationName);
	if (0 == init_counter)
	{
#if defined(XRCORE_STATIC) && defined(_WIN32)
        _clear87();
        _control87(_PC_53, MCW_PC);
        _control87(_RC_CHOP, MCW_RC);
        _control87(_RC_NEAR, MCW_RC);
        _control87(_MCW_EM, MCW_EM);
#endif
		// Init COM so we can use CoCreateInstance
		// HRESULT co_res =
#ifdef _WIN32
		Params = xr_strdup(GetCommandLine());
#elif defined(__linux__)
        int fd = open("/proc/self/cmdline", O_RDONLY);
        if (fd != -1) {
            char buf[2048];
            ssize_t n = read(fd, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = 0;
                // Command line is null-separated, replace with spaces
                for (int i = 0; i < n; i++) {
                    if (buf[i] == 0) buf[i] = ' ';
                }
                Params = xr_strdup(buf);
            } else {
                Params = xr_strdup("");
            }
            close(fd);
        } else {
            Params = xr_strdup("");
        }
#endif
		xr_strlwr(Params);
#ifdef _WIN32
		if (!strstr(Params, "-editor"))
			CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif

		string_path fn, dr, di;

		// application path
#ifdef _WIN32
		GetModuleFileName(GetModuleHandle(MODULE_NAME), fn, sizeof(fn));
#elif defined(__linux__)
        ssize_t len = readlink("/proc/self/exe", fn, sizeof(fn) - 1);
        if (len != -1) fn[len] = 0;
        else fn[0] = 0;
#endif
		_splitpath(fn, dr, di, 0, 0);
		strconcat(sizeof(ApplicationPath), ApplicationPath, dr, di);

#ifndef _EDITOR
		//        xr_strcpy(g_application_path, sizeof(g_application_path), ApplicationPath);
#endif

#ifdef _EDITOR
        // working path
        if (strstr(Params, "-wf"))
        {
            string_path c_name;
            sscanf(strstr(Core.Params, "-wf ") + 4, "%[^ ] ", c_name);
            SetCurrentDirectory(c_name);
        }
#endif

		GetCurrentDirectory(sizeof(WorkingPath), WorkingPath);

		// User/Comp Name
		DWORD sz_user = sizeof(UserName);
		GetUserName(UserName, &sz_user);

		DWORD sz_comp = sizeof(CompName);
#ifdef _WIN32
		GetComputerName(CompName, &sz_comp);
#else
        gethostname(CompName, sz_comp);
#endif

		// Mathematics & PSI detection
		CPU::Detect();

		Memory._initialize(strstr(Params, "-mem_debug") ? TRUE : FALSE);

		DUMP_PHASE;

		InitLog();
		_initialize_cpu();

		// Debug._initialize ();

		rtc_initialize();

		time_t _time = time(nullptr);
		tm* time = localtime(&_time);
		april1 = time ? (time->tm_mday == 1 && time->tm_mon == 3) : false;

		xr_FS = xr_new<CLocatorAPI>();

		xr_EFS = xr_new<EFS_Utils>();
		//. R_ASSERT (co_res==S_OK);

		//Load cmd line from file if it exists
		std::ifstream cmdlineTxt;
		char path_A[MAX_PATH];
		strcpy(path_A, Core.ApplicationPath);
		strcat(path_A, "/../commandline.txt");
		cmdlineTxt.open(path_A);
		
		if (!cmdlineTxt)
		{
			cmdlineTxt.close();
			strcpy(path_A, Core.WorkingPath);
			strcat(path_A, "/commandline.txt");
			cmdlineTxt.open(path_A);
		}

		if (cmdlineTxt)
		{
			Msg("Found commandline file!");
			std::string line;
			char temp[2048];
			xr_sprintf(temp, sizeof(temp), "%s", Params);
			strcat(temp, " ");
			while (std::getline(cmdlineTxt, line))
			{
				strcat(temp, line.c_str());
				strcat(temp, " ");
			}
			xr_free(Params);
			Params = xr_strdup(temp);
		}
		cmdlineTxt.close();
	}
	if (init_fs)
	{
		u32 flags = 0;
		if (0 != strstr(Params, "-build")) flags |= CLocatorAPI::flBuildCopy;
		if (0 != strstr(Params, "-ebuild")) flags |= CLocatorAPI::flBuildCopy | CLocatorAPI::flEBuildCopy;
#ifdef DEBUG
        if (strstr(Params, "-cache")) flags |= CLocatorAPI::flCacheFiles;
        else flags &= ~CLocatorAPI::flCacheFiles;
#endif // DEBUG
#ifdef _EDITOR // for EDITORS - no cache
        flags &= ~CLocatorAPI::flCacheFiles;
#endif // _EDITOR
		flags |= CLocatorAPI::flScanAppRoot;

#ifndef _EDITOR
#ifndef ELocatorAPIH
		if (0 != strstr(Params, "-file_activity")) flags |= CLocatorAPI::flDumpFileActivity;
#endif
#endif
		FS._initialize(flags, 0, fs_fname);
		Msg("'%s' build %d, %s\n", "xrCore", build_id, build_date);

		// demonized: Print modded exes version
		Msg("%s version %s\n", get_modded_exes_name(), get_modded_exes_version_string().c_str());
		Msg("Game started: %s\n", timeInDMYHMSMMM().c_str());
		EFS._initialize();
#ifdef DEBUG
#ifndef _EDITOR
        Msg("Process heap 0x%08x", GetProcessHeap());
#endif
#endif // DEBUG
	}
	SetLogCB(cb);
	init_counter++;
}

#ifndef _EDITOR
#include "compression_ppmd_stream.h"
extern compression::ppmd::stream* trained_model;
#endif
void xrCore::_destroy()
{
	--init_counter;
	if (0 == init_counter)
	{
		FS._destroy();
		EFS._destroy();
		xr_delete(xr_FS);
		xr_delete(xr_EFS);

#ifndef _EDITOR
		if (trained_model)
		{
			void* buffer = trained_model->buffer();
			xr_free(buffer);
			xr_delete(trained_model);
		}
#endif
		xr_free(Params);
		Memory._destroy();
	}
}

#ifndef XRCORE_STATIC

//. why ???
#ifdef _EDITOR
BOOL WINAPI DllEntryPoint(HINSTANCE hinstDLL, DWORD ul_reason_for_call, LPVOID lpvReserved)
#elif defined(_WIN32)
//BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD ul_reason_for_call, LPVOID lpvReserved)
BOOL DllMainXrCore(HANDLE hinstDLL, DWORD ul_reason_for_call, LPVOID lpvReserved)
#else
BOOL DllMainXrCore(HANDLE hinstDLL, DWORD ul_reason_for_call, LPVOID lpvReserved)
#endif
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
#ifdef _WIN32
			_clear87();
			_control87(_PC_53, MCW_PC);
			_control87(_RC_CHOP, MCW_RC);
			_control87(_RC_NEAR, MCW_RC);
			_control87(_MCW_EM, MCW_EM);
#endif
		}
		//. LogFile.reserve (256);
		break;
	case DLL_THREAD_ATTACH:
#ifdef _WIN32
		if (!strstr(GetCommandLine(), "-editor"))
			CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		timeBeginPeriod(1);
#endif
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
#ifdef USE_MEMORY_MONITOR
        memory_monitor::flush_each_time(true);
#endif // USE_MEMORY_MONITOR
		break;
	}
	return TRUE;
}
#endif // XRCORE_STATIC
