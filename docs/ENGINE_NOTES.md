# Notes on the engine of *The Return of the King* (PC, 2003)

Everything here comes from reverse engineering `rotk.exe` 1.0 (1,703,936 bytes, PE timestamp `0x3F807074`,
image size `0x5C7000`, image base `0x400000`) and from testing in the game. Addresses are virtual addresses in that
executable. Names are our own. Things marked *(unknown)* are not understood. This is a working document, not a
specification: if you find an error, please open an issue.

## Players

- The players array is at `0x57916C` (2 slots of `character*`); `0x410150` registers a character in it, `0x4101A0(n)`
  returns slot n, `0x4101B0(id)` finds a player by the identifier at `+0x70`. The unrolled loop of `0x410230` already
  covers four slots (references at `0x4102DB`, `0x410348`, `0x4103B5`); a list of instruction operands refers to the
  array and can be redirected to a larger one (`Coop.cpp`, `kPlayerRefs`).
- The model entities (the `char_xxx.scw` packages create one 0xE10 entity per hero) are registered by `0x492C40` in an
  array of 2 at `0x99CE40` (counter `0x5C0A44`); `0x492AC0(n)` creates player n's character from the level's start
  entity (`ebx`): it takes the hero from `0x47EF60` (player 1) or `0x47EF80` (player 2), finds its model and places it.
- `0x442ED0` initialises a character: player index at `+0x690` (0 for player 1's hero, otherwise 1), registers it.
  Character fields: `+0x30/+0x34/+0x38` position, `+0x70` and `+0x74` identifiers, `+0x78` hero type, `+0x7A` = 99 for
  players, `+0x8C` flags (bit 1 frozen, bit 27 "counts as an attackable character"), `+0x90` life, `+0xD0` a structure
  with the state machine (`+4` -> current state id, `+0x48` maximum life, `+0x104` time in the "die" state), `+0x5E8` the
  input slot, `+0x690` player index, `+0x810` dead/waiting flag, `+0x81C` controls enabled.
- Hero ids: 0 Aragorn, 1 Legolas, 2 Gimli, 3 Gandalf, 4 Sam, 5 Frodo, 6 Pippin, 7 Merry, 8 Faramir. Hero types in
  `+0x78`: 8, 0x22, 0xD, 0x1E, 0xC, 4, 0x44, 0x43, 0x59 respectively. Chosen heroes: `0x562A84` (player 1) and
  `0x562A88` (player 2); allies chosen by the loader: `0x562A8C/90/94`; co-op flag `0x562A98`.
- Seven other tables hold one row per player (2 rows): bases `0x578E60`, `0x578E68`, `0x578E70`, `0x578E80` (100-byte
  rows), `0x99DB80` (0x144-byte rows), `0x579B30`, `0x579B38`. A third row used to land on the difficulty table
  `0x578F48` and a memory-pool pointer (`Coop.cpp` moves them).
- Input records: block of 4 players at `0x8E7E60` (0x24690 bytes each), configured by `[inputN]` sections of `ROTK.ini`;
  only two are initialised by `0x4AC890` (calls `0x4AC670(0,1)` and `0x4AC670(1,0)`). Per-player controller record at
  `0x99CBA0 + n*0x58` (`0x49D490(n)`): sticks, buttons at `+0x40/+0x42`, button bit table `0x99CB60`.

## Levels and loading

- The loader `0x4BE870` builds a list of up to 8 packages (a counter and 8 paths of 64 bytes), copied by `0x4E0690`
  (0x204 bytes): `LoadOnce`, `GlobScen`, the players' `char_` packages, the allies' `foll_` packages, the level. The
  allies come from `0x481A50(level, n, hero1, hero2, coop)`.
- Level names are in a table at `0x562400` (0 Hel01, 1 Pat01, 2 Pat03, 3 Min02, 4 Pel02, 5 Bla01, 6 Ise01, 7 Min00,
  8 Min04, 9 Osg01, 10 She01, 11 Cir00, 12 Cra01, 13 Fro03 (the menu), 14 Cir01, 15 Pat02, 16 Pal01, 17 Pal02). The map
  chooses with `0x481440(level)`. The hero-selection scene is prepared by passing through the map: it cannot be
  skipped.

## Lives and death

- Shared lives: `0x579B28` (read by `0x43FD40`, written by `0x43FD50`; shown by `0x4ECF74`). Written by every player's
  initialisation at `0x4432CB`: 1, or 2 in level 10.
- The "die" state (id `0x28`; state table at `0x561420`: `+8` update `0x434D80`, `+0xC` enter `0x434ED0`, `+0x10` leave
  `0x434FA0` with "RessurectFx1"): on entering, if co-op, lives remain and "the other player" (`0x445A40`) is not itself
  in the "die" state, a life is consumed and the character respawns; otherwise the defeat sequence `0x43FE40`, then
  `0x4F6E90(1)`.

## Combat

- Damage is applied by `0x40CF80(character, damage, attacker)` (for players it goes through the difficulty table
  `0x578F48` and updates the HUD); events are sent with `0x401780(victim, event, data, result, attacker)`; event
  `0x259` is "hit" with `{attacker, kind, damage, ...}`.
- Melee: `0x40EF50(attacker, centre, radius)` -> `0x47D180` -> collision system (sweep and prune: `0x4795F0`,
  `0x479CA0`; volumes at `0x5BB9E4`, stride `0xE0`: `+0` shape, `+1`, `+2` type (1 body, 5 hittable), `+3` state, `+4`
  check function, `+0x10` owner). The side filter is `0x40EDC0`. The hit shapes `0x415070`, `0x4152C0`, `0x4154F0` keep
  type-5 volumes only; `0x4185C0` sends the hit. *(unknown: why another player's type-5 volume never reaches the
  search)*
- R2 execution: state "kill" `0x31` (`0x43C410`); victim chosen by `0x435F00 -> 0x435E20` (character on the ground, enemy
  side). Gandalf's area magic `0x439E40`; an area blast `0x419020`.
- Projectiles: contact `0x454940` (side rules at `0x454C12`), volume check `0x4543A0` (friendly fire), hit `0x4547C0`.
  Spear: `0x438140` (throw state), flight `0x4524C0/0x452FE0`, locked target at `+0xD4/+0x180`.
- Lock-on aim: `0x43F4A0 -> 0x411890` with `0x43F3F0` (`0x43EA10/0x43EBC0` rate candidates; hero types are excluded in
  long-range aim).

## HUD and screens

- HUD elements are screen elements: `0x4ED940(0, name)` finds one by name; `0x4F6A90(0, name, visible)` sets visibility
  (`+0x74`); `+0x76` type (100 image, 0x82 text, 0x46/0x50/0x5A bars, 10 menu group); `+0x40/+0x44` position (fractions
  of the screen, or points on 512x448 if >= 2); `+0x88` value; `+0x8C` text; `+4` -> `{count, children...}`. The drawing
  dispatcher is `0x4FD590`.
- Two player blocks: `playerhuda` (`0x8E2984`) and `playerhudb` (`0x8E2A14`), 0x24 element pointers each: `[0]` the
  block, `[2]` experience, `[3]` life, `[6]` ring, `[0x11..0x22]` hero portraits/weapons. `0x4ECD00(character)` fills a
  block (the second block only for `g_players[1]`, the first for everybody else); `0x4ED290` updates a character's HUD
  and the "g1"/"g2" marks (elements `coopmark1p/2p` at `0x8E2904/08`); `0x4ED110` writes the respawn countdown into
  the separate elements `respawna`/`respawnb`; `0x4ED4D0` level-up, `0x4ED240` hit flash.
- The game's texts: `0x4E3F00(key, 0x47EF50(0))` returns the text for a key in the current language.
- Screens: mode `0x562AC0`, previous mode `0x562AC4`, changed by `0x47FA20(mode)`. Modes: 0 game, 1 hero selection,
  2 score, 3 shop, 4 map, 6 save, 14 loading, 16 co-op statistics, 17 main menu. The selection scene object holds each
  hero's state at `+0x124 + hero*4` (computed by `0x456210`; 4 = taken, 5 available). Menu actions go through the loop
  `0x4EAA60` and the handler `0x4EA0F0`; `0x4EA200(slot)` writes the slot *excluded* from the menus (`0x56DC75`).

## Camera

- In co-op `0x46FB10` computes the points to follow of players 1 and 2 (`0x5A5BA0`, `0x5A5B90`) with `0x4402C0` and
  `0x46F530` frames the pair. `0x43DCB0(character, speed)` returns the edge of the frame a character is pushing against.
