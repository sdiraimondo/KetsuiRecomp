# Performance measurements

## Settings used (every run)

- Build: Release, clang 22, `-march=x86-64-v2`. Perf counters enabled for measurement
  (`-DREXGLUE_ENABLE_PERF_COUNTERS` in the C and C++ flags; the SDK turns them off in Release by default).
- Vulkan layers disabled: `VK_LOADER_LAYERS_DISABLE='*'`, `MANGOHUD=0`, `DISABLE_LS_FRAME_GENERATION=1`.
- GPU: AMD BC-250 (RADV GFX1013), X11 through XWayland, window 2648x1490.
- Scenario: title/attract demo, no input, about 150 s per run. Not real play.
- Frame time: `frame_time_us` in the CSV, the interval between two `Flip` calls (guest swaps).
  Written by the measurement patch in `rexglue-sdk/include/rex/perf/counter.h`.
- Cache: `--cache_root` on an empty directory outside the repo. Cold = first run on it,
  warm = second run on the same directory.

## S0: current path (Xenos emulation, Vulkan)

| Run | Frames | Mean | Median | p99 | p99.9 | max | > 17 ms | > 20 ms | > 100 ms | Pipelines |
|---|---|---|---|---|---|---|---|---|---|---|
| Cold cache | 5180 | 18.75 ms | 16.86 ms | 34.45 ms | 50.47 ms | 65.9 ms | 2196 | 624 | 0 | 27 created |
| Warm cache | 7476 | 19.95 ms | 16.94 ms | 34.75 ms | 50.34 ms | 83.4 ms | 3463 | 1428 | 0 | 13 from storage |

Milestone 4 target: p99.9 at or under 17 ms, no frame over 20 ms in 10 minutes.
Not met: p99.9 is about 50 ms.

## What this says

- The 100 to 620 ms stalls reported for Rayman on Apple M1 are **not present** here (0 frames over 100 ms).
- The persistent pipeline cache does not change the picture (mean 18.75 ms cold, 19.95 ms warm).
- The frame time is about 17 ms at the median, but 12 to 19 % of frames miss the 20 ms budget.
- The source of the misses is **not yet known**: GPU emulation, guest CPU, or thread synchronisation.
  The graphics counters (`draw_calls`, `vertices_processed`, `command_buffer_stalls`) are not fed by the
  Xenos path in SDK v0.10.0, so the CSV cannot attribute the time.

## S1: same scene with the GPU work skipped (measurement patch)

Setting: `--vulkan_measure_skip_gpu` (SDK cvar, measurement only). Shader translation (`LoadShader`),
draws (`IssueDraw`) and copies (`IssueCopy`) return immediately. The guest command protocol, ring,
swaps and frame timing are the SDK's own, unchanged. Same scene, same flags otherwise.

| Run | Frames | Mean | Median | p99 | p99.9 | max | > 17 ms | > 20 ms | > 100 ms |
|---|---|---|---|---|---|---|---|---|---|
| S0 cold (full emulation) | 5180 | 18.75 ms | 16.86 ms | 34.45 ms | 50.47 ms | 65.9 ms | 2196 | 624 | 0 |
| S0 warm (full emulation) | 7476 | 19.95 ms | 16.94 ms | 34.75 ms | 50.34 ms | 83.4 ms | 3463 | 1428 | 0 |
| S1 skip #1 | 7749 | 19.25 ms | 16.88 ms | 34.57 ms | 50.32 ms | 67.0 ms | 3379 | 1163 | 0 |
| S1 skip #2 | 8498 | 17.56 ms | 16.79 ms | 34.07 ms | 49.80 ms | 66.7 ms | 3184 | 436 | 0 |

## What S1 says

- With the GPU work removed, the frame-time distribution is the same as with full emulation.
  The p99 (about 34 ms) and p99.9 (about 50 ms) misses are **not caused by GPU emulation**.
- So a native renderer that removes the emulation would not, on its own, meet the milestone 4 target.
- The misses are at roughly 2x and 3x the 16.7 ms vblank period, so whole vblanks are being skipped.
  Candidates: guest CPU time per frame, the vsync pacing (the SDK's vsync thread sleeps 1 ms at a time),
  or presentation. Not yet separated.
- Caveat: presentation still runs (the Vulkan presenter is active), so its cost is included in S1.

## S2: where the time goes (GPU work skipped, same scene)

Method: CPU per thread from /proc during the run; a stack sampling with gdb on the busiest guest
thread; then two controlled runs with `--no-vsync`, and a read-bytes sampler from /proc/PID/io.

| Finding | Evidence |
|---|---|
| The busiest guest thread runs at 99 % of a core for the whole run | `/proc` sampling over 150 s (process: about 2.5 cores) |
| Its stack is always the same dispatch loop (`sub_829E0400` calling `sub_829E06A8`) | 12 gdb samples, identical stacks |
| With vsync off, frames drop to a median of 2.1 ms (about 117 fps) | `--no-vsync`, 60 s: mean 8.5 ms, median 2.12 ms |
| So the 16.7 ms cadence is a vblank wait, not guest work | Same scene, vsync on vs off |
| Slow frames (about 33 ms) remain with vsync off, in blocks of 5 to 10 frames | 82 blocks in the run; 1 471 frames over 20 ms, identical count in two runs |
| The slow frames coincide with heavy reads from the USB drive | Mean 2.7 MB read in the 50 ms after a slow frame, 0.2 MB at random times; 407 MB read in 60 s |
| Presentation is not blocking: the presenter uses IMMEDIATE by default | Log `presentation mode 0` = `VK_PRESENT_MODE_IMMEDIATE_KHR` (0); `vulkan_allow_present_mode_immediate` defaults to true |

## Conclusion (current)

- The guest CPU is not the limit: without vsync, a frame takes about 2 ms.
- GPU emulation is not the limit either (S1).
- The 16.7 ms cadence is the guest waiting for vblank (about 99 % of a core spent polling).
- The remaining misses (about 33 ms) line up with the guest reading assets from the USB drive on its
  main thread. This is the main lever for the 60 fps target, not a native renderer.

## S3: confirmation of the read hypothesis (negative)

| Test | Result | Verdict |
|---|---|---|
| Disk reads during the run (`read_bytes`, /proc/PID/task/TID/io) | 0 MB in 30 s, while `rchar` is 65 MB: all reads served by the page cache | USB latency is not the cause |
| Which thread reads | The process main thread (TID = PID): 62 MB. The busiest guest thread: 0 MB of its own reads | Reads are not on the frame thread |
| Presentation blocking | Mode 0 = IMMEDIATE by default | Not the cause |
| Stacks of the busiest guest thread, 40 samples, vsync off | 35/40 in `sub_829E06A8` (the callback dispatch routine), the rest in `sub_829E0400` | The guest spins in its own dispatch loop |

Conclusion: the read-causality hypothesis is not confirmed. The slow frames are probably a guest-side
wait (a busy loop on a flag set elsewhere), not I/O. Identifying the flag needs the callback target of
`sub_829E06A8` and the thread that sets it.

## S4: test 1, vblank deliveries vs slow frames

Data: `logs/s1/skip2_frames.csv` (vsync on, GPU skipped), `logs/s1/novsync_frames.csv` (vsync off).

| Run | Slow frames (> 25 ms) | Interrupts per frame, slow | Interrupts per frame, fast | Median slow / fast |
|---|---|---|---|---|
| vsync on | 436 (5.1 %) | 4 (419 of 436) | 2 (8045 of the fast frames) | 33.3 / 16.7 ms |
| vsync off | 1454 (21.6 %) | 33 to 37 | 2 to 4 | 33.2 / 1.1 ms |

SDK path (`rexglue-sdk/src/graphics/graphics_system.cpp`, lines 157 to 169):

- The vsync worker derives the interval from the refresh rate. It calls `MarkVblank()` once per missed
  interval (catch-up loop), then sleeps 1 ms.
- With vsync off, the interval is 1 ms. The 33 ms blocks therefore contain about 33 "vblanks". These runs
  do not model the 60 Hz timing of the real hardware, so the vsync-off numbers are not directly comparable.
- `MarkVblank()` dispatches the guest interrupt on the vsync host thread (`DispatchInterruptCallback`).
  The SDK has a TODO on that dispatch (line 328 to 330).

Reading: with vsync on, a slow frame receives two vblank periods' worth of interrupts. That fits a late
wake-up of the vsync thread followed by a catch-up burst. This is a correlation. It is not yet known whether
the burst causes the slow frame or results from it (the host thread is starved).

Next measurement (not done, needs approval): count catch-up bursts and the wake-up lateness of the vsync
thread, with vsync on and GPU skipped. If the bursts line up one to one with the slow frames, the vsync
thread is the candidate. Any fix must be checked against the timing the guest expects, not only the frame time.

## S5: measurement 1 with per-frame vsync counters (negative)

Build: measurement SDK with three new counters (`vsync_vblanks`, `vsync_catchup_vblanks`, `vsync_late_us`),
recorded per frame from the vsync worker (`graphics_system.cpp`). Only compiled with
`REXGLUE_ENABLE_PERF_COUNTERS`. Setting: vsync on, `--vulkan_measure_skip_gpu`, 150 s, same scene.
Data: `logs/s1/vsync_frames.csv`.

| Question | Result | Verdict |
|---|---|---|
| Does the vsync thread wake late? | Lateness per frame with a vblank: p50 0.6 ms, p99 1.7 ms, one outlier at 79 ms | No: the thread is on time |
| Does it fire catch-up bursts (several vblanks in one wake-up)? | 1 frame out of 7382 | No: the catch-up path is not the cause |
| Vblanks per frame, fast frames | 1 (5956 frames) | Normal cadence: one vblank per frame |
| Vblanks per frame, slow frames (> 25 ms) | 2 (1383 of 1424), 3 (39), 4 (2) | Slow frame = guest spans two vblank periods |
| Interrupts per frame, slow frames | 4 (1383 of 1424) | Two interrupts per vblank, as in S4 |

Reading: the vblanks arrive on time, one per wake-up. In a slow frame, the guest does not finish its frame
at vblank N and only proceeds at vblank N+1. The delay therefore sits in the guest, not in the SDK's vblank delivery.

Run-to-run variation: the slow fraction was 5.1 % in the S4 run (`skip2`) and 19.3 % here, with the same
settings. The mechanism (two vblank periods per slow frame) is the same. Treat single-run percentages with care.

Conclusion: the SDK vsync path is not the cause of the slow frames. The enquiry on the SDK side stops here.
The remaining lead is guest-side: the guest waits for a condition that is not yet met at a vblank.
Identifying that condition is the reverse-engineering work the user has not approved.

## Open points

- Confirm the causality: the read path (VFS, read sizes, caching) and whether a warm page cache removes the misses.
  Do not copy game files anywhere to test this.
- Presentation is not the blocker: mode 0 is IMMEDIATE (default), not FIFO. An earlier note in this file said FIFO; that was wrong.

## Old next measurement (superseded by S1 above)

A null graphics system that consumes the ring buffer and draws nothing (step S1 of the plan).
Its frame time is the guest CPU cost alone:
- steady under 17 ms: the emulated GPU is the bottleneck, and a native renderer is justified;
- still over 17 ms: the guest CPU or thread sync is the bottleneck, and a native renderer will not fix it.
