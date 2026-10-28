# anolis-provider-sim

[![CI](https://github.com/anolishq/anolis-provider-sim/actions/workflows/ci.yml/badge.svg)](https://github.com/anolishq/anolis-provider-sim/actions/workflows/ci.yml)

`anolis-provider-sim` is an ADPP v1 provider that serves simulated devices, so
the Anolis runtime can be run and tested without hardware. The runtime starts it
as a subprocess and talks to it over stdio with `uint32_le` length-prefixed
frames.

## Devices

A config file declares which devices exist. Each device has an `id` and a
`type`:

| Type           | Example ID      | Signals                                                                                        | Functions                                                          |
| -------------- | --------------- | ---------------------------------------------------------------------------------------------- | ------------------------------------------------------------------ |
| `tempctl`      | `tempctl0`      | `tc1_temp`, `tc2_temp`, `relay1_state`, `relay2_state`, `control_mode`, `setpoint`             | `set_mode`, `set_setpoint`, `set_relay`                            |
| `motorctl`     | `motorctl0`     | `motor1_speed`, `motor2_speed`, `motor1_duty`, `motor2_duty`                                   | `set_motor_duty`                                                   |
| `relayio`      | `relayio0`      | `relay_ch1_state` to `relay_ch4_state`, `gpio_input_1` to `gpio_input_4`                       | `set_relay_ch1` to `set_relay_ch4`                                 |
| `analogsensor` | `analogsensor0` | `voltage_ch1` to `voltage_ch4`, `sensor_quality`                                               | `calibrate_channel`, `inject_noise`                                |

The provider also always serves a `chaos_control` device for
[fault injection](#fault-injection). A configured device cannot use that ID.

Each device type has a page in [docs/devices/](docs/devices/) describing the
physical card it models.

## Download

Release builds are published for Linux x86_64 and arm64. This fetches the
latest one:

```bash
ARCH=x86_64   # or arm64
VERSION=$(curl -fsSL https://api.github.com/repos/anolishq/anolis-provider-sim/releases/latest | grep '"tag_name"' | sed 's/.*"v\([^"]*\)".*/\1/')
curl -fsSLO "https://github.com/anolishq/anolis-provider-sim/releases/download/v${VERSION}/anolis-provider-sim-${VERSION}-linux-${ARCH}.tar.gz"
curl -fsSL -o sim.SHA256SUMS "https://github.com/anolishq/anolis-provider-sim/releases/download/v${VERSION}/SHA256SUMS"
sha256sum -c --ignore-missing sim.SHA256SUMS
tar -xzf "anolis-provider-sim-${VERSION}-linux-${ARCH}.tar.gz"
```

The binary is at `./bin/anolis-provider-sim`.

## Use it with the runtime

Write a provider config, for example `provider-sim.yaml`:

<!-- validate: none -->

```yaml
devices:
  - id: tempctl0
    type: tempctl
    initial_temp: 25.0
  - id: motorctl0
    type: motorctl
    max_speed: 3000.0

simulation:
  mode: non_interacting
  tick_rate_hz: 10.0
```

Then point the runtime's `providers` section at it:

<!-- validate: runtime-config -->

```yaml
providers:
  - id: sim0
    command: ./bin/anolis-provider-sim
    args: ["--config", "provider-sim.yaml"]
```

[Try Anolis with the simulator](https://github.com/anolishq/anolis/blob/main/docs/quickstart-simulator.md)
walks through downloading both, running them, and reading and controlling a
device over the HTTP API.

## Command-line flags

| Flag                       | Effect                                                                                     |
| -------------------------- | ------------------------------------------------------------------------------------------ |
| `--config <path>`          | Load the config and serve ADPP on stdio. Required unless one of the next three is given.   |
| `--check-config <path>`    | Load the config and initialize the devices, log the result to stderr, and exit 0 if valid. |
| `--config-schema`          | Print the config schema envelope to stdout and exit, without reading a config.             |
| `--version`                | Print the version and exit.                                                                |
| `--sim-server <host:port>` | FluxGraph server address. Required for `mode: sim`; ignored with a warning in other modes. |
| `--crash-after <seconds>`  | Exit with code 42 after this many seconds, to simulate a provider crash.                   |

An unknown or malformed argument, or a missing `--config`, exits 1.

## Configuration

[docs/configuration.md](docs/configuration.md) is the full reference for the
config file. The parts most people need:

- `simulation.mode` is required and picks the engine:
  - `inert`: no physics ticker. Values change only through function calls,
    although analog sensor reads still carry simulated noise.
  - `non_interacting`: built-in first-order device models, ticked at
    `tick_rate_hz`.
  - `sim`: an external FluxGraph simulation. Needs a FluxGraph build,
    `--sim-server`, `tick_rate_hz`, and `physics_config` (a path relative to
    the config file).
- `startup_policy` is `strict` (default: stop on the first device that fails to
  initialize) or `degraded` (continue with the devices that did).

Example configs in [config/](config/):

- `provider-sim.yaml`: `tempctl0`, `motorctl0`, `relayio0` and `analogsensor0`.
- `multi-tempctl.yaml`: several devices of the same type.
- `minimal.yaml`: one device in `inert` mode.
- `provider-chamber.yaml`, `provider-extruder.yaml` and
  `multi-provider-extrusion.yaml`: the FluxGraph demos, described in
  [docs/demos/demo-chamber.md](docs/demos/demo-chamber.md) and
  [docs/demos/demo-reactor.md](docs/demos/demo-reactor.md).

[examples/](examples/README.md) has a runnable config and Python script for
each simulation mode. [docs/architecture-signal-registry.md](docs/architecture-signal-registry.md)
explains how the physics engine and the devices share signal values.

## Fault injection

Call functions on the `chaos_control` device to inject failures.

| ID | Function                    | Arguments                                         | Effect                                                                                                                                                                                 |
| -- | --------------------------- | ------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 1  | `inject_device_unavailable` | `device_id`, `duration_ms`                        | The device is left out of `ListDevices`; `DescribeDevice` and `ReadSignals` return `NOT_FOUND`; `CallFunction` returns `INVALID_ARGUMENT`.                                             |
| 2  | `inject_signal_fault`       | `device_id`, `signal_id`, `duration_ms`           | The signal reports `FAULT` quality. Its value is still read live.                                                                                                                      |
| 3  | `inject_call_latency`       | `device_id`, `latency_ms`                         | Every `CallFunction` on the device is delayed by `latency_ms`.                                                                                                                         |
| 4  | `inject_call_failure`       | `device_id`, `function_id`, `failure_rate`        | Calls to that function return `INVALID_ARGUMENT` with probability `failure_rate`.                                                                                                      |
| 5  | `clear_faults`              | none                                              | Removes every active fault.                                                                                                                                                            |
| 6  | `inject_device_unreachable` | `device_id`, `duration_ms`                        | `ReadSignals` and `CallFunction` return `UNAVAILABLE`, like a board gone dark on its bus. The device stays listed and `DescribeDevice` still answers.                                  |

Argument rules:

- `device_id`, `signal_id` and `function_id` are strings. `function_id` is the
  target function's numeric ID as a string, for example `"2"`, and must be 1 or
  more.
- `duration_ms` is an `int64` above 0, and `latency_ms` an `int64` of 0 or
  more.
- `failure_rate` is a `double` from 0.0 to 1.0.

Faults with a `duration_ms` clear themselves when it expires. Latency and call
failure faults last until `clear_faults`.

To test a client's handling of a lost device, use `inject_device_unreachable`,
not `inject_device_unavailable`: `NOT_FOUND` is not a transport failure.

Through the runtime's HTTP API, this makes `tempctl0`'s `set_setpoint`
(function 2) fail half the time:

```bash
curl -s -X POST http://127.0.0.1:8080/v0/call \
  -H 'Content-Type: application/json' \
  -d '{"provider_id": "sim0", "device_id": "chaos_control", "function_id": 4,
       "args": {"device_id": {"type": "string", "string": "tempctl0"},
                "function_id": {"type": "string", "string": "2"},
                "failure_rate": {"type": "double", "double": 0.5}}}'
```

## Safe initial state

Every device starts inactive, as the
[provider safe initialization contract](https://github.com/anolishq/anolis/blob/main/docs/providers.md#safe-initialization-contract)
requires:

- `tempctl`: both relays off, `control_mode` `open`.
- `motorctl`: both duties and speeds 0.
- `relayio`: all four relays off.
- `analogsensor`: has no outputs; its functions change only simulated sensor
  state.
- `chaos_control`: no faults active.

## Logging

Diagnostics go to stderr as:

```text
[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] [Component] message
```

Set the level with `ANOLIS_PROVIDER_SIM_LOG_LEVEL`: `debug`, `info` (default),
`warn`, `error` or `none`, in any case. An invalid value logs one warning and
uses `info`.

stdout carries only ADPP frames. Never write diagnostics to it.

## Build from source

You need CMake 3.20 or later, Ninja, vcpkg (with `VCPKG_ROOT` set), a C++20
compiler and Python 3.12. The Windows presets also need Visual Studio 2026 and
CMake 4.2 or later. Full setup is in [docs/setup-linux.md](docs/setup-linux.md)
and [docs/setup-win.md](docs/setup-win.md).

Linux:

```bash
git clone https://github.com/anolishq/anolis-provider-sim.git
cd anolis-provider-sim
cmake --preset dev-release
cmake --build --preset dev-release --parallel
./build/dev-release/anolis-provider-sim --version
```

Windows (PowerShell):

```powershell
git clone https://github.com/anolishq/anolis-provider-sim.git
Set-Location anolis-provider-sim
cmake --preset dev-windows-release
cmake --build --preset dev-windows-release --parallel
.\build\dev-windows-release\Release\anolis-provider-sim.exe --version
```

`cmake --list-presets` lists the configure presets; `cmake --list-presets=all`
also lists the build and test presets.

### FluxGraph

`mode: sim` needs a build with `ENABLE_FLUXGRAPH=ON`, which the
`*-fluxgraph` presets set. A build without it rejects `mode: sim` at startup.

```bash
cmake --preset dev-release-fluxgraph
cmake --build --preset dev-release-fluxgraph --parallel
```

The build fetches the FluxGraph proto by default. To use a local FluxGraph
checkout instead, add `-DFLUXGRAPH_DIR=../fluxgraph` to the configure step.

### Tests

Run from the repository root:

```bash
ctest --preset dev-release              # unit and provider integration tests
ctest --preset dev-release -L unit      # unit tests only
ctest --preset dev-release -L provider  # provider integration tests only
ctest --preset dev-release-fluxgraph -L fluxgraph  # FluxGraph tests; FluxGraph build only
```

On Windows use the `dev-windows-release` preset. ThreadSanitizer builds need
extra environment variables at run time; see
[docs/setup-linux.md](docs/setup-linux.md).

[docs/dependencies.md](docs/dependencies.md) covers dependency, versioning and
CI policy. For local API docs, run `doxygen docs/Doxyfile` from the repository
root; output goes to `build/docs/doxygen/html/`.
