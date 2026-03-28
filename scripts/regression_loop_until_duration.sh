#!/usr/bin/env bash
# Long regression: unit tests + stress until wall-clock duration (default 2400 s = 40 min).
# Usage: from repo root after Release build:
#   chmod +x scripts/regression_loop_until_duration.sh
#   ./scripts/regression_loop_until_duration.sh
# Optional: DURATION_SEC=1800 ./scripts/regression_loop_until_duration.sh   # 30 min

set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${BUILD_DIR:-$ROOT/build}"
TESTS="$BUILD/RazumovVocalChainTests"
STRESS="$BUILD/RazumovVocalChainStressTests"
DURATION_SEC="${DURATION_SEC:-2400}"

if [[ ! -x "$TESTS" ]] || [[ ! -x "$STRESS" ]]; then
  echo "Build tests first: cmake --build build --target RazumovVocalChainTests RazumovVocalChainStressTests" >&2
  exit 1
fi

START_TS=$(date +%s)
END_TS=$((START_TS + DURATION_SEC))
ITER=0
echo "Regression until $(date -u -r "$END_TS" +%H:%M:%S) UTC wall (~${DURATION_SEC}s from now)"
while [[ $(date +%s) -lt $END_TS ]]; do
  ITER=$((ITER + 1))
  ELAPSED=$(($(date +%s) - START_TS))
  echo "=== iteration $ITER (elapsed ${ELAPSED}s) ==="
  "$TESTS"
  "$STRESS"
done
echo "Finished $ITER full cycles (unit + stress)."
