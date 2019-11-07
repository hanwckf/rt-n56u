# Build script for ncurses

do_ncurses_get() { :; }
do_ncurses_extract() { :; }
do_ncurses_for_build() { :; }
do_ncurses_for_host() { :; }
do_ncurses_for_target() { :; }

if [ "${CT_NCURSES_TARGET}" = "y" -o "${CT_NCURSES}" = "y" ]; then

do_ncurses_get() {
    CT_Fetch NCURSES
}

do_ncurses_extract() {
    CT_ExtractPatch NCURSES
}

# We need tic that runs on the build when building ncurses for host/target
do_ncurses_for_build() {
    local -a opts

    CT_DoStep INFO "Installing ncurses for build"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-ncurses-build-${CT_BUILD}"
    opts=("--enable-symlinks" \
          "--without-manpages" \
          "--without-tests" \
          "--without-cxx" \
          "--without-cxx-binding" \
          "--without-ada" \
          "--without-fallbacks" )
    do_ncurses_backend host="${CT_BUILD}" \
                       destdir="${CT_BUILDTOOLS_PREFIX_DIR}" \
                       cflags="${CT_CFLAGS_FOR_BUILD}" \
                       ldflags="${CT_LDFLAGS_FOR_BUILD}" \
                       install_target=install.progs \
                       "${opts[@]}"
    CT_Popd
    CT_EndStep
}

if [ "${CT_NCURSES}" = "y" ]; then
do_ncurses_for_host() {
    local -a opts

    CT_DoStep INFO "Installing ncurses for host"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-ncurses-host-${CT_HOST}"
    opts=("--enable-symlinks" \
          "--without-manpages" \
          "--without-tests" \
          "--without-cxx" \
          "--without-cxx-binding" \
          "--without-ada" )
    if [ "${CT_NCURSES_HOST_DISABLE_DB}" = "y" ]; then
        opts+=( "--disable-database" \
                "--disable-db-install" )
    fi
    if [ -n "${CT_NCURSES_HOST_FALLBACKS}" ]; then
        opts+=( "--with-fallbacks=${CT_NCURSES_HOST_FALLBACKS}" )
    fi
    opts+=( "${CT_NCURSES_HOST_CONFIG_ARGS[@]}" )
    do_ncurses_backend host="${CT_HOST}" \
                       prefix="${CT_HOST_COMPLIBS_DIR}" \
                       cflags="${CT_CFLAGS_FOR_HOST}" \
                       ldflags="${CT_LDFLAGS_FOR_HOST}" \
                       "${opts[@]}"
    CT_Popd
    CT_EndStep
}
fi

if [ "${CT_NCURSES_TARGET}" = "y" ]; then
do_ncurses_for_target() {
    local prefix

    CT_DoStep INFO "Installing ncurses for target"
    CT_mkdir_pushd "${CT_BUILD_DIR}/build-ncurses-target-${CT_TARGET}"
    opts=("--without-sysmouse")
    [ "${CT_CC_LANG_CXX}" = "y" ] || opts+=("--without-cxx" "--without-cxx-binding")
    [ "${CT_CC_LANG_ADA}" = "y" ] || opts+=("--without-ada")
    if [ "${CT_NCURSES_TARGET_DISABLE_DB}" = "y" ]; then
        opts+=( "--disable-database" )
    fi
    if [ -n "${CT_NCURSES_TARGET_FALLBACKS}" ]; then
        opts+=( "--with-fallbacks=${CT_NCURSES_TARGET_FALLBACKS}" )
    fi
    opts+=( "${CT_NCURSES_TARGET_CONFIG_ARGS[@]}" )
    case "${CT_TARGET}" in
        *-*-mingw*)
            prefix="/mingw"
            ;;
        *)
            prefix="/usr"
            ;;
    esac
    do_ncurses_backend host="${CT_TARGET}" \
                       prefix="${prefix}" \
                       destdir="${CT_SYSROOT_DIR}" \
                       shared="${CT_SHARED_LIBS}" \
                       cflags="${CT_ALL_TARGET_CFLAGS}" \
                       "${opts[@]}"
    CT_Popd
    CT_EndStep
}
fi

# Build libncurses
#   Parameter     : description               : type      : default
#   host          : machine to run on         : tuple     : (none)
#   prefix        : prefix to install into    : dir       : (none)
#   cflags        : cflags to use             : string    : (empty)
#   ldflags       : ldflags to use            : string    : (empty)
#   shared        : build shared lib          : bool      : n
#   --*           : passed to configure       : n/a       : n/a
do_ncurses_backend() {
    local -a ncurses_opts
    local host
    local prefix
    local cflags
    local ldflags
    local shared
    local arg
    local install_target=install

    for arg in "$@"; do
        case "$arg" in
            --*)
                ncurses_opts+=("${arg}")
                ;;
            *)
                eval "${arg// /\\ }"
                ;;
        esac
    done

    if [ "${CT_NCURSES_NEW_ABI}" != "y" ]; then
        ncurses_opts+=("--with-abi-version=5")
    fi

    case "${host}" in
        *-*-mingw*)
            # Needed to build for mingw, see
            # http://lists.gnu.org/archive/html/info-gnu/2011-02/msg00020.html
            ncurses_opts+=("--enable-term-driver")
            ncurses_opts+=("--enable-sp-funcs")
            ;;
    esac

    if [ "${shared}" = "y" ]; then
        ncurses_opts+=("--with-shared")
    fi

    CT_DoLog EXTRA "Configuring ncurses"
    CT_DoExecLog CFG                                                    \
    CFLAGS="${cflags}"                                                  \
    LDFLAGS="${ldflags}"                                                \
    ${CONFIG_SHELL}                                                     \
    "${CT_SRC_DIR}/ncurses/configure"                                   \
        --build=${CT_BUILD}                                             \
        --host=${host}                                                  \
        --prefix="${prefix}"                                            \
        --with-install-prefix="${destdir}"                              \
        --without-debug                                                 \
        --enable-termcap                                                \
        "${ncurses_opts[@]}"

    # FIXME: old ncurses build code was removing -static from progs/Makefile,
    # claiming static linking does not work on MacOS. A knowledge base article
    # (https://developer.apple.com/library/mac/qa/qa1118/_index.html) says that
    # static linking works just fine, just do not use it for libc (or other
    # libraries that make system calls). ncurses use -static only for linking
    # the curses library, then switches back to -dynamic - so they should be fine.
    # FIXME: for target, we only need tic (terminfo compiler). However, building
    # it also builds ncurses anyway, and dedicated targets (install.includes and
    # install.progs) do not do well with parallel make (-jX).
    CT_DoLog EXTRA "Building ncurses"
    CT_DoExecLog ALL make ${CT_JOBSFLAGS}

    # STRIPPROG is handled by our wrapper around install.
    CT_DoLog EXTRA "Installing ncurses"
    CT_DoExecLog ALL \
        STRIPPROG="${host}-strip" \
        make "${install_target}"
}

fi
