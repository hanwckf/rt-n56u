# Build script for bison

do_companion_tools_bison_get()
{
    CT_Fetch BISON
}

do_companion_tools_bison_extract()
{
    CT_ExtractPatch BISON
}

do_companion_tools_bison_for_build()
{
    CT_DoStep INFO "Installing bison for build"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-bison-build"
    do_bison_backend \
        host=${CT_BUILD} \
        prefix="${CT_BUILD_COMPTOOLS_DIR}" \
        cflags="${CT_CFLAGS_FOR_BUILD}" \
        ldflags="${CT_LDFLAGS_FOR_BUILD}"
    CT_Popd
    CT_EndStep
}

do_companion_tools_bison_for_host()
{
    CT_DoStep INFO "Installing bison for host"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-bison-host"
    do_bison_backend \
        host=${CT_HOST} \
        prefix="${CT_PREFIX_DIR}" \
        cflags="${CT_CFLAGS_FOR_HOST}" \
        ldflags="${CT_LDFLAGS_FOR_HOST}"
    CT_Popd
    CT_EndStep
}

do_bison_backend()
{
    local host
    local prefix
    local cflags
    local ldflags
    local -a extra_config

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    CT_DoLog EXTRA "Configuring bison"
    CT_DoExecLog CFG \
                     CFLAGS="${cflags}" \
                     LDFLAGS="${ldflags}" \
                     ${CONFIG_SHELL} \
                     "${CT_SRC_DIR}/bison/configure" \
                     --host="${host}" \
                     --prefix="${prefix}" \
		     "${extra_config[@]}"

    CT_DoLog EXTRA "Building bison"
    CT_DoExecLog ALL make

    CT_DoLog EXTRA "Installing bison"
    CT_DoExecLog ALL make install
}
