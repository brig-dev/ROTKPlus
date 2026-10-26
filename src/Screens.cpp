// Screens.cpp - turns of the extra players in the end-of-level screens, hero selection, main menu entries
// Language: C++17, MSVC, Win32.
//
// The screens change with 0x47FA20(mode): previous (0x562AC4) = current (0x562AC0), current = new. After a victory in
// co-op: the score screen (mode 2) twice, first for player 1 (previous 0) and then for player 2 (mode 2 is requested
// again, previous 2); the co-op statistics (16) once in two columns; the shop (3) twice, player 1 (previous 16) and
// player 2 (previous 3); then saving (6). The screens pick the hero with 0x47EF60/0x47EF80 or by reading 0x562A84/0x562A88
// directly.
//
// Extra players' turn: when the game is about to leave player 2's turn of one of these screens, another round of the
// same mode is requested and, for its whole duration, player 2's choice (0x562A88) is the extra player's hero and
// "player 2's own profile" (0x562A9C) is off, so the shop does not swap the records. At the end of the round
// everything is restored. The co-op statistics are shown once more with the extra player in the second column.
// Fourth player: same road, one more round right after the third player's (input slot 3, the fourth player's hero in
// place of player 2's). Score and statistics: leaving the third player's round repeats the same mode for the fourth.
// Shop: the third player's "continue" (UpgradeContinue) passes to the fourth.
//
// Per-turn pads: 0x4EA200(slot) writes to 0x56DC75 the EXCLUDED slot, not the hooked one: the menu loop 0x4EAA60 runs
// over slots 0..3 and for the slot equal to 0x56DC75 takes only a reduced branch (4 or 6 = no slot excluded). With two
// players it is enough to exclude the other player's pad: on confirming a selection (0x457FAA) the one who just chose
// is excluded, score and shop exclude the other player's slot (0x8E2095 pad slot of player 1, written by 0x4E8180;
// 0x8E2094 of player 2, by 0x4E81A0; read by 0x4E81E0/0x4E81F0). The extra players' slots (2 and 3, Pads.cpp) are never
// excluded, so their pads used to command in every turn. Here a filter at the start of the loop (0x4EAAA0,
// MenuSlotStub): in an extra player's turn only that slot passes, in the turns of players 1 and 2 every slot but 2 and 3
// passes and the game decides the rest. (Letting only "the turn's slot" pass for players 1 and 2 did not work: in the
// turns the game changes which pad feeds records 0 and 1, so for players 1 and 2 the slot is not chosen, only the extra
// players' slots are taken out.)
//
// Hero selection (mode 1, CharSelScreen driven by the menu script): turns of player 1 and then 2 (player 2's circle goes
// back to player 1's turn); the script calls 0x47DF00 (player 1's choice) and 0x47DF20 (player 2's), then requests
// loading (0x47FA20(14), about three seconds later). Extra players' turn: that request reopens the selection
// (0x47FA20(1)); in the turn "of player 1" the extra player chooses, the choice does not touch 0x562A84 but becomes the
// extra player's hero (CharSelChoice, used by Coop.cpp at creation) and the next extra player's turn comes, or loading.
// A hero already chosen by the others reopens the selection.
//
// Selection scene object (0x456740 creates it, 0x456B40 reopens it clearing 0x562A84/0x562A88): the state of each hero
// is at +0x124 + hero*4, computed by 0x456210(object, hero): 4 if the hero is player 1's choice (+0x254) or player 2's
// (+0x258), i.e. already taken and not selectable (0x456610 answers 1); 5 available; 1, 2, 3 locked or completed.
// 0x4559E0 puts on the hero in state 4 the element Player1Number (+0x2F4, value +0x88 = 1.0) if it is player 1's choice,
// otherwise Player2Number (+0x2F8, 2.0). On confirm (0x457FAA) the scene excludes with 0x4EA200 the pad of the one who
// chose (0 for player 1's choice, 1 for player 2's). In an extra player's turn: 0x456210 answers 4 for the heroes already
// taken; Player1Number holds the extra player's number; the label of player 1 becomes "Player N".
#include "ROTKPlus.h"
#include <string.h>
#include <stdio.h>

static int* const kMode      = (int*)0x562AC0;
static int* const kPrevMode  = (int*)0x562AC4;
static int* const kModeExtra = (int*)0x562ABC;
static int* const kChoice2   = (int*)0x562A88;
static char* const kOwnProfile2 = (char*)0x562A9C;
static int* const kCoop      = (int*)0x562A98;

enum { MODE_CHARSEL = 1, MODE_SCORING = 2, MODE_UPGRADE = 3, MODE_SCENESEL = 4, MODE_SAVE = 6, MODE_LOADING = 14,
       MODE_COOPSTATS = 16, MODE_MAINMENU = 17 };

static const uint8_t kAnySlot = 0xFF;

static int  g_lastHero[4] = { -1, -1, -1, -1 };   // heroes of the extra players (slots 2 and 3) in the last level played (valid in the screens after it)
static int  g_turn;             // 0 none, otherwise the mode of the extra player's round in progress
static int  g_turnSlot = 2;     // extra player of the round in progress (2 third, 3 fourth)
static bool g_done[4][32];      // round of the extra player already done, per slot and mode
static int  g_savedChoice2;
static char g_savedOwn2;

// hero selection
static bool g_csTurn;           // an extra player's turn in the current selection
static int  g_csSlot = 2;       // extra player of that turn (2 third, 3 fourth)
static bool g_csDone;           // every extra player has chosen, for this selection
static int  g_csChoice[4] = { -1, -1, -1, -1 };   // heroes chosen by the extra players (slots 2 and 3)
static int  g_csSaved1, g_csSaved2;
static uint8_t* g_csScene;      // selection scene object (from 0x456210), valid in mode 1

// slot filter in the menu loop 0x4EAA60, recomputed every frame: kAnySlot = all,
// kOnly | s = only slot s, kExcept2 = all but slots 2 and 3 (the extra players' pads)
static const uint8_t kOnly = 0x80, kExcept2 = 0x20;
static uint8_t g_menuSlot = kAnySlot;
static uint8_t* g_csLabel;          // element of player 1's label that was changed, nullptr = not changed
static uint32_t g_csLabelOld;       // its original text (+0x8C)
static uint8_t* g_csNumber;         // Player1Number that was changed
static char     g_csLabelText[64] = "@Player 3";   // of the extra player whose turn it is
static DWORD    g_csLoadAt;         // next step deferred after an extra player's choice (GetTickCount), 0 = none

void ScreensSetHeroes(int hero3, int hero4)   // from Coop.cpp when the extra players are created (and -1 -1 for levels without)
{
    g_lastHero[2] = hero3;
    g_lastHero[3] = hero4;
    for (auto& row : g_done) for (bool& d : row) d = false;
}

static void BeginTurn(int mode, int slot)
{
    g_turn = mode;
    g_turnSlot = slot;
    g_done[slot][mode] = true;
    g_savedChoice2 = *kChoice2;
    g_savedOwn2 = *kOwnProfile2;
    *kChoice2 = g_lastHero[slot];
    *kOwnProfile2 = 0;
    Log("screens: turn of player %d (hero %d) in mode %d, pad %s", slot + 1, g_lastHero[slot], mode,
        PadConnected(slot) ? "its own" : "player 2's (pad not connected)");
}

static void EndTurn()
{
    if (!g_turn) return;
    *kChoice2 = g_savedChoice2;
    *kOwnProfile2 = g_savedOwn2;
    Log("screens: end of player %d's turn in mode %d", g_turnSlot + 1, g_turn);
    g_turn = 0;
}

static void RawSetMode(int m)
{
    *kPrevMode = *kMode;
    *kMode = m;
    *kModeExtra = -1;
    if (m == 0x12) ((void(__cdecl*)(int))0x4BCE50)(m);
}

// --- hero selection -----------------------------------------------------------------------------
int CharSelChoice(int slot) { return slot >= 2 && slot <= 3 ? g_csChoice[slot] : -1; }

// 0x4ED940(0, name): looks an element up by name (0x4F6A90 uses it and then also sets the visibility +0x74)
static uint8_t* FindElement(const char* name)
{
    return ((uint8_t*(__cdecl*)(int, const char*))0x4ED940)(0, name);
}

static void CharSelRestoreScreen()
{
    if (g_csLabel) { *(uint32_t*)(g_csLabel + 0x8C) = g_csLabelOld; g_csLabel = nullptr; }
    if (g_csNumber) { *(float*)(g_csNumber + 0x88) = 1.0f; g_csNumber = nullptr; }
}

// label and number of player 1 -> the extra player's, when the scene is (re)opened during its turn
static void CharSelMarkScreen(uint8_t* scene)
{
    if (uint8_t* num = *(uint8_t**)(scene + 0x2F4)) {
        *(float*)(num + 0x88) = (float)(g_csSlot + 1);
        g_csNumber = num;
    }
    if (g_csLabel) return;
    static const char* const names[] = { "CharSelScreen.Player1Label", "CharSelScreen.player1label",
                                         "CharSelScreen.Player1", "CharSelScreen.P1Label" };
    for (const char* n : names) {
        uint8_t* e = FindElement(n);
        if (!e) continue;
        g_csLabel = e;
        g_csLabelOld = *(uint32_t*)(e + 0x8C);
        const char* old = (const char*)(uintptr_t)g_csLabelOld;
        char shown[64] = "?";
        if (old && !IsBadStringPtrA(old, 64)) {
            strncpy_s(shown, old, _TRUNCATE);
            // literal text ("@Player 1"): the same text with the extra player's number in place of the last 1;
            // otherwise (the label is the name of a game text, "charSelPlayer1Sel") the text in the game's language
            // with the number of the extra player whose turn it is (Lang.cpp)
            if (old[0] == '@') {
                strncpy_s(g_csLabelText, old, _TRUNCATE);
                if (char* d = strrchr(g_csLabelText, '1')) *d = (char)('0' + g_csSlot + 1);
            } else {
                sprintf_s(g_csLabelText, "@%s", LangText(3, g_csSlot + 1).c_str());
            }
        }
        *(const char**)(e + 0x8C) = g_csLabelText;
        Log("selection: label %s (\"%s\") -> \"%s\"", n, shown, g_csLabelText);
        return;
    }
    Log("selection: element of player 1's label not found");
}

// replaces 0x456210(scene, hero): state of the hero in the scene
static __declspec(naked) void HeroStateOrig()   // push esi; mov esi,[esp+0Ch]; push edi
{
    __asm {
        push esi
        mov  esi, dword ptr [esp + 0x0C]
        push edi
        push 0x456216
        ret
    }
}

static int __cdecl HeroState(uint8_t* scene, int hero)
{
    if (*kMode == MODE_CHARSEL) g_csScene = scene;
    if (g_csTurn && *kMode == MODE_CHARSEL) {
        if (hero == 0) CharSelMarkScreen(scene);
        if (hero == g_csSaved1 || hero == g_csSaved2 || (g_csSlot == 3 && hero == g_csChoice[2])) return 4;   // already taken: like the other players' choice
    }
    return GameFn((int(__cdecl*)(uint8_t*, int))&HeroStateOrig)(scene, hero);
}

// --- per-turn pads in the menus -----------------------------------------------------------------
// Filter: only in co-op with extra players, with at least one extra pad connected, and in the turn-based screens.
static uint8_t TurnSlot()
{
    if (!g_cfg.coop || *kCoop != 1 || (!PadConnected(2) && !PadConnected(3))) return kAnySlot;
    const int cur = *kMode;
    // 2 players chosen in the menu: the extra players' pads do not command in the turn-based screens
    if (!Coop3Wanted())
        return cur == MODE_CHARSEL || cur == MODE_SCORING || cur == MODE_UPGRADE || cur == MODE_COOPSTATS
                   ? kExcept2 : kAnySlot;
    if (cur == MODE_CHARSEL) {
        if (g_csTurn) return PadConnected(g_csSlot) ? kOnly | (uint8_t)g_csSlot : kAnySlot;
        if (g_csScene && *(const int*)(g_csScene + 0x254) != -1 && *(const int*)(g_csScene + 0x258) != -1)
            return kAnySlot;                                     // choices made, waiting for loading
        return kExcept2;                                         // turns of players 1 and 2
    }
    if (g_lastHero[2] < 0) return kAnySlot;
    if (cur == MODE_SCORING || cur == MODE_UPGRADE || cur == MODE_COOPSTATS) {
        if (g_turn != cur) return kExcept2;
        return PadConnected(g_turnSlot) ? kOnly | (uint8_t)g_turnSlot : kAnySlot;
    }
    return kAnySlot;
}

// the slot that alone commands the menus right now (an extra player's turn), -1 if none (used by Sim.cpp)
int ScreensOnlySlot() { return (g_menuSlot != kAnySlot && (g_menuSlot & kOnly)) ? (g_menuSlot & 0x0F) : -1; }

static void UpdateMenuSlot()
{
    const uint8_t s = TurnSlot();
    if (s != g_menuSlot) {
        if (s == kAnySlot) Log("menu: pad of any player (mode %d)", *kMode);
        else if (s & kOnly) Log("menu: only slot %d (mode %d)", s & 0x0F, *kMode);
        else Log("menu: all but slots 2 and 3 (mode %d)", *kMode);
    }
    g_menuSlot = s;
}

// at the start of every round of the menu loop 0x4EAA60 (0x4EAAA0, slot in bl): a slot excluded by the filter jumps
// to the next round (0x4EB25A); otherwise the two original instructions and on to 0x4EAAAC
static __declspec(naked) void MenuSlotStub()
{
    __asm {
        mov  al, byte ptr [g_menuSlot]
        cmp  al, 0xFF
        je   orig
        test al, 0x80
        jz   except
        and  al, 0x0F
        cmp  bl, al
        je   orig
        jmp  skip
    except:
        cmp  bl, 2                   // kExcept2: skip slots 2 and 3
        jb   orig
    skip:
        push 0x4EB25A
        ret
    orig:
        cmp  bl, byte ptr ds:[0x56DC75]
        mov  byte ptr ds:[0x8E2111], bl
        push 0x4EAAAC
        ret
    }
}

// replaces 0x4EA200(slot): slot excluded from the menus (0x56DC75)
static void __cdecl LockPad(int slot)
{
    *(uint8_t*)0x56DC75 = (uint8_t)slot;
}

static void CharSelEndTurn(const char* why)
{
    if (!g_csTurn) return;
    g_csTurn = false;
    CharSelRestoreScreen();
    Log("selection: end of player %d's turn (%s)", g_csSlot + 1, why);
}

static void CharSelBeginTurn(int slot)
{
    g_csTurn = true;
    g_csSlot = slot;
    g_csSaved1 = *(const int*)0x562A84;
    g_csSaved2 = *kChoice2;
    Log("selection: turn of player %d (players 1 and 2: heroes %d and %d, third player %d), pad %s", slot + 1, g_csSaved1, g_csSaved2,
        g_csChoice[2], PadConnected(slot) ? "its own" : "any player's (pad not connected)");
}

// from 0x47DF00 (player 1) and 0x47DF20 (player 2), replaced in Support.cpp: true = the choice belongs to an extra player
bool CharSelOnChoice(int player, int hero)
{
    if (!g_csTurn || *kMode != MODE_CHARSEL) return false;
    if (g_csLoadAt) return true;    // the extra player has already chosen: waiting for the next step
    if (player != 1) return true;   // in an extra player's turn player 2 does not choose (for safety)
    if (hero < 0 || hero > 8 || hero == g_csSaved1 || hero == g_csSaved2 || (g_csSlot == 3 && hero == g_csChoice[2])) {
        Log("selection: player %d chose hero %d, already taken: the turn is repeated", g_csSlot + 1, hero);
        RawSetMode(MODE_CHARSEL);
        return true;
    }
    g_csChoice[g_csSlot] = hero;
    if (g_csSlot >= CoopPlayers() - 1) g_csDone = true;
    // reopening the screen the game clears player 2's choice (0x562A88 = -1) without going through 0x47DF20: loading
    // would start without a hero for player 2 and the game crashed
    Log("selection: choices before restoring %d and %d, restored %d and %d", *(const int*)0x562A84, *kChoice2,
        g_csSaved1, g_csSaved2);
    *(int*)0x562A84 = g_csSaved1;
    *kChoice2 = g_csSaved2;
    // as for players 1 and 2 (the script requests loading about 3.1 seconds after player 2's choice), wait for the
    // chosen hero to play its animation: loading immediately skipped it
    g_csLoadAt = GetTickCount() + 3100;
    if (!g_csLoadAt) g_csLoadAt = 1;
    Log("selection: player %d plays %d, next step in 3.1 seconds", g_csSlot + 1, hero);
    return true;
}

// every frame (Pads.cpp) and after every script call (Support.cpp): deferred actions
static void MainMenuFrame();

// end of the turn of an extra player who has chosen: the next one's turn, or loading starts
static void CharSelAdvance()
{
    *(int*)0x562A84 = g_csSaved1;
    *kChoice2 = g_csSaved2;
    CharSelEndTurn("choice made");
    if (g_csSlot < CoopPlayers() - 1) {
        CharSelBeginTurn(g_csSlot + 1);
        RawSetMode(MODE_CHARSEL);
        return;
    }
    Log("selection: loading after the last extra player's choice");
    RawSetMode(MODE_LOADING);
}

void ScreensFrame()
{
    UpdateMenuSlot();
    MainMenuFrame();
    if (!g_csLoadAt || (int)(GetTickCount() - g_csLoadAt) < 0) return;
    g_csLoadAt = 0;
    if (!g_csTurn || *kMode != MODE_CHARSEL) return;
    CharSelAdvance();
}

// replaces 0x47FA20
// Versus mode: the level is chosen on the map as usual (the versus mode is meant to work on any level, enemies
// included, "everybody against everybody"). Skipping the map left the hero selection without its 3D scene and the game
// crashed, so the level is not forced.
static void __cdecl SetMode(int m)
{
    const int cur = *kMode, prev = *kPrevMode;
    // hero selection
    if (m == MODE_CHARSEL && cur != MODE_CHARSEL) { g_csDone = false; g_csChoice[2] = g_csChoice[3] = -1; g_csScene = nullptr; }
    if (cur == MODE_CHARSEL && m == MODE_LOADING && Coop3Wanted() && *kCoop == 1 && !g_csDone && !g_csTurn) {
        CharSelBeginTurn(2);
        RawSetMode(MODE_CHARSEL);
        return;
    }
    // The script's loading request, about 3.1 s after an extra player's choice, arrives together with our timer: if more
    // extra players still have to choose it counts as the end of the turn (otherwise the fourth player would be skipped)
    if (g_csTurn && cur == MODE_CHARSEL && m == MODE_LOADING && g_csChoice[g_csSlot] >= 0 && g_csSlot < CoopPlayers() - 1) {
        g_csLoadAt = 0;
        CharSelAdvance();
        return;
    }
    if (g_csTurn && m != MODE_CHARSEL) {
        g_csLoadAt = 0;
        // leaving without the extra player's choice (e.g. circle, back to the map): players 1 and 2's choices as they were
        // before the turn
        *(int*)0x562A84 = g_csSaved1;
        *kChoice2 = g_csSaved2;
        CharSelEndTurn("left the selection");
    }
    if (g_lastHero[2] >= 0 && *kCoop == 1 && cur != m) {
        // leaving a screen: if it was an extra player's round it ends (or passes to the fourth player)
        if (g_turn == cur && !(cur == MODE_COOPSTATS && m != MODE_UPGRADE)) {
            const int next = g_turnSlot + 1;
            // the shop passes to the fourth player from "continue" (UpgradeContinue)
            if (cur != MODE_UPGRADE && next <= 3 && g_lastHero[next] >= 0 && !g_done[next][cur]) {
                EndTurn();
                BeginTurn(cur, next);
                RawSetMode(cur);
                return;
            }
            EndTurn();
        }
        // score and shop: player 2's turn is being left (previous == current) -> the third player's round
        else if ((cur == MODE_SCORING || cur == MODE_UPGRADE) && prev == cur && !g_done[2][cur]) {
            BeginTurn(cur, 2);
            RawSetMode(cur);
            return;
        }
        // co-op statistics: once in two columns -> a second time with the third player in the second column (only
        // towards the shop: the screen that was just opened changes mode by itself, and starting the third player's turn
        // at once hid player 2's page)
        else if (cur == MODE_COOPSTATS && m == MODE_UPGRADE && !g_done[2][cur]) {
            BeginTurn(cur, 2);
            RawSetMode(cur);
            return;
        }
    }
    if (m == MODE_SAVE || m == 4) EndTurn();   // safety: never leave the screens with the choice changed
    RawSetMode(m);
}

// --- shop: "continue" (0x4F4110) ----------------------------------------------------------------
// With the shop state 0x8E57A0 == 4 ("continue"): if it is player 2's turn (previous == 3) the game writes player 2's
// record back to its profile (0x47F9D0) and asks "save?" at once; otherwise, in co-op, it passes to player 2 with
// 0x47FA20(3). Here, at the end of player 2's turn, the game's part that concerns player 2 is done (closing the window
// 0x4F7920 and 0x47F9D0) and the third player's turn starts; the question comes at the end of the last extra player's
// turn. Third -> fourth: only the window is closed (the extra players do not own a profile).
static __declspec(naked) void ContinueOrig()   // mov eax,[8E57A0h]; add eax,-2
{
    __asm {
        mov  eax, dword ptr ds:[0x8E57A0]
        add  eax, -2
        push 0x4F4118
        ret
    }
}

static void __cdecl UpgradeContinue()
{
    if (*(const int*)0x8E57A0 == 4 && *kPrevMode == MODE_UPGRADE && *kMode == MODE_UPGRADE &&
        g_lastHero[2] >= 0 && *kCoop == 1) {
        int next = -1;
        if (g_turn != MODE_UPGRADE) { if (!g_done[2][MODE_UPGRADE]) next = 2; }                      // end of player 2 -> third
        else if (g_turnSlot == 2 && g_lastHero[3] >= 0 && !g_done[3][MODE_UPGRADE]) next = 3;      // end of the third -> fourth
        if (next >= 0) {
            ((void(__cdecl*)())0x4F7920)();
            if (next == 2) ((void(__cdecl*)())0x47F9D0)();   // player 2's record goes back to its profile
            else EndTurn();
            BeginTurn(MODE_UPGRADE, next);
            RawSetMode(MODE_UPGRADE);
            return;
        }
    }
    GameFn((void(__cdecl*)())&ContinueOrig)();
}

// --- main menu: "Co-op" and "Versus" with 2, 3 or 4 players --------------------------------------------
// The main menu (MainMenuScreen, mode 17) has the entries Single, Coop, Online, PlayTrailer, Exit; 0x4EEF60 enables Coop
// (and colours its child label CoopLoadText1) only with at least two controllers. The selected element is at 0x8E2F70.
// Buttons become actions in the loop 0x4EAA60 and go to the handler [0x56DC70] (default 0x4EA0F0, table at 0x4EA198):
// 0 and 2 move the cursor (0x4E9B60, 0x4E9BF0), 4 and 5 (right and left) go only to the selected element and do
// nothing on Coop. Here, with Coop selected, right and left cycle through 2, 3 and 4 players; the label of Coop shows
// the choice, which stays in ROTKPlus.ini ([Coop] Players). With 2 players no extra player is loaded or has turns.
// Coop has two child labels with the same text "mainMenuCoop", one for the normal entry and one for the highlighted
// one (changing only the first showed the original text when the cursor was on it): all child labels are changed.
//
// Adding a real entry would mean changing the screens' data; but the menu already has the entry Online, hidden by
// 0x4EEF10 (visibility from 0x4E4750) in the empty space between Credits and Exit. The two entries become
// "Co-op: N players" (Coop) and "Versus: N players" (Online, made visible and enabled with 0x4FC3A0), each with its
// own value in the ini ([Coop] Players, [PvP] Players). Actions go to the selected element's scripts: 6 the one at +0x78
// (0x4E9F70), 7 the one at +0x8C (0x4E9F00, the confirmation: Coop has its "rpn!" there), 8 the one at +0x90 (0x4E9E90).
// Confirming Versus moves the selection to Coop and lets the co-op script run, with the versus mode turned on;
// confirming Coop turns it off.
struct MenuItem {
    const char* name;      // element of the screen
    uint8_t*    elem;
    uint8_t*    texts[4];  // child labels that were changed (normal and highlighted)
    uint32_t    old[4];    // their original texts
    int         n;
    int         players;   // 2, 3 or 4 players
    char        shown[96];
};
static MenuItem g_coopItem = { "MainMenuScreen.Coop", nullptr, {}, {}, 0, 3 };
static MenuItem g_pvpItem  = { "MainMenuScreen.Online", nullptr, {}, {}, 0, 2 };
static int  g_wantN = 3;        // players of the game in progress (from the confirmed entry)

int  CoopPlayers() { return g_cfg.coop ? g_wantN : 2; }
bool Coop3Wanted() { return CoopPlayers() >= 3; }
bool Coop4Wanted() { return CoopPlayers() >= 4; }
void PvpSetActive(bool on);     // Pvp.cpp
bool PvpAllowed();              // Pvp.cpp

static void ShowChoice(MenuItem& m)
{
    const std::string label = LangText(&m == &g_coopItem ? 0 : 1);
    const std::string text = "@" + LangText(2, m.players, "", label.c_str());
    strncpy_s(m.shown, text.c_str(), _TRUNCATE);
}

static void LogTree(uint8_t* e, int depth)
{
    if (!e || depth > 3) return;
    const char* t = *(const char**)(e + 0x8C);
    Log("main menu: %*s element %08X type %u text \"%s\"", depth * 2, "", (unsigned)(uintptr_t)e,
        *(const uint16_t*)(e + 0x76), t && !IsBadStringPtrA(t, 64) ? t : "");
    const uint32_t* kids = *(const uint32_t**)(e + 4);
    if (kids) for (uint32_t i = 1; i <= kids[0] && i < 16; ++i) LogTree((uint8_t*)(uintptr_t)kids[i], depth + 1);
}

static void RestoreItem(MenuItem& m)
{
    for (int i = 0; i < m.n; ++i) *(uint32_t*)(m.texts[i] + 0x8C) = m.old[i];
    m.n = 0;
    m.elem = nullptr;
}

// finds the entry and its child labels (text elements, type 0x82); true if the entry exists
static bool HookItem(MenuItem& m)
{
    uint8_t* e = FindElement(m.name);
    if (!e) { RestoreItem(m); return false; }
    if (e == m.elem) return true;
    RestoreItem(m);
    m.elem = e;
    LogTree(e, 0);
    const uint32_t* kids = *(const uint32_t**)(e + 4);
    if (kids)
        for (uint32_t i = 1; i <= kids[0] && i < 16 && m.n < 4; ++i) {
            uint8_t* k = (uint8_t*)(uintptr_t)kids[i];
            if (!k || *(const uint16_t*)(k + 0x76) != 0x82) continue;
            const char* kt = *(const char**)(k + 0x8C);
            if (!kt || IsBadStringPtrA(kt, 64)) continue;
            m.old[m.n] = (uint32_t)(uintptr_t)kt;
            m.texts[m.n++] = k;
        }
    ShowChoice(m);
    Log("main menu: entry %s, %d labels -> \"%s\"", m.name, m.n, m.shown);
    return true;
}

// Versus right below Co-op. The cursor (0x4E9B60/0x4E9BF0) runs over the screen's children (0x8E2F74, otherwise
// 0x8E2F68; +4 -> {count, elements...}) and goes to the next visible one of type 10: the order in the list counts, not the
// position. So the Online entry is moved in the list right after Coop and the visible entries in between move down one
// place: each takes the vertical position (+0x44) of the next, and Online the first's. Changing only +0x44 of the entries
// (type 10 groups) moved the cursor correctly but nothing on the screen: the child labels have positions of their own, so
// they are moved too, by the same amount. Once per menu opening (if it is already after Coop nothing is done).
static void MoveVersusUnderCoop()
{
    uint8_t* root = *(uint8_t**)0x8E2F74;
    if (!root) root = *(uint8_t**)0x8E2F68;
    if (!root || !g_coopItem.elem || !g_pvpItem.elem) return;
    uint32_t* kids = *(uint32_t**)(root + 4);
    if (!kids) return;
    const int n = (int)kids[0];
    int ic = -1, io = -1;
    for (int i = 0; i < n && i < 64; ++i) {
        if (kids[1 + i] == (uint32_t)(uintptr_t)g_coopItem.elem) ic = i;
        if (kids[1 + i] == (uint32_t)(uintptr_t)g_pvpItem.elem) io = i;
    }
    if (ic < 0 || io < 0 || io == ic + 1 || io < ic) return;
    // visible entries between Coop and Online, then Online: the positions slide one place down
    uint8_t* row[16]; int r = 0;
    for (int i = ic + 1; i <= io && r < 16; ++i) {
        uint8_t* e = (uint8_t*)(uintptr_t)kids[1 + i];
        if (e && *(const uint16_t*)(e + 0x76) == 10 && (*(const uint16_t*)(e + 0x74) != 0 || e == g_pvpItem.elem)) row[r++] = e;
    }
    if (r >= 2) {
        // position of an entry: that of its first label (the groups may all have the same one)
        auto rowY = [](uint8_t* e) {
            const uint32_t* sub = *(const uint32_t**)(e + 4);
            return sub && sub[0] && sub[1] ? *(float*)(uintptr_t)(sub[1] + 0x44) : *(float*)(e + 0x44);
        };
        float y[16];
        for (int k = 0; k < r; ++k) y[k] = rowY(row[k]);
        for (int k = 0; k < r; ++k) {
            const float target = k + 1 < r ? y[k + 1] : y[0];   // the last (Online) takes the first's place
            const float dy = target - y[k];
            *(float*)(row[k] + 0x44) += dy;
            const uint32_t* sub = *(const uint32_t**)(row[k] + 4);
            if (sub)
                for (uint32_t i = 1; i <= sub[0] && i < 16; ++i)
                    if (uint8_t* c = (uint8_t*)(uintptr_t)sub[i]) *(float*)(c + 0x44) += dy;
        }
    }
    // in the list: Online right after Coop
    const uint32_t online = kids[1 + io];
    for (int i = io; i > ic + 1; --i) kids[1 + i] = kids[i];
    kids[1 + ic + 1] = online;
    Log("main menu: Versus moved under Co-op (%d entries moved down one place; positions %g %g %g)", r - 1,
        r > 0 ? *(float*)(row[0] + 0x44) : 0.0f, r > 1 ? *(float*)(row[1] + 0x44) : 0.0f, r > 2 ? *(float*)(row[2] + 0x44) : 0.0f);
}

// every frame in the main menu: labels of the two entries, Versus entry visible and enabled
static void MainMenuFrame()
{
    if (!g_cfg.coop || *kMode != MODE_MAINMENU) { RestoreItem(g_coopItem); RestoreItem(g_pvpItem); return; }
    if (HookItem(g_coopItem))
        for (int i = 0; i < g_coopItem.n; ++i) *(const char**)(g_coopItem.texts[i] + 0x8C) = g_coopItem.shown;
    if (!PvpAllowed()) return;
    uint8_t* before = g_pvpItem.elem;
    if (HookItem(g_pvpItem)) {
        *(uint16_t*)(g_pvpItem.elem + 0x74) = 1;
        for (int i = 0; i < g_pvpItem.n; ++i) {
            *(uint16_t*)(g_pvpItem.texts[i] + 0x74) = 1;
            *(const char**)(g_pvpItem.texts[i] + 0x8C) = g_pvpItem.shown;
        }
        if (g_pvpItem.elem != before) ((void(__cdecl*)(uint8_t*))0x4FC3A0)(g_pvpItem.elem);   // enabled
        MoveVersusUnderCoop();
    }
}

static __declspec(naked) void MenuActionOrig()   // mov al,[8E2111h]
{
    __asm {
        mov  al, byte ptr ds:[0x8E2111]
        push 0x4EA0F5
        ret
    }
}

// replaces 0x4EA0F0(action): the menus' default action handler
static void __cdecl MenuAction(int action)
{
    const uint32_t sel = *(const uint32_t*)0x8E2F70;
    if (*kMode == MODE_MAINMENU && sel) {
        MenuItem* m = sel == (uint32_t)(uintptr_t)g_coopItem.elem ? &g_coopItem
                    : sel == (uint32_t)(uintptr_t)g_pvpItem.elem ? &g_pvpItem : nullptr;
        if (m && (action == 4 || action == 5)) {   // right (4) adds a player, left (5) removes one: 2 -> 3 -> 4 and round again
            m->players += action == 4 ? 1 : -1;
            if (m->players > 4) m->players = 2;
            if (m->players < 2) m->players = 4;
            ShowChoice(*m);
            IniWriteInt(m == &g_coopItem ? "Coop" : "PvP", "Players", m->players);
            Log("main menu: %s with %d players", m == &g_coopItem ? "Co-op" : "Versus", m->players);
            return;
        }
        if (m && action == 7) {   // confirm
            const bool pvp = m == &g_pvpItem;
            g_wantN = m->players;
            PvpSetActive(pvp);
            Log("main menu: confirmed %s with %d players", m == &g_coopItem ? "Co-op" : "Versus", g_wantN);
            if (pvp && g_coopItem.elem) *(uint32_t*)0x8E2F70 = (uint32_t)(uintptr_t)g_coopItem.elem;   // the co-op script
        }
    }
    GameFn((void(__cdecl*)(int))&MenuActionOrig)(action);
}

bool ScreensInstall()
{
    {
        auto clamp = [](int v) { return v < 2 ? 2 : v > 4 ? 4 : v; };
        g_coopItem.players = clamp(IniInt("Coop", "Players", 3));
        g_pvpItem.players = clamp(IniInt("PvP", "Players", 2));
        g_wantN = g_coopItem.players;
        Log("main menu: Co-op with %d players, Versus with %d (left/right on the entry)", g_coopItem.players,
            g_pvpItem.players);
    }
    static const uint8_t actionOld[] = { 0xA0, 0x11, 0x21, 0x8E, 0x00 };
    bool okMenu = PatchJmp(0x4EA0F0, actionOld, sizeof actionOld, (const void*)&MenuAction);
    if (!okMenu) Log("main menu: choice of the number of players NOT installed");
    static const uint8_t old[] = { 0xA1, 0xC0, 0x2A, 0x56, 0x00 };
    static const uint8_t contOld[] = { 0xA1, 0xA0, 0x57, 0x8E, 0x00, 0x83, 0xC0, 0xFE };
    static const uint8_t stateOld[] = { 0x56, 0x8B, 0x74, 0x24, 0x0C, 0x57 };
    static const uint8_t lockOld[] = { 0x8A, 0x44, 0x24, 0x04, 0xA2, 0x75, 0xDC, 0x56, 0x00 };
    bool ok = PatchJmp(0x47FA20, old, sizeof old, (const void*)&SetMode);
    ok &= PatchJmp(0x456210, stateOld, sizeof stateOld, (const void*)&HeroState);
    ok &= PatchJmp(0x4EA200, lockOld, sizeof lockOld, (const void*)&LockPad);
    ok &= PatchJmp(0x4F4110, contOld, sizeof contOld, (const void*)&UpgradeContinue);
    static const uint8_t menuOld[] = { 0x3A, 0x1D, 0x75, 0xDC, 0x56, 0x00, 0x88, 0x1D, 0x11, 0x21, 0x8E, 0x00 };
    ok &= PatchJmp(0x4EAAA0, menuOld, sizeof menuOld, (const void*)&MenuSlotStub);
    Log("extra players' turns in the end-of-level screens: %s", ok ? "active" : "NOT installed");
    return ok;
}
