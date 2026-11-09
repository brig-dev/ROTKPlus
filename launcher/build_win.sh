#!/bin/sh
# ROTKPlus launcher: build on Windows with WinLibs GCC (i686) and the SDL3 mingw package.
#   sh build_win.sh            the launcher (launcher/build/ROTKPlus.exe)
#   sh build_win.sh fix        ROTK Fix (build/ROTKFix.dll, d3d8.dll, dinput8.dll)
# Needs: C:\Tools\winlibs-i686\mingw32\bin, C:\Tools\sdl\SDL3-3.4.18 (SDL_DIR to change it)
set -e
cd "$(dirname "$0")"
export PATH="/c/Tools/winlibs-i686/mingw32/bin:$PWD/tools/wrap:$PATH"
mkdir -p tools/wrap
for t in g++ gcc windres; do
  printf '#!/bin/sh\nexec %s "$@"\n' "$t" > tools/wrap/i686-w64-mingw32-$t
done
export SDL_DIR="${SDL_DIR:-/c/Tools/sdl/SDL3-3.4.18}"
if [ "$1" = fix ]; then sh build.sh; else sh launcher/build.sh; fi
