# Wrapper to build the test suite facilities
#
# Current assumption: test suites are independent of each other
#                     - no order handling required.

# List all test suite facilities, and parse their scripts
CT_TEST_SUITE_FACILITY_LIST=
for f in "${CT_LIB_DIR}/scripts/build/test_suite/"*.sh; do
    _f="$(basename "${f}" .sh)"
    __f="CT_TEST_SUITE_${_f^^}"
    if [ "${!__f}" = "y" ]; then
        CT_DoLog DEBUG "Enabling test suite '${_f}'"
        . "${f}"
        CT_TEST_SUITE_FACILITY_LIST="${CT_TEST_SUITE_FACILITY_LIST} ${_f}"
    else
        CT_DoLog DEBUG "Disabling test suite '${_f}'"
    fi
done

# Download the test suite facilities
do_test_suite_get() {
    for f in ${CT_TEST_SUITE_FACILITY_LIST}; do
        do_test_suite_${f}_get
    done
}

# Extract and patch the test suite facilities
do_test_suite_extract() {
    for f in ${CT_TEST_SUITE_FACILITY_LIST}; do
        do_test_suite_${f}_extract
    done
}

# Build the test suite facilities
do_test_suite() {
    for f in ${CT_TEST_SUITE_FACILITY_LIST}; do
        do_test_suite_${f}_build
    done
}

