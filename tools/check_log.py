#!/usr/bin/env python3
"""Summarise a ROTKPlus.log in a few lines (Python 3.8+, standard library only).

Usage:  python check_log.py [path\\to\\ROTKPlus.log] [--redact OUT.log]

Without a path it looks for ROTKPlus.log in the current folder. It tells you the ROTKPlus version, which patches were
installed, any ERROR / NOT installed / CRASH lines, what was played (modes, players, levels) and gives a verdict.
With --redact it also writes a copy of the log with the user name in Windows paths replaced, ready to attach to a report.
Exit code: 0 = no problems found, 1 = warnings, 2 = errors or crash, 3 = unreadable or not a ROTKPlus log.
"""
import re
import sys
from collections import Counter

# Startup lines of ROTKPlus 1.0.x that mean "a patch is in place": (text to look for, what it is)
EXPECTED = [
    ("hero choice and script hooks: active", "hero choice and script hooks"),
    ("sim: keyboard-simulated pads", "simulated pads (only if [Debug] SimPads=1)"),
    ("pads of players 3 and 4: active", "controllers of players 3 and 4"),
    ("HUD of the extra players: active", "HUD of the extra players"),
    ("hero of the extra players in index-based functions", "experience and kills of the extra players"),
    ("extra players' turns in the end-of-level screens: active", "turns in the end-of-level screens"),
    ("co-op with 3 or 4 players: installed", "co-op with 3 or 4 players"),
    ("pvp: versus mode patches", "versus mode"),
]
OPTIONAL = {"simulated pads (only if [Debug] SimPads=1)"}
# The menu patch is reported when the main menu first appears, not at startup
MENU_OK = "main menu: Co-op with"
MENU_BAD = "main menu: choice of the number of players NOT installed"

LINE = re.compile(r"^\[(\d\d:\d\d:\d\d\.\d{3})\] ?(.*)$")
USER_PATH = re.compile(r"(?i)([A-Z]:\\Users\\)([^\\/\r\n\"]+)")


def read_log(path):
    for enc in ("utf-8", "cp1252"):
        try:
            with open(path, encoding=enc) as f:
                return f.read().splitlines()
        except UnicodeDecodeError:
            continue
        except OSError as e:
            print(f"Cannot read {path}: {e}")
            sys.exit(3)
    print("Cannot decode the file.")
    sys.exit(3)


def main(argv):
    path, redact = "ROTKPlus.log", None
    args = argv[1:]
    while args:
        a = args.pop(0)
        if a == "--redact" and args:
            redact = args.pop(0)
        elif a in ("-h", "--help"):
            print(__doc__)
            return 0
        else:
            path = a
    raw = read_log(path)
    entries = []  # (time, text)
    for ln in raw:
        m = LINE.match(ln)
        if m:
            entries.append((m.group(1), m.group(2)))
        elif entries and ln.strip():  # continuation line without a timestamp
            entries[-1] = (entries[-1][0], entries[-1][1] + "\n" + ln)
    if not entries:
        print("This does not look like a ROTKPlus.log (no timestamped lines).")
        return 3
    texts = [t for _, t in entries]

    version = next((t[len("ROTKPlus "):] for t in texts if t.startswith("ROTKPlus ")), None)
    if version is None:
        print("This does not look like a ROTKPlus.log (no 'ROTKPlus <version>' line).")
        return 3
    print(f"ROTKPlus {version}   ({len(entries)} log lines, {entries[0][0]} to {entries[-1][0]})")

    errors, warnings = [], []
    if not re.match(r"1\.\d+", version):
        print("This script understands logs of ROTKPlus 1.x only; please update to the latest release.")
        return 1

    folder = next((t[len("game folder: "):] for t in texts if t.startswith("game folder: ")), None)
    if folder:
        print(f"Game folder: {folder}")
        if USER_PATH.search(folder):
            warnings.append("the log contains your Windows user name in a path; use --redact before posting it")
    if any("rotk.exe not recognised" in t for t in texts):
        errors.append("rotk.exe was not recognised (game version 1.0, 1,703,936 bytes, is required): nothing was changed")
    if any("[Coop] Enabled=0" in t for t in texts):
        print("Co-op is switched off in ROTKPlus.ini ([Coop] Enabled=0): only the language/menu parts do not apply.")
    ini = next((t for t in texts if t.startswith("ini: ")), None)
    if ini:
        print(ini)

    # Patches
    startup_text = "\n".join(texts)
    print("\nPatches:")
    for needle, what in EXPECTED:
        ok = needle in startup_text
        if what in OPTIONAL and not ok:
            continue
        print(f"  {'ok     ' if ok else 'MISSING'}  {what}")
        if not ok:
            errors.append(f"patch not reported: {what}")
    if any(MENU_BAD in t for t in texts):
        print("  FAILED   number-of-players choice in the main menu")
        errors.append("the main-menu choice of the number of players was NOT installed")
    elif any(t.startswith("main menu: entry MainMenuScreen.Coop") for t in texts):
        print("  ok       main menu entries (Co-op / Versus)")
    elif any(t.startswith("main menu:") for t in texts):
        warnings.append("the main menu was shown but the Co-op entry was not found in the log")
    else:
        print("  ?        the main menu was never reached in this log")
        warnings.append("the log ends before the main menu: nothing to say about the menu patches")
    lang = next((t for t in texts if t.startswith("language:")), None)
    if lang:
        print(f"  {lang}")

    # Problems
    for t in texts:
        if t.startswith("ERROR"):
            errors.append(t)
        elif "NOT installed" in t and MENU_BAD not in t:
            errors.append(t)
        elif t.startswith("CRASH"):
            errors.append(t.splitlines()[0])
        elif "not what was expected" in t or "not found" in t or "package list full" in t:
            warnings.append(t)
    if any(t.startswith("CRASH") for t in texts):
        crash = next(t for t in texts if t.startswith("CRASH"))
        print("\nThe game crashed. " + crash.splitlines()[0])
        extra = [t for t in texts if t.startswith("    callers:")]
        if extra:
            print("  " + extra[0].strip())
        print("  Please attach ROTKFix.log as well; the log is rewritten on every start.")

    # Session summary
    print("\nPlayed:")
    confirmed = [re.sub(r"^main menu: confirmed ", "", t) for t in texts if t.startswith("main menu: confirmed ")]
    if confirmed:
        print("  Modes confirmed: " + "; ".join(confirmed))
    else:
        print("  No mode was confirmed from the menu (normal 2-player game, or the log ended earlier).")
    heroes = [t for t in texts if t.startswith("co-op: player ") and " is " in t and "packages" in t]
    for t in heroes:
        print("  " + t.replace("co-op: ", ""))
    started = [t for t in texts if re.match(r"co-op with \d players:", t)]
    if started:
        print(f"  Levels loaded with extra players: {len(started)}")
    last_players = None
    for t in texts:
        m = re.match(r"co-op: (\d) players in the game, shared lives (\d+)", t)
        if m:
            last_players = (m.group(1), m.group(2))
    if last_players:
        print(f"  Last level: {last_players[0]} players, {last_players[1]} shared lives")
    pads = [t for t in texts if t.startswith("pad of player") or t.startswith("pad: SDL sees")]
    if pads:
        print(f"  Controllers (first lines): " + " | ".join(p.split(" (")[0] for p in pads[:4]))
    falls = sum(1 for t in texts if t.startswith("fall:"))
    deaths = sum(1 for t in texts if t.startswith("pvp:") and " is eliminated" in t)
    hits = sum(1 for t in texts if t.startswith("pvp:") and " hits player " in t)
    wins = [t for t in texts if t.startswith("pvp: versus game over")]
    if falls:
        print(f"  Falls into pits: {falls}")
    if hits or deaths or wins:
        print(f"  Versus: {hits} hits between players, {deaths} eliminations, {len(wins)} finished games")
    if any("end of level" in t for t in texts):
        n = sum(1 for t in texts if t.startswith("end of level"))
        print(f"  End-of-level events: {n}" + (" (includes F9 instant victory, a test key)" if any("F9" in t for t in texts) else ""))

    # Repeated lines (a sign of a loop)
    cnt = Counter(re.sub(r"[0-9A-F]{8}|\d+(\.\d+)?", "#", t) for t in texts)
    for t, n in cnt.most_common(3):
        if n >= 500:
            warnings.append(f"line repeated {n} times: {t[:80]}")

    # Verdict
    print()
    for w in dict.fromkeys(warnings):
        print("Warning: " + w)
    for e in dict.fromkeys(errors):
        print("PROBLEM: " + e)
    if errors:
        verdict, code = "PROBLEMS FOUND: please attach this log to a bug report.", 2
    elif warnings:
        verdict, code = "No errors, but see the warnings above.", 1
    else:
        verdict, code = "OK: all patches installed, no errors, no crash.", 0
    print(verdict)

    if redact:
        out = USER_PATH.sub(lambda m: m.group(1) + "USER", "\n".join(raw)) + "\n"
        with open(redact, "w", encoding="utf-8", newline="\n") as f:
            f.write(out)
        print(f"Redacted copy written to {redact}")
    return code


if __name__ == "__main__":
    sys.exit(main(sys.argv))
