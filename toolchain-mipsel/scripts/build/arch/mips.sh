# Compute MIPS-specific values

CT_DoArchTupleValues() {
    # The architecture part of the tuple
    CT_TARGET_ARCH="${CT_ARCH}${target_bits_64}${CT_ARCH_SUFFIX:-${target_endian_el}}"

    # Override CFLAGS for endianness:
    case "${CT_ARCH_ENDIAN}" in
        big)    CT_ARCH_ENDIAN_CFLAG="-EB";;
        little) CT_ARCH_ENDIAN_CFLAG="-EL";;
    esac

    # Override ABI flags
    CT_ARCH_ABI_CFLAG="-mabi=${CT_ARCH_mips_ABI}"
    CT_ARCH_WITH_ABI="--with-abi=${CT_ARCH_mips_ABI}"
}

CT_DoArchUClibcConfig() {
    local cfg="${1}"

    CT_DoArchUClibcSelectArch "${cfg}" "${CT_ARCH}"

    CT_KconfigDisableOption "CONFIG_MIPS_O32_ABI" "${cfg}"
    CT_KconfigDisableOption "CONFIG_MIPS_N32_ABI" "${cfg}"
    CT_KconfigDisableOption "CONFIG_MIPS_N64_ABI" "${cfg}"
    case "${CT_ARCH_mips_ABI}" in
        32)
            CT_KconfigEnableOption "CONFIG_MIPS_O32_ABI" "${cfg}"
            ;;
        n32)
            CT_KconfigEnableOption "CONFIG_MIPS_N32_ABI" "${cfg}"
            ;;
        64)
            CT_KconfigEnableOption "CONFIG_MIPS_N64_ABI" "${cfg}"
            ;;
    esac

    # FIXME: uClibc (!ng) allows to select ISA in the config; should
    # match from the selected ARCH_ARCH level... For now, delete and
    # fall back to default.
    CT_KconfigDeleteOption "CONFIG_MIPS_ISA_1" "${cfg}"
    CT_KconfigDeleteOption "CONFIG_MIPS_ISA_2" "${cfg}"
    CT_KconfigDeleteOption "CONFIG_MIPS_ISA_3" "${cfg}"
    CT_KconfigDeleteOption "CONFIG_MIPS_ISA_4" "${cfg}"
    CT_KconfigDeleteOption "CONFIG_MIPS_ISA_MIPS32" "${cfg}"
    CT_KconfigDeleteOption "CONFIG_MIPS_ISA_MIPS32R2" "${cfg}"
    CT_KconfigDeleteOption "CONFIG_MIPS_ISA_MIPS64" "${cfg}"
    CT_KconfigDeleteOption "CONFIG_MIPS_ISA_MIPS64R2" "${cfg}"
}

CT_DoArchUClibcHeaderDir() {
    local dir_var="${1}"
    local cflags="${2}"

    # If it is non-default multilib, add a suffix with architecture (reported by gcc)
    # to the headers installation path.
    if [ -n "${cflags}" ]; then
        eval "${dir_var}="$( ${CT_TARGET}-${CT_CC} -print-multiarch ${cflags} )
    fi
}

CT_DoArchUClibcCflags() {
    local cfg="${1}"
    local cflags="${2}"
    local f

    for f in ${cflags}; do
        case "${f}" in
            -mabi=*)
                CT_KconfigDisableOption "CONFIG_MIPS_O32_ABI" "${cfg}"
                CT_KconfigDisableOption "CONFIG_MIPS_N32_ABI" "${cfg}"
                CT_KconfigDisableOption "CONFIG_MIPS_N64_ABI" "${cfg}"
                case "${f#-mabi=}" in
                    32)  CT_KconfigEnableOption "CONFIG_MIPS_O32_ABI" "${cfg}";;
                    n32) CT_KconfigEnableOption "CONFIG_MIPS_N32_ABI" "${cfg}";;
                    64)  CT_KconfigEnableOption "CONFIG_MIPS_N64_ABI" "${cfg}";;
                    *)   CT_Abort "Unsupported ABI: ${f#-mabi=}";;
                esac
                ;;
        esac
    done
}

CT_DoArchMUSLPostInstall() {
    # GDB and MUSL maintainers seem to disagree on whether <sgidefs.h>
    # is to be provided as a part of C library. GDB guys think it is
    # a C library responsibility, while MUSL authors think GDB should
    # not be using <sgidefs.h>. Neither side is willing to reach out
    # to the other and negotiate the needed changes, and I don't want
    # to play the middle man. Hence, provide our own wrapper for
    # for <sgidefs.h> - the only solution short of telling MUSL users
    # stop using it. This is why MUSL is experimental in ct-ng and
    # will likely remain in that status.
    # References:
    #   http://www.openwall.com/lists/musl/2017/01/26/2
    #   https://sourceware.org/ml/gdb-patches/2017-01/msg00469.html
    #   https://www.sourceware.org/ml/libc-alpha/2004-11/msg00034.html
    if [ ! -r "${CT_HEADERS_DIR}/sgidefs.h" ]; then
        echo "#include <asm/sgidefs.h> // Redirected by ct-ng" > "${CT_HEADERS_DIR}/sgidefs.h"
    fi
}
