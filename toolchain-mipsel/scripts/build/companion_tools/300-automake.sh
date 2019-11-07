# Build script for automake

do_companion_tools_automake_get()
{
    CT_Fetch AUTOMAKE
}

do_companion_tools_automake_extract()
{
    CT_ExtractPatch AUTOMAKE
}

do_companion_tools_automake_for_build()
{
    CT_DoStep INFO "Installing automake for build"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-automake-build"
    do_automake_backend host=${CT_BUILD} prefix="${CT_BUILD_COMPTOOLS_DIR}"
    CT_Popd
    CT_EndStep
}

do_companion_tools_automake_for_host()
{
    CT_DoStep INFO "Installing automake for host"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-automake-host"
    do_automake_backend host=${CT_HOST} prefix="${CT_PREFIX_DIR}"
    CT_Popd
    CT_EndStep
}

do_automake_backend()
{
    local host
    local prefix

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    CT_DoLog EXTRA "Configuring automake"
    CT_DoExecLog CFG \
                     ${CONFIG_SHELL} \
                     "${CT_SRC_DIR}/automake/configure" \
                     --host="${host}" \
                     --prefix="${prefix}"

    CT_DoLog EXTRA "Building automake"
    CT_DoExecLog ALL make

    CT_DoLog EXTRA "Installing automake"
    CT_DoExecLog ALL make install
}
