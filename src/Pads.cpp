// Pads.cpp - controllers of the third and fourth player, read by ROTKPlus through SDL3 (the same SDL3.dll as ROTK Fix)
// Language: C++17, MSVC or mingw-w64 g++, Win32.
//
// ROTK Fix presents only two virtual controllers to the game. It opens the first two controllers in SDL's list in the
// order it finds them, and the order of that list changes over time (the "second" controller was once the first
// player's Steam Controller). So the extra players take the first controller that nobody has opened yet
// (SDL_GetGamepadFromID returns null), that is, the ones ROTK Fix leaves free; for testing, [Coop] PadName picks the
// third player's controller by name (part of the name, e.g. DualSense).
//
// Input records per player at 0x99CBA0 + n*0x58 (0x49D490(n)): right stick at +0x00/+0x04, left stick at
// +0x10/+0x14 (floats -1..1, like JSPOSX/JSPOSY), buttons at +0x40 (word: OR of 0x99CB60[id] for the pressed
// buttons) and +0x42 (those of the previous frame). Button ids (table 0x569C90): 0..3 d-pad up/down/left/right,
// 4 triangle, 5 cross, 6 square, 7 circle, 8 L1, 9 L2, 10 R1, 11 R2, 12 Start, 13 Select, 14 L3, 15 R3. The mapping from
// Xbox-style buttons is the same as ROTK Fix's ([input_JS_BASE_1] in ROTK.ini): A cross, B circle, X square, Y triangle,
// LB L2, RB R1, LT L1, RT R2, Back Select, Start Start, L3 L3.
// Update: after 0x4AAC10 (axes), called every frame from 0x4ABE67, right after the buttons.
#include "ROTKPlus.h"
#include <string.h>
#include <stdio.h>

typedef uint32_t SDL_JoystickID;
struct SDL_Gamepad;
enum { AX_LX, AX_LY, AX_RX, AX_RY, AX_LT, AX_RT };
enum { BT_SOUTH, BT_EAST, BT_WEST, BT_NORTH, BT_BACK, BT_GUIDE, BT_START, BT_LSTICK, BT_RSTICK, BT_LSHOULDER,
       BT_RSHOULDER, BT_UP, BT_DOWN, BT_LEFT, BT_RIGHT };

static SDL_JoystickID* (__cdecl* pGetGamepads)(int*);
static SDL_Gamepad*    (__cdecl* pOpenGamepad)(SDL_JoystickID);
static bool            (__cdecl* pGamepadConnected)(SDL_Gamepad*);
static int16_t         (__cdecl* pGetAxis)(SDL_Gamepad*, int);
static bool            (__cdecl* pGetButton)(SDL_Gamepad*, int);
static const char*     (__cdecl* pGetName)(SDL_Gamepad*);
static SDL_Gamepad*    (__cdecl* pFromId)(SDL_JoystickID);
static const char*     (__cdecl* pNameForId)(SDL_JoystickID);
static void            (__cdecl* pFree)(void*);
static const char*     (__cdecl* pGetSerial)(SDL_Gamepad*);   // optional: SDL_GetGamepadSerial
static const char*     (__cdecl* pGetPath)(SDL_Gamepad*);     // optional: SDL_GetGamepadPath
static void            (__cdecl* pCloseGamepad)(SDL_Gamepad*);

// two extra controllers: the third player (input slot 2) and the fourth (slot 3, searched only when 4 players are chosen)
struct ExtraPad { SDL_Gamepad* pad; SDL_JoystickID id; DWORD nextScan; uint16_t lastBits; bool backWas; };
static ExtraPad g_ep[2];
static char     g_padName[64];    // [Coop] PadName: pick the third player's controller by name (testing), empty = first free one
// [Coop] PadKey3, PadKey4: the controllers chosen in the ROTKPlus launcher for players 3 and 4 (the key of PadKey, the same
// one ROTK Fix makes for players 1 and 2 from [Gamepad] Player1, Player2 of ROTKFix.ini)
static char     g_padKey[2][24];
// The buttons of players 3 and 4, as set in the launcher ([GamepadButtons3] and [GamepadButtons4] of ROTKFix.ini, the same
// keys as ROTK Fix's [GamepadButtons] for players 1 and 2: action = physical button). g_btn[n][phys] = the game's button id
// for each physical button, -1 for none. Without a section: the original layout (A cross, B circle, X square, Y triangle,
// LB L2, RB R1, LT L1, RT R2, L3 L3, R3 R3).
enum { PH_A, PH_B, PH_X, PH_Y, PH_LB, PH_RB, PH_BACK, PH_START, PH_LS, PH_RS, PH_LT, PH_RT, PH_COUNT };
static const char* const kPhysId[PH_COUNT] = { "A", "B", "X", "Y", "LB", "RB", "Back", "Start", "LS", "RS", "LT", "RT" };
static const int kPhysDefault[PH_COUNT] = { 5, 7, 6, 4, 9, 10, 13, 12, 14, 15, 8, 11 };
// the moves the launcher lets you place, and the game's button for each (with the original layout above)
static const char* const kActionKey[9] = { "SpeedAttack", "PhysicalAttack", "Parry", "FierceAttack", "JumpBack",
                                           "Action", "Special", "RangedAttack", "KillingMove" };
static const int kActionId[9] = { 5, 7, 6, 4, 9, 10, 14, 8, 11 };
static int g_btn[2][PH_COUNT];

static void LoadButtons(int slot)
{
    int* b = g_btn[slot - 2];
    for (int i = 0; i < PH_COUNT; ++i) b[i] = kPhysDefault[i];
    char ini[MAX_PATH], sec[32], v[16];
    sprintf_s(ini, "%s\\ROTKFix.ini", GameDir());
    sprintf_s(sec, "GamepadButtons%d", slot + 1);
    int phys[9], found = 0;
    for (int a = 0; a < 9; ++a) {
        phys[a] = -1;
        GetPrivateProfileStringA(sec, kActionKey[a], "", v, sizeof v, ini);
        for (int ph = 0; ph < PH_COUNT; ++ph) if (v[0] && !_stricmp(v, kPhysId[ph])) phys[a] = ph;
        if (phys[a] >= 0) ++found;
    }
    if (!found) { Log("buttons of player %d: the original layout", slot + 1); return; }
    // every move leaves its default button first (a button no move is on any more does nothing), then each move goes on
    // its button; Back, Start and R3 keep theirs unless a move is put there
    for (int a = 0; a < 9; ++a) for (int ph = 0; ph < PH_COUNT; ++ph) if (b[ph] == kActionId[a]) b[ph] = -1;
    for (int a = 0; a < 9; ++a) {
        int ph = phys[a] >= 0 ? phys[a] : -1;
        if (ph < 0) for (int i = 0; i < PH_COUNT; ++i) if (kPhysDefault[i] == kActionId[a]) ph = i;   // (not set: its default)
        if (ph >= 0) b[ph] = kActionId[a];
    }
    char line[160] = "";
    for (int ph = 0; ph < PH_COUNT; ++ph) {
        char one[16];
        sprintf_s(one, " %s=%d", kPhysId[ph], b[ph]);
        strcat_s(line, one);
    }
    Log("buttons of player %d (launcher, [%s]):%s", slot + 1, sec, line);
}

static const uintptr_t kRecordBase = 0x99CBA0;
static const uintptr_t kButtonBit = 0x99CB60;   // uint16 per id
static const uintptr_t kUpdateAxes = 0x4AAC10;

static bool LoadSdl()
{
    HMODULE m = GetModuleHandleA("SDL3.dll");
    if (!m) return false;
    pGetGamepads      = (decltype(pGetGamepads))GetProcAddress(m, "SDL_GetGamepads");
    pOpenGamepad      = (decltype(pOpenGamepad))GetProcAddress(m, "SDL_OpenGamepad");
    pGamepadConnected = (decltype(pGamepadConnected))GetProcAddress(m, "SDL_GamepadConnected");
    pGetAxis          = (decltype(pGetAxis))GetProcAddress(m, "SDL_GetGamepadAxis");
    pGetButton        = (decltype(pGetButton))GetProcAddress(m, "SDL_GetGamepadButton");
    pGetName          = (decltype(pGetName))GetProcAddress(m, "SDL_GetGamepadName");
    pFree             = (decltype(pFree))GetProcAddress(m, "SDL_free");
    pFromId           = (decltype(pFromId))GetProcAddress(m, "SDL_GetGamepadFromID");
    pNameForId        = (decltype(pNameForId))GetProcAddress(m, "SDL_GetGamepadNameForID");
    pGetSerial        = (decltype(pGetSerial))GetProcAddress(m, "SDL_GetGamepadSerial");
    pGetPath          = (decltype(pGetPath))GetProcAddress(m, "SDL_GetGamepadPath");
    pCloseGamepad     = (decltype(pCloseGamepad))GetProcAddress(m, "SDL_CloseGamepad");
    return pGetGamepads && pOpenGamepad && pGamepadConnected && pGetAxis && pGetButton && pGetName && pFree &&
           pFromId && pNameForId;
}

static std::string KeyOf(SDL_Gamepad* pad);

// Looks for the extra player's controller (slot 2 or 3), at most once a second until it is found.
static void FindPad(int slot)
{
    ExtraPad& e = g_ep[slot - 2];
    if (e.pad && pGamepadConnected(e.pad)) return;
    if (e.pad) { Log("pad of player %d disconnected", slot + 1); e.pad = nullptr; }
    const DWORD now = GetTickCount();
    if (now < e.nextScan) return;
    e.nextScan = now + 1000;
    if (!pGetGamepads && !LoadSdl()) return;
    int n = 0;
    SDL_JoystickID* ids = pGetGamepads(&n);
    static int listed;   // the list of controllers SDL sees, the first few times
    if (ids && listed++ < 3)
        for (int i = 0; i < n; ++i)
            Log("pad: SDL sees controller %d \"%s\" (id %u, %s)", i + 1, pNameForId(ids[i]) ? pNameForId(ids[i]) : "?",
                ids[i], pFromId(ids[i]) ? "already open" : "free");
    const char* want = slot == 2 ? g_padName : "";
    const char* mine = g_padKey[slot - 2];
    const char* other = g_padKey[3 - slot];   // the other extra player's own controller: never taken
    // two passes: this player's own controller (chosen in the launcher), then the first free one that is nobody's
    for (int pass = mine[0] ? 0 : 1; pass < 2 && !e.pad; ++pass)
        for (int i = 0; ids && i < n && !e.pad; ++i) {
            const char* name = pNameForId(ids[i]);
            if (want[0] ? !(name && strstr(name, want)) : pFromId(ids[i]) != nullptr) continue;
            SDL_Gamepad* p = pOpenGamepad(ids[i]);
            if (!p) continue;
            const std::string key = KeyOf(p);
            const bool own = mine[0] && !_stricmp(key.c_str(), mine);
            const bool others = other[0] && !_stricmp(key.c_str(), other);
            if (pass == 0 ? !own : others) { if (pCloseGamepad) pCloseGamepad(p); continue; }
            e.id = ids[i];
            e.pad = p;
            Log("pad of player %d: \"%s\" (%d controllers connected%s)", slot + 1, name ? name : "?", n,
                own ? ", chosen in the launcher" : want[0] ? ", chosen by name" : ", the first free one");
            PlayersPadOpened(slot, PadKeyOfSlot(slot));   // a known controller brings back its player's name
        }
    if (ids) pFree(ids);
}

// A key that stays the same for a controller (Players.cpp remembers names by it): SDL's serial number if the controller
// has one (DualSense, DualShock 4, Switch Pro), otherwise its device path (the same as long as it is plugged into the same
// port or paired the same way). Hashed (FNV-1a, 64 bit), because paths have characters an ini key cannot hold. "" if the
// slot has no controller.
static std::string KeyOf(SDL_Gamepad* pad)
{
    if (!pad) return "";
    const char* serial = pGetSerial ? pGetSerial(pad) : nullptr;
    const char* path = pGetPath ? pGetPath(pad) : nullptr;
    const char* src = serial && serial[0] ? serial : path && path[0] ? path : nullptr;
    if (!src) return "";
    uint64_t h = 1469598103934665603ull;
    for (const char* p = src; *p; ++p) { h ^= (uint8_t)*p; h *= 1099511628211ull; }
    char key[32];
    sprintf_s(key, "%s%016llx", serial && serial[0] ? "S" : "P", (unsigned long long)h);
    return key;
}

std::string PadKeyOfSlot(int slot)
{
    return slot < 2 || slot > 3 ? std::string() : KeyOf(g_ep[slot - 2].pad);
}

static float Axis(SDL_Gamepad* pad, int a)
{
    const int v = pGetAxis(pad, a);
    return v < 0 ? v / 32768.0f : v / 32767.0f;
}

// The shoulder buttons of every open controller (ROTK Fix's two and ours), for the pages of the main menu
// (Players.cpp): on SteamOS the game's input records had no shoulder buttons in the menus (test of
// 04/11/2026). A button counts when it goes down; each controller is remembered by its SDL id.
int PadsShoulder(bool& sdl)
{
    sdl = false;
    if (!pGetGamepads && !LoadSdl()) return 0;
    struct Seen { SDL_JoystickID id; uint8_t held; };
    static Seen was[8];
    Seen now[8] = {};
    int n = 0, dir = 0, kept = 0;
    SDL_JoystickID* ids = pGetGamepads(&n);
    if (!ids) return 0;
    sdl = true;
    for (int i = 0; i < n && kept < 8; ++i) {
        SDL_Gamepad* p = pFromId(ids[i]);
        if (!p) continue;
        uint8_t held = 0;
        if (pGetButton(p, BT_LSHOULDER) || Axis(p, AX_LT) > 0.5f) held |= 1;
        if (pGetButton(p, BT_RSHOULDER) || Axis(p, AX_RT) > 0.5f) held |= 2;
        uint8_t before = 0;
        for (const Seen& s : was) if (s.id == ids[i]) before = s.held;
        const uint8_t down = held & ~before;
        if (down & 1) dir = -1;
        else if (down & 2) dir = 1;
        now[kept++] = Seen{ ids[i], held };
    }
    pFree(ids);
    memcpy(was, now, sizeof was);
    return dir;
}

static void UpdatePad(int slot)
{
    FindPad(slot);
    ExtraPad& e = g_ep[slot - 2];
    SDL_Gamepad* pad = e.pad;
    uint8_t* rec = (uint8_t*)(kRecordBase + slot * 0x58);
    if (!pad) {
        // No controller: unless [Coop] KeyboardFallback=1, the extra player stands still. The game feeds the keyboard
        // (player 1's keys, copied for the extra players' input sections) into every player without a pad, which made one
        // person drive several characters at once.
        if (!g_cfg.kbFallback) {
            memset(rec, 0, 0x20);
            *(uint16_t*)(rec + 0x40) = 0;
            *(uint16_t*)(rec + 0x42) = e.lastBits;
            e.lastBits = 0;
        }
        return;
    }
    // sticks with a 12% radial dead zone (like ROTK Fix)
    for (int s = 0; s < 2; ++s) {
        float x = Axis(pad, s ? AX_RX : AX_LX), y = Axis(pad, s ? AX_RY : AX_LY);
        const float m2 = x * x + y * y;
        if (m2 < 0.12f * 0.12f) x = y = 0.0f;
        float* out = (float*)(rec + (s ? 0x00 : 0x10));
        out[0] = x; out[1] = y;
    }
    // the physical buttons (as the launcher names them) through this player's layout; the D-pad is always the D-pad
    const uint16_t* bit = (const uint16_t*)kButtonBit;
    auto bitOf = [&](int id) { return bit[id] ? bit[id] : (uint16_t)(1u << id); };
    const int* map = g_btn[slot - 2];
    static const int kSdlOf[PH_COUNT] = { BT_SOUTH, BT_EAST, BT_WEST, BT_NORTH, BT_LSHOULDER, BT_RSHOULDER, BT_BACK, BT_START,
                                          BT_LSTICK, BT_RSTICK, -1, -1 };
    uint16_t bits = 0;
    for (int ph = 0; ph < PH_COUNT; ++ph) {
        bool down = kSdlOf[ph] >= 0 ? pGetButton(pad, kSdlOf[ph])
                                    : Axis(pad, ph == PH_LT ? AX_LT : AX_RT) > 0.30f;   // the triggers, past 30%
        if (down && map[ph] >= 0) bits |= bitOf(map[ph]);
    }
    static const struct { int sdl; int id; } kDpad[] = { { BT_UP, 0 }, { BT_DOWN, 1 }, { BT_LEFT, 2 }, { BT_RIGHT, 3 } };
    for (const auto& m : kDpad)
        if (pGetButton(pad, m.sdl)) bits |= bitOf(m.id);
    // Back in a level (when no move was put on it in the launcher): "bring me back next to player 1 or 2" (Coop.cpp); the
    // game does not get it there
    if (map[PH_BACK] == kPhysDefault[PH_BACK] && *(const int*)0x562AC0 == 0) {
        const bool back = pGetButton(pad, BT_BACK);
        if (back && !e.backWas) CoopAskBack(slot);
        e.backWas = back;
        bits &= (uint16_t)~bitOf(kPhysDefault[PH_BACK]);
    } else e.backWas = false;
    // +0x1C (and +0x0C for the right stick): digital directions. With the keyboard the game turns it on and the
    // character ignores the stick (0x4453A0 uses the stick only if +0x1C is 0): with a pad it must be off
    rec[0x1C] = 0;
    rec[0x0C] = 0;
    *(uint16_t*)(rec + 0x42) = e.lastBits;
    *(uint16_t*)(rec + 0x40) = bits;
    e.lastBits = bits;
}

void SupportFrame();   // Support.cpp
void ScreensFrame();   // Screens.cpp
void PvpFrame();       // Pvp.cpp
void LightsFrame();    // Spawners.cpp: how full the lighting manager's tables get

static bool RealPad(int slot);

static void __cdecl UpdateAxesAndPads()
{
    ((void(__cdecl*)())kUpdateAxes)();
    UpdatePad(2);
    if (CoopPlayers() >= 4) UpdatePad(3);   // the fourth controller is opened only if needed
    SimFrame(&RealPad);
    SupportFrame();
    ScreensFrame();
    PvpFrame();
    LightsFrame();
}

static bool RealPad(int slot) { return (slot == 2 || slot == 3) && g_ep[slot - 2].pad != nullptr; }
// with the keyboard-simulated pads (Sim.cpp) the extra players count as having a pad
bool PadConnected(int slot)
{
    return RealPad(slot) || (SimActive() && (slot == 2 || slot == 3) && CoopPlayers() >= slot + 1);
}

bool PadsInstall(const char* padName)
{
    strncpy_s(g_padName, padName ? padName : "", _TRUNCATE);
    IniText("Coop", "PadKey3", g_padKey[0], sizeof g_padKey[0]);
    IniText("Coop", "PadKey4", g_padKey[1], sizeof g_padKey[1]);
    LoadButtons(2);
    LoadButtons(3);
    if (g_padKey[0][0] || g_padKey[1][0]) Log("pads: players 3 and 4 have their own controllers (chosen in the launcher): %s %s",
                                             g_padKey[0][0] ? g_padKey[0] : "-", g_padKey[1][0] ? g_padKey[1] : "-");
    const bool ok = PatchCall(0x4ABE67, kUpdateAxes, (const void*)&UpdateAxesAndPads);
    Log("pads of players 3 and 4: %s (%s%s)", ok ? "active" : "NOT installed",
        g_padName[0] ? "third player chosen by name: " : "first free controller", g_padName);
    return ok;
}
