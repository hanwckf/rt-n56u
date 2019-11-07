# Wrapper to build the companion libs facilities

# List all companion tools facilities, and parse their scripts
CT_COMP_LIBS_FACILITY_LIST=
for f in "${CT_LIB_DIR}/scripts/build/companion_libs/"*.sh; do
    _f="$(basename "${f}" .sh)"
    _f="${_f#???-}"
    . "${f}"
    CT_COMP_LIBS_FACILITY_LIST="${CT_COMP_LIBS_FACILITY_LIST} ${_f}"
done

# Download the companion libs facilities
do_companion_libs_get() {
    for f in ${CT_COMP_LIBS_FACILITY_LIST}; do
        do_${f}_get
    done
}

# Extract and patch the companion libs facilities
do_companion_libs_extract() {
    for f in ${CT_COMP_LIBS_FACILITY_LIST}; do
        do_${f}_extract
    done
}

# Build the companion libs facilities for build
do_companion_libs_for_build() {
    for f in ${CT_COMP_LIBS_FACILITY_LIST}; do
        do_${f}_for_build
    done
}

# Build the companion libs facilities for host
do_companion_libs_for_host() {
    for f in ${CT_COMP_LIBS_FACILITY_LIST}; do
        do_${f}_for_host
    done
}

# Build the companion libs facilities for target
do_companion_libs_for_target() {
    for f in ${CT_COMP_LIBS_FACILITY_LIST}; do
        do_${f}_for_target
    done
}

