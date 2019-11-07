# This file adds functions to build the musl C library
# Copyright 2013 Timo TerÃ¤s
# Licensed under the GPL v2. See COPYING in the root of this package

# Build and install headers and start files
musl_start_files()
{
    # Start files and Headers should be configured the same way as the
    # final libc, but built and installed differently.
    musl_backend libc_mode=startfiles
}

# This function builds and install the full C library
musl_main()
{
    musl_backend libc_mode=final
}

musl_post_cc() {
    # MUSL creates dynamic linker symlink with absolute path - which works on the
    # target but not on the host. We want our cross-ldd tool to work.
    CT_MultilibFixupLDSO
}

musl_backend() {
    local libc_mode
    local arg

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    case "${libc_mode}" in
        startfiles)     CT_DoStep INFO "Installing C library headers & start files";;
        final)          CT_DoStep INFO "Installing C library";;
        *)              CT_Abort "Unsupported (or unset) libc_mode='${libc_mode}'";;
    esac

    CT_mkdir_pushd "${CT_BUILD_DIR}/build-libc-${libc_mode}"
    CT_IterateMultilibs musl_backend_once multilib libc_mode="${libc_mode}"
    CT_Popd
    CT_EndStep
}

# This backend builds the C library
# Usage: musl_backend param=value [...]
#   Parameter           : Definition                      : Type      : Default
#   libc_mode           : 'startfiles' or 'final'         : string    : (none)
musl_backend_once() {
    local libc_mode
    local -a extra_cflags
    local -a extra_config
    local src_dir="${CT_SRC_DIR}/musl"
    local multi_dir multi_os_dir multi_root multi_flags multi_index multi_count
    local multilib_dir
    local hdr_install_subdir
    local arg f l

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    CT_DoStep INFO "Building for multilib ${multi_index}/${multi_count}: '${multi_flags}'"

    multilib_dir="/usr/lib/${multi_os_dir}"
    CT_SanitizeVarDir multilib_dir
    CT_DoExecLog ALL mkdir -p "${multi_root}${multilib_dir}"

    extra_cflags=( ${multi_flags} )

    if [ "${CT_LIBC_MUSL_DEBUG}" = "y" ]; then
        extra_config+=("--enable-debug")
    fi

    if [ "${CT_LIBC_MUSL_WARNINGS}" = "y" ]; then
        extra_config+=("--enable-warnings")
    fi

    case "${CT_SHARED_LIBS}" in
        y) extra_config+=("--enable-shared");;
        *) extra_config+=("--disable-shared");;
    esac

    extra_config+=( "--enable-optimize=${CT_LIBC_MUSL_OPTIMIZE}" )

    # Same problem as with uClibc: different variants sometimes have
    # incompatible headers.
    CT_DoArchMUSLHeaderDir hdr_install_subdir "${multi_flags}"
    if [ -n "${hdr_install_subdir}" ]; then
        extra_config+=( "--includedir=/usr/include/${hdr_install_subdir}" )
    fi

    CT_SymlinkToolsMultilib

    # NOTE: musl handles the build/host/target a little bit differently
    # then one would expect:
    #   build   : not used
    #   host    : same as --target
    #   target  : the machine musl runs on
    CT_DoExecLog CFG                                      \
    CFLAGS="${extra_cflags[*]}"                           \
    CROSS_COMPILE="${CT_TARGET}-"                         \
    ${CONFIG_SHELL}                                       \
    ${src_dir}/configure                                  \
        --host="${multi_target}"                          \
        --target="${multi_target}"                        \
        --prefix="/usr"                                   \
        --libdir="${multilib_dir}"                        \
        --disable-gcc-wrapper                             \
        "${extra_config[@]}"

    if [ "${libc_mode}" = "startfiles" ]; then
        CT_DoLog EXTRA "Installing C library headers"
        CT_DoExecLog ALL make DESTDIR="${multi_root}" install-headers
        CT_DoLog EXTRA "Building C library start files"
        CT_DoExecLog ALL make DESTDIR="${multi_root}" \
            obj/crt/crt1.o obj/crt/crti.o obj/crt/crtn.o
        CT_DoLog EXTRA "Installing C library start files"
        CT_DoExecLog ALL cp -av obj/crt/crt*.o "${multi_root}${multilib_dir}"
        CT_DoExecLog ALL ${CT_TARGET}-${CT_CC} -nostdlib \
            -nostartfiles -shared -x c /dev/null -o "${multi_root}${multilib_dir}/libc.so"
    fi
    if [ "${libc_mode}" = "final" ]; then
        CT_DoLog EXTRA "Cleaning up start files"
        CT_DoExecLog ALL rm -f "${multi_root}${multilib_dir}/crt1.o" \
            "${multi_root}${multilib_dir}/crti.o" \
            "${multi_root}${multilib_dir}/crtn.o" \
            "${multi_root}${multilib_dir}/libc.so"

        CT_DoLog EXTRA "Building C library"
        CT_DoExecLog ALL make ${CT_JOBSFLAGS}

        CT_DoLog EXTRA "Installing C library"
        CT_DoExecLog ALL make DESTDIR="${multi_root}" install

        # Convert /lib/ld-* symlinks to relative paths so that they are valid
        # both on the host and on the target.
        for f in ${multi_root}/ld-musl-*; do
            [ -L "${f}" ] || continue
            l=$( readlink ${f} )
            case "${l}" in
                ${multilib_dir}/*)
                    CT_DoExecLog ALL ln -sf "../${l}" "${f}"
                    ;;
            esac
        done

        # Any additional actions for this architecture
        CT_DoArchMUSLPostInstall
    fi

    CT_EndStep
}
