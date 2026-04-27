#!/usr/bin/env bash
set -euo pipefail

TEST_TARGET_DIR="${TARGET_DIR}/root"
CC="${TARGET_CC:-aarch64-buildroot-linux-gnu-gcc}"

mkdir -p "${TEST_TARGET_DIR}"

for test_name in test_ioctl test_lfa_driver; do
    test_file="/app/lib/${test_name}.c"
    test_bin="${TEST_TARGET_DIR}/${test_name}"
    echo "Compiling ${test_file} -> ${test_bin}"
    "${CC}" -O2 -Wall -Wextra "${test_file}" -o "${test_bin}"
done
