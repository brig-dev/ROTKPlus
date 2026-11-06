# Changelog

## 1.4.0

Versus grows up: one Multiplayer page, teams of the Sun and the Moon, heroes only, timed games and tournaments, the
Last game over the map; new fight statistics. It also brings 1.3.0, the Athelas mod build, which shipped inside
Athelas 1.8.1 and was never released here on its own.

- Versus: **knock-downs on the screen**. One message per event at the bottom of the screen, above the HUD boxes, for
  three seconds ("Alex downs Robin", "Alex knocks out Robin" on the last life, "Robin falls" or "Robin is out" when
  nobody did it). Every player's lives left, or the points of a timed game, are in their own HUD box in place of the
  kills, with the heroes only; with the enemies after them, "17 kills, 2 lives" in the game's language. Between the two top boxes: the clock of a timed game, the two
  teams' totals around it in their colours ("3  1:58  0", or "3 - 2" lives left) and the round of a tournament. (The
  first version had a line of lives and the messages under the top boxes: they covered the names above the heads.)
- Versus: the boxes of players down. An eliminated player's box is replaced by "out"; the third and fourth player's
  boxes disappear during the respawn like player 2's, leaving the countdown (they showed player 1's or 2's
  experience). The level's own extras are hidden: the co-op's shared lives (icon and number) and the allies' HUD at the
  top right (a portrait in a ring with the ally's life, the Courtyard's "47/200"), which sat on the third player's box.
- Versus: the banner in the middle is measured and centred (it began at a fixed point and long ones went out on the
  right); a team's Cup is "The Team of the Sun wins the Cup!" where the language wants an article (`TeamCupWinner`).
  The Cup is drawn with an outline, light and shade.
- Versus, timed games: a player gets up after 5 seconds instead of 10 (`[PvP] TimedRespawn`); the countdown goes down
  two a second. The log notes when the HUD is missing during a timed round (the clock stands still then).
- Versus: after the winner's banner the map comes with the **Last game** over it (who won, each player's knock-downs
  and falls); confirm or back closes it and the arena is chosen as usual (`[PvP] LastGame=0`: just the map).
- Versus, timed games: **sudden death**. A round tied at the top when the time is up goes on, the clock at 0:00, until
  a knock-down puts one side ahead (two minutes at most, then a draw; `[PvP] SuddenDeath=0`: a draw at once).
- Statistics, Comparison's Versus tab: **who downed whom**, a row for each player shown ("Downed Alex") with how many
  times each of the others did it; renaming, merging or deleting a name carries it along.
- Versus with the heroes only at the Black Gate: its boss still comes for the opening scene (Frodo's mithril and its
  lines; without it only the voice and the flying mithril were left) and goes away when the fight begins, before it
  can strike (`[PvP] BossScene=0`: no boss at all). The boss's ring at the top right and the level's enemy bar are
  hidden with the heroes only.
- Versus: after the game straight to the map, without the game's score, co-op statistics, shop and save screens (ten
  of them with four players; `[PvP] SkipScoreScreens=0` keeps them).
- Statistics, Fights tab: **speed attacks** and **fierce attacks that hit** an opponent (an enemy, or another player in a
  Versus game; one per swing, and not when a shield stops it), **shields broken** (by any attack: fierce, special, the
  last blow of a combo) and **kicks** (the physical attack), landed / thrown.
- The Last game of a Versus game: how it ended ("The Team of the Sun wins the Cup! (2 - 0)"), what kind of game it was,
  the winners marked in gold, each player's team and, in a tournament, rounds won; "Most knock-downs" in place of
  "Most kills".
- The player's profile: a band with Versus games won of played, Cups and opponents knocked down.
- The pages' shoulder buttons (another player in the statistics, the other box of the Multiplayer page) are read from
  every controller: on SteamOS they did nothing in the menus.
- `[Diag] AudioLog`: its 3000 lines are counted again for every level.
- Versus: **tournaments** at the best of 3 or 5 rounds. A round won is a point; between two rounds the eliminated
  players get up and those standing get their full life back; the first to win more than half takes the **Cup** (a
  banner with a gold cup). A drawn round counts for nobody.
- Versus: **timed** games (2, 3 or 5 minutes): no lives, every knock-down of an opponent is a point, the most points
  win (by player, or by team). The clock runs only while the HUD is on the screen.
- Statistics, Versus tab: opponents eliminated, rounds played and won (in tournaments: a single
  game is not counted as a round), tournaments played, cups won, team games and timed games won, the most knock-downs
  in a timed game.
- **Multiplayer**: one main menu entry in place of Co-op and Versus. Its page chooses, on the left, the mode (campaign,
  everybody against everybody, teams), the players, the enemies, the lives and the teams, and on the right who plays;
  Start goes on to the map. Up/down stay in one box and go on to Start and Back, the shoulder buttons (or Tab, Page Up,
  Page Down) go to the other box, confirm starts from any row. Fellowship moves to the entry Versus leaves free, so the
  English edition keeps its Play Trailer entry. `[Players] MultiplayerPage=0` gives back the two entries.
- 2 vs 2: the teams are the Sun's (gold) and the Moon's (silver-blue), also in the banners (`TeamSun`, `TeamMoon` in
  `ROTKPlus.lang.ini`).
- The menu's glint (the star on the selected entry) is hidden while a page of the mod is open (it stayed over the
  page where the menu's entry was) and comes back on the menu when the page closes.
- The pages' help line: each key in a dark box with a gold edge, then what it does; up/down and left/right as arrows;
  confirm and back are no longer listed.
- The pages' labels, headings and help lines are really dimmer than the entries, as they were meant to be (the
  colour of the menu's label is above the font's own, so the dimming never showed).
- The banners in the middle of the screen (teams, winner) wait for the HUD: the teams' banner used to run out during the
  level's opening cutscene.
- Versus: **2 vs 2** (players 1 and 2 against 3 and 4, `[PvP] Team1` for other pairs): teammates cannot hit each other
  with any attack, the teams are shown at the start, the last team standing wins (both players get the win in the
  statistics).
- Versus: **heroes only**. The characters of the enemy side are not created, neither those placed in the level nor
  those of its spawners; the allies are. Chosen on the Versus entry like the number of players
  (`[PvP] Enemies`).
- Versus: the map offers only three arenas, the Southern Gate, the Courtyard of Minas Tirith and the Black Gate, open
  even if not unlocked yet; the cursor jumps from one to the next (`[PvP] ArenasOnly=0` for every level).
- Versus, experimental: a sound of the level when a player hits another one and a music at the start
  (`[PvP] Cheer_<level>`, `Music_<level>`: audio commands taken from the log, several joined by '+'). The Black Gate has
  its crowd built in: the soldiers around the arena cheer as they do when Aragorn hits its boss. `[Diag] AudioLog=1` now also writes the
  sounds the level scripts play, to find them.

## 1.3.0 (the Athelas mod build, in Athelas 1.8.1)

- ROTKPlus can also be a mod of Athelas: `build.sh` builds `ROTKPlus.dll` with mingw-w64 g++ for `ROTKFix\mods`
  (turned on in the launcher's Mods tab, started by ROTK Fix), from the same sources as `DivxMediaLib.dll`, which is
  still built with MSVC. Settings, statistics and logs stay in the game folder. With both installed only one changes
  the game, and the log says which.
- The log says how ROTKPlus was started (`started as the ...`) and whether `ROTKFix\mods\ROTKPlus.dll` is there.
- README: the game loads `DivxMediaLib.dll` as soon as it starts, not at the first movie (ROTK Fix's
  `SkipOpeningLogos` does not matter for it).

## 1.2.1

- Fixed: on the Fellowship pages Enter sometimes did nothing, or acted on the place under the mouse pointer instead of
  the selected entry. A left mouse button that Windows still reported as held (it can happen after Alt+Tab) made every
  confirm look like a click. Now a click is a press of the button a moment ago, not a button that is only down, and a
  confirm with Enter or Space held is always the keyboard's. The same check is used in the game's own menus, where a
  held button could drop the first player's confirm.
- Known issue: on some setups the game does not turn a left click into a confirm on the Fellowship pages and on the
  Fellowship and Versus entries of the main menu, so the left click does nothing there. Enter and the controllers
  always work; the pointer still selects and the right click still goes back.

## 1.2.0

### Fellowship: names and statistics

A new entry in the main menu, **Fellowship** ("Compagnia" in Italian). `[Players]` in `ROTKPlus.ini`; `Enabled=0`
turns it off.

- **Names.** Each player can have a name, typed with the keyboard or with on-screen letters for controllers. The names
  appear above the players' heads in place of g1-g4 (`HeadNames`), optionally above the HUD boxes (`HudNames=1`), in the
  hero selection ("Player 1 - Alex, select your character") and in the Versus winner banner. The third and fourth
  player's controllers remember the name of their player. Names are compared without regard to capitals.
- **Who is playing?** Confirming Single, Co-op or Versus first shows one row per player with the last line-up used for
  that number of players: left/right changes the name (each controller its own row), Guest plays without statistics,
  "New name..." opens the editor, Start goes on to the hero selection. `[Players] AskWhoPlays=0` keeps the names of the
  Fellowship page instead.
- **The Fellowship page**: the four places (with the favourite hero's portrait and the name between two arrows),
  Comparison, Player profiles, Records, Last game, Manage names; on the right a preview of what the selected entry
  holds; a help line at the bottom.
- **Comparison**: four players side by side in six tabs (Games, Levels, Fights, Enemies, Bosses and combos, Versus).
  Left/right change the tab, the shoulder buttons or Page Up / Page Down the players, up/down scroll the Enemies tab,
  confirm opens the player's profile.
- **Player profile**: the favourite hero, the time played, the main numbers, the nine heroes, the peoples of enemies
  with their captains, the bosses beaten and the levels completed; confirm opens the table of the player's heroes.
- **What is counted**, per name across sessions (`ROTKPlus_players.ini` in the game folder): levels played, won and
  completed (each level by name), time, kills (of which at range, with spears, killing moves, playing alone), assists,
  deaths, kills by rating, experience, favourite hero; kills by kind of enemy (orcs, Uruks, Uruk-hai, berserkers, the
  Dead, Easterlings, spiders, trolls, mumakil, Nazgul, with soldiers, shields, archers and captains apart), the bosses
  (Gollum, Shelob, Gorbag, the King of the Dead, the Mouth of Sauron), combos by name and enemies in one blow; blows and
  arrows parried, ladders pushed down, enemies set on fire, health, arrows and experience picked
  up; Versus games, wins, opponents knocked down and own falls; and the same per hero. Players without a name are
  guests (nothing saved); "Retry" after a defeat is still the same level.
- **Records** in three groups (in a level: kills, Perfect, assists; overall: kills, levels won; Versus: fastest win,
  wins), each with its holders, and who holds the most. **Last game**: the level, how it ended and a row per player
  (hero, kills, assists, deaths), the most kills in gold, the fellowship's total (a short summary also at the top of
  the score screen with `ScoreSummary=1`). **Manage names**: add a name, rename, merge two names, delete (merging and deleting ask
  first; renaming onto another player's name asks as a merge).
- **Portraits** in front of the heroes and the kinds of enemy: ROTKPlus ships no pictures of the game; it makes them on
  your computer from the game's own HUD portraits the first time the game shows them (`ROTKPlus_icons` in the game
  folder, `[Players] CaptureIcons`). The game has no portrait for Shelob, Gorbag and the mumakil: their rows show the
  name only (you can add your own picture, see the README).
- **Mouse**: the game's pointer selects an entry, a left click uses it, a right click goes back; the arrows can be
  clicked; the editor's letters too.
- All the texts in English, Italian, French, German and Spanish (the last three to be checked by native speakers), hero
  names and levels in the game's own words.
- A dated copy of `ROTKPlus_players.ini` in `ROTKPlus_logs` at every start, when it changed (the newest 10 are kept,
  `[Log] StatsBackups`). `ROTKPlus_add.ini` in the game folder, if present, adds its values to the statistics once
  (to correct them by hand) and is then renamed `.done`.

### Other changes

- **Fixed: silent music and sounds with 4 players.** The game keeps the sound banks of a level in tables of 7 places,
  and every hero brings one. With the extra heroes the next bank overwrote the setting that mutes groups of sounds, so
  the music, the voices or the effects (a different set each time) were silent until a cutscene or a big fight reset
  it. The tables now have 32 places (`[Coop] SoundBanks=1`, the default).
- **Faster texts.** The game draws a text one letter at a time, with a lock of the vertex buffer and a draw call for
  every letter; under DXVK the mod's pages fell to 40-55 fps. ROTKPlus now draws each of its texts in one go: 60 fps.
  `[General] FastText=2` does it for the game's own texts too (untested), `0` turns it off.
- The game's menus with the mouse: the pointer selects the main menu's entry under it again, and one click no longer
  does two things. A confirm in the first 150 ms of a new screen is dropped, as the game's own 50 ms pause (removed by
  ROTK Fix) did: leaving Credits no longer chose Single.
- The main menu's demo no longer starts while a ROTKPlus page is being used.
- Logs: `[Log] Detail` is now `0` by default; `1` adds a detailed trace of every level (for reporting problems with the
  statistics). When the game closes, a copy of `ROTKFix.log` (which ROTK Fix overwrites at every start) is put next to
  the session's log (`[Log] CopyRotkFixLog`). Audio diagnostics for testing (`[Diag] AudioLog=1`, off by default). The
  crash log no longer writes, without limit, the access violations that ROTKPlus handles itself. The players' deaths
  are noted even with the Versus mode switched off.
- `tools/check_log.py` knows ROTK Fix up to 1.8.0, the sound banks and the names-and-statistics lines, and `--redact`
  also removes the user name from Linux paths (Proton: `Z:\home\<name>`, `/home/<name>`).
- Test mode without controllers (`SimPads`): Enter, Space, Escape, Backspace and W A S D work for player 2's turns in
  the menus too. `[Debug] StatsTest=1` adds three made-up players with huge numbers to check the pages' layout.
- `TESTING.md`: how to get and run `check_log.py` without the source code; the new pages in the test round.

Upgrading from 1.1.0: replace `DivxMediaLib.dll` in the game folder with the new one (keep `DivxMediaLib_orig.dll`).
Your `ROTKPlus.ini` keeps working; the new keys are optional (see `config/ROTKPlus.ini.example`). The detailed log is
now off by default: add `Detail=1` under `[Log]` if you want it.

## 1.1.0

- **A log for every session.** Each start of the game writes a new log, `ROTKPlus_logs\ROTKPlus_<date>_<time>.log` in
  the game folder, and never deletes or overwrites the earlier ones, so the log of a session that went wrong is still
  there after restarting the game. `ROTKPlus.log` is a copy of the latest session. The log starts with the system
  (Windows, or Wine / Proton) and the sizes of the ROTK Fix files, and ends with a `session end` line (if it is missing,
  the game crashed or was closed abruptly).
- **More detail in the log** (`[Log] Detail=1`, the default): a snapshot of the players every minute in a level, and the
  game's per-hero statistics at the start and at the end of each level. `[Log] Keep` limits how many session logs are
  kept (0, the default, keeps all of them). See `config/ROTKPlus.ini.example`.
- **Fix:** a third or fourth player could start a level with the *Perfect* kills of an earlier level (the other ratings
  were already cleared).
- `tools/check_log.py` reads the newest session log from the game folder, and no longer reports as missing the parts
  that are switched off in `ROTKPlus.ini`.
- `docs/ENGINE_NOTES.md`: the per-hero statistics record.

Upgrading from 1.0.0: replace `DivxMediaLib.dll` in the game folder with the new one (keep `DivxMediaLib_orig.dll`).
Your `ROTKPlus.ini` keeps working; add a `[Log]` section only if you want to change the defaults.

## 1.0.0

First release: co-op with 3 and 4 players, Versus mode for 2 to 4 players, texts in the game's language.
