# This file adds the functions to build the ISL library
# Copyright 2009 Yann E. MORIN
# Licensed under the GPL v2. See COPYING in the root of this package

do_isl_get() { :; }
do_isl_extract() { :; }
do_isl_for_build() { :; }
do_isl_for_host() { :; }
do_isl_for_target() { :; }

# Overide functions depending on configuration
if [ "${CT_ISL}" = "y" ]; then

# Download ISL
do_isl_get() {
    CT_Fetch ISL
}

# Extract ISL
do_isl_extract() {
    CT_ExtractPatch ISL
}

# Build ISL for running on build
# - always build statically
# - install in build-tools prefix
do_isl_for_build() {
    local -a isl_opts

    case "${CT_TOOLCHAIN_TYPE}" in
        native|cross)   return 0;;
    esac

    CT_DoStep INFO "Installing ISL for build"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-isl-build-${CT_BUILD}"

    isl_opts+=( "host=${CT_BUILD}" )
    isl_opts+=( "prefix=${CT_BUILDTOOLS_PREFIX_DIR}" )
    isl_opts+=( "cflags=${CT_CFLAGS_FOR_BUILD}" )
    isl_opts+=( "cxxflags=${CT_CFLAGS_FOR_BUILD}" )
    isl_opts+=( "ldflags=${CT_LDFLAGS_FOR_BUILD}" )
    do_isl_backend "${isl_opts[@]}"

    CT_Popd
    CT_EndStep
}

# Build ISL for running on host
do_isl_for_host() {
    local -a isl_opts

    CT_DoStep INFO "Installing ISL for host"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-isl-host-${CT_HOST}"

    isl_opts+=( "host=${CT_HOST}" )
    isl_opts+=( "prefix=${CT_HOST_COMPLIBS_DIR}" )
    isl_opts+=( "cflags=${CT_CFLAGS_FOR_HOST}" )
    isl_opts+=( "cxxflags=${CT_CFLAGS_FOR_HOST}" )
    isl_opts+=( "ldflags=${CT_LDFLAGS_FOR_HOST}" )
    do_isl_backend "${isl_opts[@]}"

    CT_Popd
    CT_EndStep
}

# Build ISL
#     Parameter     : description               : type      : default
#     host          : machine to run on         : tuple     : (none)
#     prefix        : prefix to install into    : dir       : (none)
#     cflags        : cflags to use             : string    : (empty)
#     ldflags       : ldflags to use            : string    : (empty)
do_isl_backend() {
    local host
    local prefix
    local cflags
    local cxxflags
    local ldflags
    local -a extra_config
    local arg

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    CT_DoLog EXTRA "Configuring ISL"

    if [ "${CT_ISL_NEEDS_WITH_GMP}" != "y" ]; then
        extra_config+=("--with-libgmp-prefix=${prefix}")
        extra_config+=("--with-libgmpxx-prefix=${prefix}")
    fi

    if [ "${CT_ISL_HAS_WITH_PIPLIB}" != "y" ]; then
        extra_config+=("--with-piplib=no")
    fi

    CT_DoExecLog CFG                                \
    CFLAGS="${cflags}"                              \
    CXXFLAGS="${cxxflags}"                          \
    LDFLAGS="${ldflags}"                            \
    ${CONFIG_SHELL}                                 \
    "${CT_SRC_DIR}/isl/configure"                   \
        --build=${CT_BUILD}                         \
        --host=${host}                              \
        --prefix="${prefix}"                        \
        "${extra_config[@]}"                        \
        --disable-shared                            \
        --enable-static                             \
        --with-gmp=system                           \
        --with-gmp-prefix="${prefix}"               \
        --with-clang=no

    CT_DoLog EXTRA "Building ISL"
    CT_DoExecLog ALL make ${CT_JOBSFLAGS}

    if [ "${CT_COMPLIBS_CHECK}" = "y" ]; then
        if [ "${host}" = "${CT_BUILD}" ]; then
            CT_DoLog EXTRA "Checking ISL"
            CT_DoExecLog ALL make ${CT_JOBSFLAGS} -s check
        else
            # Cannot run host binaries on build in a canadian cross
            CT_DoLog EXTRA "Skipping check for ISL on the host"
        fi
    fi

    CT_DoLog EXTRA "Installing ISL"
    CT_DoExecLog ALL make install
}

fi # CT_ISL
