# The ROTKPlus launcher

The launcher of the ROTKPlus package (`ROTKPlus.exe`) and the ROTK Fix it puts in the game folder. Both come from
[Athelas and ROTK Fix](https://github.com/Shagg88/Athelas) by **Shagg** (MIT licence, see [LICENSE](LICENSE); while that
page is down, Athelas is at [SourceForge](https://sourceforge.net/projects/athelas/)): this is Athelas' launcher made
for The Return of the King (PC) only, with the ROTKPlus pages and settings in place of the Mods tab and without the
other games. Our changes are under the same MIT licence (see the repository's [LICENSE](../LICENSE)).

What is here:

| Folder | What |
|---|---|
| `launcher/src` | the launcher (C++17, Dear ImGui on SDL3): pages, settings, the Fellowship, History, screenshots, updates |
| `launcher/lang` | its texts in ten languages (`English = translation`, one per line) |
| `launcher/fonts`, `launcher/third_party` | the fonts (SIL OFL), Dear ImGui, miniz and stb_image, each with its licence |
| `src` | ROTK Fix (C), the DLL that makes the game run on modern PCs, as in Athelas 1.8.2 with our changes (fonts that can be replaced, the four players' controllers) |
| `licenses` | the licences of everything the package carries (DXVK, SDL3, OpenAL Soft, DSOAL, the fonts...) |
| `tools/package_win.ps1` | how the package is put together |

Not here: Athelas' Xbox ports, its SDK and tools (see Athelas itself); pictures (the launcher's emblem, banner and the
pictures for Steam's library are in the release package, not in the repository); the DLLs that the package takes as
they are from Athelas' release (SDL3, DXVK, DSOAL). Nothing of the game is here or in the package.

How to build it: [BUILDING.md](BUILDING.md). The package's own readme: [ROTKPLUS-README.txt](ROTKPLUS-README.txt).
