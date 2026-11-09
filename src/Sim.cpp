// Sim.cpp - simulated controllers driven by the keyboard, for testing without controllers
// Language: C++17, MSVC or mingw-w64 g++, Win32.
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
//      player 3: I J K L move, U cross, O circle, P square, Y triangle, H R2, N Back (in a level: back next to player 1 or 2)
//      player 4: numpad 8 4 5 6 move (Num Lock on), Num0 cross, Num. circle, Num1 square, Num2 triangle, Num3 R2, Num9 Back
//    Movement keys also set the digital direction buttons, so the menus can be navigated; cross confirms, circle cancels.
//    In the menus (hero selection, score, statistics, shop) Enter and Space also count as cross, Escape and Backspace as
//    circle, and W A S D as the directions for whoever's turn it is, so one set of keys drives every turn:
//     - player 3 or 4: during their turn (Screens.cpp lets only that player's pad through, ScreensOnlySlot);
//     - player 2: during the turns the game itself gives to player 2, recognised because the game excludes player 1's
//       slot from the menus (0x4EA200 writes the excluded slot to 0x56DC75: 0 = player 1's).
//    A key that is already held when such a turn starts is ignored until it is released: the Enter that confirmed the
//    previous player's choice must not also confirm the next one.
// A single person can therefore drive up to four characters one at a time (the others stand still), which is enough to
// look at the creation of the players, the HUD, deaths and respawns, the hero selection and the end-of-level screens.
// With SimPads=2 ("follow"), while a level is played, players 2, 3 and 4 also take player 1's keyboard and mouse:
// W A S D move, left button fast attack (cross), right button fierce attack (triangle), Left Ctrl physical attack
// (circle), Q parry (square), Space execution (R2), the keys of the game's default controls. Everybody then moves and
// fights together with player 1, so one person can go through a whole level with four heroes (tests of the co-op).
// With SimPads=3 ("mirror"), player 1's controller (or keyboard and mouse) drives players 2, 3 and 4: in the levels its
// whole input record is copied to theirs every frame, so four heroes move and fight together with one controller (videos
// of a four-player game made by one person); Start and Select are left out there, or the pause would be asked four times
// in the same frame. In the menus the copy goes only to the player whose turn it is (hero selection, score, shop), as
// with the shared keys above, once the buttons held when the turn started are released.
#include "ROTKPlus.h"
#include <string.h>
#include <math.h>

static bool g_sim;
static bool g_follow;   // SimPads=2: the extra players also follow player 1's keys and mouse in the levels
static bool g_mirror;   // SimPads=3: the extra players take player 1's input record (its controller)

// back: the controller's Back for players 3 and 4 in the levels ("bring me back next to player 1 or 2", Coop.cpp)
struct SimKeys { int up, down, left, right, cross, circle, square, triangle, r2, back; };
static const SimKeys kKeys[3] = {   // slot 1 (player 2), slot 2 (player 3), slot 3 (player 4)
    { VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_RCONTROL, VK_RSHIFT, VK_DELETE, VK_END, VK_NEXT, 0 },
    { 'I', 'K', 'J', 'L', 'U', 'O', 'P', 'Y', 'H', 'N' },
    { VK_NUMPAD8, VK_NUMPAD5, VK_NUMPAD4, VK_NUMPAD6, VK_NUMPAD0, VK_DECIMAL, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD9 },
};
static bool g_backWas[3];

static const uintptr_t kRecordBase = 0x99CBA0;
static const uintptr_t kButtonBit  = 0x99CB60;   // uint16 per id

static uint16_t g_last[3];
static bool     g_sharedBlocked[3];   // the shared keys were held when this slot's turn started: wait for their release
int ScreensOnlySlot();   // Screens.cpp

bool SimActive() { return g_sim; }

static bool GameHasFocus()
{
    DWORD pid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    return pid == GetCurrentProcessId();
}

static bool Down(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }

static bool InMenus() { return *(const int*)0x562AC0 != 0; }   // screen mode 0 = playing a level

// The menu turn of this simulated player, in which player 1's keys also work for it (see the top of the file)
static bool SharedKeysTurn(int slot)
{
    if (!InMenus()) return false;
    const int only = ScreensOnlySlot();
    if (slot >= 2) return only == slot;
    return only < 0 && *(const uint8_t*)0x56DC75 == 0;   // player 2: the game has excluded player 1's slot
}

static bool AnySharedKeyDown()
{
    for (int vk : { (int)VK_RETURN, (int)VK_SPACE, (int)VK_ESCAPE, (int)VK_BACK, (int)'W', (int)'A', (int)'S', (int)'D' })
        if (Down(vk)) return true;
    return false;
}

// fills the input record of the slot (1..3) from the keyboard; the same fields a real pad writes (Pads.cpp)
static void SimSlot(int slot)
{
    const SimKeys& k = kKeys[slot - 1];
    uint8_t* rec = (uint8_t*)(kRecordBase + slot * 0x58);
    const bool focus = GameHasFocus();
    // this player's turn in the menus: player 1's keys work too, once the keys held when the turn started are released
    bool turn = focus && SharedKeysTurn(slot);
    bool& blocked = g_sharedBlocked[slot - 1];
    if (!turn) blocked = AnySharedKeyDown();
    else if (blocked && !AnySharedKeyDown()) blocked = false;
    turn = turn && !blocked;
    if (slot == 1) {   // log the turns recognised for player 2, to check the rule on the excluded slot
        static int lastTurn = -1, logged;
        if ((int)turn != lastTurn && logged < 40) {
            ++logged;
            Log("sim: player 2's menu turn %s (mode %d, excluded slot %d)", turn ? "on" : "off", *(const int*)0x562AC0,
                *(const uint8_t*)0x56DC75);
        }
        lastTurn = turn;
    }
    const bool fol = g_follow && focus && !InMenus();
    const bool up = focus && (Down(k.up) || ((turn || fol) && Down('W'))), down = focus && (Down(k.down) || ((turn || fol) && Down('S')));
    const bool left = focus && (Down(k.left) || ((turn || fol) && Down('A'))), right = focus && (Down(k.right) || ((turn || fol) && Down('D')));
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
    if (focus && (Down(k.triangle) || (fol && Down(VK_RBUTTON)))) bits |= bitOf(4);
    if (focus && (Down(k.cross) || (turn && (Down(VK_RETURN) || Down(VK_SPACE))) || (fol && Down(VK_LBUTTON)))) bits |= bitOf(5);
    if (focus && (Down(k.square) || (fol && Down('Q')))) bits |= bitOf(6);
    if (focus && (Down(k.circle) || (turn && (Down(VK_ESCAPE) || Down(VK_BACK))) || (fol && Down(VK_LCONTROL)))) bits |= bitOf(7);
    if (focus && (Down(k.r2) || (fol && Down(VK_SPACE)))) bits |= bitOf(11);
    const bool back = k.back && focus && !InMenus() && Down(k.back);
    if (back && !g_backWas[slot - 1]) CoopAskBack(slot);
    g_backWas[slot - 1] = back;
    rec[0x1C] = 0;
    rec[0x0C] = 0;
    *(uint16_t*)(rec + 0x42) = g_last[slot - 1];
    *(uint16_t*)(rec + 0x40) = bits;
    g_last[slot - 1] = bits;
}

// SimPads=3: the slot (1..3) takes player 1's input record (see the top of the file). Player 1's record is already filled
// for this frame (the game's buttons and axes, then ROTK Fix's controller or the keyboard), so its sticks, digital
// direction flags and buttons are copied as they are.
static void MirrorSlot(int slot)
{
    uint8_t* rec = (uint8_t*)(kRecordBase + slot * 0x58);
    const uint8_t* p1 = (const uint8_t*)kRecordBase;
    const uint16_t* bit = (const uint16_t*)kButtonBit;
    auto bitOf = [&](int id) { return bit[id] ? bit[id] : (uint16_t)(1u << id); };
    const uint16_t held = *(const uint16_t*)(p1 + 0x40);
    const float* lst = (const float*)(p1 + 0x10);
    const bool busy = held != 0 || fabsf(lst[0]) > 0.5f || fabsf(lst[1]) > 0.5f;   // a button or the stick still held
    // this player's turn in the menus: only after what was held when it started (the confirming button) is released
    bool turn = SharedKeysTurn(slot);
    bool& blocked = g_sharedBlocked[slot - 1];
    if (!turn) blocked = busy;
    else if (blocked && !busy) blocked = false;
    uint16_t bits = 0;
    if (!InMenus()) {
        memcpy(rec, p1, 0x20);
        bits = (uint16_t)(held & ~(bitOf(12) | bitOf(13)));   // not Start and Select
    } else if (turn && !blocked) {
        memcpy(rec, p1, 0x20);
        bits = held;
    } else {
        memset(rec, 0, 0x20);
    }
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
        if (g_mirror) MirrorSlot(slot);
        else SimSlot(slot);
    }
}

// replaces 0x49D480: "there are at least two controllers"
static NAKED void TwoControllers()
{
    ASM_BEGIN
    ASM(mov  al, 2)
    ASM(ret)
    ASM_END
}

void SimInstall(int mode)
{
    if (mode <= 0) return;
    g_follow = mode == 2;
    g_mirror = mode == 3;
    // 0x49D480: call 0x4B0470; test eax,eax; setg al; inc al; ret
    const uint8_t* f = (const uint8_t*)0x49D480;
    if (f[0] != 0xE8 || f[5] != 0x85 || f[6] != 0xC0 || f[0xC] != 0xC3) {
        Log("sim: 0x49D480 is not what was expected, simulated pads not installed");
        return;
    }
    g_sim = PatchJmp(0x49D480, nullptr, 5, (const void*)&TwoControllers);
    Log("sim: %s: %s", g_mirror ? "players 2, 3 and 4 driven by player 1's controller (mirror)"
                       : g_follow ? "keyboard-simulated pads for players 2, 3 and 4, following player 1's keys and mouse in the levels"
                                  : "keyboard-simulated pads for players 2, 3 and 4",
        g_sim ? "active" : "NOT installed");
}
