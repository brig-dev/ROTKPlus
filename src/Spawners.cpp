// Spawners.cpp - more enemies with three or four players ([Coop] MoreEnemies)
//
// The levels' enemies mostly come from spawners. The game keeps them in a list at 0x5C09FC (linked at +0x28), made by
// 0x48E120 from the level's data when the level is loaded (the class table's entry at 0x5660E0, object types 1000-1099).
// A spawner: +4 the enemies still to come (-1 = no end, 0 = done; one less at each enemy), +8 how many may be alive at
// once, +0xC flags (bit 0 = off), +0x10 alive now, +0x24 -> { id, character type (+0x78 of what it makes) }. 0x48DD70
// lets one out when alive < most at once, still to come != 0 and the level script's condition says so; 0x48E0D0 is told
// when one dies (alive - 1). The level scripts talk to a spawner with events of kind 100 (0x48DFB0 -> 0x48DE70): 0 on or
// off, 1 queue, 3 and 5 set still-to-come and most-at-once, 4, 6 and 7 read still-to-come, most-at-once and alive. Their
// waves rest on those numbers, so the extra enemies are made by the spawners themselves: both numbers are multiplied
// when a spawner is made and whenever a script sets them, and the scripts count the extra enemies as their own.
//
// Only the common enemies (soldiers, shields and archers of the orcs, Uruks, Uruk-hai, the Dead and the Easterlings) are
// multiplied: captains (the objectives count some of them), berserkers, trolls, mumakil, spiders, Nazgul and bosses are
// left as they are, and so are the enemies a level places by hand. The allies (the friendly types of 0x4189F0, the
// soldiers of Gondor and Rohan; log of 08/11/2026: Minas Tirith's types 0x12 and 0x20) get the same factor when
// AllyFactor=0, or their own (10 as in the game ... 50). In the campaign, co-op or Single (Who is playing? has the same
// two choices for a Single game, kept apart: [Coop] SingleEnemyFactor and SingleAllyFactor); never in a versus game. The co-op factor comes from the Multiplayer page (Players.cpp ->
// SpawnersConfigure) or from the ini: [Coop] EnemyFactor in tenths, -1 = by players (2: as in the game, 3: x1.5,
// 4: x2); [Coop] MoreEnemies=0 turns it all off.
#include "ROTKPlus.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static const uintptr_t kSpawners = 0x5C09FC;
static bool  g_on = true;      // [Coop] MoreEnemies
static int   g_factor = -1;    // [Coop] EnemyFactor, tenths; -1 = by players
static int   g_ally = 0;       // [Coop] AllyFactor: 0 = as the enemies, 10 = as in the game, 15 ... 50
static int   g_factor1 = 10;   // [Coop] SingleEnemyFactor: a Single game (Who is playing?), tenths
static int   g_ally1 = 0;      // [Coop] SingleAllyFactor
static bool  Good(int v) { return v >= 10 && v <= kMaxFactor && v % 5 == 0; }

// No level is left out (the player's choice: 08/11/2026). At the Top of the Wall (min00) the level counts the orcs on
// the wall and the game is lost when they are too many, which a high factor reaches soon; the count is not one of the
// spawners' (no reading command of theirs in the level). Whoever wants that level as in the game lowers the factor.
static int   g_capAlive = 16;  // [Coop] MoreEnemiesMaxAlive: most at once of one spawner after the multiplication
static int   g_lines;          // log lines of this level
static int   g_lastLevel = -1;

static bool Common(int type)
{
    static const int kinds[] = { 0x03, 0x1A, 0x23,          // orcs, archers
                                 0x15, 0x25, 0x26,          // Uruks of Mordor, shields, archers
                                 0x14, 0x29, 0x2A,          // Uruk-hai, archers
                                 0x2C, 0x2D, 0x2E,          // the Dead, shields, archers
                                 0x34, 0x35 };              // Easterlings, shields
    for (int k : kinds) if (k == type) return true;
    return false;
}

static bool Ally(int type)   // as 0x4189F0 (bit 0 of +0x5E8), less the types not seen as soldiers
{
    static const int kinds[] = { 0x12, 0x20, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3F, 0x40, 0x42, 0x46, 0x47, 0x48 };
    for (int k : kinds) if (k == type) return true;
    return false;
}

// the players of this level's campaign game: two, plus the extra players loaded with it; 1 a Single game; 0 a versus game
static int LevelPlayers()
{
    if (PvpActive()) return 0;
    if (*(const int*)0x562A98 != 1) return 1;
    return 2 + (HeroOfSlot(2) >= 0) + (HeroOfSlot(3) >= 0);
}

static float Factor()
{
    const int p = LevelPlayers();
    if (!g_on || p < 1) return 1.0f;
    const int t = p == 1 ? g_factor1 : g_factor >= 10 ? g_factor : p >= 4 ? 20 : p == 3 ? 15 : 10;
    return t / 10.0f;
}

static float AllyFactor()
{
    const int p = LevelPlayers();
    if (!g_on || p < 1) return 1.0f;
    const int a = p == 1 ? g_ally1 : g_ally;
    return a == 0 ? Factor() : a / 10.0f;
}

// the factor of a spawner's type: the common enemies, the allies, nothing else
static float FactorOf(int type)
{
    if (Common(type)) return Factor();
    if (Ally(type)) return AllyFactor();
    return 1.0f;
}

void SpawnersConfigure(bool single, int factorTenths, int allyTenths)
{
    const int f = Good(factorTenths) ? factorTenths : -1, a = Good(allyTenths) ? allyTenths : 0;
    if (single) { g_factor1 = f < 0 ? 10 : f; g_ally1 = a; }
    else { g_factor = f; g_ally = a; }
    Log("spawners: from %s: enemies x%.1f, allies %s x%.1f", single ? "Who is playing? (Single)" : "the Multiplayer page",
        (f < 0 ? 10 : f) / 10.0f, a ? "" : "as the enemies,", (a ? a : f < 0 ? 10 : f) / 10.0f);
}

// Spawners of one character (1 to come, 1 at once, when the level makes them) are the story's own: at Pelennor Fields
// 1157 (type 0x46, Dernhelm with Merry, whose life is on the HUD) and 1178/1179 (two Riders of Rohan of the scene on the
// hill). Multiplied, the copy followed Aragorn for ever and the scene with the oliphants did not go on (PC and Steam
// Machine, 09/11/2026, at x1.5 too): they are never multiplied, not even when a script sets them later.
static uint32_t g_unique[64];
static int      g_uniqueN;
static bool IsUnique(uint32_t s) { for (int i = 0; i < g_uniqueN; ++i) if (g_unique[i] == s) return true; return false; }

static void NewLevelCheck()
{
    const int level = *(const int*)0x562A54;
    if (level != g_lastLevel) { g_lastLevel = level; g_lines = 0; g_uniqueN = 0; }
}

static bool LogOk() { return g_lines++ < 400; }

static int TypeOf(uint32_t s, int* id)
{
    int t = -1;
    Guard([&] {
        const int* d = *(const int* const*)(s + 0x24);
        if (d) { *id = d[0]; t = d[1]; }
    });
    return t;
}

static int Scaled(int v, float f, bool alive)
{
    if (v <= 0 || f <= 1.0f) return v;   // -1 (no end) and 0 (done) stay
    int r = (int)floorf(v * f + 0.5f);
    if (alive && r > g_capAlive) r = v > g_capAlive ? v : g_capAlive;
    return r;
}

// one spawner just made from the level's data
static void OnSpawner(uint32_t s)
{
    NewLevelCheck();
    int id = -1;
    const int type = TypeOf(s, &id);
    int left = 0, most = 0, flags = 0;
    if (!Guard([&] { left = *(const int*)(s + 4); most = *(const int*)(s + 8); flags = *(const int*)(s + 0xC); })) return;
    const bool unique = left == 1 && most == 1;
    if (unique && g_uniqueN < 64) g_unique[g_uniqueN++] = s;
    const float f = unique ? 1.0f : FactorOf(type);
    const int left2 = Scaled(left, f, false), most2 = Scaled(most, f, true);
    if (f > 1.0f) Guard([&] { *(int*)(s + 4) = left2; *(int*)(s + 8) = most2; });
    if (LogOk())
        Log("spawners: level %d, spawner %d type %X%s: %d to come, %d at once%s -> %d, %d%s", *(const int*)0x562A54, id,
            type, unique ? " (one character: not multiplied)" : Common(type) ? "" : Ally(type) ? " (ally)" : " (not multiplied)",
            left, most, (flags & 1) ? ", off" : "", left2, most2, f > 1.0f ? "" : " (unchanged)");
}

typedef char(__cdecl* MakeFn)(uint32_t);
static char __cdecl MakeSpawner(uint32_t rec)
{
    const uint32_t before = *(const uint32_t*)kSpawners;
    if (!before) g_uniqueN = 0;   // the level's first spawner: a new level (the same one played again too)
    const char r = ((MakeFn)0x48E120)(rec);
    const uint32_t s = *(const uint32_t*)kSpawners;
    if (s && s != before) OnSpawner(s);
    return r;
}

// a level script's command to a spawner, after the game handled it (3: still to come, 5: most at once)
static void __cdecl AfterCommand(uint32_t ev, uint32_t s) ASM_NAME(AfterCommand);
static void __cdecl AfterCommand(uint32_t ev, uint32_t s)
{
    int cmd = -1;
    if (!s || !Guard([&] { cmd = *(const int*)(ev + 8); }) || (cmd != 3 && cmd != 5 && cmd != 0 && cmd != 1)) return;
    NewLevelCheck();
    int id = -1;
    const int type = TypeOf(s, &id);
    const float f = IsUnique(s) ? 1.0f : FactorOf(type);
    int v = 0;
    const int off = cmd == 3 ? 4 : cmd == 5 ? 8 : cmd == 1 ? 0x18 : 0xC;
    Guard([&] { v = *(const int*)(s + off); });
    int v2 = v;
    if (cmd == 3 || cmd == 5) {
        v2 = Scaled(v, f, cmd == 5);
        if (v2 != v) Guard([&] { *(int*)(s + off) = v2; });
    }
    if (LogOk()) {
        static const char* names[] = { "on/off", "queue", "", "to come", "", "at once" };
        Log("spawners: script sets spawner %d (type %X) %s = %d", id, type, names[cmd], cmd == 0 ? (v & 1) : v);
        if (v2 != v) Log("spawners:     multiplied to %d", v2);
    }
}

// replaces the tail jump 0x48DFE1 (jmp 0x48DE70): EAX the event, ECX the spawner
static NAKED void CommandStub()
{
    ASM_BEGIN
    ASM(push ecx)
    ASM(push eax)
    ASM(mov  edx, 0x48DE70)
    ASM(call edx)
    ASM(push eax)
    ASM(push dword ptr [esp + 8])    // the spawner
    ASM(push dword ptr [esp + 8])    // the event
    ASM(call AfterCommand)
    ASM(add  esp, 8)
    ASM(pop  eax)
    ASM(add  esp, 8)
    ASM(ret)
    ASM_END
}

// --- room for more characters in the game's collision queries --------------------------------------------------------
// 0x47CF10 (called by 0x47D150, 0x47D1D0, 0x47D290, 0x47D2D0 and 0x47D820, type in cl, description in edi) takes a node of
// 0x50 bytes from a free list: head 0x5BBA40 (linked at +0x18), 0x5BBA38 in use, 0x5BBA3C free. 0x47CE20 makes the list
// from one block of 1024 nodes; it is reached from the engine's table of modules (0x566138 -> 0x478960 -> jmp 0x47CE20 at
// 0x478965). The nodes go back to the list by 0x47D0E0 and 0x47D070, which only link them in again. With many more
// characters (Southern Gate, four players, enemies x5 and allies x4: SteamOS test, 09/11/2026) the 1024 ran out:
// 0x47CF10 returned null, 0x47D2D0 too, and the movement code (0x43CF20) passed it to 0x4795F0, which read from it
// (crash at 0x4795F7, "read 00000003"). Before every call of 0x47CF10, if the list is empty, a block of our own is
// linked in; the game gives those nodes back to the same list, so they stay in it. When the engine makes the list again
// (0x47CE20), the old nodes are forgotten and our blocks are free again.
static const int kQueryNode = 0x50, kQueryBlock = 1024, kQueryBlocks = 3;
alignas(16) static uint8_t g_queryReserve[kQueryNode * kQueryBlock * kQueryBlocks];
static int g_queryBlocks;   // blocks of ours in the list since the engine made it

static void __cdecl QueryListCheck() ASM_NAME(QueryListCheck);
static void __cdecl QueryListCheck()
{
    if (*(volatile uint32_t*)0x5BBA40) return;
    if (g_queryBlocks >= kQueryBlocks) {
        static bool told;
        if (!told) { told = true; Log("collision queries: %d in use, our reserve is used up too", *(const int*)0x5BBA38); }
        return;   // (as in the game: null)
    }
    uint8_t* b = g_queryReserve + (size_t)g_queryBlocks * kQueryBlock * kQueryNode;
    memset(b, 0, (size_t)kQueryBlock * kQueryNode);
    for (int i = 1; i < kQueryBlock; ++i)   // each node to the one before, the first to none (as 0x47CE20 links them)
        *(uint32_t*)(b + i * kQueryNode + 0x18) = (uint32_t)(uintptr_t)(b + (i - 1) * kQueryNode);
    *(uint32_t*)0x5BBA40 = (uint32_t)(uintptr_t)(b + (kQueryBlock - 1) * kQueryNode);
    *(int*)0x5BBA3C += kQueryBlock;
    ++g_queryBlocks;
    Log("collision queries: all %d in use, %d more added (block %d of %d)", *(const int*)0x5BBA38, kQueryBlock,
        g_queryBlocks, kQueryBlocks);
}

static void __cdecl QueryListMade() ASM_NAME(QueryListMade);
static void __cdecl QueryListMade() { g_queryBlocks = 0; }

static NAKED void QueryTakeStub()   // in place of "call 0x47CF10": cl and edi are kept for it
{
    ASM_BEGIN
    ASM(pushad)
    ASM(call QueryListCheck)
    ASM(popad)
    ASM(push 0x47CF10)
    ASM(ret)
    ASM_END
}

static NAKED void QueryMakeStub()   // in place of "jmp 0x47CE20" at 0x478965
{
    ASM_BEGIN
    ASM(pushad)
    ASM(call QueryListMade)
    ASM(popad)
    ASM(push 0x47CE20)
    ASM(ret)
    ASM_END
}

static void QueryReserveInstall()
{
    bool ok = PatchCall(0x478965, 0x47CE20, (const void*)&QueryMakeStub);
    for (uintptr_t site : { (uintptr_t)0x47D15B, (uintptr_t)0x47D1DC, (uintptr_t)0x47D292, (uintptr_t)0x47D38D, (uintptr_t)0x47D829 })
        ok &= PatchCall(site, 0x47CF10, (const void*)&QueryTakeStub);
    Log("collision queries: %d more nodes when the game's 1024 run out: %s", kQueryBlock * kQueryBlocks,
        ok ? "ready" : "NOT installed");
}

// --- the lighting manager's two tables (09/11/2026: things flashing with light on them, x3 and more) -----------------
// The lighting manager (LightingManager.c, 0x484500..0x485F40) keeps two tables of fixed size in the exe's data, with
// no limit checked anywhere:
//  - the lights now in the scene: 12 bytes each { light, flags, list of links } from 0x5BFB48, count at 0x5C019C; room
//    for 135 (the count itself is the 136th entry); added by 0x484550, read by the loops at 0x4849C2, 0x485024, 0x485EFD;
//  - the links between the lights and what they light: 8 bytes each { next, what } from 0x5BEBA8, taken from a free list
//    (head 0x5BEBA4) or else from the next unused one (counter 0x5BEBA0, never checked); room for 500, and the 501st
//    is written over the first lights of the table above.
// All of them are set to zero when a level starts (0x48474F). With more characters (torches, fires, glows) the links
// can run past 500: the lights' table is then overwritten, which would make lights come and go and light things wrongly.
// With [Coop] MoreLights=1 both tables are moved to larger ones of ours (8192 links, 1024 lights): the
// game's code is pointed at them (13 places). Either way the log says how full they get, every 10 s in a level, and
// at once when the game's own room is passed.
static const int kLinks = 8192, kLights = 1024;
alignas(16) static uint8_t g_links[kLinks * 8];
alignas(16) static uint8_t g_lights[kLights * 12];
static bool  g_moreLights;
static int   g_maxLights, g_maxLinks, g_maxQueries;   // the highest values in this level
static bool  g_toldLights, g_toldLinks;
static DWORD g_lightsLogAt;
static char  g_lightsLevel[64];   // the level being measured ("" none)

static void MoreLightsInstall()
{
    g_moreLights = IniInt("Coop", "MoreLights", 0) != 0;   // (off by default: the lights were not the cause, 09/11/2026)
    if (!g_moreLights) { Log("lights: the game's tables as they are (135 lights, 500 links; [Coop] MoreLights=0)"); return; }
    // check every place first, then change: all or nothing
    struct Ref { uintptr_t at; uint32_t old; uint32_t add; bool links; };
    static const Ref kRefs[] = {
        { 0x4844E1, 0x5BEBA8, 0, true }, { 0x484534, 0x5BEBA8, 0, true }, { 0x4845D7, 0x5BEBA8, 0, true },
        { 0x484A5A, 0x5BEBA8, 0, true }, { 0x485E2E, 0x5BEBA8, 0, true },
        { 0x48455D, 0x5BFB48, 0, false }, { 0x484575, 0x5BFB48, 0, false }, { 0x484583, 0x5BFB48, 0, false },
        { 0x485025, 0x5BFB48, 0, false }, { 0x485E53, 0x5BFB48, 0, false }, { 0x485EFE, 0x5BFB48, 0, false },
        { 0x485007, 0x5BFB4C, 4, false }, { 0x4849C3, 0x5BFB50, 8, false },
    };
    bool ok = true;
    for (const Ref& r : kRefs) ok &= *(const uint32_t*)r.at == r.old;
    if (!ok) { Log("lights: the game's code is not what was expected, tables left as they are"); g_moreLights = false; return; }
    memcpy(g_links, (const void*)0x5BEBA8, 500 * 8);
    memcpy(g_lights, (const void*)0x5BFB48, 135 * 12);
    for (const Ref& r : kRefs)
        ok &= PatchDword(r.at, r.old, (uint32_t)(uintptr_t)(r.links ? g_links : g_lights) + r.add);
    Log("lights: room for %d lights and %d links (the game has 135 and 500): %s", kLights, kLinks, ok ? "active" : "IN PART, see above");
}

void DrawsSample();

// --- the renderer's queue (09/11/2026: the cause of the HUD and the enemies coming and going) -----------------------
// Everything drawn in a frame is put in a queue by 0x4B3CB0(draw function, data, priority, key, flag): items of 0x14
// bytes taken one after the other from an array at 0x8CEDA0 (next free one at 0x8D3DA0, set back to the start at every
// frame by 0x4B3E20 / 0x4B3E50) and linked into a list per priority. The array ends where its pointer is (0x8D3DA0):
// room for 1024 items, and when it is full 0x4B3CB6 drops what comes (jae 0x4B3D6B). With many characters (x3 when many
// die at once and their texts and effects come, x7 always: PC tests of 09/11/2026) the queue is full before the HUD
// (queued last, priority 13) and part of the characters are queued: they are not drawn, one frame yes and one no, and
// the pause menu shows only its picture of the screen. With [Coop] MoreDraws=1 (the default) the queue is an array of
// ours with room for 8192: the two starts (0x4B3E26, 0x4B3E56) and the end (0x4B3CB8) are pointed at it.
static const int kDraws = 8192;
alignas(16) static uint8_t g_draws[kDraws * 0x14];
static uint32_t g_drawsStart = 0x8CEDA0;
static int      g_maxDraws;   // the fullest queue seen in this level

static void MoreDrawsInstall()
{
    if (IniInt("Coop", "MoreDraws", 1) == 0) { Log("draws: the game's queue as it is (1024 items; [Coop] MoreDraws=0)"); return; }
    const uint32_t start = (uint32_t)(uintptr_t)g_draws, end = start + kDraws * 0x14;
    bool ok = *(const uint32_t*)0x4B3E26 == 0x8CEDA0 && *(const uint32_t*)0x4B3E56 == 0x8CEDA0 && *(const uint32_t*)0x4B3CB8 == 0x8D3DA0;
    if (!ok) { Log("draws: the game's code is not what was expected, queue left as it is"); return; }
    // the end first: while the starts still point at the game's array its pointer stays below the new end too
    ok = PatchDword(0x4B3CB8, 0x8D3DA0, end);
    ok &= PatchDword(0x4B3E26, 0x8CEDA0, start) && PatchDword(0x4B3E56, 0x8CEDA0, start);
    g_drawsStart = start;
    Log("draws: room for %d items in the renderer's queue (the game has 1024): %s", kDraws, ok ? "active" : "IN PART, see above");
}

// how many items are in the queue now (the pointer at 0x8D3DA0 is set back at every frame, so it is read while the HUD is
// drawn, from the queue: Hud.cpp)
static int DrawsNow()
{
    const uint32_t p = *(const uint32_t*)0x8D3DA0;
    return p >= g_drawsStart ? (int)((p - g_drawsStart) / 0x14) : 0;
}

void DrawsSample() { const int n = DrawsNow(); if (n > g_maxDraws) g_maxDraws = n; }

// the links in use: the ones taken (counter) less the ones given back (free list)
static int LinksInUse(int taken)
{
    int free = 0;
    Guard([&] {
        for (uint32_t p = *(const uint32_t*)0x5BEBA4; p && free <= kLinks + 600; p = *(const uint32_t*)(uintptr_t)p) ++free;
    });
    return taken - free;
}

static int CharactersNow()
{
    int n = 0;
    Guard([&] {
        int guard = 0;
        for (uint8_t* e = *(uint8_t**)0x579188; e && ++guard < 20000; e = *(uint8_t**)(e + 0x60))
            if (*(const uint32_t*)(e + 0x8C) >> 27 & 1) ++n;
    });
    return n;
}

long HudDrawsTaken();   // Hud.cpp

// --- the engine's pools that run out (09/11/2026: HUD and enemies coming and going with many characters) --------------
// The engine's pools of fixed size (UMemPool: +8 how many in all, +0xA how many free, +0xC the free list, linked by the
// first dword) give an item with 0x499110, which answers null when the pool is empty; its ~30 callers then go without
// it (nothing crashes: the thing is simply not there, one frame yes and one no). 0x499110 is replaced by the same code
// that also notes which pool ran out, how large it is and who asked: the log says it once per pool and level, and how
// many times it happened, so the pool to enlarge is known.
struct EmptyPool { uint32_t pool, caller; int total; long misses; };
static EmptyPool g_empty[16];
static int       g_emptyN;

static uint32_t __cdecl PoolTakeNote(uint8_t* pool, uint32_t caller)
{
    for (int i = 0; i < g_emptyN; ++i)
        if (g_empty[i].pool == (uint32_t)(uintptr_t)pool && g_empty[i].caller == caller) { ++g_empty[i].misses; return 0; }
    if (g_emptyN < 16) {
        g_empty[g_emptyN] = { (uint32_t)(uintptr_t)pool, caller, *(const uint16_t*)(pool + 8), 1 };
        ++g_emptyN;
        Log("pools: pool %08X (%d items) empty, asked from %08X", (uint32_t)(uintptr_t)pool, *(const uint16_t*)(pool + 8), caller);
    }
    return 0;
}

#if defined(_MSC_VER)
#include <intrin.h>
#define CALLER() ((uint32_t)(uintptr_t)_ReturnAddress())
#else
#define CALLER() ((uint32_t)(uintptr_t)__builtin_return_address(0))
#endif

static uint32_t __cdecl PoolTake(uint8_t* pool)
{
    uint32_t* head = (uint32_t*)(pool + 0xC);
    const uint32_t item = *head;
    if (!item) return PoolTakeNote(pool, CALLER());
    --*(uint16_t*)(pool + 0xA);
    *head = *(const uint32_t*)(uintptr_t)item;
    return item;
}

static void PoolWatchInstall()
{
    static const uint8_t old[] = { 0x8B, 0x4C, 0x24, 0x04, 0x8B };   // mov ecx,[esp+4]; mov eax,[ecx+0Ch]
    const bool ok = PatchJmp(0x499110, old, sizeof old, (const void*)&PoolTake);
    Log("pools: the engine's pools that run out are written in the log: %s", ok ? "active" : "NOT installed");
}

// the pools that ran out in this level, with how many times (at the end of the level)
static void PoolsLevelEnd(const char* level)
{
    for (int i = 0; i < g_emptyN; ++i)
        Log("pools: level %s, pool %08X (%d items) was empty %ld times when %08X asked", level, g_empty[i].pool,
            g_empty[i].total, g_empty[i].misses, g_empty[i].caller);
    g_emptyN = 0;
}

// every frame (Pads.cpp)
void LightsFrame()
{
    // a level is loaded while CoopLevelFile() names it (the menu's is ""; the pause keeps it)
    const char* level = CoopLevelFile();
    if (strcmp(level, g_lightsLevel) != 0) {
        if (g_lightsLevel[0]) {
            Log("lights: level %s, at most %d lights (the game's room: 135), %d links taken (500), %d collision nodes (1024), "
                "%d items in the renderer's queue (1024)", g_lightsLevel, g_maxLights, g_maxLinks, g_maxQueries, g_maxDraws);
            PoolsLevelEnd(g_lightsLevel);
        }
        strcpy_s(g_lightsLevel, level);
        g_maxLights = g_maxLinks = g_maxQueries = g_maxDraws = 0;
        g_toldLights = g_toldLinks = false;
        g_lightsLogAt = GetTickCount();
    }
    if (!g_lightsLevel[0]) return;
    const int lights = *(const int*)0x5C019C, links = *(const int*)0x5BEBA0, queries = *(const int*)0x5BBA38;
    if (lights > g_maxLights) g_maxLights = lights;
    if (links > g_maxLinks) g_maxLinks = links;
    if (queries > g_maxQueries) g_maxQueries = queries;
    const int draws = DrawsNow();
    if (lights > 135 && !g_toldLights) {
        g_toldLights = true;
        Log("lights: %d lights at once, past the game's 135%s", lights, g_moreLights ? " (in our table)" : ": the game writes past its table");
    }
    if (links > 500 && !g_toldLinks) {
        g_toldLinks = true;
        Log("lights: %d links taken, past the game's 500%s", links,
            g_moreLights ? " (in our table)" : ": the game is writing over its table of lights");
    }
    const DWORD now = GetTickCount();
    if (LogDetail() && now - g_lightsLogAt >= 10000) {   // (the snapshot every 10 s with [Log] Detail=1 only)
        g_lightsLogAt = now;
        Log("lights now: %d lights, %d links in use (%d taken), %d collision nodes, %d characters, %ld HUD elements drawn in 10 s, "
            "renderer's queue %d now (at most %d)", lights, LinksInUse(links), links, queries, CharactersNow(), HudDrawsTaken(), draws,
            g_maxDraws);
    }
}

void SpawnersInstall()
{
    QueryReserveInstall();
    MoreLightsInstall();
    MoreDrawsInstall();
    PoolWatchInstall();
    g_on = IniInt("Coop", "MoreEnemies", 1) != 0;
    g_factor = IniInt("Coop", "EnemyFactor", -1);
    if (!Good(g_factor)) g_factor = -1;
    g_ally = IniInt("Coop", "AllyFactor", 0);
    if (!Good(g_ally)) g_ally = 0;
    g_factor1 = IniInt("Coop", "SingleEnemyFactor", 10);
    if (!Good(g_factor1)) g_factor1 = 10;
    g_ally1 = IniInt("Coop", "SingleAllyFactor", 0);
    if (!Good(g_ally1)) g_ally1 = 0;
    g_capAlive = IniInt("Coop", "MoreEnemiesMaxAlive", 16);
    if (g_capAlive < 1) g_capAlive = 16;
    const bool log = IniInt("Diag", "SpawnerLog", 1) != 0;
    if (!g_on && !log) { Log("spawners: more enemies off ([Coop] MoreEnemies=0)"); return; }
    bool ok = PatchDword(0x5660E0, 0x48E120, (uint32_t)(uintptr_t)&MakeSpawner);
    static const uint8_t jmpOld[] = { 0xE9, 0x8A, 0xFE, 0xFF, 0xFF };   // jmp 0x48DE70
    ok &= PatchJmp(0x48DFE1, jmpOld, sizeof jmpOld, (const void*)&CommandStub);
    char how[64];
    if (!g_on) strcpy_s(how, "off (spawners only logged)");
    else if (g_factor < 0) strcpy_s(how, "by players (2: as in the game, 3: x1.5, 4: x2)");
    else sprintf_s(how, "x%.1f", g_factor / 10.0f);
    Log("spawners: more enemies %s, allies %s, at most %d at once per spawner: %s", how,
        g_ally ? "by their own factor" : "as the enemies", g_capAlive, ok ? "active" : "NOT installed");
}
