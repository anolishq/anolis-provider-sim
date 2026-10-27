# Device: relayio0 (Relay/IO Module)

## Physical Concept

**Form Factor:** Basic I/O module mounted in instrumentation backplane

**Core Components:**

- Microcontroller for I/O scanning and communication
- Relay output drivers (4 channels) - switches for external loads
- Digital input buffers (4 channels) - isolated monitoring inputs
- Communication interface for device protocol
- No persistent storage (stateless)

## Operational Pattern

**Primary Function:** General-purpose switching and digital signal monitoring

**Typical Control Flow:**

1. Host polls signals to read relay states and GPIO input levels
2. Host calls relay functions to control external loads
3. Relay state changes immediately, reflected in next signal poll
4. In `non_interacting` mode, GPIO inputs are recomputed from the relay states on each tick: inputs 1 and 3 equal relays 1 and 3, and inputs 2 and 4 are the inverse of relays 2 and 4. In `inert` mode they stay false. In `sim` mode an input reads from the external simulation when it drives that signal, and is false otherwise
5. Host detects GPIO changes via periodic polling

**Behavior:** Simple stateless control - no internal state machine or preconditions

## Protocol Interface

**Signals (read by host):**

- `relay_ch1_state`, `relay_ch2_state`, `relay_ch3_state`, `relay_ch4_state` (bool) - Relay states
- `gpio_input_1`, `gpio_input_2`, `gpio_input_3`, `gpio_input_4` (bool) - Digital input levels

**Functions (called by host):**

- `set_relay_ch1(enabled)` - Control relay 1
- `set_relay_ch2(enabled)` - Control relay 2
- `set_relay_ch3(enabled)` - Control relay 3
- `set_relay_ch4(enabled)` - Control relay 4

**Preconditions:** None (always callable)

## Simulation Purpose

Validates simple stateless control, boolean signal handling, and immediate command execution.

Represents: **Basic digital I/O** (simplest device pattern)
