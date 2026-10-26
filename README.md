# ROTKPlus

Co-op with **3 and 4 players** and a **player-versus-player mode** for
*The Lord of the Rings: The Return of the King* (PC, 2003).

The game ships with a 2-player co-op. ROTKPlus adds a third and a fourth hero to the same level, each with its own
controller, HUD box, mark above the head, shared lives, level-up, and turn in the end-of-level screens. It also adds a
**Versus** entry to the main menu, where 2 to 4 players fight each other on any level.

ROTKPlus is a small DLL, not a replacement for the game. It works together with
[ROTK Fix / Athelas](https://github.com/Shagg88/Athelas) by Shagg88 (which makes the game run on modern PCs), and it
changes nothing on disk except the file it replaces (see *Installation*).

> **Status:** version 1.0.0, tested on Windows 11 with Athelas 1.5 (everything) and Athelas 1.6.0 (the game starts, all
> the patches install and the menu works; gameplay was not tried on 1.6), the Italian edition of the game, and up to four
> controllers. Not tested on Linux / Steam Deck (Proton). English is the default
> language of the mod's own texts; Italian was verified in the game; French, German and Spanish are untested
> translations (see *Language*).

## Features

- Main menu: **Co-op: N players** and, right below it, **Versus: N players**. With the cursor on the entry, left/right
  cycle through 2, 3 and 4 players (the choice is remembered).
- Third and fourth player in the same level: they pick their hero after players 1 and 2 in the selection screen, are
  created next to player 2, and have their own HUD box (top right and top left), a green/yellow mark with "g3"/"g4"
  above their head, a respawn countdown, level-up effects, kills and experience.
- Lives are shared as in the original game (two with two players) and grow by one per extra player; the game is lost
  only when everybody is down.
- The camera frames the whole group, and a player who gets stuck outside the frame is brought back to a teammate.
- End-of-level screens (score, co-op statistics, shop) give each extra player a turn, controlled by that player's own
  controller only.
- Versus mode: melee hits, combos, specials, the R2 execution (half of the victim's life), arrows with lock-on, the
  spear, Gandalf's area magic, and blocking all work between players. Two lives each by default; whoever loses them
  stays on the ground; the last one standing wins, with a banner and a level-up effect, and the level closes. It works
  on any level (with the level's enemies, everybody against everybody).
- Progress of the extra players (levels, experience, purchases) shows up in the save game under the first player's
  profile: in the tests, players 1, 3 and 4 saw it when they loaded the same save slot.

## Requirements

- The game, in the version that Athelas calls "Game v1.0" (`rotk.exe` of 1,703,936 bytes). Other
  executables are detected and ROTKPlus does nothing.
- [Athelas](https://github.com/Shagg88/Athelas) with ROTK Fix set up for the game (it provides `SDL3.dll`, the
  controller layer and the graphics fix). Tested with Athelas 1.5 and 1.6.0.
- One controller per player (Xbox-style, PlayStation, anything SDL3 supports). Without a controller an extra player
  stands still (or, with `KeyboardFallback=1`, moves with player 1's keys).

## Installation

1. Close the game. Make a backup copy of the game folder if you wish.
2. In the game folder, rename `DivxMediaLib.dll` to `DivxMediaLib_orig.dll`.
3. Copy `DivxMediaLib.dll` from ROTKPlus (the file you downloaded or built) into the game folder. ROTKPlus *is* a
   `DivxMediaLib.dll`: it forwards every call to the original and adds the mod. The game loads it when the first movie
   starts, so keep ROTK Fix's `SkipOpeningLogos = 0`.
4. Copy `ROTKPlus.ini` (from `config/ROTKPlus.ini.example`) next to `rotk.exe` and set `[Coop] Enabled=1`. Without the
   ini file the mod stays **off**.
5. Start the game through Athelas as usual. `ROTKPlus.log` in the game folder tells you what happened.

To remove it, delete ROTKPlus's `DivxMediaLib.dll` and rename `DivxMediaLib_orig.dll` back.

## Using it

In the main menu choose **Co-op: 3 players** (or 4), or **Versus: 3 players**, and confirm. Players 1 and 2 choose
as usual; then the third and fourth players choose their heroes, one after the other, with their own controllers. In a
level the extra players join right away. Use the same save slot for players 1 and 2 to see everybody's progress.

Key `F9` (only if `[Diag] VictoryKey=1`) ends the level with a victory: useful to see the end-of-level screens.

### Testing without controllers

With `[Debug] SimPads=1` players 2, 3 and 4 are driven by the keyboard (keys at the top of `src/Sim.cpp`) and the game
offers the Co-op entry even with no controller connected; player 1 uses the game's own keyboard and mouse controls.
One person can drive one character at a time, which is enough to look at the creation of the players, the HUD, deaths and
respawns, the hero selection and the end-of-level screens. It is a testing aid, not a way to play.

## Configuration

`ROTKPlus.ini` (see `config/ROTKPlus.ini.example`, every key is documented there):

| Section | Key | Meaning |
|---|---|---|
| `[Coop]` | `Enabled` | `0` turns the whole mod off |
| | `Players` | 2, 3 or 4 (also changed from the menu) |
| | `Hero` | hero of the third player if not chosen (`auto`, `aragorn`, ...) |
| | `Pad`, `PadName` | use controllers for the extra players; pick the third one by part of its name |
| | `Hud`, `HudX`, `HudY`, `HudRow`, `HudScale` | position and size of the extra HUD boxes |
| `[PvP]` | `Enabled`, `Players`, `Lives` | the Versus entry, its number of players, lives per player |
| `[General]` | `Language` | `auto`, `en`, `it`, `fr`, `de`, `es` or your own code |

## Language

The mod writes a few texts (the two menu entries, "Player N" in the selection screen, the winner banner). Everything
else is the game's own text. With `Language=auto` the language is recognised from the game's own text for "Player 1".
Built-in: English, Italian (verified), French, German, Spanish (unverified, no accents because the game's menu font
may not have them). To fix or add a language without recompiling, put a `ROTKPlus.lang.ini` next to `rotk.exe`; see
`config/ROTKPlus.lang.ini.example`. Contributions of translations are welcome.

## Building

Visual Studio 2022 or later (the "Desktop development with C++" workload), MSVC, 32-bit:

    msbuild ROTKPlus.slnx /p:Configuration=Release /p:Platform=x86

The output is `src/Release/DivxMediaLib.dll`. The code uses MSVC inline assembly and will not build with other
compilers. The `src` folder has one file per area; the top of each file explains what it patches and why.

## How it works

ROTKPlus patches the running game in memory at about 150 places (each one verified byte by byte before writing, so
an unknown executable is left alone). The array of players, the array of character models and seven other per-player tables of
the game have room for two players; they are moved to larger copies. The level loader gets one more `char_` package
per extra player, the game's player-creation function is called again for each of them, and every place that decides
"player 1 or player 2" is rewritten to handle up to four. [`docs/ENGINE_NOTES.md`](docs/ENGINE_NOTES.md) collects what
was learnt about the game's internals (addresses are for `rotk.exe` 1.0); it may help other mod authors.

## Help wanted

This is a one-person hobby project, and it has been tested on very few setups. **Any help is very welcome:**

- **Testing.** Try it with your own controllers and edition of the game, with 3 and 4 players, in different levels, in the
  Versus mode, and on Linux / Steam Deck (Proton). Tell us what works and what does not, with `ROTKPlus.log` attached.
  There is a 15-minute test round in [TESTING.md](TESTING.md) and a **test report** form when you open an issue.
- **Improvements and integrations.** Ideas that would be great: team play (2 against 2) in Versus mode, a Versus ending that
  does not go through the co-op screens, a "defeated" caption for eliminated players, separate save profiles for the extra
  players, integration with Athelas / ROTK Fix's own mod system. Pull requests and suggestions are both welcome.
- **Translations** of the mod's few texts (see *Language*).
- **Knowledge of the engine.** Corrections and additions to [`docs/ENGINE_NOTES.md`](docs/ENGINE_NOTES.md).

See [CONTRIBUTING.md](CONTRIBUTING.md) for how.

## Credits and legal

- Created by **brig-dev**, with the help of Claude (Anthropic) for the analysis and the code. The work consisted of
  reverse engineering the 2003 executable, writing the patches, and testing every feature in the game.
- Built to run with [Athelas and ROTK Fix](https://github.com/Shagg88/Athelas) by Shagg88 (MIT). ROTKPlus does not
  include or modify any of their files, and was not made or endorsed by them.
- *The Lord of the Rings: The Return of the King* is a trademark and a work of its respective owners (Electronic Arts,
  New Line Cinema, Warner Bros., the Tolkien Estate and others). ROTKPlus is an unofficial fan project, not affiliated
  with or endorsed by any of them. It contains no game code, data, art or sound; you need your own copy of the game.

## License

[MIT](LICENSE).
