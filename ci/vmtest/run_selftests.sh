#!/bin/bash

# run_selftest.sh will run the tests within /${PROJECT_NAME}/selftests/bpf
# If no specific test names are given, all test will be ran, otherwise, it will
# run the test passed as parameters.
# There is 2 ways to pass test names.
# 1) command-line arguments to this script
# 2) a comma-separated list of test names passed as `run_tests` boot parameters.
# test names passed as any of those methods will be ran.

set -euo pipefail

source $(cd $(dirname $0) && pwd)/helpers.sh

ARCH=$(uname -m)

STATUS_FILE=/exitstatus

declare -a TEST_NAMES_FROM_BOOT

read_lists() {
	(for path in "$@"; do
		if [[ -s "$path" ]]; then
			cat "$path"
		fi;
	done) | cut -d'#' -f1 | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' | tr -s '\n' ','
}

TEST_PROGS_ARGS=""
# Disabled due to issue
# if [[ "$(nproc)" -gt 2 ]]; then
#   TEST_PROGS_ARGS="-j"
# fi

read_tests_from_boot_parameters() {
    foldable start read_test_from_boot "Reading tests from boot parameters"
    # Check if test names were passed as boot parameter.
    # We expect `run_tests` to be a comma-separated list of test names.
    IFS=',' read -r -a TEST_NAMES_FROM_BOOT <<< \
        "$(sed -n 's/.*run_tests=\([^ ]*\).*/\1/p' /proc/cmdline)"

    echo "${#TEST_NAMES_FROM_BOOT[@]} tests: ${TEST_NAMES_FROM_BOOT[@]}"
    foldable end read_test_from_boot
}

test_progs() {
  foldable start test_progs "Testing test_progs"
  # "&& true" does not change the return code (it is not executed
  # if the Python script fails), but it prevents exiting on a
  # failure due to the "set -e".
  ./test_progs ${DENYLIST:+-d"$DENYLIST"} ${ALLOWLIST:+-a"$ALLOWLIST"} ${TEST_PROGS_ARGS} && true
  echo "test_progs:$?" >>"${STATUS_FILE}"
  foldable end test_progs
}

test_progs_no_alu32() {
  foldable start test_progs-no_alu32 "Testing test_progs-no_alu32"
  ./test_progs-no_alu32 ${DENYLIST:+-d"$DENYLIST"} ${ALLOWLIST:+-a"$ALLOWLIST"} ${TEST_PROGS_ARGS} && true
  echo "test_progs-no_alu32:$?" >>"${STATUS_FILE}"
  foldable end test_progs-no_alu32
}

test_maps() {
  foldable start test_maps "Testing test_maps"
  taskset 0xF ./test_maps && true
  echo "test_maps:$?" >>"${STATUS_FILE}"
  foldable end test_maps
}

test_verifier() {
  foldable start test_verifier "Testing test_verifier"
  ./test_verifier && true
  echo "test_verifier:$?" >>"${STATUS_FILE}"
  foldable end test_verifier
}

foldable end vm_init

foldable start kernel_config "Kconfig"

zcat /proc/config.gz

foldable end kernel_config

configs_path=${PROJECT_NAME}/selftests/bpf
local_configs_path=${PROJECT_NAME}/vmtest/configs
DENYLIST=$(read_lists \
	"$configs_path/DENYLIST" \
	"$configs_path/DENYLIST.${ARCH}" \
	"$local_configs_path/DENYLIST" \
	"$local_configs_path/DENYLIST.${ARCH}" \
)
ALLOWLIST=$(read_lists \
	"$configs_path/ALLOWLIST" \
	"$configs_path/ALLOWLIST.${ARCH}" \
	"$local_configs_path/ALLOWLIST" \
	"$local_configs_path/ALLOWLIST.${ARCH}" \
)

echo "DENYLIST: ${DENYLIST}"
echo "ALLOWLIST: ${ALLOWLIST}"

cd ${PROJECT_NAME}/selftests/bpf

# populate TEST_NAMES_FROM_BOOT
read_tests_from_boot_parameters
# Sort and only keep unique test names from both boot params and arguments
readarray -t test_names < \
    <(printf '%s\0' "${TEST_NAMES_FROM_BOOT[@]}" "$@" | sort -z -u | xargs -0n1)
# if we don't have any test name provided to the script, we run all tests.
if [ ${test_names[@]} -eq 0 ]; then
	test_progs
	test_progs_no_alu32
	test_maps
	test_verifier
else
	# else we run the tests passed as command-line arguments and through boot
	# parameter.
	for test_name in "${test_names[@]}" "$@"; do
		"${test_name}"
	done
fi
