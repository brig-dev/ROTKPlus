// Sim.cpp - simulated controllers driven by the keyboard, for testing without controllers
// Language: C++17, MSVC, Win32.
//
// Enabled with [Debug] SimPads=1 in ROTKPlus.ini. It exists so that the co-op can be tried with a keyboard and mouse
// only (no controller at all):
//  - The game enables the Co-op entry of the main menu with 0x49D480, which answers 2 only when at least one joystick
//    exists (it calls 0x4B0470 and returns 1 + (count > 0)). It is replaced by a function that always answers 2.
//  - Player 1 keeps using the game's own keyboard and mouse controls (start the game from Athelas' Play button without a
//    controller connected, so that it sets that up). Players 2, 3 and 4 get their input records (0x99CBA0 + slot*0x58,
//    see Pads.cpp) filled from the keyboard every frame, from the same place and in the same way as a real pad.
//  - Keys (only while the game window has the focus):
//      player 2: arrows move, Right Ctrl cross (fast attack), Right Shift circle (heavy attack), Delete square (parry),
//                End triangle (fierce attack), Page Down R2 (execution)
//      player 3: I J K L move, U cross, O circle, P square, Y triangle, H R2
//      player 4: numpad 8 4 5 6 move (Num Lock on), Num0 cross, Num. circle, Num1 square, Num2 triangle, Num3 R2
//    Movement keys also set the digital direction buttons, so the menus can be navigated; cross confirms, circle cancels.
//    During an extra player's turn in the menus (hero selection, score, statistics, shop) only that player's pad is
//    read, so for that player Enter and Space also count as cross, Escape and Backspace as circle, and W A S D as the
//    directions: the keys you use for player 1 work for whoever's turn it is.
// A single person can therefore drive up to four characters one at a time (the others stand still), which is enough to
// look at the creation of the players, the HUD, deaths and respawns, the hero selection and the end-of-level screens.
#include "ROTKPlus.h"

static bool g_sim;

struct SimKeys { int up, down, left, right, cross, circle, square, triangle, r2; };
static const SimKeys kKeys[3] = {   // slot 1 (player 2), slot 2 (player 3), slot 3 (player 4)
    { VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_RCONTROL, VK_RSHIFT, VK_DELETE, VK_END, VK_NEXT },
    { 'I', 'K', 'J', 'L', 'U', 'O', 'P', 'Y', 'H' },
    { VK_NUMPAD8, VK_NUMPAD5, VK_NUMPAD4, VK_NUMPAD6, VK_NUMPAD0, VK_DECIMAL, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3 },
};

static const uintptr_t kRecordBase = 0x99CBA0;
static const uintptr_t kButtonBit  = 0x99CB60;   // uint16 per id

static uint16_t g_last[3];
int ScreensOnlySlot();   // Screens.cpp

bool SimActive() { return g_sim; }

static bool GameHasFocus()
{
    DWORD pid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    return pid == GetCurrentProcessId();
}

static bool Down(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }

// fills the input record of the slot (1..3) from the keyboard; the same fields a real pad writes (Pads.cpp)
static void SimSlot(int slot)
{
    const SimKeys& k = kKeys[slot - 1];
    uint8_t* rec = (uint8_t*)(kRecordBase + slot * 0x58);
    const bool focus = GameHasFocus();
    const bool turn = focus && ScreensOnlySlot() == slot;   // this player's turn in the menus: player 1's keys work too
    const bool up = focus && (Down(k.up) || (turn && Down('W'))), down = focus && (Down(k.down) || (turn && Down('S')));
    const bool left = focus && (Down(k.left) || (turn && Down('A'))), right = focus && (Down(k.right) || (turn && Down('D')));
    float* stick = (float*)(rec + 0x10);
    stick[0] = (right ? 1.0f : 0.0f) - (left ? 1.0f : 0.0f);
    stick[1] = (down ? 1.0f : 0.0f) - (up ? 1.0f : 0.0f);        // SDL convention: up is negative
    float* stickR = (float*)rec;
    stickR[0] = stickR[1] = 0.0f;
    const uint16_t* bit = (const uint16_t*)kButtonBit;
    auto bitOf = [&](int id) { return bit[id] ? bit[id] : (uint16_t)(1u << id); };
    uint16_t bits = 0;
    if (up) bits |= bitOf(0);
    if (down) bits |= bitOf(1);
    if (left) bits |= bitOf(2);
    if (right) bits |= bitOf(3);
    if (focus && Down(k.triangle)) bits |= bitOf(4);
    if (focus && (Down(k.cross) || (turn && (Down(VK_RETURN) || Down(VK_SPACE))))) bits |= bitOf(5);
    if (focus && Down(k.square)) bits |= bitOf(6);
    if (focus && (Down(k.circle) || (turn && (Down(VK_ESCAPE) || Down(VK_BACK))))) bits |= bitOf(7);
    if (focus && Down(k.r2)) bits |= bitOf(11);
    rec[0x1C] = 0;
    rec[0x0C] = 0;
    *(uint16_t*)(rec + 0x42) = g_last[slot - 1];
    *(uint16_t*)(rec + 0x40) = bits;
    g_last[slot - 1] = bits;
}

// called every frame from Pads.cpp after the real pads; a slot with a real pad is left alone
void SimFrame(bool (*hasPad)(int slot))
{
    if (!g_sim) return;
    for (int slot = 1; slot <= 3; ++slot) {
        if (slot >= 2 && hasPad(slot)) continue;
        if (slot >= 2 && CoopPlayers() < slot + 1) continue;
        SimSlot(slot);
    }
}

// replaces 0x49D480: "there are at least two controllers"
static __declspec(naked) void TwoControllers()
{
    __asm {
        mov  al, 2
        ret
    }
}

void SimInstall(bool on)
{
    if (!on) return;
    // 0x49D480: call 0x4B0470; test eax,eax; setg al; inc al; ret
    const uint8_t* f = (const uint8_t*)0x49D480;
    if (f[0] != 0xE8 || f[5] != 0x85 || f[6] != 0xC0 || f[0xC] != 0xC3) {
        Log("sim: 0x49D480 is not what was expected, simulated pads not installed");
        return;
    }
    g_sim = PatchJmp(0x49D480, nullptr, 5, (const void*)&TwoControllers);
    Log("sim: keyboard-simulated pads for players 2, 3 and 4: %s", g_sim ? "active" : "NOT installed");
}
