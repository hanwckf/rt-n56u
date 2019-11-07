# This file adds the functions to build the MPC library
# Copyright 2009 Yann E. MORIN
# Licensed under the GPL v2. See COPYING in the root of this package

do_mpc_get() { :; }
do_mpc_extract() { :; }
do_mpc_for_build() { :; }
do_mpc_for_host() { :; }
do_mpc_for_target() { :; }

# Overide functions depending on configuration
if [ "${CT_MPC}" = "y" ]; then

# Download MPC
do_mpc_get() {
    CT_Fetch MPC
}

# Extract MPC
do_mpc_extract() {
    CT_ExtractPatch MPC
}

# Build MPC for running on build
# - always build statically
# - install in build-tools prefix
do_mpc_for_build() {
    local -a mpc_opts

    case "${CT_TOOLCHAIN_TYPE}" in
        native|cross)   return 0;;
    esac

    CT_DoStep INFO "Installing MPC for build"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-mpc-build-${CT_BUILD}"

    mpc_opts+=( "host=${CT_BUILD}" )
    mpc_opts+=( "prefix=${CT_BUILDTOOLS_PREFIX_DIR}" )
    mpc_opts+=( "cflags=${CT_CFLAGS_FOR_BUILD}" )
    mpc_opts+=( "ldflags=${CT_LDFLAGS_FOR_BUILD}" )
    do_mpc_backend "${mpc_opts[@]}"

    CT_Popd
    CT_EndStep
}

# Build MPC for running on host
do_mpc_for_host() {
    local -a mpc_opts

    CT_DoStep INFO "Installing MPC for host"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-mpc-host-${CT_HOST}"

    mpc_opts+=( "host=${CT_HOST}" )
    mpc_opts+=( "prefix=${CT_HOST_COMPLIBS_DIR}" )
    mpc_opts+=( "cflags=${CT_CFLAGS_FOR_HOST}" )
    mpc_opts+=( "ldflags=${CT_LDFLAGS_FOR_HOST}" )
    do_mpc_backend "${mpc_opts[@]}"

    CT_Popd
    CT_EndStep
}

# Build MPC
#     Parameter     : description               : type      : default
#     host          : machine to run on         : tuple     : (none)
#     prefix        : prefix to install into    : dir       : (none)
#     cflags        : cflags to use             : string    : (empty)
#     ldflags       : ldflags to use            : string    : (empty)
do_mpc_backend() {
    local host
    local prefix
    local cflags
    local ldflags
    local arg

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    CT_DoLog EXTRA "Configuring MPC"

    CT_DoExecLog CFG                                \
    CFLAGS="${cflags}"                              \
    LDFLAGS="${ldflags}"                            \
    ${CONFIG_SHELL}                                 \
    "${CT_SRC_DIR}/mpc/configure"                   \
        --build=${CT_BUILD}                         \
        --host=${host}                              \
        --prefix="${prefix}"                        \
        --with-gmp="${prefix}"                      \
        --with-mpfr="${prefix}"                     \
        --disable-shared                            \
        --enable-static

    CT_DoLog EXTRA "Building MPC"
    CT_DoExecLog ALL make ${CT_JOBSFLAGS}

    if [ "${CT_COMPLIBS_CHECK}" = "y" ]; then
        if [ "${host}" = "${CT_BUILD}" ]; then
            CT_DoLog EXTRA "Checking MPC"
            CT_DoExecLog ALL make ${CT_JOBSFLAGS} -s check
        else
            # Cannot run host binaries on build in a canadian cross
            CT_DoLog EXTRA "Skipping check for MPC on the host"
        fi
    fi

    CT_DoLog EXTRA "Installing MPC"
    CT_DoExecLog ALL make install
}

fi # CT_MPC
