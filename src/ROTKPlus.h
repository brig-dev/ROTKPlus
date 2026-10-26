// ROTKPlus.h - declarations shared by all modules
// Language: C++17. Compiler: MSVC (Visual Studio 2022 or later), Win32 (x86).
//
// ROTKPlus is a mod for The Lord of the Rings: The Return of the King (PC, 2003): rotk.exe 1.0,
// 1,703,936 bytes. It is loaded in place of DivxMediaLib.dll (the game's movie library, renamed
// DivxMediaLib_orig.dll, to which every export is forwarded) and coexists with ROTK Fix / Athelas.
#pragma once

#if !defined(_M_IX86)
#error "ROTKPlus must be built for Win32 (x86): rotk.exe is a 32-bit program."
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <string>

#define ROTKPLUS_VERSION "1.0.0"

// Call a naked trampoline (one that continues into the game's code) from C++: GameFn((type)&Trampoline)(...).
// When it sees a static naked function in the same translation unit, MSVC may assume that it does not touch the
// volatile registers and keep values in them across the call. This once broke the whole HUD: a value kept in xmm0
// across a call into the game was overwritten by the game (and by DXVK) and the "restore" corrupted the texture
// coordinates of every HUD image for good. Reading the pointer through a volatile variable hides the callee and
// forces the standard convention (eax, ecx, edx and xmm0-7 are assumed lost).
template <class F> inline F GameFn(F f) { F volatile v = f; return v; }

// --- Main.cpp: log, game folder, writing into the game's code ------------------------------------------------
void        Log(const char* fmt, ...);
const char* GameDir();
// Writes n bytes at addr if the current bytes are the expected ones (expect, n bytes). Returns false otherwise.
bool        Patch(uintptr_t addr, const void* expect, const void* data, size_t n);
// Replaces the 32-bit value at addr (inside an instruction) if it equals oldValue.
bool        PatchDword(uintptr_t addr, uint32_t oldValue, uint32_t newValue);
// Replaces the target of the rel32 call/jmp at addr (opcode E8/E9) if it points to oldTarget.
bool        PatchCall(uintptr_t addr, uintptr_t oldTarget, const void* newTarget);
// Writes a rel32 jmp at addr to target, if the first n bytes are the expected ones.
bool        PatchJmp(uintptr_t addr, const void* expect, size_t n, const void* target);

// --- Config.cpp: ROTKPlus.ini in the game folder -------------------------------------------------------------
struct Config {
    bool coop       = false;  // [Coop] Enabled: third and fourth player in co-op
    int  hero3      = -1;     // [Coop] Hero: 0 Aragorn ... 8 Faramir, -1 = automatic choice (third player only)
    int  input3     = 2;      // [Coop] Input: input slot of the third player (2 = its own; 0 = copies player 1)
    bool pads       = true;   // [Coop] Pad: controllers for the third and fourth player; 0 = keyboard only
    bool kbFallback = false;  // [Coop] KeyboardFallback: an extra player without a controller moves with player 1's keys
    char pad3Name[64] = "";   // [Coop] PadName: for testing, pick the third player's controller by part of its name
    bool  hud       = true;   // [Coop] Hud: HUD boxes of the extra players
    float hudDx     = 0.0f;   // [Coop] HudX/HudY: offset of the extra players' boxes from player 2's box, as
    float hudDy     = -0.72f; //   fractions of the screen (negative = up)
    bool  hudRow    = false;  // [Coop] HudRow: 1 = all boxes in a reduced row at the bottom, 0 = extra boxes at the top
    float hudScale  = 0.66f;  // [Coop] HudScale: size of the boxes in the row
};
extern Config g_cfg;
void LoadConfig();
// ROTKPlus.ini reads: the current section is [Coop] / [PvP]; the old [Coop3] section is still accepted.
int  IniInt(const char* section, const char* key, int def);
void IniWriteInt(const char* section, const char* key, int value);

// --- Coop.cpp: third and fourth player ------------------------------------------------------------------------
bool CoopInstall();
int  HeroOfSlot(int slot);        // hero of an extra player (slot 2 = third, 3 = fourth) in the current level, -1 none

// --- Pads.cpp: controllers of the third and fourth player (SDL3) -----------------------------------------------
bool PadsInstall(const char* padName);
bool PadConnected(int slot);      // controller opened for slot 2 (third) or 3 (fourth)

// --- Sim.cpp: keyboard-simulated controllers for testing ([Debug] SimPads=1) ------------------------------
void SimInstall(bool on);
bool SimActive();
void SimFrame(bool (*hasPad)(int slot));

// --- HeroIndex.cpp: hero selection in functions that pick by player index --------------------------------------
bool HeroInstall();

// --- Screens.cpp: turns in the end-of-level screens, hero selection, main menu ----------------------------------
bool ScreensInstall();
int  CharSelChoice(int slot);                 // hero chosen by an extra player in the selection screen, -1 none
bool CharSelOnChoice(int player, int hero);   // from 0x47DF00/0x47DF20: true = the choice belongs to an extra player
int  CoopPlayers();                           // players chosen in the main menu: 2, 3 or 4 (2 if the mod is off)
bool Coop3Wanted();                           // at least 3 players
bool Coop4Wanted();                           // 4 players

// --- Lang.cpp: texts in the game's language ---------------------------------------------------------------------
// id: 0 Co-op menu entry, 1 Versus menu entry, 2 "{label}: {n} players", 3 "Player {n}", 4 winner, 5 draw
std::string LangText(int id, int n = 0, const char* hero = "", const char* label = "");

// --- Hud.cpp: HUD of the third and fourth player -----------------------------------------------------------------
bool HudInstall(float dx, float dy, bool row, float rowScale);
