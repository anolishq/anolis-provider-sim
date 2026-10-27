# Device: analogsensor0 (Analog Sensor Module)

## Physical Concept

**Form Factor:** Data acquisition module mounted in instrumentation backplane

**Core Components:**

- Microcontroller with ADC sampling and calibration logic
- Analog-to-digital converter (4 channels, voltage_ch1-voltage_ch4, 0-10V range)
- Input conditioning (filtering, protection)
- Communication interface for device protocol
- No calibration storage is modelled: calibration state is held in memory and is reset when the device is initialized

## Operational Pattern

**Primary Function:** Voltage measurement with quality assessment

**Quality Management:**
Controller tracks measurement quality with a timer-driven state machine that runs only in `mode=non_interacting`; in `mode=inert` and `mode=sim` quality stays GOOD and noise stays at 10 mV std dev, even with noise injection enabled. The quality timer counts up whether or not noise injection is enabled, and only `calibrate_channel()` or `inject_noise(false)` resets it. While noise injection is enabled, quality moves from GOOD to NOISY once the quality timer passes 30 s, and from NOISY to FAULT once it passes 60 s. Each quality state sets the noise level:

- **GOOD:** Low noise, stable readings
- **NOISY:** Elevated noise (sensor degradation or interference)
- **FAULT:** High noise (200 mV std dev). Locally generated readings are clamped to 0-10V, and quality never depends on the reading, so there is no out-of-range detection

**Typical Control Flow:**

1. Host polls voltage signals and quality indicator
2. If quality degrades, host responds to degradation
3. Host calls `calibrate_channel()` to restore accuracy (requires GOOD quality)
4. Controller zeroes the drift accumulator for all channels and resets the quality timer (there are no per-channel coefficients)
5. For testing: Host calls `inject_noise()` to simulate degradation

## Protocol Interface

**Signals (read by host):**

- `voltage_ch1`, `voltage_ch2`, `voltage_ch3`, `voltage_ch4` (double, V) - Voltage readings; locally generated readings are clamped to 0-10V, physics-driven values are returned unclamped
- `sensor_quality` (string) - Quality state: "GOOD", "NOISY", or "FAULT"

**Functions (called by host):**

- `calibrate_channel(channel)` - Execute calibration (requires GOOD quality)
- `inject_noise(enabled)` - Testing: simulate signal degradation

**Preconditions:** Calibration blocked when quality is not GOOD

## Simulation Purpose

Validates floating-point signals, quality state machines, preconditions based on signal state, and gradual degradation.

Represents: **Continuous measurement with quality assessment**
