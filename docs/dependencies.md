# Dependency, Build, and CI Governance

This document defines provider-sim policy for dependencies, CI lane tiers, presets, and cross-repo compatibility.

## vcpkg Policy

1. `vcpkg-configuration.json` is the baseline source of truth.
2. Lockfile pinning is not planned: the baseline SHA is the lock (org decision in anolishq/.github#93).
3. Determinism is enforced via baseline pinning plus reviewed `vcpkg.json` changes.
4. Windows builds use pinned MSVC/vcpkg ABI settings via the `x64-windows-v145` overlay triplet (`triplets/x64-windows-v145.cmake`).

## Python Dependency Policy

1. CI installs from `requirements-lock.txt`, not floating `requirements.txt`.
2. `requirements.txt` defines ranges for maintainers; `requirements-lock.txt` is the execution source in CI.
3. Lockfile updates must be committed in the same PR as dependency range changes.

## Versioning Policy

- `anolis-provider-sim` follows independent SemVer (`MAJOR.MINOR.PATCH`).
- Provider public behavior/build-surface changes require version-bump decision and release note.
- The runtime records the provider-sim release it is tested against as the `provider_sim` pin in `anolis/.ci/dependency-pins.yml`.

## CI Lane Tiers

- **Required** (through the `ok` check, on every PR that changes code; docs-only PRs skip them):
  - Linux release (`fluxgraph OFF`)
  - Windows release (`fluxgraph OFF`)
  - Linux release (`fluxgraph ON`)
- **Nightly/on-demand** (skipped on PRs and pushes):
  - Windows release (`fluxgraph ON`, build validation)
- **Optional** (on PRs and pushes to `main`, not required):
  - sanitizer lanes (TSAN and Valgrind, `hardening.yml`)

## Dual-Run Policy

When a CI path is migrated (for example, to presets):

- run legacy and new paths in parallel,
- minimum 5 consecutive green runs,
- preferred 10 runs before legacy path removal.

## Preset Baseline and Exception Policy

Baseline names:

- `dev-debug`, `dev-release`, `ci-linux-release`, `ci-windows-release`
- feature extension lanes: `ci-linux-release-fluxgraph`, `ci-windows-release-fluxgraph`

Rules:

1. CI jobs should call presets directly.
2. CI-only deviations must be explicit and documented.
3. Feature-specific extension presets are allowed when documented.
4. Every preset must have an active owner/use-case (CI lane, script default, or documented workflow); remove unreferenced presets.

Contributor quick check:

```bash
cmake --list-presets
ctest --list-presets
```

On Windows local development, use `dev-windows-*` presets.

## Cross-Repo Coupling Policy

- `anolis-protocol` is consumed through `anolis-provider-sdk`: `CMakeLists.txt` pins the SDK release tarball with
  FetchContent, and the SDK declares the `anolis-protocol` release pin. No submodule or sibling checkout is required.
- FluxGraph integration remains explicit opt-in (`ENABLE_FLUXGRAPH`, `FLUXGRAPH_DIR`).
- Configure resolves dependencies over the network: vcpkg manifest packages and pinned, SHA256-checked FetchContent tarballs (the SDK, which fetches `anolis-protocol`, and the FluxGraph proto when `ENABLE_FLUXGRAPH` is on). `FETCHCONTENT_SOURCE_DIR_<NAME>` and `FLUXGRAPH_DIR` point the tarballs at local source trees.
- FluxGraph CI lanes use the v1.0.6 release: the Linux lane checks out the `ref` in `.ci/dependency-pins.yml`, and the Windows lane uses the FetchContent tarball in `CMakeLists.txt`.
- The Linux FluxGraph lane logs the resolved FluxGraph commit.
- The Linux lane's `ref` is a release tag, not an immutable commit SHA; revisit SHA pinning before release hardening.
