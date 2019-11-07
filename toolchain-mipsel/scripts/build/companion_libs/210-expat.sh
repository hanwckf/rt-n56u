# Build script for expat

do_expat_get() { :; }
do_expat_extract() { :; }
do_expat_for_build() { :; }
do_expat_for_host() { :; }
do_expat_for_target() { :; }

if [ "${CT_EXPAT_TARGET}" = "y" -o "${CT_EXPAT}" = "y" ]; then

do_expat_get() {
    CT_Fetch EXPAT
}

do_expat_extract() {
    CT_ExtractPatch EXPAT
}

if [ "${CT_EXPAT}" = "y" ]; then
# Do not need expat for build at this time.

do_expat_for_host() {
    local -a expat_opts

    CT_DoStep INFO "Installing expat for host"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-expat-host-${CT_HOST}"

    expat_opts+=( "host=${CT_HOST}" )
    expat_opts+=( "prefix=${CT_HOST_COMPLIBS_DIR}" )
    expat_opts+=( "cflags=${CT_CFLAGS_FOR_HOST}" )
    expat_opts+=( "ldflags=${CT_LDFLAGS_FOR_HOST}" )

    do_expat_backend "${expat_opts[@]}"

    CT_Popd
    CT_EndStep
}
fi

if [ "${CT_EXPAT_TARGET}" = "y" ]; then
do_expat_for_target() {
    local -a expat_opts
    local prefix

    CT_DoStep INFO "Installing expat for target"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-expat-target-${CT_TARGET}"

    expat_opts+=( "host=${CT_TARGET}" )
    case "${CT_TARGET}" in
        *-*-mingw*)
            prefix="/mingw"
            ;;
        *)
            prefix="/usr"
            ;;
    esac
    expat_opts+=( "cflags=${CT_ALL_TARGET_CFLAGS}" )
    expat_opts+=( "prefix=${prefix}" )
    expat_opts+=( "destdir=${CT_SYSROOT_DIR}" )
    expat_opts+=( "shared=${CT_SHARED_LIBS}" )

    do_expat_backend "${expat_opts[@]}"

    CT_Popd
    CT_EndStep
}
fi

# Build libexpat
#   Parameter     : description               : type      : default
#   host          : machine to run on         : tuple     : (none)
#   prefix        : prefix to install into    : dir       : (none)
#   destdir       : install destination       : dir       : (none)
do_expat_backend() {
    local host
    local prefix
    local cflags
    local ldflags
    local shared
    local arg
    local -a extra_config

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    if [ "${shared}" != "y" ]; then
        extra_config+=("--disable-shared")
    fi

    CT_DoLog EXTRA "Configuring expat"

    CT_DoExecLog CFG                                                \
    CFLAGS="${cflags}"                                              \
    LDFLAGS="${ldflags}"                                            \
    ${CONFIG_SHELL}                                                 \
    "${CT_SRC_DIR}/expat/configure"                                 \
        --build=${CT_BUILD}                                         \
        --host=${host}                                              \
        --prefix="${prefix}"                                        \
        --enable-static                                             \
        --without-docbook                                           \
        "${extra_config[@]}"

    CT_DoLog EXTRA "Building expat"
    CT_DoExecLog ALL make ${CT_JOBSFLAGS}
    CT_DoLog EXTRA "Installing expat"
    CT_DoExecLog ALL make install DESTDIR="${destdir}"
}

fi
