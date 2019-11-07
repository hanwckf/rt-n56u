# Build script for libiconv

do_libiconv_get() { :; }
do_libiconv_extract() { :; }
do_libiconv_for_build() { :; }
do_libiconv_for_host() { :; }
do_libiconv_for_target() { :; }

if [ "${CT_LIBICONV}" = "y" ]; then

do_libiconv_get() {
    CT_Fetch LIBICONV
}

do_libiconv_extract() {
    CT_ExtractPatch LIBICONV
}

# Build libiconv for running on build
do_libiconv_for_build() {
    local -a libiconv_opts

    case "${CT_TOOLCHAIN_TYPE}" in
        native|cross)   return 0;;
    esac

    CT_DoStep INFO "Installing libiconv for build"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-libiconv-build-${CT_BUILD}"

    libiconv_opts+=( "host=${CT_BUILD}" )
    libiconv_opts+=( "prefix=${CT_BUILDTOOLS_PREFIX_DIR}" )
    libiconv_opts+=( "cflags=${CT_CFLAGS_FOR_BUILD}" )
    libiconv_opts+=( "ldflags=${CT_LDFLAGS_FOR_BUILD}" )
    do_libiconv_backend "${libiconv_opts[@]}"

    CT_Popd
    CT_EndStep
}

# Build libiconv for running on host
do_libiconv_for_host() {
    local -a libiconv_opts

    CT_DoStep INFO "Installing libiconv for host"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-libiconv-host-${CT_HOST}"

    libiconv_opts+=( "host=${CT_HOST}" )
    libiconv_opts+=( "prefix=${CT_HOST_COMPLIBS_DIR}" )
    libiconv_opts+=( "cflags=${CT_CFLAGS_FOR_HOST}" )
    libiconv_opts+=( "ldflags=${CT_LDFLAGS_FOR_HOST}" )
    do_libiconv_backend "${libiconv_opts[@]}"

    CT_Popd
    CT_EndStep
}

# Build libiconv
#     Parameter     : description               : type      : default
#     host          : machine to run on         : tuple     : (none)
#     prefix        : prefix to install into    : dir       : (none)
#     shared        : build shared lib          : bool      : no
#     cflags        : host cflags to use        : string    : (empty)
#     ldflags       : host ldflags to use       : string    : (empty)
do_libiconv_backend() {
    local host
    local prefix
    local shared
    local cflags
    local ldflags
    local arg
    local -a extra_config

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    case "${host}" in
        *-linux-gnu*)
            CT_DoLog EXTRA "Skipping (included in GNU C library)"
            return
            ;;
    esac

    if [ "${shared}" != "y" ]; then
        extra_config+=("--disable-shared")
    fi

    CT_DoLog EXTRA "Configuring libiconv"

    CT_DoExecLog CFG                                          \
    CFLAGS="${cflags}"                                        \
    LDFLAGS="${ldflags}"                                      \
    ${CONFIG_SHELL}                                           \
    "${CT_SRC_DIR}/libiconv/configure"                        \
        --build=${CT_BUILD}                                   \
        --host="${host}"                                      \
        --prefix="${prefix}"                                  \
        --enable-static                                       \
        --disable-nls                                         \
        "${extra_config[@]}"                                  \

    CT_DoLog EXTRA "Building libiconv"
    CT_DoExecLog ALL make CC="${host}-gcc ${cflags}" ${CT_JOBSFLAGS}

    CT_DoLog EXTRA "Installing libiconv"
    CT_DoExecLog ALL make install CC="${host}-gcc ${cflags}"
}

fi
