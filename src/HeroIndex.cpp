// HeroIndex.cpp - game functions that pick the hero with "index 0 -> player 1, otherwise -> player 2"
// Language: C++17, MSVC, Win32.
//
// Many functions choose a player's hero like this: index 0 -> 0x47EF60 (hero of player 1), otherwise ->
// 0x47EF80 (hero of player 2). For the third player (index 2) they used the hero of player 2: kills by Gimli gave
// experience and statistics to Gandalf. Here the main functions are wrapped: if they concern an extra player
// (index 2 or 3), 0x47EF80 answers with that player's hero for the duration of the call (g_spawningP3 = index - 1,
// the same flag used when the extra players are created, see Coop.cpp).
//  - 0x40D110: experience and statistics of a kill; player index in EAX, two arguments on the stack.
//  - 0x417E70(character, x): called from 11 places; looks at the index at +0x690 of the character.
#include "ROTKPlus.h"

extern volatile int g_spawningP3;   // Coop.cpp

// trampolines: the first instructions copied, then the rest of the function
static __declspec(naked) void KillExpOrig()   // 0x40D110: sub esp,14h; mov edx,[573E48h]
{
    __asm {
        sub  esp, 0x14
        mov  edx, dword ptr ds:[0x573E48]
        push 0x40D119
        ret
    }
}

static __declspec(naked) void KillExp()
{
    __asm {
        cmp  eax, 2
        jb   plain
        cmp  eax, 3
        ja   plain
        lea  edx, [eax - 1]
        mov  g_spawningP3, edx
        push dword ptr [esp + 8]
        push dword ptr [esp + 8]
        call KillExpOrig
        add  esp, 8
        mov  g_spawningP3, 0
        ret
    plain:
        jmp  KillExpOrig
    }
}

static __declspec(naked) void HeroOfCharOrig()   // 0x417E70: push esi; push edi; mov edi,[esp+0Ch]
{
    __asm {
        push esi
        push edi
        mov  edi, dword ptr [esp + 0x0C]
        push 0x417E76
        ret
    }
}

static __declspec(naked) void HeroOfChar()
{
    __asm {
        mov  ecx, dword ptr [esp + 4]
        test ecx, ecx
        jz   plain
        mov  edx, dword ptr [ecx + 0x690]
        cmp  edx, 2
        jb   plain
        cmp  edx, 3
        ja   plain
        dec  edx
        mov  g_spawningP3, edx
        push dword ptr [esp + 8]
        push dword ptr [esp + 8]
        call HeroOfCharOrig
        add  esp, 8
        mov  g_spawningP3, 0
        ret
    plain:
        jmp  HeroOfCharOrig
    }
}

bool HeroInstall()
{
    static const uint8_t killOld[] = { 0x83, 0xEC, 0x14, 0x8B, 0x15, 0x48, 0x3E, 0x57, 0x00 };
    static const uint8_t heroOld[] = { 0x56, 0x57, 0x8B, 0x7C, 0x24, 0x0C };
    bool ok = true;
    ok &= PatchJmp(0x40D110, killOld, sizeof killOld, (const void*)&KillExp);
    ok &= PatchJmp(0x417E70, heroOld, sizeof heroOld, (const void*)&HeroOfChar);
    Log("hero of the extra players in index-based functions (experience, kills): %s", ok ? "active" : "NOT installed");
    return ok;
}
