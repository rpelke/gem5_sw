#!/usr/bin/env bash
set -euo pipefail

TEST_FILE=/app/lib/test_ioctl.c
TEST_TARGET_DIR="${TARGET_DIR}/root"
TEST_BIN="${TEST_TARGET_DIR}/test_ioctl"

CC="${TARGET_CC:-aarch64-buildroot-linux-gnu-gcc}"

echo "Compiling ${TEST_FILE} -> ${TEST_BIN}"

mkdir -p "${TEST_TARGET_DIR}"
"${CC}" -O2 -Wall -Wextra "${TEST_FILE}" -o "${TEST_BIN}"
