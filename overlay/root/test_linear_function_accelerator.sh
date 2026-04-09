#!/bin/sh
set -eu

BASE_ADDR=0x1c150000

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

printf 'Configuring linear function accelerator at %s\n' "$BASE_ADDR"

# control register @ 0x1c150000
# bits[31:0]   = a
# bits[63:32]  = b
write_reg 0x1c150000 0x00000002  # a = 2
write_reg 0x1c150004 0x0000000a  # b = 10

# input vector lanes @ 0x1c150010 + lane*4
write_reg 0x1c150010 0x00000001  # x0 = 1
write_reg 0x1c150014 0x00000002  # x1 = 2
write_reg 0x1c150018 0x00000003  # x2 = 3
write_reg 0x1c15001c 0x00000004  # x3 = 4

printf '\nReading output vector lanes\n'
lane0=$(read_reg 0x1c150020)
lane1=$(read_reg 0x1c150024)
lane2=$(read_reg 0x1c150028)
lane3=$(read_reg 0x1c15002c)

printf 'lane0: %s\n' "$lane0"
printf 'lane1: %s\n' "$lane1"
printf 'lane2: %s\n' "$lane2"
printf 'lane3: %s\n' "$lane3"

expected_lane0=0x0000000C
expected_lane1=0x0000000E
expected_lane2=0x00000010
expected_lane3=0x00000012

if [ "$lane0" != "$expected_lane0" ] || \
   [ "$lane1" != "$expected_lane1" ] || \
   [ "$lane2" != "$expected_lane2" ] || \
   [ "$lane3" != "$expected_lane3" ]; then
    printf '\nTest failed\n' >&2
    printf 'Expected: %s %s %s %s\n' \
        "$expected_lane0" "$expected_lane1" "$expected_lane2" "$expected_lane3" >&2
    printf 'Got:      %s %s %s %s\n' \
        "$lane0" "$lane1" "$lane2" "$lane3" >&2
    exit 1
fi

printf '\nTest passed\n'
