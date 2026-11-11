#!/usr/bin/env bash
# Desktop demo: simulator + all four displays as windows.
#   tools/run-desktop.sh [scenario]      (default: full-demo, looping)
set -euo pipefail
here="$(cd "$(dirname "$0")/.." && pwd)"
build="${BUILD_DIR:-$here/build}"
export CIVI_PPS_ROOT="${CIVI_PPS_ROOT:-/tmp/civi-pps}"
export CIVI_CONFIG="${CIVI_CONFIG:-$here/config/civi-config.json}"
scenario="${1:-full-demo}"

pids=()
cleanup() { kill "${pids[@]}" 2>/dev/null || true; }
trap cleanup EXIT INT TERM

python3 -I "$here/tools/vsim/civi_vsim.py" --scenario "$scenario" --loop & pids+=($!)
sleep 0.5
"$build/safety/civi-telltaled/civi-telltaled" & pids+=($!)
for d in cluster center passenger hud; do "$build/hmi/civi-$d" & pids+=($!); done
wait
