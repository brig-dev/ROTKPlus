# Changelog

## Unreleased

- `tools/check_log.py --redact` also removes the user name from Linux paths (Proton: `Z:\home\<name>`, `/home/<name>`).
- `TESTING.md`: how to download and run `check_log.py` without the source code; the round is described more honestly
  (about 45 minutes, even the first steps help). README: the keys of the test mode without controllers.

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
