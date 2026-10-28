// Main.cpp - DLL entry point, log, and the functions that modify the game's code
// Language: C++17, MSVC, Win32.
#include "ROTKPlus.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>

static char             g_dir[MAX_PATH];
static CRITICAL_SECTION g_logCs;
static FILE*            g_log;        // this session's own file: ROTKPlus_logs\ROTKPlus_<date>_<time>.log
static FILE*            g_logLatest;  // ROTKPlus.log: a copy of the latest session, always in the same place
static bool             g_detail = true;
static DWORD            g_startTick;

const char* GameDir() { return g_dir; }
bool LogDetail() { return g_detail; }

void Log(const char* fmt, ...)
{
    if (!g_log && !g_logLatest) return;
    EnterCriticalSection(&g_logCs);
    SYSTEMTIME t; GetLocalTime(&t);
    char line[1024];
    int n = sprintf_s(line, "[%02d:%02d:%02d.%03d] ", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
    va_list ap; va_start(ap, fmt);
    const int room = (int)sizeof line - n - 2;
    const int m = vsnprintf(line + n, room, fmt, ap);
    va_end(ap);
    n += (m < 0 || m >= room) ? room - 1 : m;
    line[n++] = '\n';
    for (FILE* f : { g_log, g_logLatest }) if (f) { fwrite(line, 1, n, f); fflush(f); }
    LeaveCriticalSection(&g_logCs);
}

// Opens a new log file for this session without touching the earlier ones, and refreshes ROTKPlus.log.
static void OpenLogs()
{
    char dir[MAX_PATH], path[MAX_PATH];
    sprintf_s(dir, "%s\\ROTKPlus_logs", g_dir);
    CreateDirectoryA(dir, nullptr);
    SYSTEMTIME t; GetLocalTime(&t);
    for (int k = 0; k < 100 && !g_log; ++k) {
        char suffix[8] = ""; if (k) sprintf_s(suffix, "_%d", k + 1);
        sprintf_s(path, "%s\\ROTKPlus_%04d-%02d-%02d_%02d-%02d-%02d%s.log", dir, t.wYear, t.wMonth, t.wDay,
                  t.wHour, t.wMinute, t.wSecond, suffix);
        if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) continue;   // restarted within the same second
        fopen_s(&g_log, path, "w");
    }
    sprintf_s(path, "%s\\ROTKPlus.log", g_dir);
    fopen_s(&g_logLatest, path, "w");
}

// [Log] Keep=N: keeps only the N newest session files (0, the default, never deletes anything).
static void DeleteOldLogs(int keep)
{
    if (keep <= 0) return;
    char pattern[MAX_PATH];
    sprintf_s(pattern, "%s\\ROTKPlus_logs\\ROTKPlus_*.log", g_dir);
    std::vector<std::string> names;
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do names.push_back(fd.cFileName); while (FindNextFileA(h, &fd));
    FindClose(h);
    std::sort(names.begin(), names.end());   // the names start with the date: oldest first
    for (size_t i = 0; i + keep < names.size(); ++i) {
        char p[MAX_PATH]; sprintf_s(p, "%s\\ROTKPlus_logs\\%s", g_dir, names[i].c_str());
        DeleteFileA(p);
    }
}

// The first lines of every session: the setup, so that a report does not need to ask for it.
static void LogSessionHeader()
{
    SYSTEMTIME t; GetLocalTime(&t);
    Log("session start %04d-%02d-%02d %02d:%02d:%02d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    HMODULE nt = GetModuleHandleA("ntdll.dll");
    typedef const char*(__cdecl* WineVer)(void);
    typedef LONG(WINAPI* RtlVer)(OSVERSIONINFOW*);
    WineVer wine = nt ? (WineVer)GetProcAddress(nt, "wine_get_version") : nullptr;
    RtlVer rtl = nt ? (RtlVer)GetProcAddress(nt, "RtlGetVersion") : nullptr;
    OSVERSIONINFOW os = { sizeof os };
    if (rtl) rtl(&os);
    SYSTEM_INFO si; GetNativeSystemInfo(&si);
    if (wine) Log("system: Wine/Proton %s (reports Windows %lu.%lu build %lu), %lu logical CPUs", wine(),
                  os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber, si.dwNumberOfProcessors);
    else      Log("system: Windows %s (version %lu.%lu build %lu), %lu logical CPUs",
                  os.dwBuildNumber >= 22000 ? "11" : os.dwMajorVersion >= 10 ? "10" : "older than 10",
                  os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber, si.dwNumberOfProcessors);
    for (const char* name : { "ROTKFix.dll", "SDL3.dll", "d3d8.dll", "dinput8.dll", "DivxMediaLib_orig.dll" }) {
        char p[MAX_PATH]; sprintf_s(p, "%s\\%s", g_dir, name);
        WIN32_FILE_ATTRIBUTE_DATA a;
        if (GetFileAttributesExA(p, GetFileExInfoStandard, &a)) {
            SYSTEMTIME w; FileTimeToSystemTime(&a.ftLastWriteTime, &w);
            Log("file: %s %lu bytes, modified %04d-%02d-%02d", name, a.nFileSizeLow, w.wYear, w.wMonth, w.wDay);
        } else Log("file: %s not present", name);
    }
}

static bool WriteCode(uintptr_t addr, const void* data, size_t n)
{
    DWORD old;
    if (!VirtualProtect((void*)addr, n, PAGE_EXECUTE_READWRITE, &old)) return false;
    memcpy((void*)addr, data, n);
    VirtualProtect((void*)addr, n, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (void*)addr, n);
    return true;
}

bool Patch(uintptr_t addr, const void* expect, const void* data, size_t n)
{
    if (expect && memcmp((const void*)addr, expect, n) != 0) {
        Log("ERROR: unexpected bytes at %08X, patch skipped", (unsigned)addr);
        return false;
    }
    return WriteCode(addr, data, n);
}

bool PatchDword(uintptr_t addr, uint32_t oldValue, uint32_t newValue)
{
    if (*(const uint32_t*)addr != oldValue) {
        Log("ERROR: %08X holds %08X instead of %08X, patch skipped", (unsigned)addr, *(const uint32_t*)addr, oldValue);
        return false;
    }
    return WriteCode(addr, &newValue, 4);
}

bool PatchCall(uintptr_t addr, uintptr_t oldTarget, const void* newTarget)
{
    const uint8_t op = *(const uint8_t*)addr;
    const uintptr_t cur = addr + 5 + *(const int32_t*)(addr + 1);
    if ((op != 0xE8 && op != 0xE9) || cur != oldTarget) {
        Log("ERROR: no call to %08X at %08X, patch skipped", (unsigned)oldTarget, (unsigned)addr);
        return false;
    }
    const int32_t rel = (int32_t)((uintptr_t)newTarget - (addr + 5));
    return WriteCode(addr + 1, &rel, 4);
}

bool PatchJmp(uintptr_t addr, const void* expect, size_t n, const void* target)
{
    if (expect && memcmp((const void*)addr, expect, n) != 0) {
        Log("ERROR: unexpected bytes at %08X, detour skipped", (unsigned)addr);
        return false;
    }
    uint8_t j[5] = { 0xE9 };
    *(int32_t*)(j + 1) = (int32_t)((uintptr_t)target - (addr + 5));
    return WriteCode(addr, j, 5);
}

// Checks that rotk.exe is the version the addresses were written for (version 1.0, 1,703,936 bytes):
// compile timestamp and image size in the PE header. Athelas' own signatures are not needed here, because ROTK Fix
// patches exactly the same places and the individual patches above verify the bytes they replace.
static bool GameIsSupported()
{
    const BYTE* base = (const BYTE*)0x400000;
    const IMAGE_NT_HEADERS32* nt = (const IMAGE_NT_HEADERS32*)(base + ((const IMAGE_DOS_HEADER*)base)->e_lfanew);
    return nt->FileHeader.TimeDateStamp == 0x3F807074 && nt->OptionalHeader.SizeOfImage == 0x5C7000;
}

// Logs access violations (first few only) that happen in rotk.exe's code or in this DLL: address, registers and
// plausible return addresses on the stack. It does not handle them: the game carries on as it would without ROTKPlus
// (usually with its own error box).
static LONG CALLBACK CrashLogger(EXCEPTION_POINTERS* ep)
{
    const EXCEPTION_RECORD* er = ep->ExceptionRecord;
    const CONTEXT* c = ep->ContextRecord;
    if (er->ExceptionCode != EXCEPTION_ACCESS_VIOLATION) return EXCEPTION_CONTINUE_SEARCH;
    // crash in the game or in ROTKPlus (other modules, e.g. drivers and DXVK, handle their own exceptions)
    HMODULE self = nullptr;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCSTR)&CrashLogger, &self);
    HMODULE at = nullptr;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCSTR)(uintptr_t)c->Eip, &at);
    const bool inGame = c->Eip >= 0x401000 && c->Eip < 0x535000;
    if (!inGame && at != self) return EXCEPTION_CONTINUE_SEARCH;
    if (!inGame) Log("CRASH inside ROTKPlus, at +%X from the start of the DLL", (unsigned)(c->Eip - (uintptr_t)self));
    static LONG once;
    if (InterlockedIncrement(&once) > 3) return EXCEPTION_CONTINUE_SEARCH;
    Log("CRASH at %08X (%s %08X)", c->Eip, er->ExceptionInformation[0] ? "write" : "read",
        (unsigned)er->ExceptionInformation[1]);
    Log("    eax=%08X ebx=%08X ecx=%08X edx=%08X esi=%08X edi=%08X ebp=%08X esp=%08X",
        c->Eax, c->Ebx, c->Ecx, c->Edx, c->Esi, c->Edi, c->Ebp, c->Esp);
    char line[512]; int n = 0;
    const uint32_t* sp = (const uint32_t*)c->Esp;
    for (int i = 0, found = 0; i < 512 && found < 16; ++i) {
        uint32_t v;
        __try { v = sp[i]; } __except (EXCEPTION_EXECUTE_HANDLER) { break; }
        if (v < 0x401005 || v >= 0x535000) continue;
        const uint8_t* r = (const uint8_t*)v;
        if (r[-5] == 0xE8 || (r[-6] == 0xFF && (r[-5] & 0x38) == 0x10) || (r[-2] == 0xFF && (r[-1] & 0x38) == 0x10)) {
            n += sprintf_s(line + n, sizeof line - n, " %08X(+%X)", v, i * 4);
            ++found;
        }
    }
    Log("    callers:%s", line);
    return EXCEPTION_CONTINUE_SEARCH;
}

void PvpInstall();                // Pvp.cpp
void SupportInstall(bool victoryKey);   // Support.cpp

static void Start()
{
    g_startTick = GetTickCount();
    g_detail = IniInt("Log", "Detail", 1) != 0;
    OpenLogs();
    DeleteOldLogs(IniInt("Log", "Keep", 0));
    Log("ROTKPlus %s", ROTKPLUS_VERSION);
    Log("game folder: %s", g_dir);
    LogSessionHeader();
    Log("log: detail %d, keep %d session files (0 = all)", g_detail, IniInt("Log", "Keep", 0));
    if ((uintptr_t)GetModuleHandleA(NULL) != 0x400000 || !GameIsSupported()) {
        Log("rotk.exe not recognised (version 1.0, 1,703,936 bytes, is required): nothing was changed");
        return;
    }
    AddVectoredExceptionHandler(1, CrashLogger);
    LoadConfig();
    SupportInstall(IniInt("Diag", "VictoryKey", 0) != 0);
    SimInstall(IniInt("Debug", "SimPads", 0) != 0);
    if (g_cfg.coop) CoopInstall();
    else Log("co-op with 3 or 4 players is off ([Coop] Enabled=0)");
    PvpInstall();
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(mod);
        InitializeCriticalSection(&g_logCs);
        GetModuleFileNameA(mod, g_dir, MAX_PATH);
        if (char* s = strrchr(g_dir, '\\')) *s = 0;
        Start();
    } else if (reason == DLL_PROCESS_DETACH) {
        __try {
            const DWORD s = (GetTickCount() - g_startTick) / 1000;
            Log("session end after %lu min %lu s", s / 60, s % 60);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    return TRUE;
}
