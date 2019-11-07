# This file adds the functions to build the CLooG library
# Copyright 2009 Yann E. MORIN
# Licensed under the GPL v2. See COPYING in the root of this package

do_cloog_get() { :; }
do_cloog_extract() { :; }
do_cloog_for_build() { :; }
do_cloog_for_host() { :; }
do_cloog_for_target() { :; }

# Overide functions depending on configuration
if [ "${CT_CLOOG}" = "y" ]; then

# Download CLooG
do_cloog_get() {
    CT_Fetch CLOOG
}

# Extract CLooG
do_cloog_extract() {
    CT_ExtractPatch CLOOG
}

# Build CLooG for running on build
# - always build statically
# - install in build-tools prefix
do_cloog_for_build() {
    local -a cloog_opts

    case "${CT_TOOLCHAIN_TYPE}" in
        native|cross)   return 0;;
    esac

    CT_DoStep INFO "Installing CLooG for build"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-cloog-build-${CT_BUILD}"

    cloog_opts+=( "host=${CT_BUILD}" )
    cloog_opts+=( "prefix=${CT_BUILDTOOLS_PREFIX_DIR}" )
    cloog_opts+=( "cflags=${CT_CFLAGS_FOR_BUILD}" )
    cloog_opts+=( "ldflags=${CT_LDFLAGS_FOR_BUILD}" )
    do_cloog_backend "${cloog_opts[@]}"

    CT_Popd
    CT_EndStep
}

# Build CLooG for running on host
do_cloog_for_host() {
    local -a cloog_opts

    CT_DoStep INFO "Installing CLooG for host"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-cloog-host-${CT_HOST}"

    cloog_opts+=( "host=${CT_HOST}" )
    cloog_opts+=( "prefix=${CT_HOST_COMPLIBS_DIR}" )
    cloog_opts+=( "cflags=${CT_CFLAGS_FOR_HOST}" )
    cloog_opts+=( "ldflags=${CT_LDFLAGS_FOR_HOST}" )
    do_cloog_backend "${cloog_opts[@]}"

    CT_Popd
    CT_EndStep
}

# Build CLooG
#     Parameter     : description               : type      : default
#     host          : machine to run on         : tuple     : (none)
#     prefix        : prefix to install into    : dir       : (none)
#     cflags        : cflags to use             : string    : (empty)
#     ldflags       : ldflags to use            : string    : (empty)
do_cloog_backend() {
    local host
    local prefix
    local cflags
    local ldflags
    local arg
    local -a cloog_opts

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    cloog_opts+=( --with-gmp=system --with-gmp-prefix="${prefix}" )
    cloog_opts+=( --with-isl=system --with-isl-prefix="${prefix}" )
    cloog_opts+=( --without-osl )

    CT_DoLog EXTRA "Configuring CLooG"

    CT_DoExecLog CFG                                    \
    CFLAGS="${cflags}"                                  \
    LDFLAGS="${ldflags}"                                \
    LIBS="-lm"                                          \
    ${CONFIG_SHELL}                                     \
    "${CT_SRC_DIR}/cloog/configure"                     \
        --build=${CT_BUILD}                             \
        --host=${host}                                  \
        --prefix="${prefix}"                            \
        --with-bits=gmp                                 \
        --with-host-libstdcxx='-lstdc++'                \
        --disable-shared                                \
        --enable-static                                 \
        "${cloog_opts[@]}"

    CT_DoLog EXTRA "Building CLooG"
    CT_DoExecLog ALL make ${CT_JOBSFLAGS}

    if [ "${CT_COMPLIBS_CHECK}" = "y" ]; then
        if [ "${host}" = "${CT_BUILD}" ]; then
            CT_DoLog EXTRA "Checking CLooG"
            CT_DoExecLog ALL make ${CT_JOBSFLAGS} -s check
        else
            # Cannot run host binaries on build in a canadian cross
            CT_DoLog EXTRA "Skipping check for CLooG on the host"
        fi
    fi

    CT_DoLog EXTRA "Installing CLooG"
    CT_DoExecLog ALL make install
}

fi # CT_CLOOG
