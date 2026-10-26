# Testing ROTKPlus

Thank you for helping! ROTKPlus has been tested on very few setups, so every report is useful, **including "everything worked"**.
A full round takes about 15 minutes. You do not have to do everything: do what you can and tell us what you tried.

## What you need

- The game (version 1.0, the one Athelas calls "Game v1.0") and Athelas / ROTK Fix set up for it.
- ROTKPlus installed (see the README). Check that `ROTKPlus.log` appears in the game folder after the first start.
- One controller per player. Two players can share a keyboard only for the first player; see the README for the test mode
  without controllers (`SimPads`), which is also welcome.

## A quick test round

1. **Start the game from Athelas.** In `ROTKPlus.log`, look for the lines "active": any line with `ERROR`, `NOT installed`
   or `CRASH` is important.
2. **Main menu.** Do you see "Co-op: N players" and below it "Versus: N players" (or the equivalents in your language)? Do
   left/right cycle through 2, 3 and 4? Do the entries show your language?
3. **Co-op with 3 players.** Pick Co-op 3, choose heroes (players 3 and 4 choose after 1 and 2), start any level.
   Do all heroes appear, does each controller drive the right hero, do the HUD boxes and the g3 / g4 marks look right?
4. **Fight, die and respawn.** Let an extra player die: is there a respawn countdown, does it come back? Are the lives sensible
   (two with two players, one more per extra player)?
5. **Finish the level.** Do the score, statistics and shop screens give every player a turn, with their own controller only?
6. **Co-op with 4 players**, the same checks.
7. **Versus mode** with 2, 3 and 4 players: do hits, combos, specials, arrows, the spear and blocking work between players?
   Does the last player standing win, with a banner, and does the level close?
8. **Save and load.** Save, quit, load the same slot: do players 3 and 4 keep their progress?

## Things we especially want to know

- Your edition and language of the game, and whether the mod's texts look right in it.
- Controllers other than Xbox / PlayStation / Steam Controller; two identical controllers; a controller unplugged while playing.
- Linux / Steam Deck (Proton).
- Levels other than the first ones: levels with computer allies, long levels, the last levels, levels with cutscenes.
- Athelas 1.6 in real gameplay.

## Check your log in a few seconds

`tools/check_log.py` reads `ROTKPlus.log` and tells you the ROTKPlus version, which patches were installed, any `ERROR`, `NOT installed` or `CRASH` lines and what you played. It needs only Python 3 (no extra packages):

```
python tools\check_log.py C:\Games\ROTK\ROTKPlus.log --redact ROTKPlus_redacted.log
```

`--redact` writes a copy with your Windows user name removed from the paths, ready to attach to a report. If the summary says `OK`, the mod installed correctly on your setup and the interesting part of your report is how the game behaved.

## How to report

Open an issue (there are forms for a **test report** and for a **bug**) and attach `ROTKPlus.log` (and `ROTKFix.log` if the
game crashed). The log is rewritten every time the game starts, so copy it before restarting. **Remove your user name
from the log first**: it shows the path of your game folder. Tell us:

- game edition / language, Windows version, Athelas version;
- number of players and the controllers you used;
- the level, and what you did just before the problem.

If something looks wrong on screen, a screenshot or a short video helps a lot.
