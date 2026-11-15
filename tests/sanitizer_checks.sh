#!/usr/bin/env sh
# The checks that need no game data, run under the sanitizers (docs/architecture-plan.md, 1.5): the configs through
# tutankhamun --check-configs, and every saved game of tests/saves read chunk by chunk (and decompressed) by
# tutankhamun_sim --diff-saves against itself. A sanitizer report stops the program and fails the check.
#
#     tests/sanitizer_checks.sh out/build/linux-asan     (or linux-ubsan)
#
# The saved game regression itself needs the user's copy of the game: tests/run_regression.py.
set -e
build=${1:?usage: sanitizer_checks.sh <build dir>}
cd "$(dirname "$0")/.."

export ASAN_OPTIONS="${ASAN_OPTIONS:-abort_on_error=1:detect_leaks=0}"
export UBSAN_OPTIONS="${UBSAN_OPTIONS:-print_stacktrace=1:halt_on_error=1}"

echo "--- the configs"
"$build/bin/tutankhamun" --check-configs config

for save in tests/saves/*.sav; do
    echo "--- $save"
    "$build/bin/tutankhamun_sim" --diff-saves "$save" "$save"
done
echo "sanitizer_checks.sh: every check passed"
