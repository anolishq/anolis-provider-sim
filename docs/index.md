# Anolis' Provider Simulator

`anolis-provider-sim` is an ADPP provider that serves simulated devices, so the runtime and new providers can be developed and tested without hardware.
The repository [README](https://github.com/anolishq/anolis-provider-sim/blob/main/README.md) covers the device roster, protocol and usage.
To write a provider of your own, start with the SDK's [Writing a provider](https://github.com/anolishq/anolis-provider-sdk/blob/main/docs/writing-a-provider.md); the simulator's [`src/devices/`](https://github.com/anolishq/anolis-provider-sim/tree/main/src/devices) is a worked example built on it.

## Pages

- [Overview](overview.md): what the simulator is for.
- [Linux setup](setup-linux.md): build and test on Linux with the repository presets.
- [Windows setup](setup-win.md): build and test on Windows with the Visual Studio presets.
- [Configuration](configuration.md): the YAML config file read at startup, including the simulation modes.
- [Signal registry](architecture-signal-registry.md): how the physics engine and the devices exchange signals.
- [Multi-provider scenario](multi-provider.md): two providers against one FluxGraph server, and the coupling test.
- [Dependencies and CI](dependencies.md): vcpkg, Python, versioning, CI lane and preset policy.

Demos:

- [Chamber](demos/demo-chamber.md): one provider (`tempctl0`) with external FluxGraph simulation.
- [Multi-provider coupling](demos/demo-reactor.md): two providers sharing one FluxGraph graph.

Devices, by the ids that `config/provider-sim.yaml` gives them (ids come from the config file, so another config can name them differently):

- [tempctl0](devices/device_tempctl0.md): temperature control card.
- [motorctl0](devices/device_motorctl0.md): motor control card.
- [relayio0](devices/device_relayio0.md): relay and I/O module.
- [analogsensor0](devices/device_analogsensor0.md): analog sensor module.

The provider also always serves `chaos_control`, a fault-injection device, after the configured ones.
Its id is reserved, so a config file cannot use it.
It is documented in the README [Fault Injection API](https://github.com/anolishq/anolis-provider-sim/blob/main/README.md#fault-injection-api) section.

## Local API docs

Run `doxygen docs/Doxyfile` from the repository root.
The HTML goes to `build/docs/doxygen/html/`, which is not committed.
