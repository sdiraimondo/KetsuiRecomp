# rexglue CLI (SDK v0.10.0)

Binary: `rexglue-sdk/out/linux-amd64/Release/rexglue`. Output captured with `--help`, not retyped.

## rexglue --help

```text
ReXGlue v0.10.0 - Xbox 360 Recompilation Toolkit 


rexglue [OPTIONS] SUBCOMMAND


OPTIONS:
  -h,     --help              Print this help message and exit 
          --version           Display program version information and exit 
          --log-level LEVEL   Log level (trace, debug, info, warn, error) 
          --log-file PATH     Append diagnostics to file 
  -v,     --verbose           Equivalent to --log-level=trace 
  -f,     --force             Skip confirmations and proceed 

Configuration Variables:
          --clock_no_scaling, --no-clock_no_scaling{false} 
                              Disable clock scaling (inverted: false = scaling enabled) 
          --clock_source_raw, --no-clock_source_raw{false} 
                              Use raw clock source without scaling 
          --log_level TEXT    Global log level: trace, debug, info, warn, error, critical, off 
          --log_file TEXT     Log file path (empty = auto sequential naming) 
          --log_verbose, --no-log_verbose{false} 
                              Enable verbose logging (sets level to trace) 
          --log_noisy, --no-log_noisy{false} 
                              Enable noisy/high-frequency log macros 
          --log_flush_interval TEXT 
                              Periodic flush interval in seconds (0 = off) 
          --log_max_file_size_mb TEXT 
                              Max log file size in MB before rotation 
          --log_max_files TEXT 
                              Max number of rotated log files to keep 
          --writable_executable_memory, --no-writable_executable_memory{false} 
                              Allow executable memory to be writable 
          --perf_log_csv TEXT Path to write per-frame CSV log (empty = disabled) 
          --allow_game_relative_writes, --no-allow_game_relative_writes{false} 
                              Not useful to non-developers. Allows code to write to paths 
                              relative to game://. Used for generating test data to compare 
                              with original hardware. 
          --host_present_from_non_ui_thread, --no-host_present_from_non_ui_thread{false} 
                              Allow presentation from non-UI thread 
          --present_letterbox, --no-present_letterbox{false} 
                              Enable letterboxing for non-native aspect ratios 
          --present_safe_area_x TEXT 
                              Horizontal safe area percentage (0-100) 
          --present_safe_area_y TEXT 
                              Vertical safe area percentage (0-100) 
          --present_effect TEXT 
                              Guest output effect: bilinear 
          --present_dither, --no-present_dither{false} 
                              Enable output dithering in the final present pass 
          --present_allow_overscan_cutoff, --no-present_allow_overscan_cutoff{false} 
                              Allow overscan cutoff based on safe area settings 
          --window_width TEXT Startup window width in logical pixels (0 = use app default) 
          --window_height TEXT 
                              Startup window height in logical pixels (0 = use app default) 
          --fullscreen, --no-fullscreen{false} 
                              Start the window in fullscreen mode 
          --monitor TEXT      Monitor index to display on (0 = default, 1 = primary, 2 = second 
                              monitor, etc.) 
          --video_driver TEXT SDL video driver to use, such as "wayland" or "x11". Empty picks 
                              SDL's default for the session 
          --video_mode_width TEXT 
                              Guest video mode width in pixels 
          --video_mode_height TEXT 
                              Guest video mode height in pixels 
          --resolution TEXT   Common resolution preset for both guest video mode and startup 
                              window (for example: 720p, 1080p, 1440p, 4k, 1280x720) 
          --video_mode_refresh_rate TEXT 
                              Guest video mode refresh rate in Hz 
          --vulkan_require_fragment_stores_and_atomics, --no-vulkan_require_fragment_stores_and_atomics{false} 
                              Deprecated and ignored for parity; fragmentStoresAndAtomics is 
                              always required for Vulkan GPU emulation 
          --vulkan_require_vertex_pipeline_stores_and_atomics, --no-vulkan_require_vertex_pipeline_stores_and_atomics{false} 
                              Deprecated and ignored for parity; vertexPipelineStoresAndAtomics 
                              is always required for Vulkan GPU emulation 
          --vulkan_require_geometry_shader, --no-vulkan_require_geometry_shader{false} 
                              Require geometryShader support for Vulkan GPU emulation (disable 
                              to allow fallback primitive emulation paths) 
          --vulkan_require_fill_mode_non_solid, --no-vulkan_require_fill_mode_non_solid{false} 
                              Require fillModeNonSolid support for Vulkan GPU emulation 
                              (disable to allow fallback to solid fill for line/point polygon 
                              modes) 
          --vulkan_log_debug_messages, --no-vulkan_log_debug_messages{false} 
                              Log Vulkan debug messages 
          --present_render_pass_clear, --no-present_render_pass_clear{false} 
                              Clear render pass during presentation 
          --vulkan_allow_present_mode_immediate, --no-vulkan_allow_present_mode_immediate{false} 
                              Allow immediate present mode (no vsync) 
          --vulkan_allow_present_mode_mailbox, --no-vulkan_allow_present_mode_mailbox{false} 
                              Allow mailbox present mode (triple buffering) 
          --vulkan_allow_present_mode_fifo_relaxed, --no-vulkan_allow_present_mode_fifo_relaxed{false} 
                              Allow FIFO relaxed present mode 
          --vulkan_validation_enabled, --no-vulkan_validation_enabled{false} 
                              Enable Vulkan validation layers 
          --vulkan_device TEXT 
                              Vulkan device index (-1 for auto selection) 
          --vulkan_prefer_geometry_shader, --no-vulkan_prefer_geometry_shader{false} 
                              Prefer physical devices supporting geometryShader when 
                              auto-selecting 
          --vulkan_prefer_fragment_stores_and_atomics, --no-vulkan_prefer_fragment_stores_and_atomics{false} 
                              Prefer physical devices supporting fragmentStoresAndAtomics when 
                              auto-selecting 
          --vulkan_prefer_vertex_pipeline_stores_and_atomics, --no-vulkan_prefer_vertex_pipeline_stores_and_atomics{false} 
                              Prefer physical devices supporting vertexPipelineStoresAndAtomics 
                              when auto-selecting 
          --vulkan_prefer_fill_mode_non_solid, --no-vulkan_prefer_fill_mode_non_solid{false} 
                              Prefer physical devices supporting fillModeNonSolid when 
                              auto-selecting 
          --audio_maxqframes TEXT 
                              Max buffered audio frames (range 4-64). Lower reduces latency but 
                              may cause stuttering. 
          --ffmpeg_verbose, --no-ffmpeg_verbose{false} 
                              Verbose FFmpeg output (debug and above) 
          --audio_mute, --no-audio_mute{false} 
                              Mute audio output 
          --input_backend TEXT 
                              Input backend: sdl, xinput 
          --guide_button, --no-guide_button{false} 
                              Enable guide button pass-through 
          --mnk_mode, --no-mnk_mode{false} 
                              Enable keyboard/mouse controller emulation 
          --mnk_mouse, --no-mnk_mouse{false} 
                              Use the mouse for the right stick. Off means the right stick 
                              comes from the keybind_rstick_* keys only 
          --mnk_sensitivity TEXT 
                              Mouse sensitivity for right stick 
          --keybind_a TEXT    A button 
          --keybind_b TEXT    B button 
          --keybind_x TEXT    X button 
          --keybind_y TEXT    Y button 
          --keybind_left_trigger TEXT 
                              Left trigger 
          --keybind_right_trigger TEXT 
                              Right trigger 
          --keybind_left_shoulder TEXT 
                              Left shoulder 
          --keybind_right_shoulder TEXT 
                              Right shoulder 
          --keybind_lstick_up TEXT 
                              Left stick up 
          --keybind_lstick_down TEXT 
                              Left stick down 
          --keybind_lstick_left TEXT 
                              Left stick left 
          --keybind_lstick_right TEXT 
                              Left stick right 
          --keybind_lstick_press TEXT 
                              Left stick press 
          --keybind_rstick_up TEXT 
                              Right stick up 
          --keybind_rstick_down TEXT 
                              Right stick down 
          --keybind_rstick_left TEXT 
                              Right stick left 
          --keybind_rstick_right TEXT 
                              Right stick right 
          --keybind_rstick_press TEXT 
                              Right stick press 
          --keybind_dpad_up TEXT 
                              D-pad up 
          --keybind_dpad_down TEXT 
                              D-pad down 
          --keybind_dpad_left TEXT 
                              D-pad left 
          --keybind_dpad_right TEXT 
                              D-pad right 
          --keybind_back TEXT Back button 
          --keybind_start TEXT 
                              Start button 
          --keybind_guide TEXT 
                              Guide button 
          --hid_mappings_file TEXT 
                              Path to SDL gamecontroller mappings file 
          --game_data_root TEXT 
                              Override game data path 
          --user_data_root TEXT 
                              Override user data path 
          --update_data_root TEXT 
                              Override update data path 
          --cache_root TEXT   Override shader cache path 
          --metadata_root TEXT 
                              Override metadata path 
          --protect_zero, --no-protect_zero{false} 
                              Protect the zero page from reads and writes 
          --protect_on_release, --no-protect_on_release{false} 
                              Protect released memory to prevent accesses 
          --scribble_heap, --no-scribble_heap{false} 
                              Scribble 0xCD into all allocated heap memory 
          --ignore_thread_priorities, --no-ignore_thread_priorities{false} 
                              Ignores game-specified thread priorities 
          --ignore_thread_affinities, --no-ignore_thread_affinities{false} 
                              Ignores game-specified thread affinities 
          --license_mask TEXT Set license mask for activated content 
          --user_country TEXT User's country ID 
          --headless, --no-headless{false} 
                              Don't display any UI, using defaults for prompts as needed 
          --user_language TEXT 
                              User's language ID 
          --log_high_frequency_kernel_calls, --no-log_high_frequency_kernel_calls{false} 
                              Log kernel calls with the kHighFrequency tag 
          --cl TEXT           Specify additional command-line provided to guest 
          --kernel_debug_monitor, --no-kernel_debug_monitor{false} 
                              Enable debug monitor 
          --kernel_cert_monitor, --no-kernel_cert_monitor{false} 
                              Enable cert monitor 
          --rexcrt_heap_enable, --no-rexcrt_heap_enable{false} 
                              Enable o1heap-backed CRT heap for 
                              RtlAllocateHeap/Free/Size/ReAlloc 
          --rexcrt_heap_size_mb TEXT 
                              Heap size in megabytes 
          --max_file_size_bytes TEXT 
                              Target source file size in bytes; sets how many recomp files a 
                              project is split into, chosen once and then persisted 
          --progress_log_frequency TEXT 
                              Log progress every N functions 
          --max_discovery_iterations TEXT 
                              Max iterations for function discovery convergence 
          --max_vtable_iterations TEXT 
                              Max iterations for vtable discovery 
          --max_resolve_iterations TEXT 
                              Max iterations for call target resolution 
          --max_eh_states TEXT 
                              Max C++ EH states before rejecting handler 
          --max_eh_try_blocks TEXT 
                              Max try blocks before rejecting handler 
          --max_eh_ip_map_entries TEXT 
                              Max IP-to-state map entries 
          --max_seh_scope_entries TEXT 
                              Max SEH scope table entries 
          --backward_scan_limit TEXT 
                              Max instructions to scan backward for jump table patterns 
          --max_jump_table_entries TEXT 
                              Max entries per detected jump table 
          --max_blocks_per_function TEXT 
                              Safety limit on blocks per function 

SUBCOMMANDS:
  codegen                     Analyze and generate C++ code 
  init                        Initialize a new project 
  recompile-tests             Generate Catch2 tests from PPC assembly 
```

## rexglue init --help

```text
Initialize a new project 


rexglue init [OPTIONS] [SUBCOMMAND]


OPTIONS:
  -h,     --help              Print this help message and exit 
          --project-name NAME REQUIRED 
                              Project name (becomes [project].name in the manifest) 
          --xex-path PATH REQUIRED 
                              Path to entrypoint XEX (e.g. assets/Default.xex) 
          --game-root PATH    Game asset root for DLL guest-path derivation 
          --project-root PATH Where to create the project (defaults to current directory) 
          --scan-dll          Scan --game-root for .dll files and add each as a [[modules]] 
                              entry 
          --template-dir PATH Custom template directory for overrides 

SUBCOMMANDS:
  module                      Add a DLL module to an existing project 
  achievements                Extract achievement metadata from XEX to TOML 
```

## rexglue codegen --help

```text
Analyze and generate C++ code 


rexglue codegen [OPTIONS] [config]


POSITIONALS:
  config PATH                 Path to manifest TOML (auto-discovered in cwd if omitted) 

OPTIONS:
  -h,     --help              Print this help message and exit 
          --target NAME ...   DLL target to build (repeatable; entrypoint always included) 
          --ignore-stamp      Regenerate even when inputs match the recorded stamp 
```
