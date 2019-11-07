# Parses all samples on the command line, and for each of them, prints
# the versions of the main tools

# Use tools discovered by ./configure
. "${CT_LIB_DIR}/scripts/functions"

[ "$1" = "-v" ] && opt="$1" && shift

# GREP_OPTIONS screws things up.
export GREP_OPTIONS=

fieldwidth=15

# Dummy version which is invoked from .config
CT_Mirrors() { :; }

# Dump a short package description with a name and version in a format
# " <name>[-<version>]"
dump_pkgs_desc()
{
    local category="${1}"
    local field="${2}"
    local pkgs
    shift 2
    local show_version
    local tmp p

    eval "pkgs=\"\${CT_ALL_${category}_CHOICES}\""
    printf "    %-*s :" ${fieldwidth} "${field}"
    for p in ${pkgs}; do
        # FIXME: multiple choices use category_package; single choice
        # use category_package for the primary selection and category_package_SHOW
        # for all other selections enabled by the primary. Cannot unify this syntax
        # without a really extensive change.
        eval "tmp=\"\${CT_${category}_${p}}\${CT_${category}_${p}_SHOW}\""
        if [ -n "${tmp}" ]; then
            CT_GetPkgBuildVersion "${category}" "${p}" show_version
            printf " %s" "${show_version}"
        fi
    done
    printf "\n"
}

# Dump a single sample
# Note: we use the specific .config.sample config file
dump_single_sample()
{
    local verbose=0
    local complibs
    [ "$1" = "-v" ] && verbose=1 && shift
    local sample="$1"
    . $(pwd)/.config.sample

    case "${sample}" in
        current)
            sample_type="l"
            sample="$( ${CT_NG} show-tuple )"
            case "${CT_TOOLCHAIN_TYPE}" in
                canadian)
                    sample="${CT_HOST},${sample}"
                    ;;
            esac
            ;;
        *)  if [ -f "${CT_TOP_DIR}/samples/${sample}/crosstool.config" ]; then
                sample_top="${CT_TOP_DIR}"
                sample_type="L"
            else
                sample_top="${CT_LIB_DIR}"
                sample_type="G"
            fi
            ;;
    esac
    printf "[%s" "${sample_type}"
    [ -f "${sample_top}/samples/${sample}/broken" ] && printf "B" || printf "."
    [ "${CT_CONFIG_VERSION}" != "${CT_CONFIG_VERSION_CURRENT}" ] && printf "O" || printf "."
    [ "${CT_EXPERIMENTAL}" = "y" ] && printf "X" || printf "."
    printf "]   %s\n" "${sample}"
    if [ ${verbose} -ne 0 ]; then
        case "${CT_TOOLCHAIN_TYPE}" in
            cross)  ;;
            canadian)
                printf "    %-*s : %s\n" ${fieldwidth} "Host" "${CT_HOST}"
                ;;
        esac
        printf "    %-*s : %s" ${fieldwidth} "Languages" "C"
        [ "${CT_CC_LANG_CXX}" = "y"     ] && printf ",C++"
        [ "${CT_CC_LANG_FORTRAN}" = "y" ] && printf ",Fortran"
        [ "${CT_CC_LANG_JAVA}" = "y"    ] && printf ",Java"
        [ "${CT_CC_LANG_ADA}" = "y"     ] && printf ",ADA"
        [ "${CT_CC_LANG_OBJC}" = "y"    ] && printf ",Objective-C"
        [ "${CT_CC_LANG_OBJCXX}" = "y"  ] && printf ",Objective-C++"
        [ "${CT_CC_LANG_GOLANG}" = "y"  ] && printf ",Go"
        [ -n "${CT_CC_LANG_OTHERS}"     ] && printf ",${CT_CC_LANG_OTHERS}"
        printf "\n"

        dump_pkgs_desc KERNEL "OS"
        dump_pkgs_desc BINUTILS "Binutils"
        dump_pkgs_desc CC "Compiler"
        dump_pkgs_desc LIBC "C library"
        dump_pkgs_desc DEBUG "Debug tools"
        dump_pkgs_desc COMP_LIBS "Companion libs"
        dump_pkgs_desc COMP_TOOLS "Companion tools"
    fi
}

for sample in "${@}"; do
    dump_single_sample ${opt} "${sample}"
done
