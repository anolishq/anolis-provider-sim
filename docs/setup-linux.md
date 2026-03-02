# anolis-provider-sim Linux Setup

Build and validate `anolis-provider-sim` on Linux using the repository presets and wrapper scripts.

## Prerequisites

- Ubuntu 22.04+ / Debian 12+ (or equivalent modern distro)
- GCC/Clang with C++17 support
- CMake >= 3.20
- Ninja
- Python 3.12+
- Git

## 1) Install system dependencies

Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build git curl zip unzip tar python3 python3-pip
```

## 2) Install and configure vcpkg

```bash
git clone https://github.com/microsoft/vcpkg.git "$HOME/tools/vcpkg"
"$HOME/tools/vcpkg/bootstrap-vcpkg.sh"
```

Set `VCPKG_ROOT` in your shell profile:

```bash
echo 'export VCPKG_ROOT="$HOME/tools/vcpkg"' >> ~/.bashrc
source ~/.bashrc
```

Verify:

```bash
echo "$VCPKG_ROOT"
```

## 3) Clone repository

```bash
git clone https://github.com/FEASTorg/anolis-provider-sim.git
cd anolis-provider-sim
git submodule update --init --recursive
```

## 4) Configure and build

```bash
bash ./scripts/build.sh --preset dev-release
```

This configures into `build/dev-release` and builds `anolis-provider-sim` there.

## 5) Run locally (requires `--config`)

```bash
bash ./scripts/run_local.sh --preset dev-release -- --config config/provider-sim.yaml
```

Notes:

- `--config` is required.
- Provider transport is stdio framed ADPP, so it waits for client frames on stdin.

## 6) Validate with tests

Smoke test:

```bash
bash ./scripts/test.sh --preset dev-release --suite smoke
```

Full provider baseline suite:

```bash
bash ./scripts/test.sh --preset dev-release --suite all
```

## Optional: FluxGraph-enabled build (`sim` mode)

```bash
bash ./scripts/build.sh --preset ci-linux-release-fluxgraph -- -DFLUXGRAPH_DIR=../fluxgraph
bash ./scripts/test.sh --preset ci-linux-release-fluxgraph --suite fluxgraph
```

## Troubleshooting

`Could NOT find Protobuf` or other package errors:

- Confirm `VCPKG_ROOT` is set.
- Re-run configure via `bash ./scripts/build.sh --preset dev-release --clean`.

`mode=sim requires FluxGraph support`:

- Build with a FluxGraph-enabled preset (`*-fluxgraph`).

`FATAL: --config argument is required`:

- Start provider with `-- --config <path>` when using `scripts/run_local.sh`.
