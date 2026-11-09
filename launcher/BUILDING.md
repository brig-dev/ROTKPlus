# Building the ROTKPlus launcher and ROTK Fix

Everything is 32-bit (the game is), built with mingw-w64 g++ and the SDL3 mingw development package.

On Windows, with [WinLibs](https://winlibs.com/) GCC for i686 (MSVCRT or UCRT) and SDL3 3.4.x (`SDL3-devel-*-mingw`),
from Git Bash in this folder:

    sh build_win.sh          # the launcher: launcher/build/ROTKPlus.exe
    sh build_win.sh fix      # ROTK Fix: build/ROTKFix.dll, d3d8.dll, dinput8.dll

`build_win.sh` expects GCC in `C:\Tools\winlibs-i686\mingw32\bin` and SDL3 in `C:\Tools\sdl\SDL3-3.4.18`: change the
two paths at its top, or set `SDL_DIR`. On Linux, with `i686-w64-mingw32-g++` and `SDL_DIR` pointing to the SDL3 package,
`sh launcher/build.sh` and `sh build.sh` build the same files.

The versions: `ROTKPLUS_VERSION` (the launcher has ROTKPlus' version) and `VERSION` (ROTK Fix's, as in Athelas).

The package (PowerShell 5.1, Windows), once the launcher, ROTK Fix and the mod (`sh build.sh` in the repository's root,
`ROTKPlus.dll`) are built:

    powershell -File tools\package_win.ps1 -Out <new folder> -Base <Athelas 1.8.2's zip, extracted> -PlusRepo <the repository> -Version 1.5.0

It takes SDL3, DXVK and the licences with the LGPL sources as they are from Athelas' release, leaves out the Xbox ports
and the other mods, and adds our ROTK Fix, the mod, the launcher's texts and the pictures. The pictures (the emblem in
`covers\rotk.png` and those in `steam`) are not in the repository: take them from a released package.
