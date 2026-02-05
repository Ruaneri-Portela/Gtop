# GTOP - GPU Monitor for macOS

**GTOP** is a command-line GPU monitoring tool for macOS that uses IOKit APIs (IOAccelerator, IOReport, and IOHID) to display real-time GPU usage, power consumption, and per-process GPU activity in a `top`-like interface.[1][2][3]

## Overview

GTOP provides detailed GPU metrics by leveraging:

- **IOKit/IOAccelerator** for GPU discovery and `PerformanceStatistics` dictionary reading (utilization, memory, scenes, recoveries, etc.).[4][5]
- **IOReport** (via private `libIOReport.dylib`) for:
  - GPU performance states (P-states) → weighted average frequency and voltage.[6][1]
  - Energy consumption (mJ/µJ/nJ → mW/W power calculation).[1][6]
  - Utilization stats ("GPU Stats", "Energy Model" groups).[7][1]
- **IOHID** fallback for GPU temperature sensors when IOReport doesn't provide it.[2][6]
- Per-process tracking: GPU time deltas to compute usage %, APIs (e.g., Metal), sorted by highest usage.

Supports multiple GPUs, discovered via `IOAccelerator` class iteration.[2][4]

## Features

- Real-time refresh (1s interval, ANSI clear screen).
- Global stats per GPU: device/renderer/tiler utilization, system/driver/PB memory, tiled scenes, splits/recoveries, freq (MHz), voltage (V), power (W), temp (°C).
- Process table: PID, GPU %, APIs, process name (sorted by usage).
- Handles Apple Silicon natively; partial support for older Intel/dedicated GPUs.

## Requirements

- macOS 12+ (optimized for Apple Silicon; some metrics unavailable on older Intel GPUs).[4][2]
- Xcode/Command Line Tools with C++17+ support.
- Frameworks: CoreFoundation, IOKit, IOHIDEventSystem.

## Build

```bash
clang++ -std=c++17 \
  -framework CoreFoundation \
  -framework IOKit \
  -framework IOSurface \
  -o gtop \
  main.cpp gpu.cpp iokit.cpp
```

Adjust source files as needed.

## Usage

```bash
./gtop
```

**Output example** (per GPU):
```
GPU #0
  Name: Apple M4 Pro GPU (16 cores)

  Stats: device=45%  renderer=52%  tiler=38%  alloc_sys=128.5MB  in_use_sys=245.3MB  in_use_drv=89.2MB  pb_alloc=512MB
           tiled=1567MB  splits=2345  recov=0  freq=2450MHz  volt=1.05V  watts=23.4W  temp=67.2C

  Processes:
    PID     GPU %     API            NAME
     1234   34.2%     Metal         Blender
     5678   12.1%     Metal,OpenCL  Safari
```

Press `Ctrl+C` to exit.

## Limitations

- **Private APIs**: IOReport is undocumented and may change across macOS versions (functions loaded via `dlopen`).[7][1]
- **GPU Support**: Not all metrics (e.g., temp, certain channels) available on every GPU (esp. older Intel/dedicated).[5][4]
- **Permissions**: Runs as user; no sudo needed, but some IOHID sensors may require entitlements in sandboxed apps.

## License

Apache License 2.0. See `LICENSE` or source headers.[1]

**Author**: Ruaneri Portela (ruaneriportela@outlook.com)  
**Copyright**: 2026  

For issues or contributions: fork and PR on GitHub (if published).

Fontes
[1] WIP decompile of private IOKit library, IOReport https://github.com/dehydratedpotato/IOReport_decompile
[2] A tool to get GPU information from Intel and Apple silicon based macs https://iosexample.com/a-tool-to-get-gpu-information-from-intel-and-apple-silicon-based-macs/
[3] GitHub - palle-k/SwiftyGPU: A simple command line GPU usage monitor for macOS https://github.com/palle-k/SwiftyGPU
[4] Programmatically get GPU percent usage in OS X https://stackoverflow.com/questions/10110658/programmatically-get-gpu-percent-usage-in-os-x
[5] What is the meaning of the keys in the GPU[@"PerformanceStatistics"]? https://stackoverflow.com/questions/56361500/what-is-the-meaning-of-the-keys-in-the-gpuperformancestatistics
[6] [RFC] Programmatic Energy Consumption Measurement ... https://github.com/ml-energy/zeus/issues/159
[7] GitHub - freedomtan/test-ioreport: A little program to show how to dump all the IOKit IOReport information https://github.com/freedomtan/test-ioreport
