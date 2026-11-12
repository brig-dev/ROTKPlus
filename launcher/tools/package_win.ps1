# PowerShell 5.1. Assembles the ROTKPlus package (launcher + ROTK Fix + the ROTKPlus mod) into -Out.
# Built pieces: launcher\build\ROTKPlus.exe, build\ROTKFix.dll / d3d8.dll / dinput8.dll (sh build_win.sh, sh build.sh),
# the ROTKPlus mod (ROTKPlus.dll from the ROTKPlus repository's build.sh). Taken as they are from Athelas' own release
# (-Base, the extracted zip): SDL3.dll, DXVK (ROTKFix\dxvk) and the licences with the
# LGPL sources. Left out: the Xbox ports, the upscalers (DX12, Xbox ports only), Valar Boons (another mod).
param(
    [Parameter(Mandatory = $true)][string]$Out,
    [Parameter(Mandatory = $true)][string]$Base,       # Athelas 1.8.2's release zip, extracted
    [Parameter(Mandatory = $true)][string]$PlusRepo,   # the ROTKPlus repository, with ROTKPlus.dll built (sh build.sh)
    [string]$Version = "dev"
)
$ErrorActionPreference = "Stop"
$repo = Split-Path -Parent $PSScriptRoot
if (Test-Path $Out) { throw "$Out exists: choose a new folder (nothing is overwritten)" }
New-Item -ItemType Directory -Force $Out | Out-Null
$fix = Join-Path $Out "ROTK-PC\pcfix"
# the launcher, its SDL3 and its translations
Copy-Item "$repo\launcher\build\ROTKPlus.exe" $Out
Copy-Item "$Base\SDL3.dll" $Out
Copy-Item "$repo\launcher\lang" $Out -Recurse   # (ours: Athelas' with the ROTKPlus texts)
Copy-Item "$Base\licenses" $Out -Recurse
# what Play puts in the game folder: Athelas' pcfix, with our ROTK Fix build and the ROTKPlus mod only
Copy-Item "$Base\ROTK-PC" $Out -Recurse
Copy-Item "$repo\build\ROTKFix.dll", "$repo\build\d3d8.dll", "$repo\build\dinput8.dll" $fix -Force
Remove-Item "$fix\ROTKFix\mods\ValarBoons*" -Force
Copy-Item "$PlusRepo\ROTKPlus.dll" "$fix\ROTKFix\mods\ROTKPlus.dll" -Force
Copy-Item "$PlusRepo\config\ROTKPlus.ini.example" "$fix\ROTKFix\mods\ROTKPlus.ini.example" -Force
Copy-Item "$PlusRepo\athelas\README.md" "$fix\ROTKFix\mods\ROTKPlus-README.md" -Force
Copy-Item "$PlusRepo\LICENSE" "$fix\ROTKFix\licenses\ROTKPlus-LICENSE.txt" -Force
# the mod's entry: on (this launcher carries it and has its own tab for it)
$ini = Get-Content "$PlusRepo\athelas\ROTKPlus.ini"
$ini = $ini -replace '^Enabled = 0', 'Enabled = 1' -replace '^Version = .*', "Version = $Version"
$ini | Set-Content "$fix\ROTKFix\mods\ROTKPlus.ini" -Encoding ASCII
# who made what
Copy-Item "$repo\LICENSE" "$Out\licenses\Athelas-LICENSE.txt"
Copy-Item "$PlusRepo\LICENSE" "$Out\licenses\ROTKPlus-LICENSE.txt"
Copy-Item "$repo\ROTKPLUS-README.txt" "$Out\README.txt"
# the front page's cover (ours, not the game's box art) and What's new (ROTKPlus' CHANGELOG, without the backticks)
New-Item -ItemType Directory -Force "$Out\covers" | Out-Null
# (the pictures are not in the public repository: without them the package has none, and the launcher shows its own)
if (Test-Path "$repo\launcher\art\rotk_cover.png") { Copy-Item "$repo\launcher\art\rotk_cover.png" "$Out\covers\rotk.png" }
else { Write-Warning "launcher\art\rotk_cover.png missing: no cover in the package" }
# the mark (the star of four gems) for the pages' header, since 1.5.1
if (Test-Path "$repo\launcher\art\rotk_mark.png") { Copy-Item "$repo\launcher\art\rotk_mark.png" "$Out\covers\rotk-mark.png" }
# the pictures for Steam's library (ROTKPlus as a non-Steam game), with how to set them
New-Item -ItemType Directory -Force "$Out\steam" | Out-Null
Copy-Item "$repo\launcher\art\steam\*" "$Out\steam\"
$notes = (Get-Content "$PlusRepo\CHANGELOG.md" -Encoding UTF8) -replace '`', ''
[System.IO.File]::WriteAllLines("$Out\WHATS-NEW.txt", $notes, (New-Object System.Text.UTF8Encoding($false)))
Get-ChildItem $Out -Recurse -File | Measure-Object -Property Length -Sum | ForEach-Object { "{0} files, {1:N1} MB" -f $_.Count, ($_.Sum / 1MB) }
