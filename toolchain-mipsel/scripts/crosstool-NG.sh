# Copyright 2007 Yann E. MORIN
# Licensed under the GPL v2. See COPYING in the root of this package.

# This is the main entry point to crosstool
# This will:
#   - download, extract and patch the toolchain components
#   - build and install each components in turn
#   - and eventually test the resulting toolchain

# What this file does is prepare the environment, based upon the user-choosen
# options. It also checks the existing environment for un-friendly variables,
# and builds the tools.

# Parse the common functions
# Note: some initialisation and sanitizing is done while parsing this file,
# most notably:
#  - set trap handler on errors,
#  - don't hash commands lookups,
. "${CT_LIB_DIR}/scripts/functions"

# Read the sample settings
CT_LoadConfig

# Yes! We can do full logging from now on! Clean any old log file content.
CT_LogEnable clean=yes

# Check running as root
if [ -z "${CT_ALLOW_BUILD_AS_ROOT_SURE}" ]; then
    if [ $(id -u) -eq 0 ]; then
        CT_DoLog ERROR "You must NOT be root to run crosstool-NG"
        exit 1
    fi
fi

CT_TestAndAbort "Invalid configuration. Run 'ct-ng menuconfig' and check which options select INVALID_CONFIGURATION." -n "${CT_INVALID_CONFIGURATION}"

# If we want an interactive debug-shell, we must ensure these FDs
# are indeed connected to a terminal (and not redirected in any way).
if [ "${CT_DEBUG_INTERACTIVE}" = "y" -a ! \( -t 0 -a -t 6 -a -t 2 \) ]; then
    CT_DoLog ERROR "Can't spawn interactive debug-shell,"
    CT_DoLog ERROR "because stdout/stderr has been redirected."
    exit 1
fi

CT_TrapEnvExport

# Override the locale early, in case we ever translate crosstool-NG messages
if [ -z "${CT_NO_OVERRIDE_LC_MESSAGES}" ]; then
    export LC_ALL=C
    export LANG=C
fi

# remove . from PATH since it can cause gcc build failures
CT_SanitizePath

# Some sanity checks in the environment and needed tools
CT_DoLog INFO "Performing some trivial sanity checks"
CT_TestAndAbort "Don't set LD_LIBRARY_PATH. It screws up the build." -n "${LD_LIBRARY_PATH+set}"
CT_TestAndAbort "Don't set LIBRARY_PATH. It screws up the build." -n "${LIBRARY_PATH+set}"
CT_TestAndAbort "Don't set LPATH. It screws up the build." -n "${LPATH+set}"
CT_TestAndAbort "Don't set CPATH. It screws up the build." -n "${CPATH+set}"
CT_TestAndAbort "Don't set C_INCLUDE_PATH. It screws up the build." -n "${C_INCLUDE_PATH+set}"
CT_TestAndAbort "Don't set CPLUS_INCLUDE_PATH. It screws up the build." -n "${CPLUS_INCLUDE_PATH+set}"
CT_TestAndAbort "Don't set OBJC_INCLUDE_PATH. It screws up the build." -n "${OBJC_INCLUDE_PATH+set}"
CT_TestAndAbort "Don't set CFLAGS. It screws up the build." -n "${CFLAGS+set}"
CT_TestAndAbort "Don't set CXXFLAGS. It screws up the build." -n "${CXXFLAGS+set}"
CT_TestAndAbort "Don't set CC. It screws up the build." -n "${CC+set}"
CT_TestAndAbort "Don't set CXX. It screws up the build." -n "${CXX+set}"
CT_Test "GREP_OPTIONS screws up the build. Unsetting." -n "${GREP_OPTIONS+set}"
unset GREP_OPTIONS
# Workaround against openSUSE 12.1 that breaks ./configure for cross-compilation:
export CONFIG_SITE=

# Some sanity checks on paths content
for d in            \
    LOCAL_TARBALLS  \
    WORK            \
    PREFIX          \
    BUILD_TOP       \
    INSTALL         \
    ; do
        eval dir="\${CT_${d}_DIR}"
        case "${dir}" in
            *" "*)
                CT_Abort "'CT_${d}_DIR'='${dir}' contains a space in it.\nDon't use spaces in paths, it breaks things."
                ;;
            *:*)
                CT_Abort "'CT_${d}_DIR'='${dir}' contains a colon in it.\nDon't use colons in paths, it breaks things."
                ;;
            *,*)
                CT_Abort "'CT_${d}_DIR'='${dir}' contains a comma in it.\nDon't use commas in paths, it breaks things."
                ;;
        esac
        case "${dir}" in
            /*)
                # Absolute path, okay
                ;;
            *)
                # Relative path from CT_TOP_DIR, make absolute
                eval CT_${d}_DIR="${CT_TOP_DIR}/${dir}"
                # Having .. inside CT_PREFIX breaks relocatability.
                CT_SanitizeVarDir CT_${d}_DIR
                ;;
        esac
done

n_open_files=$(ulimit -n)
if [ "${n_open_files}" -lt 2048 ]; then
    # Newer ld seems to keep a lot of open file descriptors, hitting the default limit
    # (1024) for example during uClibc-ng link.
    CT_DoLog WARN "Number of open files ${n_open_files} may not be sufficient to build the toolchain; increasing to 2048"
    ulimit -n 2048
fi

# Where will we work?
CT_WORK_DIR="${CT_WORK_DIR:-${CT_TOP_DIR}/.build}"
CT_BUILD_DIR="${CT_BUILD_TOP_DIR}/build"
CT_DoExecLog ALL mkdir -p "${CT_WORK_DIR}"
CT_DoExecLog DEBUG rm -f "${CT_WORK_DIR}/backtrace"

# Check build file system case-sensitiveness
CT_DoExecLog DEBUG touch "${CT_WORK_DIR}/foo"
CT_TestAndAbort "Your file system in '${CT_WORK_DIR}' is *not* case-sensitive!" -f "${CT_WORK_DIR}/FOO"
CT_DoExecLog DEBUG rm -f "${CT_WORK_DIR}/foo"

# Check the user is using an existing SHELL to be used by ./configure and Makefiles
CT_TestOrAbort "The CONFIG_SHELL '${CT_CONFIG_SHELL}' is not valid" -f "${CT_CONFIG_SHELL}" -a -x "${CT_CONFIG_SHELL}"

# Create the bin-override early
# Contains symlinks to the tools found by ./configure
# Note: CT_DoLog and CT_DoExecLog do not use any of those tool, so
# they can be safely used
CT_TOOLS_OVERRIDE_DIR="${CT_WORK_DIR}/tools"
CT_DoLog DEBUG "Creating bin-override for tools in '${CT_TOOLS_OVERRIDE_DIR}'"
CT_DoExecLog DEBUG mkdir -p "${CT_TOOLS_OVERRIDE_DIR}/bin"
cat "${paths_sh_location}" |while read trash line; do
    tool="${line%%=*}"
    # Suppress extra quoting
    eval path=${line#*=}
    if [ ! -r "${CT_LIB_DIR}/scripts/override/$tool" ]; then
         if [ -n "${path}" ]; then
             CT_DoExecLog ALL rm -f "${CT_TOOLS_OVERRIDE_DIR}/bin/${tool}"
             CT_DoExecLog ALL ln -s "${path}" "${CT_TOOLS_OVERRIDE_DIR}/bin/${tool}"
         fi
         continue
    fi
    tmpl="${CT_LIB_DIR}/scripts/override/$tool"
    CT_DoLog DEBUG "Creating script-override for '${tool}' -> '${path}' using '${tmpl}' template"
    CT_DoExecLog ALL cp "${tmpl}" "${CT_TOOLS_OVERRIDE_DIR}/bin/${tool}"
    CT_DoExecLog ALL ${sed} -i -r \
              -e "s#@INSTALL_WITH_STRIP_PROGRAM@#${CT_CONFIGURE_has_install_with_strip_program}#g" \
              -e "s#@CONFIG_SHELL@#${CT_CONFIG_SHELL}#g" \
              -e "s#@TOOL_PATH@#${path}#g" \
              -e "s#@TOOLS_OVERRIDE_DIR@#${CT_TOOLS_OVERRIDE_DIR}#g" \
              "${CT_TOOLS_OVERRIDE_DIR}/bin/${tool}"
    CT_DoExecLog ALL chmod 700 "${CT_TOOLS_OVERRIDE_DIR}/bin/${tool}"
done
export PATH="${CT_TOOLS_OVERRIDE_DIR}/bin:${PATH}"

# Start date. Can't be done until we know the locale
# Also requires the bin-override tools
CT_STAR_DATE=$(CT_DoDate +%s%N)
CT_STAR_DATE_HUMAN=$(CT_DoDate +%Y%m%d.%H%M%S)

# Log real begining of build, now
CT_DoLog INFO "Build started ${CT_STAR_DATE_HUMAN}"

CT_DoStep DEBUG "Dumping user-supplied crosstool-NG configuration"
CT_DoExecLog DEBUG ${grep} -E '^(# )?CT_' .config
CT_EndStep

CT_DoLog DEBUG "Unsetting MAKEFLAGS"
unset MAKEFLAGS

# Set the shell to be used by ./configure scripts and by Makefiles (those
# that support it!).
export CONFIG_SHELL="${CT_CONFIG_SHELL}"    # for ./configure
export SHELL="${CT_CONFIG_SHELL}"           # for Makefiles

CT_DoLog INFO "Building environment variables"

# Sanity check some directories
CT_TestAndAbort "'CT_PREFIX_DIR' is not set: where should I install?" -z "${CT_PREFIX_DIR}"

# Avoid multiple '/' in the prefix dir, it breaks relocatability
CT_PREFIX_DIR="$( ${sed} -r -e 's:/+:/:g; s:/*$::;' <<<"${CT_PREFIX_DIR}" )"

# Second kludge: merge user-supplied target CFLAGS with architecture-provided
# target CFLAGS. Do the same for LDFLAGS in case it happens in the future.
# Put user-supplied flags at the end, so that they take precedence.
CT_ALL_TARGET_CFLAGS="${CT_ARCH_TARGET_CFLAGS} ${CT_TARGET_CFLAGS}"
CT_ALL_TARGET_LDFLAGS="${CT_ARCH_TARGET_LDFLAGS} ${CT_TARGET_LDFLAGS}"

# FIXME move to gcc.sh
CT_CC_GCC_CORE_EXTRA_CONFIG_ARRAY=( ${CT_ARCH_CC_CORE_EXTRA_CONFIG} "${CT_CC_GCC_CORE_EXTRA_CONFIG_ARRAY[@]}" )
CT_CC_GCC_EXTRA_CONFIG_ARRAY=( ${CT_ARCH_CC_EXTRA_CONFIG} "${CT_CC_GCC_EXTRA_CONFIG_ARRAY[@]}" )

# Starting with 1.0.20, applications using uClibc-ng do not link with
# the default libgcc_c_spec used by GCC if only static libc.a exists - unless
# -static is thrown in. The difference is that with -static, gcc passes
# "--start-group -lgcc -lc --end-group" and without -static, it passes
# "-lgcc -lc -lgcc" instead. The latter leaves a symbol from 2nd libgcc
# (dl_iterate_phdr) unresolved because -lc is already done at this point.
# Force static link on the target.
if [ "${CT_SHARED_LIBS}" != "y" ]; then
    CT_TARGET_LDFLAGS+=" -static"
fi

# Compute the package version string
if [ "${CT_SHOW_CT_VERSION}" = "y" ]; then
    CT_PKGVERSION="crosstool-NG ${CT_VERSION}${CT_TOOLCHAIN_PKGVERSION:+ - ${CT_TOOLCHAIN_PKGVERSION}}"
else
    CT_PKGVERSION="${CT_TOOLCHAIN_PKGVERSION}"
fi

# Compute the working directories names
CT_TARBALLS_DIR="${CT_WORK_DIR}/tarballs"
CT_COMMON_SRC_DIR="${CT_WORK_DIR}/src"
CT_SRC_DIR="${CT_BUILD_TOP_DIR}/src"
CT_BUILDTOOLS_PREFIX_DIR="${CT_BUILD_TOP_DIR}/buildtools"
CT_STATE_DIR="${CT_BUILD_TOP_DIR}/state"
# Note about HOST_COMPLIBS_DIR: it's always gonna be in the buildtools dir, or a
# sub-dir. So we won't have to save/restore it, not even create it.
# In case of cross or native, host-complibs are used for build-complibs;
# in case of canadian or cross-native, host-complibs are specific
# Note about BUILD_COMPTOOLS_DIR: if installing companion tools for "host" in
# a native or simple cross, we can can use the same binaries we built for
# "build". However, we need companion tools for "build" early - as other
# components may depend on them - so we may skip building for "host" rather
# than for "build" in that case.
case "${CT_TOOLCHAIN_TYPE}" in
    native|cross)
        CT_HOST_COMPLIBS_DIR="${CT_BUILDTOOLS_PREFIX_DIR}"
        if [ -n "${CT_COMP_TOOLS_FOR_HOST}" ]; then
            CT_BUILD_COMPTOOLS_DIR="${CT_PREFIX_DIR}"
        else
            CT_BUILD_COMPTOOLS_DIR="${CT_BUILDTOOLS_PREFIX_DIR}"
        fi
        ;;
    canadian|cross-native)
        CT_HOST_COMPLIBS_DIR="${CT_BUILDTOOLS_PREFIX_DIR}/complibs-host"
        CT_BUILD_COMPTOOLS_DIR="${CT_BUILDTOOLS_PREFIX_DIR}"
        ;;
esac

# Compute test suite install directory
CT_TEST_SUITE_DIR=${CT_PREFIX_DIR}/test-suite

# We must ensure that we can restart if asked for!
if [ -n "${CT_RESTART}" -a ! -d "${CT_STATE_DIR}"  ]; then
    CT_DoLog ERROR "You asked to restart a non-restartable build"
    CT_DoLog ERROR "This happened because you didn't set CT_DEBUG_CT_SAVE_STEPS"
    CT_DoLog ERROR "in the config options for the previous build, or the state"
    CT_DoLog ERROR "directory for the previous build was deleted."
    CT_Abort "I will stop here to avoid any carnage"
fi

# If the local tarball directory does not exist, say so, and don't try to save there!
if [    "${CT_SAVE_TARBALLS}" = "y"     \
     -a ! -d "${CT_LOCAL_TARBALLS_DIR}" ]; then
    CT_DoLog WARN "Directory '${CT_LOCAL_TARBALLS_DIR}' does not exist."
    CT_DoLog WARN "Will not save downloaded tarballs to local storage."
    CT_SAVE_TARBALLS=
fi

# Good, now grab a bit of informations on the system we're being run on,
# just in case something goes awok, and it's not our fault:
CT_SYS_USER=$(id -un)
CT_SYS_HOSTNAME=$(hostname -f 2>/dev/null || true)
# Hmmm. Some non-DHCP-enabled machines do not have an FQDN... Fall back to node name.
CT_SYS_HOSTNAME="${CT_SYS_HOSTNAME:-$(uname -n)}"
CT_SYS_KERNEL=$(uname -s)
CT_SYS_REVISION=$(uname -r)
CT_SYS_OS=$(uname -s)
CT_SYS_MACHINE=$(uname -m)
CT_SYS_PROCESSOR=$(uname -p)
CT_SYS_GCC=$(${CT_BUILD_PREFIX}gcc${CT_BUILD_SUFFIX} -dumpversion)
CT_SYS_TARGET=$(CT_DoConfigGuess)
CT_TOOLCHAIN_ID="crosstool-${CT_VERSION} build ${CT_STAR_DATE_HUMAN} by ${CT_SYS_USER}@${CT_SYS_HOSTNAME}"

# Adjust the list of multilibs, if needed
CT_DoArchMultilibList

CT_DoLog EXTRA "Preparing working directories"

# Ah! The build directory shall be eradicated, even if we restart!
# Ditto for the build tools install dir
CT_DoForceRmdir "${CT_BUILD_DIR}" "${CT_BUILDTOOLS_PREFIX_DIR}"

# Don't eradicate directories if we need to restart
if [ -z "${CT_RESTART}" ]; then
    # Per-target sources: eliminate
    CT_DoForceRmdir "${CT_SRC_DIR}"
    # Get rid of pre-existing installed toolchain and previous build directories.
    if [ "${CT_FORCE_DOWNLOAD}" = "y" -a -d "${CT_TARBALLS_DIR}" ]; then
        CT_DoForceRmdir "${CT_TARBALLS_DIR}"
    fi
    if [ "${CT_FORCE_EXTRACT}" = "y" -a -d "${CT_COMMON_SRC_DIR}" ]; then
        CT_DoForceRmdir "${CT_COMMON_SRC_DIR}"
    fi
    if [ -d "${CT_PREFIX_DIR}" -a "${CT_RM_RF_PREFIX_DIR}" = "y" ]; then
        CT_DoForceRmdir "${CT_PREFIX_DIR}"
    fi
    # In case we start anew, get rid of the previously saved state directory
    if [ -d "${CT_STATE_DIR}" ]; then
        CT_DoForceRmdir "${CT_STATE_DIR}"
    fi
fi

# Create the directories we'll use, even if restarting: it does no harm to
# create already existent directories, and CT_BUILD_DIR needs to be created
# anyway
CT_DoExecLog ALL mkdir -p "${CT_TARBALLS_DIR}"
CT_DoExecLog ALL mkdir -p "${CT_COMMON_SRC_DIR}"
CT_DoExecLog ALL mkdir -p "${CT_SRC_DIR}"
CT_DoExecLog ALL mkdir -p "${CT_BUILD_DIR}"
CT_DoExecLog ALL mkdir -p "${CT_BUILDTOOLS_PREFIX_DIR}/bin"
CT_DoExecLog ALL mkdir -p "${CT_PREFIX_DIR}"
CT_DoExecLog ALL mkdir -p "${CT_HOST_COMPLIBS_DIR}"

# Only create the state dir if asked for a restartable build
[ -n "${CT_DEBUG_CT_SAVE_STEPS}" ] && CT_DoExecLog ALL mkdir -p "${CT_STATE_DIR}"

# Kludge: CT_PREFIX_DIR might have grown read-only if
# the previous build was successful.
CT_DoExecLog ALL chmod -R u+w "${CT_PREFIX_DIR}"

# Check install file system case-sensitiveness
CT_DoExecLog DEBUG touch "${CT_PREFIX_DIR}/foo"
CT_TestAndAbort "Your file system in '${CT_PREFIX_DIR}' is *not* case-sensitive!" -f "${CT_PREFIX_DIR}/FOO"
CT_DoExecLog DEBUG rm -f "${CT_PREFIX_DIR}/foo"

# Setting up the rest of the environment only if not restarting
if [ -z "${CT_RESTART}" ]; then
    case "${CT_SYSROOT_NAME}" in
        "")     CT_SYSROOT_NAME="sysroot";;
        .)      CT_Abort "Sysroot name is set to '.' which is forbidden";;
        *' '*)  CT_Abort "Sysroot name contains forbidden space(s): '${CT_SYSROOT_NAME}'";;
        */*)    CT_Abort "Sysroot name contains forbidden slash(es): '${CT_SYSROOT_NAME}'";;
    esac

    # Arrange paths depending on whether we use sysroot or not.
    if [ "${CT_USE_SYSROOT}" = "y" ]; then
        CT_SYSROOT_DIR="${CT_PREFIX_DIR}/${CT_TARGET}/${CT_SYSROOT_DIR_PREFIX}/${CT_SYSROOT_NAME}"
        CT_DEBUGROOT_DIR="${CT_PREFIX_DIR}/${CT_TARGET}/${CT_SYSROOT_DIR_PREFIX}/debug-root"
        CT_HEADERS_DIR="${CT_SYSROOT_DIR}/usr/include"
        CT_SanitizeVarDir CT_SYSROOT_DIR CT_DEBUGROOT_DIR CT_HEADERS_DIR
        CT_BINUTILS_SYSROOT_ARG="--with-sysroot=${CT_SYSROOT_DIR}"
        CT_CC_CORE_SYSROOT_ARG="--with-sysroot=${CT_SYSROOT_DIR}"
        CT_CC_SYSROOT_ARG="--with-sysroot=${CT_SYSROOT_DIR}"
        # glibc's prefix must be exactly /usr, else --with-sysroot'd gcc will get
        # confused when $sysroot/usr/include is not present.
        # Note: --prefix=/usr is magic!
        # See http://www.gnu.org/software/libc/FAQ.html#s-2.2
    else
        # plain old way. All libraries in prefix/target/lib
        CT_SYSROOT_DIR="${CT_PREFIX_DIR}/${CT_TARGET}"
        CT_DEBUGROOT_DIR="${CT_SYSROOT_DIR}"
        CT_HEADERS_DIR="${CT_SYSROOT_DIR}/include"
        CT_SanitizeVarDir CT_SYSROOT_DIR CT_DEBUGROOT_DIR CT_HEADERS_DIR
        # hack!  Always use --with-sysroot for binutils.
        # binutils 2.14 and later obey it, older binutils ignore it.
        # Lets you build a working 32->64 bit cross gcc
        CT_BINUTILS_SYSROOT_ARG="--with-sysroot=${CT_SYSROOT_DIR}"
        # Use --with-headers, else final gcc will define disable_glibc while
        # building libgcc, and you'll have no profiling
        CT_CC_CORE_SYSROOT_ARG="--without-headers"
        CT_CC_SYSROOT_ARG="--with-headers=${CT_HEADERS_DIR}"
    fi
    CT_DoExecLog ALL mkdir -p "${CT_SYSROOT_DIR}"
    CT_DoExecLog ALL mkdir -p "${CT_DEBUGROOT_DIR}"
    CT_DoExecLog ALL mkdir -p "${CT_HEADERS_DIR}"

    # Need the non-multilib directories: GCC's multi-os-directory is based off them, so
    # even if the /lib is not used for any of the multilibs, it must be present so that
    # the paths like 'lib/../lib64' still work.
    CT_DoExecLog ALL mkdir -p "${CT_PREFIX_DIR}/lib"
    CT_DoExecLog ALL mkdir -p "${CT_SYSROOT_DIR}/lib"
    CT_DoExecLog ALL mkdir -p "${CT_SYSROOT_DIR}/usr/lib"

    # Determine build system if not set by the user
    if [ -z "${CT_BUILD}" ]; then
        CT_BUILD=$(CT_DoConfigGuess)
    fi

    # Prepare mangling patterns to later modify BUILD and HOST (see below)
    case "${CT_TOOLCHAIN_TYPE}" in
        cross)
            # A cross-compiler runs on the same machine it is built on
            CT_HOST="${CT_BUILD}"
            build_mangle="build_"
            host_mangle="build_"
            target_mangle=""
            install_build_tools_for="BUILD"
            ;;
        canadian)
            build_mangle="build_"
            host_mangle="host_"
            target_mangle=""
            install_build_tools_for="BUILD HOST"
            ;;
        *)  CT_Abort "No code for '${CT_TOOLCHAIN_TYPE}' toolchain type!"
            ;;
    esac

    # Save the real tuples to generate shell-wrappers to the real tools
    CT_REAL_BUILD="${CT_BUILD}"
    CT_REAL_HOST="${CT_HOST}"
    CT_REAL_TARGET="${CT_TARGET}"

    # Canonicalise CT_BUILD and CT_HOST
    # Not only will it give us full-qualified tuples, but it will also ensure
    # that they are valid tuples (in case of typo with user-provided tuples)
    # That's way better than trying to rewrite config.sub ourselves...
    # CT_TARGET is already made canonical in CT_DoBuildTargetTuple
    CT_BUILD=$(CT_DoConfigSub "${CT_BUILD}")
    CT_HOST=$(CT_DoConfigSub "${CT_HOST}")

    # Modify BUILD and HOST so that gcc always generate a cross-compiler
    # even if any of the build, host or target machines are the same.
    # NOTE: we'll have to mangle the (BUILD|HOST)->TARGET x-compiler to
    #       support canadain build, later...
    CT_BUILD="${CT_BUILD/-/-${build_mangle}}"
    CT_HOST="${CT_HOST/-/-${host_mangle}}"
    CT_TARGET="${CT_TARGET/-/-${target_mangle}}"

    # Now we have mangled our BUILD and HOST tuples, we must fake the new
    # cross-tools for those mangled tuples.
    CT_DoLog DEBUG "Making build system tools available"
    for m in ${install_build_tools_for}; do
        r="CT_REAL_${m}"
        v="CT_${m}"
        p="CT_${m}_PREFIX"
        s="CT_${m}_SUFFIX"
        if [ -n "${!p}" ]; then
            t="${!p}"
        else
            t="${!r}-"
        fi

        for tool in ar as dlltool gcc g++ gcj gnatbind gnatmake ld nm objcopy objdump ranlib strip windres; do
            # First try with prefix + suffix
            # Then try with prefix only
            # Then try with suffix only, but only for BUILD, and HOST iff REAL_BUILD == REAL_HOST
            # Finally try with neither prefix nor suffix, but only for BUILD, and HOST iff REAL_BUILD == REAL_HOST
            # This is needed, because some tools have a prefix and
            # a suffix (eg. gcc), while others may have only one,
            # or even none (eg. binutils)
            where=$(CT_Which "${t}${tool}${!s}")
            [ -z "${where}" ] && where=$(CT_Which "${t}${tool}")
            if [    -z "${where}"                         \
                 -a \(    "${m}" = "BUILD"                \
                       -o "${CT_REAL_BUILD}" = "${!r}" \) ]; then
                where=$(CT_Which "${tool}${!s}")
            fi
            if [ -z "${where}"                            \
                 -a \(    "${m}" = "BUILD"                \
                       -o "${CT_REAL_BUILD}" = "${!r}" \) ]; then
                where=$(CT_Which "${tool}")
            fi

            # Not all tools are available for all platforms, but some are required.
            # TBD do we need these as shell wrappers? exec is slow on Cygwin, and this makes exec twice for each compiler/linker run
            if [ -n "${where}" ]; then
                CT_DoLog DEBUG "  '${!v}-${tool}' -> '${where}'"
                printf "#${BANG}${CT_CONFIG_SHELL}\nexec '${where}' \"\${@}\"\n" >"${CT_BUILDTOOLS_PREFIX_DIR}/bin/${!v}-${tool}"
                CT_DoExecLog ALL chmod 700 "${CT_BUILDTOOLS_PREFIX_DIR}/bin/${!v}-${tool}"
            else
                case "${tool}" in
                    # We'll at least need some of them...
                    ar|as|gcc|ld|nm|objcopy|objdump|ranlib)
                        CT_Abort "Missing: '${t}${tool}${!s}' or '${t}${tool}' or '${tool}' : either needed!"
                        ;;
                    # Some are conditionally required
                    # Add them in alphabetical (C locale) ordering
                    g++)
                        # g++ (needed for companion lib), only needed for HOST
                        CT_TestAndAbort "Missing: '${t}${tool}${!s}' or '${t}${tool}' or '${tool}' : either needed!" "${m}" = "HOST"
                        ;;
                    gcj)
                        CT_TestAndAbort "Missing: '${t}${tool}${!s}' or '${t}${tool}' or '${tool}' : either needed!" "${CT_CC_LANG_JAVA}" = "y"
                        ;;
                    strip)
                        CT_TestAndAbort "Missing: '${t}${tool}${!s}' or '${t}${tool}' or '${tool}' : either needed!" "${CT_STRIP_HOST_TOOLCHAIN_EXECUTABLES}" = "y"
                        ;;
                    # If any other is missing, only warn at low level
                    *)
                        # It does not deserve a WARN level.
                        CT_DoLog DEBUG "  Missing: '${t}${tool}${!s}' or '${t}${tool}' or '${tool}' : not required."
                        ;;
                esac
            fi
        done
    done

    # Some makeinfo versions are a pain in [put your most sensible body part here].
    # Go ahead with those, by creating a wrapper that keeps partial files, and that
    # never fails:
    CT_DoLog DEBUG "  'makeinfo' -> '$(CT_Which makeinfo)'"
    printf "#${BANG}${CT_CONFIG_SHELL}\n$(CT_Which makeinfo) --force \"\${@}\"\ntrue\n" >"${CT_BUILDTOOLS_PREFIX_DIR}/bin/makeinfo"
    CT_DoExecLog ALL chmod 700 "${CT_BUILDTOOLS_PREFIX_DIR}/bin/makeinfo"

    # Carefully add paths in the order we want them:
    #  - first try in ${CT_PREFIX_DIR}/bin
    #  - then try the buildtools dir
    #  - fall back to searching user's PATH
    # Of course, neither cross-native nor canadian can run on BUILD,
    # so don't add those PATHs in this case...
    # For native and simple cross, build==host, combine the extra CFLAGS/LDFLAGS
    # supplied for both (so that it doesn't matter where the user supplied them).
    case "${CT_TOOLCHAIN_TYPE}" in
        cross|native)
            export PATH="${CT_PREFIX_DIR}/bin:${CT_BUILDTOOLS_PREFIX_DIR}/bin:${PATH}"
            bh_cflags="${CT_EXTRA_CFLAGS_FOR_BUILD} ${CT_EXTRA_CFLAGS_FOR_HOST}"
            bh_ldflags="${CT_EXTRA_LDFLAGS_FOR_BUILD} ${CT_EXTRA_LDFLAGS_FOR_HOST}"
            CT_EXTRA_CFLAGS_FOR_BUILD="${bh_cflags}"
            CT_EXTRA_CFLAGS_FOR_HOST="${bh_cflags}"
            CT_EXTRA_LDFLAGS_FOR_BUILD="${bh_ldflags}"
            CT_EXTRA_LDFLAGS_FOR_HOST="${bh_ldflags}"
            ;;
        canadian|cross-native)
            export PATH="${CT_BUILDTOOLS_PREFIX_DIR}/bin:${PATH}"
            # build!=host in this case
            ;;
        *)
            ;;
    esac

    # Help build gcc
    # Explicitly optimise, else the lines below will overide the
    # package's default optimisation flags
    CT_CFLAGS_FOR_BUILD="-O2 -g -I${CT_BUILDTOOLS_PREFIX_DIR}/include"
    CT_CFLAGS_FOR_BUILD+=" ${CT_EXTRA_CFLAGS_FOR_BUILD}"
    CT_LDFLAGS_FOR_BUILD="-L${CT_BUILDTOOLS_PREFIX_DIR}/lib"
    CT_LDFLAGS_FOR_BUILD+=" ${CT_EXTRA_LDFLAGS_FOR_BUILD}"

    if ${CT_BUILD}-gcc --version 2>&1 | grep clang; then
        CT_CFLAGS_FOR_BUILD+=" -Qunused-arguments"
    fi
    case "${CT_BUILD}" in
        *darwin*)
            # Two issues while building on MacOS. Really, we should be checking for
            # clang instead.
            # - gettext static library fails to link unless CoreFoundation framework
            #   is included
            # - ranlib on MacOS does not include common symbols into the symbol index
            #   for a static library, and hence linker fails to pull in the right
            #   archive members; hence, avoid common symbols. Alternative is to
            #   have ranlib wrapper in buildtools/bin supply -c option.
            CT_CFLAGS_FOR_BUILD+=" -fno-common"
            CT_LDFLAGS_FOR_BUILD+=" -framework CoreFoundation"
            ;;
    esac

    CT_DoLog DEBUG "CFLAGS for build compiler: '${CT_CFLAGS_FOR_BUILD}'"
    CT_DoLog DEBUG "LDFLAGS for build compiler: '${CT_LDFLAGS_FOR_BUILD}'"

    # Help host gcc
    # Explicitly optimise, else the lines below will overide the
    # package's default optimisation flags
    CT_CFLAGS_FOR_HOST="-O2 -g"
    [ "${CT_USE_PIPES}" = "y" ] && CT_CFLAGS_FOR_HOST+=" -pipe"
    CT_CFLAGS_FOR_HOST+=" -I${CT_HOST_COMPLIBS_DIR}/include"
    CT_CFLAGS_FOR_HOST+=" ${CT_EXTRA_CFLAGS_FOR_HOST}"
    CT_LDFLAGS_FOR_HOST="-L${CT_HOST_COMPLIBS_DIR}/lib"
    CT_LDFLAGS_FOR_HOST+=" ${CT_EXTRA_LDFLAGS_FOR_HOST}"
    if ${CT_HOST}-gcc --version 2>&1 | grep clang; then
        CT_CFLAGS_FOR_HOST+=" -Qunused-arguments"
    fi
    case "${CT_HOST}" in
        *darwin*)
            # Same as above, for host
            CT_CFLAGS_FOR_HOST+=" -fno-common"
            CT_LDFLAGS_FOR_HOST+=" -framework CoreFoundation"
            ;;
    esac
    CT_DoLog DEBUG "CFLAGS for host compiler: '${CT_CFLAGS_FOR_HOST}'"
    CT_DoLog DEBUG "LDFLAGS for host compiler: '${CT_LDFLAGS_FOR_HOST}'"

    # And help make go faster
    CT_JOBSFLAGS=
    # Override the configured jobs with what's been given on the command line
    if [ -n "${CT_JOBS}" ]; then
        if [ ! -z "`echo "${CT_JOBS}" | ${sed} 's/[0-9]//g'`" ]; then
            CT_Abort "Number of parallel jobs must be integer."
        fi
        CT_PARALLEL_JOBS="${CT_JOBS}"
    fi
    # Use the number of processors+1 when automatically setting the number of
    # parallel jobs.
    AUTO_JOBS=$[ BUILD_NCPUS + 1 ]
    [ ${CT_PARALLEL_JOBS} -eq 0 ] && CT_JOBSFLAGS="${CT_JOBSFLAGS} -j${AUTO_JOBS}"
    [ ${CT_PARALLEL_JOBS} -gt 0 ] && CT_JOBSFLAGS="${CT_JOBSFLAGS} -j${CT_PARALLEL_JOBS}"
    CT_JOBSFLAGS="${CT_JOBSFLAGS} -l${CT_LOAD}"

    # Override 'download only' option
    [ -n "${CT_SOURCE}" ] && CT_ONLY_DOWNLOAD=y

    # Now that we've set up $PATH and $CT_CFLAGS_FOR_HOST, sanity test that gcc
    # is runnable so that the user can troubleshoot problems if not.
    CT_DoStep DEBUG "Checking that we can run gcc -v"
    CT_DoExecLog DEBUG "${CT_HOST}-gcc" -v
    CT_EndStep

    # Create a simple C program for testing.
    testc="${CT_BUILD_DIR}/test.c"
    printf "int main() { return 0; }\n" >"${testc}"
    gccout="${CT_BUILD_DIR}/.gccout"

    CT_DoStep DEBUG "Checking that gcc can compile a trivial program"
    CT_DoExecLog DEBUG "${CT_HOST}-gcc" ${CT_CFLAGS_FOR_HOST} ${CT_LDFLAGS_FOR_HOST} "${testc}" -o "${gccout}"
    rm -f "${gccout}"
    CT_EndStep

    # These tests are only enabled if we need static linking on the *build*
    if [ "${CT_WANTS_STATIC_LINK}" = "y" ]; then
        CT_DoStep DEBUG "Checking that gcc can compile a trivial statically linked program (CT_WANTS_STATIC_LINK)"
        CT_DoLog DEBUG "You may need to ensure that static libraries such as libc.a are installed on your system"
        CT_DoExecLog DEBUG "${CT_HOST}-gcc" ${CT_CFLAGS_FOR_BUILD} ${CT_LDFLAGS_FOR_BUILD} "${testc}" -static -o "${gccout}"
        rm -f "${gccout}"
        CT_EndStep
    fi
    if [ "${CT_WANTS_STATIC_LINK_CXX}" = "y" ]; then
        CT_DoStep DEBUG "Checking that gcc can statically link libstdc++ (CT_WANTS_STATIC_LINK_CXX)"
        CT_DoLog DEBUG "You may need to ensure that libstdc++.a is installed on your system"
        CT_DoExecLog DEBUG "${CT_HOST}-gcc" ${CT_CFLAGS_FOR_BUILD} ${CT_LDFLAGS_FOR_BUILD} "${testc}" -static -lstdc++ -o "${gccout}"
        rm -f "${gccout}"
        CT_EndStep
    fi
    rm -f "${testc}"

    CT_DoLog EXTRA "Installing user-supplied crosstool-NG configuration"
    CT_InstallConfigurationFile .config ct-ng

    CT_DoStep EXTRA "Dumping internal crosstool-NG configuration"
    CT_DoLog EXTRA "Building a toolchain for:"
    CT_DoLog EXTRA "  build  = ${CT_REAL_BUILD}"
    CT_DoLog EXTRA "  host   = ${CT_REAL_HOST}"
    CT_DoLog EXTRA "  target = ${CT_TARGET}"
    set |${grep} -E '^CT_.+=' |sort |CT_DoLog DEBUG
    CT_DoLog DEBUG "Other environment:"
    printenv |${grep} -v -E '^CT_.+=' |CT_DoLog DEBUG
    CT_EndStep

    CT_DoStep INFO "Retrieving needed toolchain components' tarballs"
    do_companion_tools_get
    do_kernel_get
    do_companion_libs_get
    do_binutils_get
    do_cc_get
    do_libc_get
    do_debug_get
    do_test_suite_get
    CT_EndStep

    if [ "${CT_ONLY_DOWNLOAD}" != "y" ]; then
        if [ "${CT_FORCE_EXTRACT}" = "y" ]; then
            CT_DoForceRmdir "${CT_SRC_DIR}"
            CT_DoExecLog ALL mkdir -p "${CT_SRC_DIR}"
        fi

        CT_DoStep INFO "Extracting and patching toolchain components"
        do_companion_tools_extract
        do_kernel_extract
        do_companion_libs_extract
        do_binutils_extract
        do_cc_extract
        do_libc_extract
        do_debug_extract
        do_test_suite_extract
        CT_EndStep
    fi
fi

# Now for the job by itself. Go have a coffee!
if [ "${CT_ONLY_DOWNLOAD}" != "y" -a "${CT_ONLY_EXTRACT}" != "y" ]; then
    # Because of CT_RESTART, this becomes quite complex
    do_stop=0
    prev_step=
    [ -n "${CT_RESTART}" ] && do_it=0 || do_it=1
    for step in ${CT_STEPS}; do
        if [ ${do_it} -eq 0 ]; then
            if [ "${CT_RESTART}" = "${step}" ]; then
                CT_DoLoadState "${step}"
                do_it=1
                do_stop=0
            fi
        else
            CT_DoSaveState ${step}
            if [ ${do_stop} -eq 1 ]; then
                CT_DoLog INFO "Stopping just after step '${prev_step}', as requested."
                exit 0
            fi
        fi
        if [ ${do_it} -eq 1 ]; then
            ( do_${step} )
            # POSIX 1003.1-2008 does not say if "set -e" should catch a
            # sub-shell ending with !0. bash-3 does not, while bash-4 does,
            # so the following line is for bash-3; bash-4 would choke above.
            [ $? -eq 0 ]
            # Pick up environment changes.
            if [ -r "${CT_BUILD_DIR}/env.modify.sh" ]; then
                CT_DoLog DEBUG "Step '${step}' modified the environment:"
                CT_DoExecLog DEBUG cat "${CT_BUILD_DIR}/env.modify.sh"
                . "${CT_BUILD_DIR}/env.modify.sh"
                CT_DoExecLog DEBUG rm -f "${CT_BUILD_DIR}/env.modify.sh"

            fi
            if [ "${CT_STOP}" = "${step}" ]; then
                do_stop=1
            fi
            if [ "${CT_DEBUG_PAUSE_STEPS}" = "y" ]; then
                CT_DoPause "Step '${step}' finished"
            fi
        fi
        prev_step="${step}"
    done
fi

CT_DoEnd INFO

# From now-on, it can become impossible to log any time, because
# either we're compressing the log file, or it can become RO any
# moment...
CT_DoLog INFO "Finishing installation (may take a few seconds)..."
CT_LogDisable
rm -f ${CT_PREFIX_DIR}/build.log.bz2
if [ "${CT_LOG_TO_FILE}" = "y" ]; then
    cp "${CT_BUILD_LOG}" "${CT_PREFIX_DIR}/build.log"
    if [ "${CT_LOG_FILE_COMPRESS}" = y ]; then
        bzip2 -9 "${CT_PREFIX_DIR}/build.log"
    fi
fi
if [ "${CT_PREFIX_DIR_RO}" = "y" ]; then
    chmod -R a-w "${CT_PREFIX_DIR}"
fi
# CT_TEST_SUITE_DIR may not exist if only downloading or extracting
if [ "${CT_TEST_SUITE}" = "y" -a -d "${CT_TEST_SUITE_DIR}" ]; then
    chmod -R u+w "${CT_TEST_SUITE_DIR}"
fi

trap - EXIT
