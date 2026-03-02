# Inert Mode Example

`inert` mode is the no-simulation path:

- No ticker thread
- No automatic physics updates
- Device state only changes through explicit calls

## Config

- Provider config: `examples/inert_mode/provider.yaml`

## Run

Linux/macOS:

```bash
bash ./scripts/build.sh --preset dev-release
bash ./scripts/run_local.sh --preset dev-release -- --config examples/inert_mode/provider.yaml
```

Windows:

```powershell
.\scripts\build.ps1 -Preset dev-windows-release
.\scripts\run_local.ps1 -Preset dev-windows-release -- --config examples/inert_mode/provider.yaml
```

## Related

- [Non-interacting mode](../non_interacting_mode/)
- [Sim mode](../sim_mode/)
