# Deploying CIVI on QNX SDP 8.0 / Snapdragon cockpit

See `docs/QNX_QUALCOMM_BRINGUP.md` for the full bring-up guide. Files here:

| File | Purpose |
|---|---|
| `civi-startup.sh` | Start order: telltales → cluster/HUD → infotainment, with security types and priorities |
| `civi.build` | IFS build-file fragment for the boot-critical pieces |
| `secpol/civi.secpol` | Least-privilege security policy sketch (validate with `secpolcompile`) |
