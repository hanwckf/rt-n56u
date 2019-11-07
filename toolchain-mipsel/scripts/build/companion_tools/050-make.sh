# Build script for make

do_companion_tools_make_get()
{
    CT_Fetch MAKE
}

do_companion_tools_make_extract()
{
    CT_ExtractPatch MAKE
}

do_companion_tools_make_for_build()
{
    CT_DoStep INFO "Installing make for build"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-make-build"
    do_make_backend \
        host=${CT_BUILD} \
        prefix="${CT_BUILD_COMPTOOLS_DIR}" \
        cflags="${CT_CFLAGS_FOR_BUILD}" \
        ldflags="${CT_LDFLAGS_FOR_BUILD}"
    CT_Popd
    if [ "${CT_MAKE_GMAKE_SYMLINK}" = "y" ]; then
        CT_DoExecLog ALL ln -sv make "${CT_BUILD_COMPTOOLS_DIR}/bin/gmake"
    fi
    if [ "${CT_MAKE_GNUMAKE_SYMLINK}" = "y" ]; then
        CT_DoExecLog ALL ln -sv make "${CT_BUILD_COMPTOOLS_DIR}/bin/gnumake"
    fi
    CT_EndStep
}

do_companion_tools_make_for_host()
{
    CT_DoStep INFO "Installing make for host"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-make-host"
    do_make_backend \
        host=${CT_HOST} \
        prefix="${CT_PREFIX_DIR}" \
        cflags="${CT_CFLAGS_FOR_HOST}" \
        ldflags="${CT_LDFLAGS_FOR_HOST}"
    CT_Popd
    if [ "${CT_MAKE_GMAKE_SYMLINK}" = "y" ]; then
        CT_DoExecLog ALL ln -sv make "${CT_PREFIX_DIR}/bin/gmake"
    fi
    if [ "${CT_MAKE_GNUMAKE_SYMLINK}" = "y" ]; then
        CT_DoExecLog ALL ln -sv make "${CT_PREFIX_DIR}/bin/gnumake"
    fi
    CT_EndStep
}

do_make_backend()
{
    local host
    local prefix
    local cflags
    local ldflags
    local -a extra_config

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    if [ "${host}" != "${CT_BUILD}" ]; then
        extra_config+=( --without-guile )
    fi

    CT_DoLog EXTRA "Configuring make"
    CT_DoExecLog CFG \
                     CFLAGS="${cflags}" \
                     LDFLAGS="${ldflags}" \
                     ${CONFIG_SHELL} \
                     "${CT_SRC_DIR}/make/configure" \
                     --host="${host}" \
                     --prefix="${prefix}" \
		     "${extra_config[@]}"

    CT_DoLog EXTRA "Building make"
    CT_DoExecLog ALL make

    CT_DoLog EXTRA "Installing make"
    CT_DoExecLog ALL make install
}
