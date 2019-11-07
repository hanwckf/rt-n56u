# Compute ARM-specific values

CT_DoArchTupleValues() {
    # The architecture part of the tuple:
    case "${CT_ARCH_BITNESS}" in
        32)
            CT_TARGET_ARCH="${CT_ARCH}${CT_ARCH_SUFFIX:-${target_endian_eb}}"
            ;;
        64)
            # ARM 64 (aka AArch64) is special
            [ "${CT_ARCH_BE}" = "y" ] && target_endian_eb="_be"
            CT_TARGET_ARCH="aarch64${CT_ARCH_SUFFIX:-${target_endian_eb}}"
            ;;
    esac

    # The system part of the tuple:
    case "${CT_LIBC},${CT_ARCH_ARM_EABI}" in
        glibc,y)    CT_TARGET_SYS=gnueabi;;
        uClibc,y)   CT_TARGET_SYS=uclibc${CT_LIBC_UCLIBC_USE_GNU_SUFFIX:+gnu}eabi;;
        musl,y)     CT_TARGET_SYS=musleabi;;
        bionic,y)   CT_TARGET_SYS=androideabi;;
        *,y)        CT_TARGET_SYS=eabi;;
    esac

    # Set the default instruction set mode
    case "${CT_ARCH_ARM_MODE}" in
        arm)    ;;
        thumb)
            CT_ARCH_CC_CORE_EXTRA_CONFIG="--with-mode=thumb"
            CT_ARCH_CC_EXTRA_CONFIG="--with-mode=thumb"
            ;;
    esac

    if [ "${CT_ARCH_ARM_INTERWORKING}" = "y" ]; then
        CT_ARCH_TARGET_CFLAGS+=" -mthumb-interwork"
    fi

    if [ "${CT_ARCH_ARM_TUPLE_USE_EABIHF}" = "y" ]; then
        CT_TARGET_SYS="${CT_TARGET_SYS}hf"
    fi

    # If building multilib, zero out any WITH_*/*_CFLAG - GCC on ARM does not allow
    # any of them with multilib.
    if [ "${CT_MULTILIB}" = "y" ]; then
        CT_ARCH_WITH_ARCH=
        CT_ARCH_WITH_ABI=
        CT_ARCH_WITH_CPU=
        CT_ARCH_WITH_TUNE=
        CT_ARCH_WITH_FPU=
        CT_ARCH_WITH_FLOAT=
        CT_ARCH_ARCH_CFLAG=
        CT_ARCH_ABI_CFLAG=
        CT_ARCH_CPU_CFLAG=
        CT_ARCH_TUNE_CFLAG=
        CT_ARCH_FPU_CFLAG=
        CT_ARCH_FLOAT_CFLAG=
    fi
}

CT_DoArchUClibcConfig() {
    local cfg="${1}"

    if [ "${CT_ARCH_BITNESS}" = 64 ]; then
        CT_DoArchUClibcSelectArch "${cfg}" "aarch64"
    else
        CT_DoArchUClibcSelectArch "${cfg}" "arm"
        case "${CT_ARCH_ARM_MODE}" in
            arm)
                CT_KconfigDisableOption "COMPILE_IN_THUMB_MODE" "${cfg}"
                ;;
            thumb)
                CT_KconfigEnableOption "COMPILE_IN_THUMB_MODE" "${cfg}"
                ;;
        esac
        # FIXME: CONFIG_ARM_OABI does not exist in neither uClibc/uClibc-ng
        # FIXME: CONFIG_ARM_EABI does not seem to affect anything in either of them, too
        # (both check the compiler's built-in define, __ARM_EABI__ instead) except for
        # a check for match between toolchain configuration and uClibc-ng in
        # uClibc_arch_features.h
        if [ "${CT_ARCH_ARM_EABI}" = "y" ]; then
            CT_KconfigDisableOption "CONFIG_ARM_OABI" "${cfg}"
            CT_KconfigEnableOption "CONFIG_ARM_EABI" "${cfg}"
        else
            CT_KconfigDisableOption "CONFIG_ARM_EABI" "${cfg}"
            CT_KconfigEnableOption "CONFIG_ARM_OABI" "${cfg}"
        fi
    fi
}

CT_DoArchUClibcCflags() {
    local cfg="${1}"
    local cflags="${2}"
    local f

    for f in ${cflags}; do
        case "${f}" in
            -mthumb)
                CT_KconfigEnableOption "COMPILE_IN_THUMB_MODE" "${cfg}"
                CT_KconfigDisableOption "UCLIBC_HAS_CONTEXT_FUNCS" "${cfg}"
                ;;
            -marm)
                CT_KconfigDisableOption "COMPILE_IN_THUMB_MODE" "${cfg}"
                ;;
            -mlittle-endian)
                CT_KconfigDisableOption "ARCH_BIG_ENDIAN" "${cfg}"
                CT_KconfigDisableOption "ARCH_WANTS_BIG_ENDIAN" "${cfg}"
                CT_KconfigEnableOption "ARCH_LITTLE_ENDIAN" "${cfg}"
                CT_KconfigEnableOption "ARCH_WANTS_LITTLE_ENDIAN" "${cfg}"
                ;;
            -mbig-endian)
                CT_KconfigEnableOption "ARCH_BIG_ENDIAN" "${cfg}"
                CT_KconfigEnableOption "ARCH_WANTS_BIG_ENDIAN" "${cfg}"
                CT_KconfigDisableOption "ARCH_LITTLE_ENDIAN" "${cfg}"
                CT_KconfigDisableOption "ARCH_WANTS_LITTLE_ENDIAN" "${cfg}"
                ;;
            -mhard-float|-mfloat-abi=hard|-mfloat-abi=softfp)
                CT_KconfigEnableOption "UCLIBC_HAS_FPU" "${cfg}"
                ;;
            -msoft-float|-mfloat-abi=soft)
                CT_KconfigDisableOption "UCLIBC_HAS_FPU" "${cfg}"
                ;;
        esac
    done
}
