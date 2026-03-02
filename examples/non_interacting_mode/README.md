# Non-Interacting Mode Example

`non_interacting` mode runs built-in local first-order dynamics with no external simulator.

- Ticker thread runs at `simulation.tick_rate_hz`
- Device models evolve independently
- No cross-device coupling

## Config

- Provider config: `examples/non_interacting_mode/provider.yaml`

## Run

Linux/macOS:

```bash
bash ./scripts/build.sh --preset dev-release
bash ./scripts/run_local.sh --preset dev-release -- --config examples/non_interacting_mode/provider.yaml
```

Windows:

```powershell
.\scripts\build.ps1 -Preset dev-windows-release
.\scripts\run_local.ps1 -Preset dev-windows-release -- --config examples/non_interacting_mode/provider.yaml
```

## Related

- [Inert mode](../inert_mode/)
- [Sim mode](../sim_mode/)
