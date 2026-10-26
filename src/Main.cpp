// Main.cpp - DLL entry point, log, and the functions that modify the game's code
// Language: C++17, MSVC, Win32.
#include "ROTKPlus.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static char             g_dir[MAX_PATH];
static CRITICAL_SECTION g_logCs;
static FILE*            g_log;

const char* GameDir() { return g_dir; }

void Log(const char* fmt, ...)
{
    if (!g_log) return;
    EnterCriticalSection(&g_logCs);
    SYSTEMTIME t; GetLocalTime(&t);
    fprintf(g_log, "[%02d:%02d:%02d.%03d] ", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
    va_list ap; va_start(ap, fmt); vfprintf(g_log, fmt, ap); va_end(ap);
    fputc('\n', g_log);
    fflush(g_log);
    LeaveCriticalSection(&g_logCs);
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
    char log[MAX_PATH];
    sprintf_s(log, "%s\\ROTKPlus.log", g_dir);
    fopen_s(&g_log, log, "w");
    Log("ROTKPlus %s", ROTKPLUS_VERSION);
    Log("game folder: %s", g_dir);
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
    }
    return TRUE;
}
