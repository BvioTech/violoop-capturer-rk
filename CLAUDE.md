# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

C application that captures video frames from V4L2 devices on Rockchip SoCs (RK1106/RK3576), hardware-encodes them to H.264 using the Rockchip MPI (VENC), and streams encoded frames out over a Unix domain socket.

## Build

Must be cross-compiled for aarch64 using a Docker container. The toolchain is external (from `BvioTech/violoop-builder`).

```bash
# Build (defaults to rv1106 if no arg)
./scripts/build.sh [rv1106|rk3576]

# What happens inside Docker:
# cmake -DCMAKE_TOOLCHAIN_FILE=/workspace/toolchain.cmake ..
# make -j$(nproc)
# Output binary: dist/capture
```

Build tests (inside Docker or with toolchain configured):
```bash
cd dist && cmake -DBUILD_TEST=ON -DCMAKE_TOOLCHAIN_FILE=/workspace/toolchain.cmake .. && make
```

Tests are hardware-specific (`hw_*`) or unit tests (`unit_*`). Unit tests: `unit_args_test`, `unit_utils_test`, `unit_socket_test`, `unit_video_test`.

## Versioning & Release

- Version stored in plain `version` file (e.g. `0.1.7`)
- `scripts/version.sh [major|minor|patch]` bumps version, commits, tags, and pushes
- Pushing a `v*` tag triggers the GitHub Actions release workflow

## Architecture

The binary has two main threads managed from `main.c`:

- **Input thread** (`input_loop`): reads raw NV12 frames from a V4L2 device and feeds them into the Rockchip VENC hardware encoder
- **Output thread** (`output_loop`): reads H.264 encoded packets from VENC and sends them via a Unix socket using a framing protocol

Key modules in `src/`:
- `video.c/h` — V4L2 capture setup and Rockchip VENC init/encode/teardown. Uses DMA buffer sharing between V4L2 and VENC.
- `socket.c/h` — Unix domain socket creation and frame transmission protocol
- `args.c/h` — CLI argument parsing (width, height, input device, output socket path, bitrate, GOP)
- `utils.c/h` — Timing helpers and pixel format size calculations

`include/` contains Rockchip MPI SDK headers (rk_mpi_*, rk_comm_*) — these are vendor-provided and should not be modified.

## CLI Arguments

The `capture` binary accepts: width, height, input device path (e.g. `/dev/video0`), output socket path, bit rate, and GOP size.
