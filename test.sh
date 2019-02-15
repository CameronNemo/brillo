#!/bin/sh

set -eu

: ${BRILLO_BIN:=./brillo}
: ${BRILLO_VALGRIND:=valgrind --leak-check=full --show-leak-kinds=all}

_ckvg() {
	local id="$1"

	shift

	local out="$(${BRILLO_VALGRIND} "$BRILLO_BIN" $@ 2>&1)"

	printf '%s' "${out}" | grep -q "${ckstr}" || {
		printf 'Valgrind reported errors for test: %s\n' "${id}"
		printf '%s\n' "${out}"
		ret=1
	}
}

ret=0
ckstr='ERROR SUMMARY: 0 errors from 0 context'

_ckvg "opmode=help" -h
_ckvg "opmode=version" -V

test ! -d /sys/class/backlight || {
	_ckvg "opmode=get"
	_ckvg "opmode=list" -L
}

test ! -d /sys/class/leds || {
	_ckvg "opmode=get target=keyboard" -k
	_ckvg "opmode=list target=keyboard" -Lk
}

exit "${ret}"
