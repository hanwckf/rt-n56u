# Build script for ltrace

do_debug_ltrace_get() {
    CT_Fetch LTRACE
}

do_debug_ltrace_extract() {
    CT_ExtractPatch LTRACE
}

do_debug_ltrace_build() {
    local ltrace_host

    CT_DoStep INFO "Installing ltrace"

    CT_DoLog EXTRA "Copying sources to build dir"
    CT_DoExecLog ALL cp -av "${CT_SRC_DIR}/ltrace/." \
                            "${CT_BUILD_DIR}/build-ltrace"
    CT_Pushd "${CT_BUILD_DIR}/build-ltrace"

    CT_DoLog EXTRA "Configuring ltrace"
    # ltrace-0.5.3 has a unique hand-crafted configure script. Releases
    # 0.5.2 and earlier as well as 0.6.0 and later use GNU autotools.
    if [ "${LTRACE_0_5_3_CONFIGURE}" = "y" ]; then
        case "${CT_ARCH}:${CT_ARCH_BITNESS}" in
            x86:32)     ltrace_host="i386";;
            x86:64)     ltrace_host="x86_64";;
            powerpc:*)  ltrace_host="ppc";;
            mips:*)     ltrace_host="mipsel";;
            *)          ltrace_host="${CT_ARCH}";;
        esac
        CT_DoExecLog CFG                \
        CC="${CT_TARGET}-${CT_CC}"      \
        AR="${CT_TARGET}-ar"            \
        HOST="${ltrace_host}"           \
        HOST_OS="${CT_TARGET_KERNEL}"   \
        CFLAGS="${CT_ALL_TARGET_CFLAGS}"\
        ${CONFIG_SHELL}                 \
        ./configure --prefix=/usr
    else
        CT_DoExecLog CFG        \
        ${CONFIG_SHELL}         \
        ./configure             \
            --build=${CT_BUILD} \
            --host=${CT_TARGET} \
            --prefix=/usr
    fi

    CT_DoLog EXTRA "Building ltrace"
    CT_DoExecLog ALL make

    CT_DoLog EXTRA "Installing ltrace"
    CT_DoExecLog ALL make DESTDIR="${CT_DEBUGROOT_DIR}" install

    CT_Popd
    CT_EndStep
}
