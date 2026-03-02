# Provider-Sim Examples

This directory contains configuration-first examples for each simulation mode.

## Example Directories

- `inert_mode/`: `mode=inert` (no ticker, no automatic simulation updates)
- `non_interacting_mode/`: `mode=non_interacting` (built-in local first-order dynamics)
- `sim_mode/`: `mode=sim` (external FluxGraph integration)

## Quick Start

Build provider-sim first:

Linux/macOS:

```bash
bash ./scripts/build.sh --preset dev-release
```

Windows:

```powershell
.\scripts\build.ps1 -Preset dev-windows-release
```

Run each mode with its example config:

Linux/macOS:

```bash
bash ./scripts/run_local.sh --preset dev-release -- --config examples/inert_mode/provider.yaml
bash ./scripts/run_local.sh --preset dev-release -- --config examples/non_interacting_mode/provider.yaml
```

Windows:

```powershell
.\scripts\run_local.ps1 -Preset dev-windows-release -- --config examples/inert_mode/provider.yaml
.\scripts\run_local.ps1 -Preset dev-windows-release -- --config examples/non_interacting_mode/provider.yaml
```

For `sim_mode`, build FluxGraph-enabled provider and pass `--sim-server`.

## FluxGraph (`sim_mode`)

Linux/macOS:

```bash
bash ./scripts/build.sh --preset ci-linux-release-fluxgraph -- -DFLUXGRAPH_DIR=../fluxgraph
bash ./scripts/run_local.sh --preset ci-linux-release-fluxgraph -- --config examples/sim_mode/provider.yaml --sim-server localhost:50051
```

Windows:

```powershell
.\scripts\build.ps1 -Preset dev-windows-release-fluxgraph -- -DFLUXGRAPH_DIR=..\fluxgraph
.\scripts\run_local.ps1 -Preset dev-windows-release-fluxgraph -- --config examples/sim_mode/provider.yaml --sim-server localhost:50051
```

## Notes

- Example Python scripts in each folder are scenario demos.
- CI-grade integration validation lives under `tests/`.
