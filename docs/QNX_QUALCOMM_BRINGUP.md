# Bring-up on QNX SDP 8.0 and a Snapdragon cockpit platform

Target: Qualcomm Snapdragon Cockpit reference hardware (SA8295P "Gen 4", or SA8775P /
SA8255P class), or an equivalent automotive SoC with a QNX 8.0 BSP.

> **Confirm before committing a demo date:** the exact QNX 8.0 BSP release for your
> board, whether the hypervisor build is part of it, and the Qt-for-QNX version
> licensed for SDP 8.0. Supported combinations change with each BSP drop.

## 1. Prerequisites
| Item | Notes |
|---|---|
| QNX SDP 8.0 + licence | `qnxsdp-env.sh` sets `QNX_HOST`, `QNX_TARGET` |
| Qualcomm QNX BSP for the board | Screen display driver, Adreno GLES, audio, early RVC (AIS/qcarcam) |
| Qt for QNX (commercial) | Qt 6.8 LTS or later built for SDP 8.0, aarch64le; modules: Core, Gui, Qml, Quick (+ Multimedia for passenger video) |
| Python 3 on target (optional) | to run `tools/vsim` on the bench without a vehicle |
| Fonts | OEM brand face with full French coverage (demo uses Inter) |

## 2. Cross-build
```bash
source ~/qnx800/qnxsdp-env.sh
cmake -B build-qnx -S . \
  -DCMAKE_TOOLCHAIN_FILE=cmake/qnx-sdp8-aarch64le.cmake \
  -DCMAKE_PREFIX_PATH=$HOME/Qt/6.8.3/qnx800-aarch64le \
  -DCMAKE_INSTALL_PREFIX=/opt/civi -DCIVI_BUILD_TESTS=OFF
cmake --build build-qnx -j
DESTDIR=$PWD/stage cmake --install build-qnx
```
`civi-telltaled` links only `libscreen`; it has no Qt dependency.

## 3. Display mapping
Each display role maps to a Screen display index in `config/civi-config.json`
(`displays.<role>.index`). Read the board's indices with `screeninfo` or from the BSP
`graphics.conf`, then set them. Recommended on Snapdragon:

* Give the telltale window its own display-controller pipeline (`SCREEN_PROPERTY_PIPELINE`)
  so it composites independently of the cluster HMI layers.
* Set `cluster.telltales_from_safe_renderer: true` on target so the QML cluster leaves
  the telltale strip empty.
* For the HUD, black is transparent on the combiner; confirm the HUD display's colour
  format with the HUD supplier.

## 4. Install and start
1. Copy `stage/opt/civi` to the target's persistent filesystem.
2. Merge `deploy/qnx/civi.build` into the BSP build file (telltales + config in the IFS).
3. Create users `civi_hmi`, `civi_tt`; compile and load `deploy/qnx/secpol/civi.secpol`.
4. Start `deploy/qnx/civi-startup.sh` after `screen` and `pps` (from the BSP start-up).

## 5. Bench demo without a vehicle
```bash
export CIVI_PPS_ROOT=/pps/civi
python3 /opt/civi/tools/vsim/civi_vsim.py --scenario full-demo --loop
```
Each scenario drives the PPS objects the real vehicle-services server and TCU will own.

## 6. What to measure on the first board
| Measurement | Requirement | How |
|---|---|---|
| Power-on → telltales lit, at −40 °C | PLT-003 | Camera on cluster + `slog2info` timestamps |
| Power-on → first HMI frame per display | PLT-005 | `QSG_INFO=1`, Screen stats |
| Takeover request → first presented frame | AVR-001 | `hmi/metrics_cluster` and `hmi/metrics_hud` PPS objects; scenario `takeover` |
| Kill `civi-center` under load → telltales unaffected | PLT-002 | `slay civi-center` while running `winter-drive` |
| CPU/GPU/memory headroom with all displays animating | PLT-007 | `hogs`, `pidin`, Adreno counters |
