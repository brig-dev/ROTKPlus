// Support.cpp - small hooks the other modules depend on: hero choice in the selection screen, script calls, ending
// a level on demand
// Language: C++17, MSVC, Win32.
#include "ROTKPlus.h"
#include <stdio.h>
#include <string.h>

void ScreensFrame();   // Screens.cpp

static int Mode() { return *(const int*)0x562AC0; }
static bool InLevel() { return Mode() == 0; }   // 0 = playing (1 is the hero selection screen)

// --- 0x47DF00 / 0x47DF20: hero choice of player 1 / player 2 (selection screen) ---------------------------------
// The selection screen is driven by the screen's script, which calls these two functions. Screens.cpp decides when a
// choice belongs to the third or fourth player instead (CharSelOnChoice).
static void __cdecl SetChoice1(int h)
{
    Log("selection: hero choice of player 1 = %d (mode %d)", h, Mode());
    if (CharSelOnChoice(1, h)) return;
    if (h > -1 && h < 9) *(int*)0x562A84 = h;
}

static void __cdecl SetChoice2(int h)
{
    Log("selection: hero choice of player 2 = %d (mode %d)", h, Mode());
    if (CharSelOnChoice(2, h)) return;
    if (h > -1 && h < 9) *(int*)0x562A88 = h;
}

// --- 0x48ED90: calls made by the screens' scripts ----------------------------------------------------------------
// 0x48ED90(type, x, code, args, result, ...) dispatches script calls. Outside the main per-frame update this is the
// only regular place where the selection screen runs, so the deferred actions of Screens.cpp (ScreensFrame) are also
// given a chance after every script call.
static __declspec(naked) void ScriptCallOrig()   // mov edx,[esp+4]; sub esp,24h
{
    __asm {
        mov  edx, dword ptr [esp + 4]
        sub  esp, 0x24
        push 0x48ED97
        ret
    }
}

static uint32_t __cdecl ScriptCall(int type, int x, uint32_t code, float* args, float* res, int a6, int a7)
{
    const uint32_t r = GameFn((uint32_t(__cdecl*)(int, int, uint32_t, float*, float*, int, int))&ScriptCallOrig)(
        type, x, code, args, res, a6, a7);
    ScreensFrame();
    return r;
}

// --- ending a level on demand ----------------------------------------------------------------------------------
// The "end of level" case (0x62C3F8B4) of the script dispatcher 0x480830 starts at 0x480B68 and uses: esi = script
// message ([esi+0xC] -> float parameter, 1 victory / 2 defeat), ebx = 0, the frame of 0x480830's prologue.
// EndLevelNow rebuilds the prologue and jumps there: the case ends with the original epilogue and returns here. Used
// by the versus mode (RequestEndLevel) and, with [Diag] VictoryKey=1, by F9 (instant victory, handy for looking at the
// end-of-level screens without playing the level).
static float g_endParam;
static int   g_endMsg[8];

static __declspec(naked) void __cdecl EndLevelNow(int* /*msg*/)
{
    __asm {
        push ebp
        mov  ebp, esp
        and  esp, 0xFFFFFFF8
        sub  esp, 0x0C
        push ebx
        push esi
        mov  esi, dword ptr [ebp + 8]
        xor  ebx, ebx
        push edi
        push 0x480B68
        ret
    }
}

static bool g_victoryKey;
static volatile bool g_endPending;   // end of level requested by other modules (Pvp.cpp), run on the next frame

void RequestEndLevel() { g_endPending = true; }

static void EndLevel(const char* why)
{
    g_endParam = 1.0f;
    g_endMsg[0] = 2000;
    g_endMsg[3] = (int)(uintptr_t)&g_endParam;
    Log("end of level: %s", why);
    GameFn(&EndLevelNow)(g_endMsg);
}

void SupportFrame()   // from Pads.cpp, every frame, on the game's thread
{
    static bool down;
    if (g_endPending && InLevel()) {
        g_endPending = false;
        EndLevel("requested");
    }
    if (!g_victoryKey) return;
    const bool now = (GetAsyncKeyState(VK_F9) & 0x8000) != 0;
    if (now && !down && InLevel()) EndLevel("F9, instant victory");
    down = now;
}

void SupportInstall(bool victoryKey)
{
    static const uint8_t choiceOld[] = { 0x8B, 0x44, 0x24, 0x04, 0x83, 0xF8, 0xFF };
    static const uint8_t scriptOld[] = { 0x8B, 0x54, 0x24, 0x04, 0x83, 0xEC, 0x24 };
    bool ok = PatchJmp(0x47DF00, choiceOld, sizeof choiceOld, (const void*)&SetChoice1);
    ok &= PatchJmp(0x47DF20, choiceOld, sizeof choiceOld, (const void*)&SetChoice2);
    ok &= PatchJmp(0x48ED90, scriptOld, sizeof scriptOld, (const void*)&ScriptCall);
    g_victoryKey = victoryKey;
    Log("hero choice and script hooks: %s%s", ok ? "active" : "NOT installed", victoryKey ? " (F9 = instant victory)" : "");
}
