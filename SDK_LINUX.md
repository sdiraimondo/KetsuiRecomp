# ReXGlue SDK on Linux - findings

SDK: `rexglue/rexglue-sdk` tag `v0.10.0` (latest stable, published 2026-08-21), commit `f5337cdc947ff6d4c4196737e2c807a48f2a1fc2`.
Pre-releases (`nightly-*`) were not used.
Checkout: `PROJECT_DIR/rexglue-sdk` (submodules initialised with `--depth 1 --recursive`).

## Confirmed in the source

| Question | Answer | Where |
| --- | --- | --- |
| (a) Linux is a supported target | Yes. `REX_PLATFORM=linux-amd64`, `REX_PLATFORM_LINUX=1`. Windows, Linux and macOS only. | `CMakeLists.txt` (platform block), `CMakePresets.json` (`linux-amd64`), `.github/workflows/build-linux-amd64.yaml` |
| (b) Graphics backend on Linux | Vulkan, ON by default on Linux. D3D12 is Windows only. | `CMakeLists.txt` lines 36-42 (`REXGLUE_USE_VULKAN`) |
| (c) Gamepad input | SDL3, `sdl_input_driver.cpp`. Windowing is SDL3 as well (`window_sdl.cpp`). Audio is SDL3 (`src/audio/sdl/`). | `src/input/CMakeLists.txt`, `src/ui/CMakeLists.txt`, `src/audio/CMakeLists.txt` |
| SDL3 source | Vendored as submodule `thirdparty/sdl3` (SDL is built static, no system SDL needed). | `thirdparty/CMakeLists.txt` |
| Vulkan stack | Vendored: Vulkan-Headers, VulkanMemoryAllocator, SPIRV-Headers, SPIRV-Tools, glslang. The Vulkan loader is vendored only for Apple. The system `vulkan-loader-devel` is not needed. | `thirdparty/CMakeLists.txt` |
| libdrm / libva | Not referenced anywhere in the SDK CMake. Not needed. | `grep -rn libdrm|libva` |
| FFmpeg | Vendored, built from a static source list (libavutil and libavcodec only, no hwaccel). | `thirdparty/CMakeLists.txt` |

## Linux build dependencies: SDK requirement vs. host

Requirements taken from the SDK CMake and the Linux CI job (`ubuntu:22.04`, clang-20, cmake, ninja):

| Requirement | Who needs it | Host state |
| --- | --- | --- |
| cmake >= 3.25 | SDK | OK, 4.3.0 |
| ninja | presets (Ninja Multi-Config) | OK, 1.13.2 |
| clang / clang++ | presets | OK, 22.1.8 |
| lld | optional | OK, 22.1.8 |
| git | submodules | OK |
| pkg-config | `pkg_check_modules` | OK, `/usr/bin/pkg-config` |
| python3 | glslang, codegen tooling | OK, 3.14.8 |
| **`x11-xcb` (pkg-config, REQUIRED)** | `src/ui/CMakeLists.txt:151`, `pkg_check_modules(X11_XCB REQUIRED x11-xcb)` | **MISSING** |
| **`wayland-client` (pkg-config, REQUIRED)** | `src/ui/CMakeLists.txt:153`, REQUIRED because `include/rex/ui/vulkan/api.h` forces `VK_USE_PLATFORM_WAYLAND_KHR` and `VK_USE_PLATFORM_XCB_KHR` | **MISSING** |
| wayland-scanner, wayland-protocols, wayland-egl, wayland-cursor | SDL3 Wayland video driver (optional, SDL warns and disables) | MISSING |
| libX11, libXext, libXcursor, libXi, libXrandr, libXrender, libXss, libXtst, libXfixes (dev) | SDL3 X11 video driver (optional, disabled) | MISSING |
| libxkbcommon (dev) | SDL3 Wayland keyboard (optional) | MISSING |
| alsa-lib-devel | SDL3 ALSA audio (optional, disabled) | MISSING |
| pulseaudio-libs-devel, pipewire-devel | SDL3 PulseAudio / PipeWire audio (optional, disabled) | MISSING |
| libdrm-devel, libva-devel | not required by the SDK | not installed, not needed |

Runtime `.so` files for X11, Wayland, xkbcommon, ALSA, PulseAudio and PipeWire exist under `/usr/lib64`, but there are no headers, no `.pc` files, no `wayland-scanner` and no unversioned `.so` symlinks. SDL's runtime-loading path needs the unversioned names, so these drivers cannot be used either.

## What the first configure gave

`cmake --preset linux-amd64 -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++`

1. SDL3 summary: `Video drivers: offscreen`, `Audio drivers: dummy`, `Joystick drivers: hidapi linux virtual`. SDL has no real window system, so the game cannot display a window.
2. Configure stops with:

```
-- Checking for module 'x11-xcb'
--   Package 'x11-xcb' not found
CMake Error at /usr/share/cmake/Modules/FindPkgConfig.cmake:1093 (message):
  The following required packages were not found:

   - x11-xcb

Call Stack (most recent call first):
  /usr/share/cmake/Modules/FindPkgConfig.cmake:1166 (_pkg_check_modules_internal)
  src/ui/CMakeLists.txt:151 (pkg_check_modules)

-- Configuring incomplete, errors occurred!
```

Host search for the dev files (`find /usr /opt ~/.local ~/Apps ~/Dev`, names `x11-xcb.pc`, `wayland-client.pc`, `wayland-client.h`, `Xlib.h`, `xcb.h`): nothing found.

## Re-check after the user installed the -devel packages

Hard requirements (SDK `REQUIRED`), all present:
- `x11-xcb` 1.8.13 (libX11-devel, libxcb-devel)
- `wayland-client` 1.26.0 (wayland-devel)
- `wayland-scanner` at `/usr/bin/wayland-scanner`

Optional packages still missing, so the SDL3 video drivers stay disabled:
- X11 driver: libXext-devel, libXcursor-devel, libXi-devel, libXrandr-devel, libXrender-devel, libXfixes-devel, libXtst-devel, libXScrnSaver-devel
- Wayland driver: mesa-libEGL-devel (the `egl` pkg-config file), libdecor-devel (optional client-side decorations)

Present: wayland-protocols 1.49, xkbcommon 1.13.1, wayland-egl 18.1.0, wayland-cursor 1.26.0, alsa 1.2.16.1, libpipewire-0.3 1.6.9, libpulse 17.0.

Latest check (user installed everything except mesa-libEGL-devel):
- All listed X11, XCB, Wayland, xkbcommon, libdecor, ALSA, PipeWire and PulseAudio packages present.
- `mesa-libEGL-devel` (egl.pc) not installed. Not required for X11. SDL's Wayland driver needs egl.pc, so native Wayland stays OFF.
- Reconfigure result: `Video drivers: offscreen x11(dynamic)`, `Audio drivers: alsa(dynamic) pipewire(dynamic) pulseaudio(dynamic)`. The X11 path goes through XWayland on Bazzite (GNOME).

Configure result after the installs: passes. SDL3 reports `Audio drivers: alsa(dynamic) pipewire(dynamic) pulseaudio(dynamic)`, `Video drivers: offscreen`.

## Status

Configure passes. Build started in `rexglue-sdk/out/build/linux-amd64` (Release). No window is possible until the X11 or Wayland video driver is enabled, which needs the optional packages above.
