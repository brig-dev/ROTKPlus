# ROTKPlus

Co-op with **3 and 4 players** and a **player-versus-player mode** for
*The Lord of the Rings: The Return of the King* (PC, 2003).

The game ships with a 2-player co-op. ROTKPlus adds a third and a fourth hero to the same level, each with its own
controller, HUD box, mark above the head, shared lives, level-up, and turn in the end-of-level screens. It also adds a
**Versus** mode for 2 to 4 players (everybody against everybody or 2 vs 2, timed games, tournaments) and names and
statistics for every player. Everything starts from one **Multiplayer** entry in the main menu. From 1.5 the campaign
can also have **more enemies and more allies** (from x1.5 to x7), for battles as big as the films.

From 1.5 ROTKPlus comes in one download with everything: the **ROTKPlus launcher**, ROTK Fix (widescreen, a modern
renderer, controllers, 3D audio) and the mod. The launcher has the Fellowship with each player's statistics and deeds, a
card to share as a picture, the screenshots, a controller and the buttons for each of the four players, and every page
usable with a controller. The mod alone is still there for Athelas (see *Installation*).

**Download:** [GitHub Releases](https://github.com/brig-dev/ROTKPlus/releases) or
[ModDB](https://www.moddb.com/mods/rotkplus/downloads) (same files). Mod page: <https://www.moddb.com/mods/rotkplus>. What changed: [CHANGELOG.md](CHANGELOG.md)

> **Status:** version 1.5.0. Tested on Windows 11 with the ROTKPlus launcher and with Athelas 1.5 to 1.8.2, the Italian
> edition of the game, up to four controllers, keyboard and mouse: the campaign with four players and more enemies up to
> x7 (Minas Tirith, Pelennor Fields), the Versus modes with two and four players. On SteamOS (Proton): the Versus mode
> with three players (1.0.0), the whole campaign with Athelas 1.7.x (1.2.0), the 1.4 pages and `FastText=2` with Athelas
> 1.8.1, and a test build of the ROTKPlus launcher. Not tested on the Steam Deck. English is the default language
> of the mod's own texts; Italian was verified in the game; French, German and Spanish are untested translations (see
> *Language*). The launcher speaks ten languages.

## ROTKPlus and Athelas

ROTKPlus exists thanks to [Athelas and ROTK Fix](https://github.com/Shagg88/Athelas) by **Shagg** (MIT licence; while
that page is down, Athelas is at [SourceForge](https://sourceforge.net/projects/athelas/)), which make the game run on
modern PCs. The ROTKPlus launcher is Athelas' launcher made for this one game: the ROTKPlus settings in place of the Mods
tab, nothing of the other games, and the pages of ROTKPlus (Fellowship, History, screenshots). It exists because
ROTKPlus needed a launcher for The Return of the King only, with four players; it does not replace Athelas, which
also runs The Two Towers, The Fellowship of the Ring and The Third Age. If you use Athelas, keep using it: ROTKPlus is
also a mod of Athelas (from Athelas 1.8.1), and the mod alone is in every release. The launcher's sources are in
[`launcher`](launcher), with Shagg's copyright kept as the licence asks.

## Features

- **More enemies and allies** in the campaign, chosen on the Multiplayer page: as in the game, x1.5, x2, x3, x5 or x7
  (with a line of Gimli's), recommended x1.5 with three players and x2 with four; the allies as many more as the
  enemies or a factor of their own; the same for a single player on *Who is playing?*. The levels' common enemies
  (soldiers, shields, archers) come out of their spawners more times and the level scripts count them as their own, so
  the waves end as before; captains, trolls, mumakil, spiders, bosses and story characters stay as they are. At the
  Top of the Wall the level is lost when too many orcs are on the wall: a high factor gets there soon.
- Main menu: **Multiplayer**, with a page where the mode (campaign, everybody against everybody, teams), the number of
  players, the enemies, the lives, the teams and who plays are chosen together (see *Using it*).
- Third and fourth player in the same level: they pick their hero after players 1 and 2 in the selection screen, are
  created next to player 2, and have their own HUD box (top right and top left), a green/yellow mark with "g3"/"g4"
  above their head, a respawn countdown, level-up effects, kills and experience.
- Lives are shared as in the original game (two with two players) and grow by one per extra player; the game is lost
  only when everybody is down.
- The camera frames the whole group (players 1 and 2 only during the level's scenes, which the scripts drive for them).
  A player stuck outside the frame or running in place against a wall is brought back to a teammate, the third and
  fourth player left far behind players 1 and 2 are brought next to them, and they come along when the level moves
  players 1 and 2 to another place. Players 3 and 4 can also press **Back** (View, Create) to come back next to player
  1 or 2 at any time.
- End-of-level screens (score, co-op statistics, shop) give each extra player a turn, controlled by that player's own
  controller only.
- Versus mode: melee hits, combos, specials, the R2 / RT execution (half of the victim's life), arrows with lock-on, the
  spear, Gandalf's area magic, and blocking all work between players. Two lives each by default; whoever loses them
  stays on the ground; the last one standing wins, with a banner and a level-up effect, and the level closes. The map
  offers three arenas: the Southern Gate, the Courtyard of Minas Tirith and the Black Gate (open even if not unlocked
  yet; `[PvP] ArenasOnly=0` gives back every level). With **2 vs 2** players 1 and 2 play against 3 and 4
  (`[PvP] Team1` changes the pairs): teammates cannot hit each other, the teams are shown at the start and the last team
  standing wins; the teams are the Sun's and the Moon's. With **heroes only** the enemies are not created (the allies
  are); otherwise the level's enemies fight everybody. Every player's lives left are in their HUD box, the knock-downs
  appear at the bottom as they happen ("Alex downs Robin"), and the teams' totals between the top boxes. The
  Multiplayer page also chooses the **victory** (last one standing, or **timed**: 2, 3 or 5 minutes, the most
  knock-downs win, back on your feet after 5 seconds) and a **tournament** at the best of 3 or 5 rounds, with a Cup for
  the winner; a timed round tied at the end goes to **sudden death** (the next knock-down wins). After the game the map
  comes back at once, without the game's score screens, with the **Last game** over it (who won, everybody's
  knock-downs and falls). At the Black Gate with the heroes only, its boss still plays its opening scene (Frodo's
  mithril) and then leaves before the fight. A **parry** stops speed attacks, plain arrows and spears, but not a fierce
  attack (Triangle), the last move of a combo (Orc Slayer's glowing blow) or a fully drawn shot.
  A sound of the level when a player hits another one and a music at the start can be set in the ini (see
  `[PvP] Cheer_<level>`, *experimental*).
- Progress of the extra players (levels, experience, purchases) shows up in the save game under the first player's
  profile: in the tests, players 1, 3 and 4 saw it when they loaded the same save slot.
- **Fellowship** entry in the main menu (from 1.2; "Compagnia" in Italian): a name for each player, typed with the
  keyboard or with letters on the screen for controllers, and **statistics per name**, added up over all your sessions:
  levels played, won and completed, time, kills (of which at range, with spears, killing moves and in single-player
  games), assists (you hit an enemy, a teammate finishes it within 15 seconds), deaths, kills by rating, experience,
  favourite hero; kills by kind of enemy (with soldiers, shields, archers and captains apart) and of every boss, combos
  by name, parries, ladders pushed down, enemies set on fire, kicks, things picked up; in Versus games,
  wins, opponents knocked down and own falls, who knocked down whom, tournaments and Cups; and the same per hero. A **Comparison** of four players in six tabs, a
  **profile** of each player, **records**, the **last game**, and **name management** (add, rename, merge two names,
  delete). **Who is playing?** before every game gives each place its name. The names also appear above the players'
  heads and in the hero selection ("Player 1 - Alex, select your character") and in the Versus winner banner. The third
  and fourth player's controllers remember their player's name. Kept in `ROTKPlus_players.ini` in the game folder, with
  a dated copy in `ROTKPlus_logs` whenever it changes.

## Requirements

- The game, in the version that Athelas calls "Game v1.0" (`rotk.exe` of 1,703,936 bytes). Other
  executables are detected and ROTKPlus does nothing.
- The ROTKPlus launcher (in the package), or [Athelas](https://github.com/Shagg88/Athelas) 1.5 to 1.8.2 with ROTK Fix
  set up for the game. Either one provides `SDL3.dll`, the controller layer and the graphics fix.
- One controller per player (Xbox-style, PlayStation, anything SDL3 supports). Without a controller an extra player
  stands still (or, with `KeyboardFallback=1`, moves with player 1's keys).

## Installation

Every release has two files, on [GitHub Releases](https://github.com/brig-dev/ROTKPlus/releases) and on
[ModDB](https://www.moddb.com/mods/rotkplus/downloads).

**`ROTKPlus-Pack-1.5.0.zip`, everything in one (recommended):**

1. Install the game (version 1.0) in a folder of its own, for example `C:\Games\ROTK`.
2. Unzip the package in another folder, for example `C:\Games\ROTKPlus` (never inside the game's folder).
3. Start `ROTKPlus.exe`, choose the game's folder (the launcher looks for it by itself and offers what it finds), and
   press Play: the first Play puts ROTK Fix and ROTKPlus in the game's folder, next to `rotk.exe`. The game's own files
   are not changed.
4. Settings: the launcher's tabs (ROTKPlus' own on the Gameplay tab) and, in the game, the Multiplayer page. The
   launcher tells when a new version is out.

Steam Deck and SteamOS: add `ROTKPlus.exe` to Steam as a non-Steam game, with a recent Proton; the package's `steam`
folder has pictures for the library and how to set them.

**`ROTKPlus-1.5.0-mod.zip`, the mod alone, for Athelas:** copy `ROTKPlus.dll` and `ROTKPlus.ini` from its `athelas_mod`
folder into `ROTKFix\mods\` in the game folder and turn ROTKPlus on in Athelas' Mods tab.

**The classic way** (`DivxMediaLib.dll`, in the same mod zip, for any setup with ROTK Fix):

1. Close the game. Make a backup copy of the game folder if you wish.
2. In the game folder, rename `DivxMediaLib.dll` to `DivxMediaLib_orig.dll`.
3. Copy `DivxMediaLib.dll` from the zip (or the one you built) into the game folder. ROTKPlus *is* a
   `DivxMediaLib.dll`: it forwards every call to the original and adds the mod. The game loads it as soon as it starts,
   before the opening movies.
4. Copy `ROTKPlus.ini` from the zip next to `rotk.exe`; it already has `[Coop] Enabled=1`. If you built the mod yourself,
   copy `config/ROTKPlus.ini.example` and rename it `ROTKPlus.ini` (it has the same defaults). Without the ini file the
   mod stays **off**; `Enabled=0` turns it off again.
5. Start the game through Athelas as usual. Every start writes a new log in `ROTKPlus_logs` (game folder), named with
   the date and time, and never overwrites the earlier ones; `ROTKPlus.log` is a copy of the latest. See *Logs*.

To remove it, delete ROTKPlus's `DivxMediaLib.dll` and rename `DivxMediaLib_orig.dll` back.

Use one way only (with both the `DivxMediaLib.dll` and the mod, only one changes the game, and the log says which).
Moving to the package from the classic way: delete ROTKPlus's `DivxMediaLib.dll` and rename `DivxMediaLib_orig.dll`
back; your `ROTKPlus.ini` and statistics stay.

## Logs

Each start of the game creates `ROTKPlus_logs\ROTKPlus_<date>_<time>.log` and keeps the older ones, so after a long
session you can still find the log of last night. `ROTKPlus.log` next to `rotk.exe` is a copy of the latest session. The
log starts with the setup (ROTKPlus version, Windows or Wine/Proton, sizes of the ROTK Fix files), then records every
patch, the menu choices, the heroes, deaths and falls. With `[Log] Detail=1` (off by default since 1.2.0) it also keeps
a detailed trace of every level: a snapshot of the players every minute, the game's statistics at the start and end of
each level, every kill, hit and pick-up; turn it on when you report a problem with the statistics. A line
`session end` closes the file when the game exits normally. `[Log] Keep=N` in `ROTKPlus.ini` keeps only the N newest
files (default 0: nothing is ever deleted; a session of an hour is usually well under 100 KB). `ROTKFix.log` is written
by ROTK Fix and is overwritten at every start; when the game closes, ROTKPlus puts a copy of it next to the session's
log, with the same date and time (`[Log] CopyRotkFixLog=0` turns this off).

`tools/check_log.py` (Python 3) summarises a log in a few seconds and can remove your user name from it before you post
it; [TESTING.md](TESTING.md) explains how to download and run it.

## Using it

In the main menu choose **Multiplayer** and confirm. Its page has two boxes: on the left the game (mode: campaign,
everybody against everybody or teams; the players, 2 to 4; for the Versus modes the enemies or the heroes only, the
lives and the pairs of the teams), on the right who plays (a name for each place, as in *Who is playing?*). Up/down go
through the rows of a box, the shoulder buttons (or Tab) go to the other box, left/right change the selected row, each
extra controller changes its own place; confirm or **Start** goes on to
the map. Players 1 and 2 choose their heroes as usual; then the third and fourth players choose theirs, one after the
other, with their own controllers. In a level the extra players join right away. Use the same save slot for players 1
and 2 to see everybody's progress. (`[Players] MultiplayerPage=0` gives back the two entries of before, **Co-op: N
players** and **Versus: N players**, with their choices on left/right.)

**Fellowship** (main menu): up/down choose an entry; on the right a preview shows what it holds. On one of the four
places, left/right (or the arrows) pick one of the names used before, Guest (no statistics) or *New name...*; confirm
opens that player's profile, or the editor for a new name. In the editor a keyboard types directly (Enter saves,
Escape cancels, Backspace deletes); with a controller move on the letters and confirm each one, *Delete* removes the
last letter, *Done* saves and back cancels. A word starts with a capital letter, the rest are lower case.

- **Comparison**: four players side by side in six tabs. Left/right change the tab, the shoulder buttons (or Page Up /
  Page Down) the players, up/down scroll the Enemies tab, confirm opens the profile of the selected player.
- **Player profiles**: one player's main numbers, heroes, enemies, bosses and levels completed; left/right another
  player, confirm the table of their heroes.
- **Records**, **Last game**, and **Manage names** (add a name, or choose one and rename it, merge it into another one,
  or delete it with its statistics; deleting and merging ask for confirmation).

**Who is playing?** comes after Single (and after Co-op or Versus with `MultiplayerPage=0`; the Multiplayer page has
its own *who plays* box): one row per player with the last line-up, left/right change a
row's name (every controller its own row), Start goes on to the hero selection. The statistics of a level are written
when the level is left (won, lost, or quit to the map or the menu); a level interrupted by a crash is not counted. A
player without a name is a guest: shown as "Player N", nothing is saved.

**Mouse.** On the pages the game's pointer selects an entry and a right click goes back. A left click uses the entry
only when the game turns it into a confirm, and on some setups it does not (then the left click does nothing on the
pages and on the Fellowship and Versus entries of the main menu): Enter and the controllers always work.

**Portraits.** The pages show the heroes' and the enemies' portraits. ROTKPlus does not ship pictures of the game: it
makes them on your computer from the game's own HUD portraits, the first time the game shows each of them (so an enemy
gets its portrait once you have met it), and keeps them in `ROTKPlus_icons` in the game folder. The game has no portrait
for Shelob, Gorbag and the mumakil, so their rows show the name only. To add a picture of your own, put a file
`FoeShelob.raw`, `FoeGorbag.raw` or `FoeMumakil.raw` in `ROTKPlus_icons`: the 4 bytes `RPI1`, the width and the height
(32-bit numbers, little-endian), then the pixels row by row from the top, 4 bytes each in the order blue, green, red,
alpha (128 x 128 looks like the others). An icon that is already there is never replaced.

Key `F9` (only if `[Diag] VictoryKey=1`) ends the level with a victory: useful to see the end-of-level screens.

### Testing without controllers

With `[Debug] SimPads=1` in `ROTKPlus.ini` players 2, 3 and 4 are driven by the keyboard and the game offers the
Multiplayer entry even with no controller connected; player 1 uses the game's own keyboard and mouse controls. Start the game from
Athelas with no controller connected. The keys work only while the game window has the focus:

| Player | Move | Cross (fast attack) | Circle (heavy attack) | Square (parry) | Triangle (fierce attack) | R2 (execution) |
|---|---|---|---|---|---|---|
| 2 | arrow keys | Right Ctrl | Right Shift | Delete | End | Page Down |
| 3 | I J K L | U | O | P | Y | H |
| 4 | numpad 8 4 5 6 (Num Lock on) | Num 0 | Num . | Num 1 | Num 2 | Num 3 |

In a level, N (player 3) and Num 9 (player 4) are their controller's Back: back next to player 1 or 2.
In the menus (hero selection, score, statistics, shop) the keys of player 1 also work for whoever's turn it is, players 2,
3 and 4 included: W A S D to move, Enter or Space to confirm, Escape or Backspace to go back.
One person can drive one character at a time, which is enough to look at the creation of the players, the HUD, deaths and
respawns, the hero selection and the end-of-level screens. It is a testing aid, not a way to play.

With `[Debug] SimPads=3` one controller drives all four players instead: in the levels players 2, 3 and 4 copy every
move of player 1 (Start and Select excepted), in the menus player 1's controller serves whoever's turn it is. Four heroes
on screen with one person at the controller, for videos and tests; not a way to play either.

## Configuration

`ROTKPlus.ini` (see `config/ROTKPlus.ini.example`, every key is documented there):

| Section | Key | Meaning |
|---|---|---|
| `[Coop]` | `Enabled` | `0` turns the whole mod off |
| | `Players` | 2, 3 or 4 (also changed from the menu) |
| | `Hero` | hero of the third player if not chosen (`auto`, `aragorn`, ...) |
| | `Pad`, `PadName` | use controllers for the extra players; pick the third one by part of its name |
| | `Hud`, `HudX`, `HudY`, `HudRow`, `HudScale` | position and size of the extra HUD boxes |
| | `MoreEnemies`, `EnemyFactor`, `AllyFactor` | more enemies and allies in the co-op campaign, in tenths (10 ... 70; -1 recommended, 0 allies as the enemies) |
| | `SingleEnemyFactor`, `SingleAllyFactor`, `MoreEnemiesMaxAlive` | the same for a single player; at most this many alive at once from one spawner (16) |
| | `Unstick`, `FollowTeleports` | bring back stuck or far players (1); the third and fourth follow the level's moves of players 1 and 2 (1) |
| | `MoreDraws` | room for 8192 items in the renderer's queue (1, default; with many enemies the HUD flickered) |
| `[PvP]` | `Enabled`, `Players`, `Lives` | the Versus modes, their number of players, lives per player |
| | `Teams`, `Enemies`, `Team1` | 2 vs 2 and heroes only (also changed from the menu); the two players of team 1 (`12`, `13`, `14`) |
| | `ArenasOnly` | the map offers only the three arenas in Versus games (1, default) |
| | `Rule`, `Time`, `Rounds` | last one standing (0) or timed (1), the minutes, the rounds of a tournament (1, 3, 5); also on the Multiplayer page |
| | `TimedRespawn` | seconds on the ground in a timed game (5; the game's own is 10) |
| | `SkipScoreScreens` | after a Versus game straight to the map (1, default) or the game's score screens (0) |
| | `BossScene` | with the heroes only, the Black Gate's boss comes for its opening scene and then goes away (1, default) |
| | `LastGame` | after a Versus game the Last game over the map until confirm or back (1, default) |
| | `HeavyUnblockable` | between players a parry cannot stop fierce attacks, the last moves of the combos and fully drawn shots (1, default) |
| | `SuddenDeath` | a timed round tied at the end goes on until the next knock-down (1, default) |
| | `Cheer_<level>`, `Music_<level>`, `CheerEvery` | audio commands of the level for hits and for the start (see the example ini) |
| `[Players]` | `Enabled`, `TextScale`, `TableScale`, `Shade` | the Fellowship entry (names and statistics); size of its texts; the dark rectangle behind them (0 none, 1 black) |
| | `AskWhoPlays` | "Who is playing?" before every game (1, default) or the names of the Fellowship page (0) |
| | `MultiplayerPage` | one Multiplayer entry with its page (1, default) or the Co-op and Versus entries (0) |
| | `HeadNames`, `HudNames`, `NameScale` | names above the heads in place of g1-g4 (on by default) and above the HUD boxes (off) |
| | `ScoreSummary`, `SummaryScale`, `SummaryY` | the level summary on the score screen (off by default) |
| | `CaptureIcons` | make the portraits from the game's own (1, default) |
| `[General]` | `Language` | `auto`, `en`, `it`, `fr`, `de`, `es` or your own code |
| | `FastText` | the mod's texts drawn in one go (1, default), also the game's own (2, tested on Windows and SteamOS), never (0) |
| `[Log]` | `Detail`, `Keep` | detailed level trace (0, default); how many session logs to keep (0 = all) |
| | `StatsBackups`, `CopyRotkFixLog` | dated copies of the statistics to keep (10); a copy of `ROTKFix.log` (1) |

## Language

The mod writes its own texts: the menu entries, "Player N" in the selection screen, the winner banner, and the Fellowship
pages (about 150 texts). Level names are the game's own. With `Language=auto` the language is recognised from
the game's own text for "Player 1". Built-in: English, Italian (verified), French, German, Spanish (unverified, no
accents because the game's menu font may not have them). To fix or add a language without recompiling, put a
`ROTKPlus.lang.ini` next to `rotk.exe`; see `config/ROTKPlus.lang.ini.example`. Any text can be replaced there by its
key: the keys of the menu texts are in the example, those of the Fellowship pages are the first column of the table
`kTxt` at the top of `src/Players.cpp`. Contributions of translations are welcome.

## Building

The same sources build two ways:

- **`DivxMediaLib.dll`** (the install described above), with Visual Studio 2022 or later (the "Desktop development
  with C++" workload), MSVC, 32-bit:

      msbuild ROTKPlus.slnx /p:Configuration=Release /p:Platform=x86

  The output is `src/Release/DivxMediaLib.dll`.
- **`ROTKPlus.dll`**, a mod of Athelas, with the 32-bit mingw-w64 g++ (Debian and Ubuntu: `g++-mingw-w64-i686`), on
  Linux or with a mingw-w64 toolchain on Windows:

      sh build.sh

  Copy `ROTKPlus.dll` and `athelas/ROTKPlus.ini` into `ROTKFix\mods\` in the game folder and turn it on in the
  launcher's Mods tab; ROTK Fix starts it (`ModInit`). Settings, statistics and logs stay in the game folder. If the
  `DivxMediaLib.dll` install is there too, only one of the two changes the game and the log says which.

- **The ROTKPlus launcher** and its ROTK Fix: see [`launcher/BUILDING.md`](launcher/BUILDING.md) (32-bit mingw-w64 g++,
  SDL3).

The inline assembly is written once for both compilers (`ASM(...)` lines, see `src/ROTKPlus.h`), and the reads of game
objects that may be gone go through `Guard` (`src/Guard.cpp`). The `src` folder has one file per area; the top of each
file explains what it patches and why.

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
  Versus mode, and on the Steam Deck or other Linux setups (Proton). Tell us what works and what does not, with the log
  of that session (`ROTKPlus_logs`) attached. [TESTING.md](TESTING.md) has a test round (even its first steps help), the
  log checker, and how to report; there is a **test report** form when you open an issue.
- **Improvements and integrations.** Ideas that would be great: cheers and music in the Southern Gate and the Courtyard
  (only the Black Gate has its crowd), separate save profiles for the extra players, the siege engines (ballistas,
  catapults) in the statistics. Pull requests and suggestions are both welcome.
- **Translations** of the mod's few texts (see *Language*).
- **Knowledge of the engine.** Corrections and additions to [`docs/ENGINE_NOTES.md`](docs/ENGINE_NOTES.md).

See [CONTRIBUTING.md](CONTRIBUTING.md) for how.

## Credits and legal

- Created by **brig-dev**, with the help of Claude (Anthropic) for the analysis and the code. The work consisted of
  reverse engineering the 2003 executable, writing the patches, and testing every feature in the game.
- The ROTKPlus launcher and ROTK Fix are [Athelas and ROTK Fix](https://github.com/Shagg88/Athelas) by **Shagg** (MIT;
  also at [SourceForge](https://sourceforge.net/projects/athelas/)), with our changes; his copyright notice is in
  [`launcher/LICENSE`](launcher/LICENSE). The mod itself includes none of their files. ROTKPlus was not made or endorsed
  by Shagg. DXVK, SDL3, Dear ImGui, miniz, OpenAL Soft, DSOAL and the fonts keep their own licences
  ([`launcher/licenses`](launcher/licenses)).
- *The Lord of the Rings: The Return of the King* is a trademark and a work of its respective owners (Electronic Arts,
  New Line Cinema, Warner Bros., the Tolkien Estate and others). ROTKPlus is an unofficial fan project, not affiliated
  with or endorsed by any of them. It contains no game code, data, art or sound; you need your own copy of the game.

## License

[MIT](LICENSE).
