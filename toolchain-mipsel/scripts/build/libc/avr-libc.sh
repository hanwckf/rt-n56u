# This file adds functions to build the avr-libc C library

avr_libc_post_cc()
{
    CT_DoStep INFO "Installing C library"

    CT_DoLog EXTRA "Copying sources to build directory"
    CT_DoExecLog ALL cp -av "${CT_SRC_DIR}/avr-libc/." \
                            "${CT_BUILD_DIR}/build-libc-post-cc"
    cd "${CT_BUILD_DIR}/build-libc-post-cc"

    CT_DoLog EXTRA "Configuring C library"

    CT_DoExecLog CFG                \
    ${CONFIG_SHELL}                 \
    ./configure                     \
        --build=${CT_BUILD}         \
        --host=${CT_TARGET}         \
        --prefix=${CT_PREFIX_DIR}   \
        "${CT_LIBC_AVR_LIBC_EXTRA_CONFIG_ARRAY[@]}"

    CT_DoLog EXTRA "Building C library"
    CT_DoExecLog ALL make ${CT_JOBSFLAGS}

    CT_DoLog EXTRA "Installing C library"
    CT_DoExecLog ALL make install

    CT_EndStep
}
