# Build script for libelf

do_libelf_get() { :; }
do_libelf_extract() { :; }
do_libelf_for_build() { :; }
do_libelf_for_host() { :; }
do_libelf_for_target() { :; }

if [ "${CT_LIBELF}" = "y" -o "${CT_LIBELF_TARGET}" = "y" ]; then

do_libelf_get() {
    CT_Fetch LIBELF
}

do_libelf_extract() {
    CT_ExtractPatch LIBELF
}

if [ "${CT_LIBELF}" = "y" ]; then

# Build libelf for running on build
# - always build statically
# - install in build-tools prefix
do_libelf_for_build() {
    local -a libelf_opts

    case "${CT_TOOLCHAIN_TYPE}" in
        native|cross)   return 0;;
    esac

    CT_DoStep INFO "Installing libelf for build"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-libelf-build-${CT_BUILD}"

    libelf_opts+=( "host=${CT_BUILD}" )
    libelf_opts+=( "prefix=${CT_BUILDTOOLS_PREFIX_DIR}" )
    libelf_opts+=( "cflags=${CT_CFLAGS_FOR_BUILD}" )
    libelf_opts+=( "ldflags=${CT_LDFLAGS_FOR_BUILD}" )
    do_libelf_backend "${libelf_opts[@]}"

    CT_Popd
    CT_EndStep
}

# Build libelf for running on host
do_libelf_for_host() {
    local -a libelf_opts

    CT_DoStep INFO "Installing libelf for host"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-libelf-host-${CT_HOST}"

    libelf_opts+=( "host=${CT_HOST}" )
    libelf_opts+=( "prefix=${CT_HOST_COMPLIBS_DIR}" )
    libelf_opts+=( "cflags=${CT_CFLAGS_FOR_HOST}" )
    libelf_opts+=( "ldflags=${CT_LDFLAGS_FOR_HOST}" )
    do_libelf_backend "${libelf_opts[@]}"

    CT_Popd
    CT_EndStep
}

fi # CT_LIBELF

if [ "${CT_LIBELF_TARGET}" = "y" ]; then

do_libelf_for_target() {
    local -a libelf_opts
    local prefix

    CT_DoStep INFO "Installing libelf for the target"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-libelf-target-${CT_TARGET}"

    case "${CT_TARGET}" in
        *-*-mingw*)
            prefix="/mingw"
            ;;
        *)
            prefix="/usr"
            ;;
    esac

    libelf_opts+=( "destdir=${CT_SYSROOT_DIR}" )
    libelf_opts+=( "host=${CT_TARGET}" )

    libelf_opts+=( "cflags=${CT_ALL_TARGET_CFLAGS}" )
    libelf_opts+=( "prefix=${prefix}" )
    libelf_opts+=( "shared=${CT_SHARED_LIBS}" )
    do_libelf_backend "${libelf_opts[@]}"

    CT_Popd
    CT_EndStep
}

fi # CT_LIBELF_TARGET

# Build libelf
#     Parameter     : description               : type      : default
#     destdir       : out-of-tree install dir   : string    : /
#     host          : machine to run on         : tuple     : (none)
#     prefix        : prefix to install into    : dir       : (none)
#     cflags        : cflags to use             : string    : (empty)
#     ldflags       : ldflags to use            : string    : (empty)
#     shared        : also buils shared lib     : bool      : n
do_libelf_backend() {
    local destdir="/"
    local host
    local prefix
    local cflags
    local ldflags
    local shared
    local -a extra_config
    local arg

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    CT_DoLog EXTRA "Configuring libelf"

    if [ "${shared}" = "y" ]; then
        extra_config+=( --enable-shared )
    else
        extra_config+=( --disable-shared )
    fi

    CT_DoExecLog CFG                                        \
    CC="${host}-gcc"                                        \
    RANLIB="${host}-ranlib"                                 \
    CFLAGS="${cflags} -fPIC"                                \
    LDFLAGS="${ldflags}"                                    \
    ${CONFIG_SHELL}                                         \
    "${CT_SRC_DIR}/libelf/configure"                        \
        --build=${CT_BUILD}                                 \
        --host=${host}                                      \
        --target=${CT_TARGET}                               \
        --prefix="${prefix}"                                \
        --enable-compat                                     \
        --enable-elf64                                      \
        --enable-extended-format                            \
        --enable-static                                     \
        "${extra_config[@]}"

    CT_DoLog EXTRA "Building libelf"
    CT_DoExecLog ALL make

    CT_DoLog EXTRA "Installing libelf"

    # Guard against $destdir$prefix == //
    # which is a UNC path on Cygwin/MSYS2
    if [[ ${destdir} == / ]] && [[ ${prefix} == /* ]]; then
        destdir=
    fi

    CT_DoExecLog ALL make instroot="${destdir}" install
}

fi # CT_LIBELF || CT_LIBELF_TARGET
