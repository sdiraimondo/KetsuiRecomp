# Ketsui (Xbox 360) on Linux

A native Linux port of **Ketsui: Kizuna Jigoku Tachi** (Xbox 360), made by statically
recompiling the game's PowerPC code with the [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk)
v0.10.0. Rendering goes through Vulkan, input through SDL3.

**This repository does not contain the game.** You need your own copy of the Xbox 360
game files (the folder with `default.xex`, `Data/`, `nxeart` and `$SystemUpdate/`).
Nothing from the game is committed, copied or downloaded by these scripts.

Status: version 1.1. Tested on Fedora Atomic (Bazzite) with an AMD GPU (RADV), X11 via
XWayland, and a PS4 controller seen by SDL.

## What you need

- Linux x86_64, with a working Vulkan driver (RADV, ANV or similar).
- Build tools: `git`, `cmake` (3.25 or newer), `ninja`, `clang`/`clang++`, `pkg-config`.
- Development packages for the SDL3 windowing and audio backends. On Fedora:
  `libX11-devel libxcb-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel
  libXrender-devel libXfixes-devel libXtst-devel libXScrnSaver-devel libxkbcommon-devel
  wayland-devel wayland-protocols-devel libdecor-devel mesa-libEGL-devel
  alsa-lib-devel pipewire-devel pulseaudio-libs-devel`.
  Without `mesa-libEGL-devel` the native Wayland driver stays off; X11 (through XWayland)
  still works.
- `zenity` (used by SDL for the first-run folder picker).

## Build

The SDK is not vendored here. Clone it next to this project at the release tag, with its
submodules, and apply the patches kept in `sdk_patches.diff`:

```sh
git clone https://github.com/<you>/ketsui.git
cd ketsui

git clone --branch v0.10.0 https://github.com/rexglue/rexglue-sdk.git rexglue-sdk
git -C rexglue-sdk submodule update --init --depth 1 --recursive
git -C rexglue-sdk apply ../sdk_patches.diff
```

Configure and build the game (this also builds the SDK and its `rexglue` code generator):

```sh
cmake -S . -B build/ketsui -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_FLAGS=-march=x86-64-v2 -DCMAKE_CXX_FLAGS=-march=x86-64-v2 \
  -DREXSDK_DIR="$PWD/rexglue-sdk"
cmake --build build/ketsui --parallel
```

The `-march=x86-64-v2` flags match the SDK's own Linux preset and are needed: the SDK
uses SSSE3 intrinsics.

## Point the project at your copy of the game

The code generator reads the game through `ketsui_manifest.toml`. Edit the two paths so
they point to the folder that holds your `default.xex`. They must match the real case of
the folder names (`rexglue init` writes them lowercase; this is an SDK bug, see below).

```toml
[project]
game_root = "/path/to/your/Ketsui"

[entrypoint]
file_path = "/path/to/your/Ketsui/default.xex"
```

Then run the build again. The code generator writes `generated/default/` from your copy.
It is not committed.

## Run

```sh
build/ketsui/ketsui
```

The Xenos renderer (`librexgpu-xenos.so`) is loaded by default. `--gpu_plugin xenos` is not
needed. Without a renderer the game runs with no picture at all.

On the first run, if no game folder is known, a message box asks you to choose it, then a
folder picker opens. Choose the folder that contains `default.xex`. The choice is saved in
`~/.config/ketsui/ketsui.toml` (`$XDG_CONFIG_HOME` is honoured). To pass a folder
explicitly, use `--game_data_root /path/to/your/Ketsui`.

Optional: a desktop launcher for your user (no system files are touched):

```sh
tools/install_desktop.sh
```

It writes `~/.local/share/applications/ketsui.desktop` and an icon under
`~/.local/share/icons/`.

## Settings

Press **F4** in the game to open the settings overlay, and **Save to config** to store
them. Most settings are the SDK's cvars (`--help` is not handled by the game; see
`SDK_CLI.md` for the list of options).

## Known issues and limits

- Native Wayland video is disabled (no `egl.pc` in the build). The game runs through X11
  and XWayland.
- No TATE (vertical) display mode. Rotate the display in your desktop settings if you
  need it.
- `rexglue init` writes lowercase paths into the manifest (SDK bug on case-sensitive
  file systems). Fix the two paths by hand as shown above.
- The SDK's `SaveConfig` (F4 "Save to config") also stores options given on the command
  line.
- A few guest functions are larger than the code generator's 1 MiB file target
  (warning only).
- Some guest code is still stubbed (for example `XamContentResolve`).
- **60 fps is not guaranteed.** On the test machine, most frames take about 16.7 ms, but
  between 5 % and 20 % of frames (depending on the run) take about 33 ms, which is two
  vblank periods. The measurements in `PERF.md` show that the GPU emulation is not the cause
  and that the vblank delivery is on time. The cause is in the guest code and is not fixed.

## Fixes and where they live

- `config/functions.toml`, `config/functions_scan.toml`: guest function boundaries that
  the automatic discovery missed. Addresses only, found by disassembling your copy.
- `sdk_patches.diff`: the only changes made to the SDK (8 files). Fixes:
  - a lock around the input device table (a double free when several threads poll the
    controller), with the matching header change;
  - an unlink of the shared-memory arena right after creation (a leftover multi-gigabyte
    tmpfs file after an abort);
  - the SDK version lookup reads the project's own version, not a parent git repository's
    tag (`CMakeLists.txt`).

  Measurement only, inactive by default:
  - per-frame CSV: the frame period is recorded at each flip, and the CSV is flushed every
    frame (`--perf_log_csv`); vsync counters (`vsync_vblanks`, `vsync_catchup_vblanks`,
    `vsync_late_us`);
  - `--vulkan_measure_skip_gpu`: skips shader translation, draws and copies. It keeps the
    guest command protocol running, to separate the cost of the GPU work.

## License

The project's own code and documentation are under the GNU General Public License,
version 2 or (at your option) any later version. See `LICENSE`.

The ReXGlue SDK is a separate project with its own license (see its `LICENSE`). The game
is not part of this repository and is not covered by this license.
