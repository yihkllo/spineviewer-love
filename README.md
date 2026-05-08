<div align="center">

# SpineLove

</div>

SpineLove is a Direct3D 11 Spine animation viewer focused on local preview,
runtime compatibility, multi-Spine composition, slot inspection, and lightweight
animation tools.

## Features

- Preview Spine skeletons from 2.1 through 4.2.
- Load `.skel` and `.json` skeleton files with atlas textures.
- D3D11 rendering with premultiplied alpha, background images, and multi-Spine layers.
- Animation playback, queue playback, track mix, skin mix, and per-Spine transform controls.
- Slot visibility tools, slot filtering, mouse hover selection, and outline inspection.
- Folder scanning, favorites, language switching, theme controls, and fullscreen mode.

## Build

Open `spinelove.sln` with Visual Studio 2022 and build the `SpineLove` project
for `x64` / `Release`.

The built executable is written to:

```text
out/build/sln/bin/x64/Release/spine love.exe
```

## Controls

| Input | Action |
|---|---|
| Mouse wheel | Scale the active Spine around the mouse position. |
| Shift + mouse wheel | Scale all visible Spine layers together. |
| Left drag | Move the active Spine. |
| Shift + left drag | Move all visible Spine layers together. |
| Middle click | Reset scale and fit the active Spine. |
| Shift + middle click | Reset all visible Spine layers. |
| Ctrl + left drag | Move the background image. |
| Ctrl + mouse wheel | Scale the background image. |
| F11 | Toggle fullscreen. |

## Main Panels

| Panel | Purpose |
|---|---|
| File | Load a single Spine skeleton. |
| Select Folder | Scan a folder and list available Spine files. |
| Spines | Manage multiple loaded Spine layers. |
| Animations | Play animations from the loaded skeleton. |
| Skin / Skin Mix | Apply one or more skins. |
| Size / Flip | Inspect size, mirror, rotate, and adjust transforms. |
| Track Mix | Add secondary animations on extra tracks. |
| Slot | Filter, hide, hover, pin, and outline slots. |
| Queue | Build an ordered animation playback list. |

## Third-Party Code

Project code is licensed under the repository license. Bundled third-party
libraries keep their own license notices in their source files, including Dear
ImGui and Esoteric Software's Spine runtimes.
