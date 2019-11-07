# Build script for m4

do_companion_tools_m4_get()
{
    CT_Fetch M4
}

do_companion_tools_m4_extract()
{
    CT_ExtractPatch M4
}

do_companion_tools_m4_for_build()
{
    CT_DoStep INFO "Installing m4 for build"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-m4-build"
    do_m4_backend \
        host=${CT_BUILD} \
        prefix="${CT_BUILD_COMPTOOLS_DIR}" \
        cflags="${CT_CFLAGS_FOR_BUILD}" \
        ldflags="${CT_LDFLAGS_FOR_BUILD}"
    CT_Popd
    CT_EndStep
}

do_companion_tools_m4_for_host()
{
    CT_DoStep INFO "Installing m4 for host"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-m4-host"
    do_m4_backend \
        host=${CT_HOST} \
        prefix="${CT_PREFIX_DIR}" \
        cflags="${CT_CFLAGS_FOR_HOST}" \
        ldflags="${CT_LDFLAGS_FOR_HOST}"
    CT_Popd
    CT_EndStep
}

do_m4_backend()
{
    local host
    local prefix
    local cflags
    local ldflags

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    case "${host}" in
        *-uclibc)
            # uClibc has posix_spawn in librt, but m4 configure only
            # searches in libc. This leads to a later failure when
            # it includes system <spawn.h> but expects a locally-built
            # posix_spawn().
            ldflags="${ldflags} -lrt"
    esac

    CT_DoLog EXTRA "Configuring m4"
    CT_DoExecLog CFG \
                     CFLAGS="${cflags}" \
                     LDFLAGS="${ldflags}" \
                     ${CONFIG_SHELL} \
                     "${CT_SRC_DIR}/m4/configure" \
                     --host="${host}" \
                     --prefix="${prefix}"

    CT_DoLog EXTRA "Building m4"
    CT_DoExecLog ALL make

    CT_DoLog EXTRA "Installing m4"
    CT_DoExecLog ALL make install
}
