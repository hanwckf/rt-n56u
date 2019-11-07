# Build script for gettext

do_gettext_get() { :; }
do_gettext_extract() { :; }
do_gettext_for_build() { :; }
do_gettext_for_host() { :; }
do_gettext_for_target() { :; }

if [ "${CT_GETTEXT}" = "y" ]; then

do_gettext_get() {
    CT_Fetch GETTEXT
}

do_gettext_extract() {
    CT_ExtractPatch GETTEXT
}

# Build gettext for running on build
do_gettext_for_build() {
    local -a gettext_opts

    case "${CT_TOOLCHAIN_TYPE}" in
        native|cross)   return 0;;
    esac

    CT_DoStep INFO "Installing gettext for build"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-gettext-build-${CT_BUILD}"

    gettext_opts+=( "host=${CT_BUILD}" )
    gettext_opts+=( "prefix=${CT_BUILDTOOLS_PREFIX_DIR}" )
    gettext_opts+=( "cflags=${CT_CFLAGS_FOR_BUILD}" )
    gettext_opts+=( "ldflags=${CT_LDFLAGS_FOR_BUILD}" )
    do_gettext_backend "${gettext_opts[@]}"

    CT_Popd
    CT_EndStep
}

# Build gettext for running on host
do_gettext_for_host() {
    local -a gettext_opts

    CT_DoStep INFO "Installing gettext for host"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-gettext-host-${CT_HOST}"

    gettext_opts+=( "host=${CT_HOST}" )
    gettext_opts+=( "prefix=${CT_HOST_COMPLIBS_DIR}" )
    gettext_opts+=( "cflags=${CT_CFLAGS_FOR_HOST}" )
    gettext_opts+=( "ldflags=${CT_LDFLAGS_FOR_HOST}" )
    do_gettext_backend "${gettext_opts[@]}"

    CT_Popd
    CT_EndStep
}

# Build gettext
#     Parameter     : description               : type      : default
#     host          : machine to run on         : tuple     : (none)
#     prefix        : prefix to install into    : dir       : (none)
#     shared        : build shared lib          : bool      : no
#     cflags        : host cflags to use        : string    : (empty)
#     ldflags       : host ldflags to use       : string    : (empty)
do_gettext_backend() {
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

        # A bit ugly. D__USE_MINGW_ANSI_STDIO=1 has its own {v}asprintf functions
        # but gettext configure doesn't see this flag when it checks for that. An
        # alternative may be to use CC="${host}-gcc ${cflags}" but that didn't
        # work.
        # -O2 works around bug at http://savannah.gnu.org/bugs/?36443
        # gettext needs some fixing for MinGW-w64 it would seem.
        # -DLIBXML_STATIC needed to link with libxml (provided by gnulib) under
        # MinGW: without this flag, xmlFree is defined as `dllimport` by libxml
        # headers and hence fails to link.
        *mingw*)
            case "${cflags}" in
                *D__USE_MINGW_ANSI_STDIO=1*)
                    extra_config+=( --disable-libasprintf )
                    ;;
            esac
            extra_config+=( --enable-threads=win32 )
            cflags=$cflags" -O2 -DLIBXML_STATIC"
        ;;
    esac

    if [ "${shared}" != "y" ]; then
        extra_config+=("--disable-shared")
    fi

    CT_DoLog EXTRA "Configuring gettext"

    CT_DoExecLog CFG                                        \
    CFLAGS="${cflags}"                                      \
    LDFLAGS="${ldflags}"                                    \
    ${CONFIG_SHELL}                                         \
    "${CT_SRC_DIR}/gettext/configure"                       \
        --build=${CT_BUILD}                                 \
        --host="${host}"                                    \
        --prefix="${prefix}"                                \
        --enable-static                                     \
        --disable-java                                      \
        --disable-native-java                               \
        --disable-csharp                                    \
        --without-emacs                                     \
        --disable-openmp                                    \
        --with-included-libxml                              \
        --with-included-gettext                             \
        --with-included-glib                                \
        --with-included-libcroco                            \
        --with-included-libunistring                        \
        --with-libncurses-prefix="${prefix}"                \
        --with-libiconv-prefix="${prefix}"                  \
        --without-libpth-prefix                             \
        "${extra_config[@]}"

    CT_DoLog EXTRA "Building gettext"
    CT_DoExecLog ALL make ${CT_JOBSFLAGS}

    CT_DoLog EXTRA "Installing gettext"
    CT_DoExecLog ALL make install
}

fi
