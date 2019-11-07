# Build script for strace

do_debug_strace_get()
{
    CT_Fetch STRACE
}

do_debug_strace_extract()
{
    CT_ExtractPatch STRACE
}

do_debug_strace_build()
{
    local cflags="${CT_ALL_TARGET_CFLAGS}"

    CT_DoStep INFO "Installing strace"

    if [ "${CT_LIBC_MUSL}" = "y" ]; then
        # Otherwise kernel headers cause errors when included, e.g.
        # <netinet/in.h> and <linux/in6.h>. Kernel's libc-compat.h
        # only cares about GLIBC.  uClibc-ng does the same
        # internally, pretending it's GLIBC for kernel headers inclusion.
        cflags+=" -D__GLIBC__"
    fi

    CT_mkdir_pushd "${CT_BUILD_DIR}/build-strace"

    CT_DoLog EXTRA "Configuring strace"
    CT_DoExecLog CFG                                           \
    CC="${CT_TARGET}-${CT_CC}"                                 \
    CFLAGS="${cflags}"                                         \
    LDFLAGS="${CT_ALL_TARGET_LDFLAGS}"                         \
    CPP="${CT_TARGET}-cpp"                                     \
    LD="${CT_TARGET}-ld"                                       \
    ${CONFIG_SHELL}                                            \
    "${CT_SRC_DIR}/strace/configure"                           \
        --build=${CT_BUILD}                                    \
        --host=${CT_TARGET}                                    \
        --prefix=/usr                                          \
        --enable-mpers=check

    CT_DoLog EXTRA "Building strace"
    CT_DoExecLog ALL make

    CT_DoLog EXTRA "Installing strace"
    CT_DoExecLog ALL make DESTDIR="${CT_DEBUGROOT_DIR}" install

    CT_Popd
    CT_EndStep
}

