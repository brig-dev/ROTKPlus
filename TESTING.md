# Testing ROTKPlus

Thank you for helping! ROTKPlus has been tested on very few setups, so every report is useful, **including "everything worked"**.
The full round below takes about 45 minutes, but you do not have to do everything: even steps 1 to 3 are useful. Do what
you can and tell us what you tried.

## What you need

- The game (version 1.0, the one Athelas calls "Game v1.0") and Athelas / ROTK Fix set up for it.
- **ROTKPlus 1.1.0 or newer** installed (see the README or `INSTALL.txt` in the zip). From 1.1.0 every start of the game
  writes its own log in the `ROTKPlus_logs` folder of the game folder and keeps the older ones; `ROTKPlus.log` is a copy
  of the latest.
- One controller per player. Only player 1 can use the keyboard and mouse; players 2, 3 and 4 need a controller each.
  With fewer controllers you can still help with the test mode without controllers (`SimPads`, see the README).
- Optional but handy: Python 3 (from python.org, or `python3` on Linux) to run the log checker below.

## Get the log checker

`check_log.py` reads a ROTKPlus log and tells you in a few seconds the ROTKPlus version, which patches were installed,
any `ERROR`, `NOT installed` or `CRASH` lines and what you played. With `--redact` it also writes a copy of the log with
your user name removed from the paths, ready to attach to a report. It needs only Python 3, no extra packages. Download it
once, into any folder:

- [check_log.py](https://raw.githubusercontent.com/brig-dev/ROTKPlus/main/tools/check_log.py) (right click, "Save link as").
- Or on Windows, in PowerShell:
  `Invoke-WebRequest https://raw.githubusercontent.com/brig-dev/ROTKPlus/main/tools/check_log.py -OutFile check_log.py`
- Or on Linux / Steam Deck (Desktop mode, Konsole):
  `curl -O https://raw.githubusercontent.com/brig-dev/ROTKPlus/main/tools/check_log.py`

Then, from the folder where you saved it, give it your game folder (it reads the newest session):

```
python check_log.py "C:\Games\ROTK" --redact ROTKPlus_redacted.log
```

On Linux use `python3` and the Linux path of the game folder, for example `python3 check_log.py ~/Games/ROTK --redact
ROTKPlus_redacted.log`. `ROTKPlus_redacted.log` is written in the folder you are in. If the summary says `OK`, the mod
installed correctly on your setup and the interesting part of your report is how the game behaved.

## A quick test round

1. **Start the game from Athelas**, reach the main menu and quit. Run `check_log.py` on the game folder: it should say
   `ROTKPlus 1.4.0` (or newer) and `OK`. Without Python, open the newest file in `ROTKPlus_logs`: any line with `ERROR`,
   `NOT installed` or `CRASH` is important.
2. **Main menu.** Do you see "Multiplayer" (or the equivalent in your language)? Confirm it: on the left the game (mode,
   players, enemies, victory, lives, tournament), on the right who plays. Do up/down, left/right and the shoulder buttons
   (or Tab) work, and does each extra controller change its own place? Are the texts in your language?
3. **Co-op with 3 players.** On the Multiplayer page pick Campaign and 3 players, Start, choose heroes (player 3, and then player 4, choose after players 1 and 2),
   start any level. Do all heroes appear, does each controller drive the right hero, do the HUD boxes and the g3 / g4
   marks look right?
4. **Fight, die and respawn.** Let an extra player die: is there a respawn countdown, does it come back? Are the lives
   sensible (two with two players, one more per extra player)?
5. **Finish the level.** Do the score, statistics and shop screens give every player a turn, with their own controller
   only? Do the kill counts of players 3 and 4 start from zero in every level?
6. **Co-op with 4 players**, the same checks.
7. **Versus** with 2, 3 and 4 players, and 2 vs 2: do hits, combos, specials, arrows, the spear and blocking work between
   players, and not between teammates? With the heroes only, last one standing and timed (a tie goes to sudden death),
   and a tournament: is the winner right, and does the map come back with the Last game over it?
8. **Save and load.** Save, quit, load the same slot: do players 3 and 4 keep their progress?
9. **Fellowship entry** (1.2 and later): give names with the keyboard and with a controller, play a level, look at
   Comparison (all six tabs), a player's profile, Records and Last game. Are the texts readable in your language and at
   your screen size, do kills, assists and deaths look right for each player? Does the mouse work on the pages?
10. **Who is playing?** (1.2 and later): after Single, or on the right of the Multiplayer page, does it show one row per
    player with the last line-up? Can each controller change its own row, and does Start lead to the hero selection with those names?

## Things we especially want to know

- Your edition and language of the game, and whether the mod's texts look right in it.
- Controllers other than Xbox / PlayStation / Steam Controller; two identical controllers; a controller unplugged while playing.
- The Steam Deck, and four players on Linux (three players on SteamOS with Proton have been tested).
- Levels other than the first ones: levels with computer allies, long levels, the last levels, levels with cutscenes.
- Athelas versions newer than 1.8.1, and ROTKPlus as an Athelas mod (the launcher's Mods tab).
- Wide screens (21:9 and wider): the Fellowship pages were only seen at 16:9.

## How to report

Open an issue (there are forms for a **test report** and for a **bug**) and attach the log of the session you are
reporting: the file in `ROTKPlus_logs` whose name has the date and time you played (each start of the game makes a new
one, and older ones are kept). **Remove your user name from the log first**: the log shows the path of your game folder,
which often contains it. `check_log.py --redact` does it for you; without Python, replace the name by hand in a text
editor. Attach `ROTKFix.log` too if the game crashed: that one is written by ROTK Fix and is overwritten at every start,
so copy it before restarting. Tell us:

- game edition / language, Windows or Linux version, Athelas version;
- number of players and the controllers you used;
- the level, and what you did just before the problem.

If something looks wrong on screen, a screenshot or a short video helps a lot.
