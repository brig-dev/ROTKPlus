# Contributing

Thank you for looking at ROTKPlus. The most useful things you can do:

1. **Report a problem.** Open an issue and attach the log of that session (`ROTKPlus_logs`: every start of the game
   writes a new file and keeps the old ones). Say which game edition and language you use, how many players and controllers, which
   level, and what you did just before. A screenshot helps. Please remove anything personal from the log first (the log
   shows your game folder path).
2. **Translate.** The mod writes six short texts. Add a section for your language to `config/ROTKPlus.lang.ini.example`
   and, if it is verified in the game, to the table in `src/Lang.cpp`.
3. **Test on other setups.** Steam Deck and other Linux setups (Proton), other editions of the game, Athelas versions newer than 1.6.0.
4. **Fix or extend the code.** Open an issue first for anything bigger than a small fix, so the work is not wasted.

## Notes for code changes

- The code is C++17 for MSVC, 32-bit, with inline assembly in "naked" trampolines. See the comment at the top of
  `src/ROTKPlus.h` about `GameFn`: calls from C++ to a naked trampoline must go through it.
- Every patch checks the bytes it replaces (`Patch`, `PatchDword`, `PatchCall`, `PatchJmp`). Keep it that way: an
  executable that is not the expected one must be left untouched.
- Addresses are for `rotk.exe` 1.0 (1,703,936 bytes). Comments give the address and what the function does;
  `docs/ENGINE_NOTES.md` has the overview.
- A build that compiles is not a tested build. Describe in the pull request what you tried in the game.
