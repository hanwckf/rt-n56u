# This file adds the functions to build the zlib library
# Copyright 2017 Alexey Neyman
# Licensed under the GPL v2. See COPYING in the root of this package

do_zlib_get() { :; }
do_zlib_extract() { :; }
do_zlib_for_build() { :; }
do_zlib_for_host() { :; }
do_zlib_for_target() { :; }

# Overide functions depending on configuration
if [ "${CT_ZLIB}" = "y" ]; then

# Download zlib
do_zlib_get() {
    CT_Fetch ZLIB
}

# Extract zlib
do_zlib_extract() {
    CT_ExtractPatch ZLIB
}

# Build zlib for running on build
# - always build statically
# - install in build-tools prefix
do_zlib_for_build() {
    local -a zlib_opts

    case "${CT_TOOLCHAIN_TYPE}" in
        native|cross)   return 0;;
    esac

    CT_DoStep INFO "Installing zlib for build"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-zlib-build-${CT_BUILD}"

    zlib_opts+=( "host=${CT_BUILD}" )
    zlib_opts+=( "prefix=${CT_BUILDTOOLS_PREFIX_DIR}" )
    zlib_opts+=( "cflags=${CT_CFLAGS_FOR_BUILD}" )
    zlib_opts+=( "ldflags=${CT_LDFLAGS_FOR_BUILD}" )
    do_zlib_backend "${zlib_opts[@]}"

    CT_Popd
    CT_EndStep
}

# Build zlib for running on host
do_zlib_for_host() {
    local -a zlib_opts

    CT_DoStep INFO "Installing zlib for host"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-zlib-host-${CT_HOST}"

    zlib_opts+=( "host=${CT_HOST}" )
    zlib_opts+=( "prefix=${CT_HOST_COMPLIBS_DIR}" )
    zlib_opts+=( "cflags=${CT_CFLAGS_FOR_HOST}" )
    zlib_opts+=( "ldflags=${CT_LDFLAGS_FOR_HOST}" )
    do_zlib_backend "${zlib_opts[@]}"

    CT_Popd
    CT_EndStep
}

# Build zlib
#     Parameter     : description               : type      : default
#     host          : machine to run on         : tuple     : (none)
#     prefix        : prefix to install into    : dir       : (none)
#     cflags        : cflags to use             : string    : (empty)
#     ldflags       : ldflags to use            : string    : (empty)
do_zlib_backend() {
    local host
    local prefix
    local cflags
    local ldflags
    local arg
    local -a extra_config
    local -a extra_make

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    case "${host}" in
    *-mingw32)
        # zlib treats mingw host differently and requires using a different
        # makefile rather than configure+make. It also does not support
        # out-of-tree building.
        cp -av "${CT_SRC_DIR}/zlib/." .
        extra_make=( -f win32/Makefile.gcc \
            PREFIX="${host}-" \
            SHAREDLIB= \
            IMPLIB= \
            LIBRARY_PATH="${prefix}/lib" \
            INCLUDE_PATH="${prefix}/include" \
            BINARY_PATH="${prefix}/bin" \
            prefix="${prefix}" \
            )
        ;;

    *)
        CT_DoLog EXTRA "Configuring zlib"

        CT_DoExecLog CFG                                  \
        CFLAGS="${cflags}"                                \
        LDFLAGS="${ldflags}"                              \
        CHOST="${host}"                                   \
        ${CONFIG_SHELL}                                   \
        "${CT_SRC_DIR}/zlib/configure"                    \
            --prefix="${prefix}"                          \
            --static                                      \
            "${extra_config[@]}"
        ;;
    esac

    CT_DoLog EXTRA "Building zlib"
    CT_DoExecLog ALL make "${extra_make[@]}" ${CT_JOBSFLAGS}

    if [ "${CT_COMPLIBS_CHECK}" = "y" ]; then
        if [ "${host}" = "${CT_BUILD}" ]; then
            CT_DoLog EXTRA "Checking zlib"
            CT_DoExecLog ALL make "${extra_make[@]}" -s test
        else
            # Cannot run host binaries on build in a canadian cross
            CT_DoLog EXTRA "Skipping check for zlib on the host"
        fi
    fi

    CT_DoLog EXTRA "Installing zlib"
    CT_DoExecLog ALL make "${extra_make[@]}" install
}

fi # CT_ZLIB
