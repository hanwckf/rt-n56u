# This file adds the functions to build the MPFR library
# Copyright 2008 Yann E. MORIN
# Licensed under the GPL v2. See COPYING in the root of this package

do_mpfr_get() { :; }
do_mpfr_extract() { :; }
do_mpfr_for_build() { :; }
do_mpfr_for_host() { :; }
do_mpfr_for_target() { :; }

# Overide function depending on configuration
if [ "${CT_MPFR}" = "y" ]; then

# Download MPFR
do_mpfr_get() {
    CT_Fetch MPFR
}

# Extract MPFR
do_mpfr_extract() {
    CT_ExtractPatch MPFR

    # TBD is it a problem with 2.4.x? The comment says it is not, yet the code is run
    # OK, Gentoo have a sanity check that libtool.m4 and ltmain.sh have the
    # same version number. Unfortunately, some tarballs of MPFR are not
    # built sanely, and thus ./configure fails on Gentoo.
    # See: http://sourceware.org/ml/crossgcc/2008-05/msg00080.html
    # and: http://sourceware.org/ml/crossgcc/2008-06/msg00005.html
    # This hack is not bad per se, but the MPFR guys would be better not to
    # do that in the future...
    # It seems that MPFR >= 2.4.0 do not need this...
    case "${CT_MPFR_VERSION}" in
        2.4.*)
            CT_Pushd "${CT_SRC_DIR}/mpfr-${CT_MPFR_VERSION}"
            if [ ! -f .autoreconf.ct-ng ]; then
                CT_DoLog DEBUG "Running autoreconf"
                CT_DoExecLog ALL autoreconf
                touch .autoreconf.ct-ng
            fi
            CT_Popd
            ;;
    esac
}

# Build MPFR for running on build
# - always build statically
# - install in build-tools prefix
do_mpfr_for_build() {
    local -a mpfr_opts

    case "${CT_TOOLCHAIN_TYPE}" in
        native|cross)   return 0;;
    esac

    CT_DoStep INFO "Installing MPFR for build"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-mpfr-build-${CT_BUILD}"

    mpfr_opts+=( "host=${CT_BUILD}" )
    mpfr_opts+=( "prefix=${CT_BUILDTOOLS_PREFIX_DIR}" )
    mpfr_opts+=( "cflags=${CT_CFLAGS_FOR_BUILD}" )
    mpfr_opts+=( "ldflags=${CT_LDFLAGS_FOR_BUILD}" )
    do_mpfr_backend "${mpfr_opts[@]}"

    CT_Popd
    CT_EndStep
}

# Build MPFR for running on host
do_mpfr_for_host() {
    local -a mpfr_opts

    CT_DoStep INFO "Installing MPFR for host"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-mpfr-host-${CT_HOST}"

    mpfr_opts+=( "host=${CT_HOST}" )
    mpfr_opts+=( "prefix=${CT_HOST_COMPLIBS_DIR}" )
    mpfr_opts+=( "cflags=${CT_CFLAGS_FOR_HOST}" )
    mpfr_opts+=( "ldflags=${CT_LDFLAGS_FOR_HOST}" )
    do_mpfr_backend "${mpfr_opts[@]}"

    CT_Popd
    CT_EndStep
}

# Build MPFR
#     Parameter     : description               : type      : default
#     host          : machine to run on         : tuple     : (none)
#     prefix        : prefix to install into    : dir       : (none)
#     cflags        : cflags to use             : string    : (empty)
#     ldflags       : ldflags to use            : string    : (empty)
do_mpfr_backend() {
    local host
    local prefix
    local cflags
    local ldflags
    local arg

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    # Under Cygwin, we can't build a thread-safe library
    case "${CT_HOST}" in
        *cygwin*)   mpfr_opts+=( --disable-thread-safe );;
        *mingw*)    mpfr_opts+=( --disable-thread-safe );;
        *darwin*)   mpfr_opts+=( --disable-thread-safe );;
        *)          mpfr_opts+=( --enable-thread-safe  );;
    esac

    CT_DoLog EXTRA "Configuring MPFR"
    CT_DoExecLog CFG                                    \
    CC="${host}-gcc"                                    \
    CFLAGS="${cflags}"                                  \
    LDFLAGS="${ldflags}"                                \
    ${CONFIG_SHELL}                                     \
    "${CT_SRC_DIR}/mpfr/configure"                      \
        --build=${CT_BUILD}                             \
        --host=${host}                                  \
        --prefix="${prefix}"                            \
        --with-gmp="${prefix}"                          \
        --disable-shared                                \
        --enable-static

    CT_DoLog EXTRA "Building MPFR"
    CT_DoExecLog ALL make ${CT_JOBSFLAGS}

    if [ "${CT_COMPLIBS_CHECK}" = "y" ]; then
        if [ "${host}" = "${CT_BUILD}" ]; then
            CT_DoLog EXTRA "Checking MPFR"
            CT_DoExecLog ALL make ${CT_JOBSFLAGS} -s check
        else
            # Cannot run host binaries on build in a canadian cross
            CT_DoLog EXTRA "Skipping check for MPFR on the host"
        fi
    fi

    CT_DoLog EXTRA "Installing MPFR"
    CT_DoExecLog ALL make install
}

fi # CT_MPFR
