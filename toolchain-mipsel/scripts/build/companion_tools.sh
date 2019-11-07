# Wrapper to build the companion tools facilities

# List all companion tools facilities, and parse their scripts
CT_COMP_TOOLS_FACILITY_LIST=
for f in "${CT_LIB_DIR}/scripts/build/companion_tools/"*.sh; do
    _f="$(basename "${f}" .sh)"
    _f="${_f#???-}"
    __f="CT_COMP_TOOLS_${_f^^}"
    if [ "${!__f}" = "y" ]; then
        CT_DoLog DEBUG "Enabling companion tool '${_f}'"
        . "${f}"
        CT_COMP_TOOLS_FACILITY_LIST="${CT_COMP_TOOLS_FACILITY_LIST} ${_f}"
    else
        CT_DoLog DEBUG "Disabling companion tool '${_f}'"
    fi
done

# Download the companion tools facilities
do_companion_tools_get() {
    for f in ${CT_COMP_TOOLS_FACILITY_LIST}; do
        do_companion_tools_${f}_get
    done
}

# Extract and patch the companion tools facilities
do_companion_tools_extract() {
    for f in ${CT_COMP_TOOLS_FACILITY_LIST}; do
        do_companion_tools_${f}_extract
    done
}

# Build the companion tools facilities for build
do_companion_tools_for_build() {
    for f in ${CT_COMP_TOOLS_FACILITY_LIST}; do
        do_companion_tools_${f}_for_build
    done
}

# Build the companion tools facilities for host
do_companion_tools_for_host() {
    case "${CT_TOOLCHAIN_TYPE}" in
        # For native/cross, build==host, skip: the tools were built
        # earlier by do_companion_tools_for_build.
        native|cross)
            return
            ;;
        # For canadian/cross-native, only need to build tools for host
        # if explicitly requested.
        canadian|cross-native)
            if [ -z "${CT_COMP_TOOLS_FOR_HOST}" ]; then
                return
            fi
            ;;
    esac
    for f in ${CT_COMP_TOOLS_FACILITY_LIST}; do
        do_companion_tools_${f}_for_host
    done
}
