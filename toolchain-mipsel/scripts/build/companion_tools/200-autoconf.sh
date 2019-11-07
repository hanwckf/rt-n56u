# Build script for autoconf

do_companion_tools_autoconf_get()
{
    CT_Fetch AUTOCONF
}

do_companion_tools_autoconf_extract()
{
    CT_ExtractPatch AUTOCONF
}

do_companion_tools_autoconf_for_build()
{
    CT_DoStep INFO "Installing autoconf for build"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-autoconf-build"
    do_autoconf_backend host=${CT_BUILD} prefix="${CT_BUILD_COMPTOOLS_DIR}"
    CT_Popd
    CT_EndStep
}

do_companion_tools_autoconf_for_host()
{
    CT_DoStep INFO "Installing autoconf for host"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-autoconf-host"
    do_autoconf_backend host=${CT_HOST} prefix="${CT_PREFIX_DIR}"
    CT_Popd
    CT_EndStep
}

do_autoconf_backend()
{
    local host
    local prefix

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    # Ensure configure gets run using the CONFIG_SHELL as configure seems to
    # have trouble when CONFIG_SHELL is set and /bin/sh isn't bash
    # For reference see:
    # http://www.gnu.org/software/autoconf/manual/autoconf.html#CONFIG_005fSHELL
    CT_DoLog EXTRA "Configuring autoconf"
    CT_DoExecLog CFG \
    ${CONFIG_SHELL} \
    "${CT_SRC_DIR}/autoconf/configure" \
                     --host="${host}" \
                     --prefix="${prefix}"

    CT_DoLog EXTRA "Building autoconf"
    CT_DoExecLog ALL make

    CT_DoLog EXTRA "Installing autoconf"
    CT_DoExecLog ALL make install
}
