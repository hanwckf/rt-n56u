# Functions to build the moxiebox runtime.

. "${CT_LIB_DIR}/scripts/build/libc/newlib.sh"

moxiebox_get()
{
    CT_Fetch NEWLIB
    CT_Fetch MOXIEBOX
}

moxiebox_extract()
{
    CT_ExtractPatch NEWLIB
    CT_ExtractPatch MOXIEBOX
}

moxiebox_start_files()
{
    newlib_start_files
}

moxiebox_main()
{
    newlib_main
}

moxiebox_post_cc()
{
    CT_DoStep INFO "Installing moxiebox runtime and VM"

    CT_mkdir_pushd "${CT_BUILD_DIR}/build-libc-moxiebox"
    CT_DoExecLog ALL cp -av "${CT_SRC_DIR}/moxiebox/." .

    CT_DoLog EXTRA "Building SHA256-only libcrypto"
    # Moxiebox needs libcrypto on the host, but it only uses SHA256 digest functions
    # from it. We don't want to pull the whole OpenSSL for the host; fortunately,
    # moxiebox comes with a standalone SHA256 implementation - which it only uses
    # for the target library. Help it use the same implementation for the host.
    CT_mkdir_pushd openssl
    CT_DoExecLog ALL cp -v "${CT_LIB_DIR}/packages/moxiebox/"sha*.[ch] ./
    CT_DoExecLog ALL "${CT_HOST}-gcc" -c sha256_wrap.c -O2 -Wall
    CT_DoExecLog ALL "${CT_HOST}-ar" cru libcrypto.a sha256_wrap.o
    CT_Popd

    # Moxiebox includes a VM which we're building for the
    # host machine.
    CT_DoLog EXTRA "Configuring moxiebox"

    CT_DoExecLog CFG ./autogen.sh

    # moxiebox build script creates symlinks from the installation location to the build
    # directory for the moxiebox library. This seems backwards. Instead, pass the search
    # as part of the MOX_GCC definition.
    # moxiebox also depends on the tools being named moxiebox-{gcc,as,ar}. However, failure
    # to detect such tools is non-fatal in the configure and we need to override it in
    # make's command line anyway.
    CT_DoExecLog CFG \
            LDFLAGS="${CT_LDFLAGS_FOR_HOST} -L${CT_BUILD_DIR}/build-libc-moxiebox/openssl" \
            CFLAGS="${CT_CFLAGS_FOR_HOST} -I${CT_BUILD_DIR}/build-libc-moxiebox" \
            CXXFLAGS="${CT_CFLAGS_FOR_HOST} -I${CT_BUILD_DIR}/build-libc-moxiebox" \
            ./configure \
            --host="${CT_HOST}"
    CT_DoLog EXTRA "Building moxiebox"
    CT_DoExecLog CFG make all \
            MOX_GCC="${CT_TARGET}-gcc -B ${CT_BUILD_DIR}/build-libc-moxiebox/runtime -B ${CT_SYSROOT_DIR}/lib" \
            MOX_AS="${CT_TARGET}-as" \
            MOX_AR="${CT_TARGET}-ar"

    CT_DoLog EXTRA "Installing moxiebox"

    # moxiebox does not have install target. Copy the interesting stuff manually.
    CT_DoExecLog ALL cp -v "${CT_BUILD_DIR}/build-libc-moxiebox/runtime/libsandboxrt.a" \
            "${CT_BUILD_DIR}/build-libc-moxiebox/runtime/crt0.o" \
            "${CT_SYSROOT_DIR}/lib/"
    CT_DoExecLog ALL cp -v "${CT_BUILD_DIR}/build-libc-moxiebox/src/sandbox" \
            "${CT_PREFIX_DIR}/bin"
    CT_Popd
    CT_EndStep
}
