# Ketsui (Xbox 360) on Linux

An unofficial effort to statically recompile the Xbox 360 version of **Ketsui: Kizuna
Jigoku Tachi** into a native Linux program, using the [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk)
v0.10.0.

> [!IMPORTANT]
> **This repository contains no game code or assets.** You need your own legally obtained
> copy of the Xbox 360 game. Everything derived from it (the recompiled C++, the
> configuration generated from your copy, the executable built from it) is produced
> locally and must never be committed, uploaded or shared here.

Status: **version 1.1.1**. Tested on Fedora Atomic (Bazzite) with an AMD GPU (RADV), X11
through XWayland, and a PS4 controller seen by SDL.

## Status

The game boots to its title screen and runs its attract mode, its menus and the settings.
Rendering goes through the SDK's Xbox 360 GPU emulation (Vulkan). Input goes through SDL3,
with keyboard and controller support.

| Milestone | Goal | Status |
|---|---|---|
| 1 | The whole executable is recompiled and boots to the title and demo | ✅ Done |
| 2 | Stable play: no crash in a 10-minute session | ✅ Done (validated by a user session) |
| 3 | Input record and replay, for repeatable tests | ⏳ Not started |
| 4 | 60 fps: 99.9th percentile frame time ≤ 17 ms, no frame above 20 ms in 10 minutes | ❌ Not met (see below) |
| 5 | Desktop integration: first-run folder picker, saved configuration, launcher | ✅ Done |
| 6 | Release: no game file in the repository, reproducible build | ✅ Repository clean; release archives contain no game file |

The release archives contain the recompiled game code (translated from the game's PowerPC
code) but not the game's data. See [Legal](#legal).

### What works

- Title screen, attract mode, main menu, CONFIG (VISUAL, AUDIO, CONTROLS, DATA).
- Saved settings: CONFIG values are written to the save data and read back on the next
  start.
- First run: a message asks for the game folder, then a folder picker. The choice is saved.
- Exit: Escape (keyboard) or B (controller) on the main menu opens "Exit game?", with No
  selected by default.

### Known limits

- **60 fps is not guaranteed.** Most frames take about 16.7 ms, but 5 % to 20 % of frames
  (depending on the run) take about 33 ms, which is two vblank periods. The measurements in
  [PERF.md](PERF.md) show that GPU emulation is not the cause, and that the vblank delivery
  is on time. The delay sits in the game's own code and is not fixed.
- Native Wayland video is disabled (no `egl.pc` in the build). The game runs through X11
  or XWayland.
- Some guest code is still stubbed (for example `XamContentResolve` beyond the save path).

## Controls

Keyboard (default bindings, see the SDK's `keybind_*` cvars):

| Action | Key |
|---|---|
| Move | W A S D |
| A button (confirm, fire) | Space or `;` |
| B button (cancel, bomb) | Backspace or `'` |
| Start | Enter or X |
| Back | Tab or Z |
| Exit confirmation | Escape |

Controller: the usual face buttons and D-pad. **B** opens the exit confirmation on the main
menu only.

## Quick start (release archive)

1. Download `ketsui-<version>-linux-x86_64.zip` from the
   [Releases](https://github.com/sdiraimondo/KetsuiRecomp/releases) page and unzip it.
2. Run `./ketsui`. On the first run, choose the folder that contains `default.xex`
   (your copy of the game, with `Data/`, `nxeart` and `$SystemUpdate/`).

The archive needs an x86-64 CPU with SSSE3 and a Vulkan driver.

## Building from source

Requirements: `git`, `cmake` (3.25 or newer), `ninja`, `clang`/`clang++`, `pkg-config`, and
the SDL3 development packages. On Fedora, the SDK's `SDK_LINUX.md` lists them.
`zenity` is used by SDL for the first-run folder picker.

The SDK is not vendored here. Clone it next to this project at the release tag, with its
submodules, and apply the patches kept in `sdk_patches.diff`:

```sh
git clone https://github.com/sdiraimondo/KetsuiRecomp.git ketsui
cd ketsui

git clone --branch v0.10.0 https://github.com/rexglue/rexglue-sdk.git rexglue-sdk
git -C rexglue-sdk submodule update --init --depth 1 --recursive
git -C rexglue-sdk apply ../sdk_patches.diff
```

Point the project at your copy of the game. The code generator reads the game through
`ketsui_manifest.toml`: set `game_root` and `file_path` to the folder that holds your
`default.xex`, with the real case of the folder names.

```toml
[project]
game_root = "/path/to/your/Ketsui"

[entrypoint]
file_path = "/path/to/your/Ketsui/default.xex"
```

Configure and build (this also builds the SDK and its code generator):

```sh
cmake -S . -B build/ketsui -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_FLAGS=-march=x86-64-v2 -DCMAKE_CXX_FLAGS=-march=x86-64-v2 \
  -DREXSDK_DIR="$PWD/rexglue-sdk"
cmake --build build/ketsui --parallel
```

The `-march=x86-64-v2` flags match the SDK's own Linux preset and are needed: the SDK uses
SSSE3 intrinsics. The code generator writes `generated/default/` from your copy of the
game; that directory is never committed.

Run it:

```sh
build/ketsui/ketsui
# or, to pass the folder explicitly:
build/ketsui/ketsui --game_data_root /path/to/your/Ketsui
```

`tools/install_desktop.sh` writes a desktop launcher for your user only
(`~/.local/share/applications/ketsui.desktop`, and an icon under `~/.local/share/icons/`).

## How it works

1. The ReXGlue code generator turns the PowerPC code of `default.xex` into C++ (`generated/`).
2. `config/functions.toml` and `config/functions_scan.toml` give the function boundaries
   that automatic discovery missed. They hold addresses only, found by disassembling your
   copy. `generated/` is never edited by hand.
3. `src/` holds the host side: the application (folder picker, saved configuration, exit
   confirmation) and the save-data handling.
4. `sdk_patches.diff` lists every change made to the SDK: small fixes, and measurement tools
   that are inactive by default.

## Repository layout

- `src/`: the application (`ketsui_app.cpp` and `main.cpp`).
- `config/`: function boundaries for the code generator.
- `tools/`: `play.sh` (scripted test run), `install_desktop.sh` (launcher).
- `packaging/`: icon.
- `PERF.md`: frame-time measurements and what they show.
- `SDK_LINUX.md`, `SDK_CLI.md`, `GAME_ID.md`: notes on the SDK, its options, and the game
  identifiers.

## Legal

- Ketsui is a trademark of its publishers (CAVE, 5pb.). This project is **not affiliated
  with, endorsed by or sponsored by CAVE or 5pb.**
- The repository contains only original code, configuration values (addresses and function
  boundaries) and third-party code under its own license. It contains **no** copyrighted
  game code, assets, or keys.
- The release archives contain the **recompiled game code** (a translation of the game's
  PowerPC code), but not the game's data. Using and distributing them is your responsibility,
  under the copyright law of your country.
- Do not open issues or pull requests containing game files, or links to download the game.
- Please support the official release: buy the game.

## Credits

- [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk) (BSD-3-Clause) by Tom Clay and
  contributors: the recompiler, the runtime and the Xbox 360 GPU emulation.
- [Xenia](https://github.com/xenia-canary/xenia-canary) (BSD-3-Clause), the reference for many
  Xbox 360 kernel and GPU behaviours used by the SDK.

See `rexglue-sdk/LICENSE` (after the SDK is cloned) for the SDK's own notices.

## License

The project's own code and documentation are under the GNU General Public License, version
2 or (at your option) any later version. See [LICENSE](LICENSE).

The ReXGlue SDK is a separate project with its own license (see its `LICENSE`). The game is
not part of this repository and is not covered by this license.
