# This file adds the functions to build the GCC test suite
# Copyright 2010 DoréDevelopment
# Created by Martin Lund <mgl@doredevelopment.dk>
# Licensed under the GPL v2. See COPYING in the root of this package

do_test_suite_gcc_get() { :; }
do_test_suite_gcc_extract() { :; }
do_test_suite_gcc_build() { :; }

# Overide functions depending on configuration
if [ "${CT_TEST_SUITE_GCC}" = "y" ]; then

do_test_suite_gcc_build() {
 
    CT_DoStep INFO "Installing GCC test suite"

    CT_DoExecLog ALL mkdir -p "${CT_TEST_SUITE_DIR}/gcc"
    CT_DoExecLog ALL cp -av "${CT_LIB_DIR}/contrib/gcc-test-suite/default.cfg"      \
                            "${CT_LIB_DIR}/contrib/gcc-test-suite/Makefile"         \
                            "${CT_LIB_DIR}/contrib/gcc-test-suite/README"           \
                            "${CT_SRC_DIR}/gcc/gcc/testsuite"  \
                            "${CT_TEST_SUITE_DIR}/gcc"

    CT_DoExecLog ALL sed -i -r -e "s/@@DG_TARGET@@/${CT_TARGET}/g;"     \
                         "${CT_TEST_SUITE_DIR}/gcc/Makefile"

    CT_EndStep
}

fi # CT_TEST_SUITE_GCC
