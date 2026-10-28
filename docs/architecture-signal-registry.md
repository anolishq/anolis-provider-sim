# Signal Registry Architecture

## Overview

The Signal Registry pattern provides clean separation between physics simulation and device protocol implementation.
Physics engine operates on abstract signal paths, devices expose protocol-specific interfaces.

## Key Components

### ISignalSource Interface

```cpp
class ISignalSource {
    virtual std::optional<double> read_signal(const std::string& path) = 0;
    virtual void write_signal(const std::string& path, double value) = 0;
};
```

**Contract:** Path format is `device_id/signal_id`. Returns `nullopt` if signal unavailable.

### SignalRegistry

Thread-safe cache implementing `ISignalSource`. Coordinates between physics ticker and device request handlers.

**Key members:**

- `signal_cache_` - Cached signal values (`map<string, double>`)
- `physics_driven_signals_` - Signals managed by physics (`set<string>`)
- `device_reader_` - Callback to read actual device state (for actuators)
- `registry_mutex_` - Protects all operations

### Ticker and SimulationEngine

The ticker thread in `src/devices/common/device_manager.cpp` drives a
`sim_engine::SimulationEngine` backend: `LocalEngine` (`mode=non_interacting`)
or `RemoteEngine` with `FluxGraphAdapter` (`mode=sim`). In `mode=inert`,
`NullEngine` is installed but no ticker thread starts. Engines take and return maps of signal path to
value; the ticker, not the engine, talks to the `SignalRegistry`. The
`SimulationEngine` interface, `RemoteEngine` and the FluxGraph adapter carry no
ADPP types. `LocalEngine` includes device headers that pull in the ADPP
protobuf (`protocol.pb.h`).

**Signal flow (each tick):**

1. Run device-local control loops (`update_control`)
2. Collect actuator values via `g_signal_registry->read_signal()` for actuator paths that are not physics-driven; in `mode=sim`, also add the constant ambient input when `simulation.ambient_temp_c` is set (path `simulation.ambient_signal_path`, default `environment/ambient_temp`)
3. Call `SimulationEngine::tick()`: in `mode=sim` the external FluxGraph server evaluates the signal graph (transforms, routing) and updates its models; `LocalEngine` calls each device's `update_physics`. provider-sim evaluates no graph itself.
4. Write the returned sensor values via `g_signal_registry->write_signal()`
5. Execute any commands the engine returned

### Device Integration

Devices check registry before returning internal state:

```cpp
// In read_signals():
std::string path = device_id + "/" + signal_id;
if (g_signal_registry && g_signal_registry->is_physics_driven(path)) {
    auto val = g_signal_registry->read_signal(path);
    if (val) return *val;
}
return internal_state[signal_id];
```

## Thread Safety

**Lock policy:**

- Ticker: each registry call (`is_physics_driven`, `read_signal`, `write_signal`) acquires and releases the registry mutex itself; the mutex is not held across a tick
- Device handlers: Acquire registry mutex → read values → release
- No lock held during physics model evaluation (only during registry I/O)

**Deadlock prevention:** Locks can nest: analogsensor `read_signals()` takes the
registry mutex while holding its device `g_state_mutex`. The order is always
device state mutex, then registry mutex, and `read_signal()` releases
`registry_mutex_` before it calls `device_reader_`.

## Signal Routing

In `mode=sim`, routing is declarative in the FluxGraph graph file referenced by
`simulation.physics_config` (provider config only carries the path).

<!-- validate: none -->

```yaml
edges:
  - source: device_a/actuator # Read from device
    target: model_b/input # Write to model
    transform: { type: linear, params: { scale: 100.0, offset: 0.0 } }

  - source: model_b/output # Read from model
    target: device_c/sensor # Write to device (registry cache)
    transform: { type: first_order_lag, params: { tau_s: 2.0 } }
```

FluxGraph processes this schema; provider-sim does not define or validate it.
See FluxGraph docs for authoritative schema details (`docs/schema-yaml.md` in
the FluxGraph repository).

Device reads from the registry return physics-computed values when the signal
is marked physics-driven.

## Extension Points

**Adding graph transforms/models/rules:** update the FluxGraph graph and
FluxGraph runtime as needed.

**Adding devices:** include registry checks in device `read_signals()`
implementations.

## Benefits

- **Protocol independence** - The `SimulationEngine` interface and the FluxGraph remote path have no ADPP dependencies; `LocalEngine` depends on device headers that include the ADPP protobuf
- **Reusability** - Backends are pluggable through the `SimulationEngine` and `ProtocolAdapter` interfaces
- **Declarative config** - The graph YAML declares the FluxGraph routing; device-local control loops and the set of actuator signals are defined in C++
- **Testability** - The engine boundary is `SimulationEngine`; `NullEngine` is an inert implementation

## Design Trade-offs

**Cost:** Additional indirection layer, mutex overhead on signal access

**Benefit:** Clean architecture boundaries, long-term maintainability, future-proof for signal recording/replay features
