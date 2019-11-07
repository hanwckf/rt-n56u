# Compute powerpc-specific values

CT_DoArchTupleValues () {
    # The architecture part of the tuple
    CT_TARGET_ARCH="powerpc${target_bits_64}${target_endian_le}${CT_ARCH_SUFFIX}"

    # Only override values when ABI is not the default
    case "${CT_ARCH_powerpc_ABI}" in
        eabi)
            # EABI is only for bare-metal, so libc ∈ [none,newlib]
            CT_TARGET_SYS="eabi"
            ;;
        spe)
            case "${CT_LIBC}" in
                none|newlib)    CT_TARGET_SYS="elfspe";;
                *glibc)         CT_TARGET_SYS="gnuspe";;
                uClibc)         CT_TARGET_SYS="uclibcgnuspe";;
            esac
            ;;
    esac

    # Add extra flags for SPE if needed. SPE is obsolete in GCC8.
    if [ "${CT_ARCH_powerpc_ABI_SPE}" = "y" ]; then
        CT_ARCH_TARGET_CFLAGS="-mabi=spe -mspe"
        CT_ARCH_CC_CORE_EXTRA_CONFIG="--enable-e500_double --enable-obsolete"
        CT_ARCH_CC_EXTRA_CONFIG="--enable-e500_double --enable-obsolete"
    fi
}

#------------------------------------------------------------------------------
# Get multilib architecture-specific target
# Usage: CT_DoArchMultilibTarget "target variable" "multilib flags"
CT_DoArchMultilibTarget ()
{
    local target_var="${1}"; shift
    local -a multi_flags=( "$@" )
    local target_

    local m32=false
    local m64=false
    local mlittle=false
    local mbig=false

    for m in "${multi_flags[@]}"; do
        case "$m" in
            -m32)     m32=true ;;
            -m64)     m64=true ;;
            -mbig)    mbig=true ;;
            -mlittle) mlittle=true ;;
        esac
    done

    eval target_=\"\${${target_var}}\"

    # Fix up bitness
    case "${target_}" in
        powerpc-*)      $m64 && target_=${target_/#powerpc-/powerpc64-} ;;
        powerpcle-*)    $m64 && target_=${target_/#powerpcle-/powerpc64le-} ;;
        powerpc64-*)    $m32 && target_=${target_/#powerpc64-/powerpc-} ;;
        powerpc64le-*)  $m32 && target_=${target_/#powerpc64le-/powerpcle-} ;;
    esac

    # Fix up endianness
    case "${target_}" in
        powerpc-*)      $mlittle && target_=${target_/#powerpc-/powerpcle-} ;;
        powerpcle-*)    $mbig && target_=${target_/#powerpcle-/powerpc-} ;;
        powerpc64-*)    $mlittle && target_=${target_/#powerpc64-/powerpc64le-} ;;
        powerpc64le-*)  $mbig && target_=${target_/#powerpc64le-/powerpc64-} ;;
    esac

    # Set the target variable
    eval ${target_var}=\"${target_}\"
}

CT_DoArchUClibcConfig() {
    local cfg="${1}"

    CT_DoArchUClibcSelectArch "${cfg}" "powerpc"

    CT_KconfigDisableOption "CONFIG_E500" "${cfg}"
    CT_KconfigDisableOption "CONFIG_CLASSIC" "${cfg}"
    CT_KconfigDeleteOption "TARGET_SUBARCH" "${cfg}"
    if [ "${CT_ARCH_powerpc_ABI}" = "spe" ]; then
        CT_KconfigEnableOption "CONFIG_E500" "${cfg}"
        CT_KconfigSetOption "TARGET_SUBARCH" "e500" "${cfg}"
    else
        CT_KconfigEnableOption "CONFIG_CLASSIC" "${cfg}"
        CT_KconfigSetOption "TARGET_SUBARCH" "classic" "${cfg}"
    fi
}
