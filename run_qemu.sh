#!/usr/bin/env bash
set -e

# Usage: ./build_and_run.sh <board> <timeout_in_seconds>
BOARD="${1:-mps2/an521/cpu0}"
TIMEOUT="${2:-10}"

# Build the Zephyr application
west build -d build -p always -b "${BOARD}" .

echo
echo "=== Running in QEMU for ${TIMEOUT} seconds ==="
status=0

# Run QEMU with the designated timeout
timeout --foreground "${TIMEOUT}" \
    "${ZEPHYR_SDK_INSTALL_DIR}/sysroots/x86_64-pokysdk-linux/usr/bin/qemu-system-arm" \
    -machine mps2-an521 -cpu cortex-m33 -m 16 -nographic -vga none \
    -device loader,file=build/zephyr/zephyr.elf || status=$?

# Exit code 124 indicates the timeout command stopped QEMU. 
# Since QEMU boards lack a native software power-off mechanism, 124 is expected and successful.
[ "${status}" -eq 124 ] || exit "${status}"