#!/usr/bin/env bash
# Bilingual screenshot run: every display × scenario × language, offscreen.
# Evidence for French text-fit (CIVI-HMI-002) and demo collateral.
#   tools/screenshots.sh [build-dir] [out-dir]
set -euo pipefail
here="$(cd "$(dirname "$0")/.." && pwd)"
build="${1:-$here/build}"
out="${2:-$here/docs/screenshots}"
export CIVI_PPS_ROOT="$(mktemp -d)"
export QT_QPA_PLATFORM=offscreen
mkdir -p "$out"

set_lang() { printf '@settings\nlanguage::%s\n' "$1" > "$CIVI_PPS_ROOT/hmi/settings.tmp" && mv "$CIVI_PPS_ROOT/hmi/settings.tmp" "$CIVI_PPS_ROOT/hmi/settings"; }

shot() { # scenario until display file [root-property...]
  local sc=$1 until=$2 disp=$3 file=$4; shift 4
  local props=(); for p in "$@"; do props+=(--root-property "$p"); done
  "$build/hmi/civi-$disp" --screenshot "$out/$file" --screenshot-delay 1800 "${props[@]}" >/dev/null 2>&1 \
    || echo "FAILED: $file"
}

run() { # scenario until lang shots...
  local sc=$1 until=$2 lang=$3; shift 3
  python3 -I "$here/tools/vsim/civi_vsim.py" --scenario "$sc" --until "$until" --duration 60 >/dev/null 2>&1 &
  local pid=$!
  sleep "$(python3 -c "print($until + 1.2)")"
  mkdir -p "$CIVI_PPS_ROOT/hmi"; set_lang "$lang"
  while [ $# -gt 0 ]; do eval "shot $sc $until $1"; shift; done
  kill $pid 2>/dev/null || true; wait $pid 2>/dev/null || true
}

for lang in fr en; do
  run parked 0 $lang "center center-home-$lang.png" "center center-updates-$lang.png page=6" \
                     "center center-privacy-$lang.png page=5" "center center-settings-$lang.png page=7" \
                     "passenger passenger-$lang.png"
  run winter-drive 7 $lang "cluster cluster-winter-$lang.png" "center center-nav-$lang.png page=1" \
                           "center center-media-$lang.png page=2" "hud hud-winter-$lang.png"
  run tunnel 5 $lang "center center-tunnel-connectivity-$lang.png page=4" "cluster cluster-tunnel-$lang.png"
  run takeover 6 $lang "cluster cluster-takeover-$lang.png" "hud hud-takeover-$lang.png"
  run wildfire 4 $lang "center center-wildfire-$lang.png" "passenger passenger-wildfire-$lang.png"
  run crash 5 $lang "center center-crash-$lang.png page=8" "cluster cluster-crash-$lang.png"
done
rm -rf "$CIVI_PPS_ROOT"
echo "screenshots in $out"
