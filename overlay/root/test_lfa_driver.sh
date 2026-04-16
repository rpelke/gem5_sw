#!/bin/sh
set -eu

BASE_ADDR=0x1c150000
DEV_NODE="${1:-/dev/lfa0}"

write_reg() {
    addr="$1"
    value="$2"
    printf 'devmem %s 32 %s\n' "$addr" "$value"
    devmem "$addr" 32 "$value" >/dev/null
}

read_reg() {
    addr="$1"
    devmem "$addr" 32
}

die() {
    printf '\nTest failed: %s\n' "$1" >&2
    exit 1
}

printf 'Configuring linear function accelerator at %s\n' "$BASE_ADDR"

# control register @ 0x1c150000
# bits[31:0]   = a
# bits[63:32]  = b
write_reg 0x1c150000 0x00000002  # a = 2
write_reg 0x1c150004 0x0000000a  # b = 10

if [ ! -e "$DEV_NODE" ]; then
    die "device node $DEV_NODE does not exist"
fi

if [ ! -r "$DEV_NODE" ] || [ ! -w "$DEV_NODE" ]; then
    die "device node $DEV_NODE needs read/write permissions"
fi

tmp_in="$(mktemp)"
trap 'rm -f "$tmp_in"' EXIT INT TERM

# 4 little-endian int32 input values: [1, 2, 3, 4]
printf '\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00\x04\x00\x00\x00' >"$tmp_in"

printf '\nWriting input vector to %s\n' "$DEV_NODE"
dd if="$tmp_in" of="$DEV_NODE" bs=16 count=1 2>/dev/null || die "write to $DEV_NODE failed"

printf '\nReading output vector lanes\n'
raw_lanes="$(dd if="$DEV_NODE" bs=16 count=1 2>/dev/null | od -An -N16 -tx4)"
[ -n "$raw_lanes" ] || die "read from $DEV_NODE returned no data"

set -- $(printf '%s\n' "$raw_lanes" | awk '{for (i = 1; i <= NF; i++) printf "0x%s\n", tolower($i)}')
[ "$#" -eq 4 ] || die "expected 4 output lanes, got $#"

lane0="$1"
lane1="$2"
lane2="$3"
lane3="$4"

printf 'lane0: %s\n' "$lane0"
printf 'lane1: %s\n' "$lane1"
printf 'lane2: %s\n' "$lane2"
printf 'lane3: %s\n' "$lane3"

expected_lane0=0x0000000c
expected_lane1=0x0000000e
expected_lane2=0x00000010
expected_lane3=0x00000012

if [ "$lane0" != "$expected_lane0" ] || \
   [ "$lane1" != "$expected_lane1" ] || \
   [ "$lane2" != "$expected_lane2" ] || \
   [ "$lane3" != "$expected_lane3" ]; then
    printf 'Expected: %s %s %s %s\n' \
        "$expected_lane0" "$expected_lane1" "$expected_lane2" "$expected_lane3" >&2
    printf 'Got:      %s %s %s %s\n' \
        "$lane0" "$lane1" "$lane2" "$lane3" >&2
    exit 1
fi

printf '\nTest passed\n'
