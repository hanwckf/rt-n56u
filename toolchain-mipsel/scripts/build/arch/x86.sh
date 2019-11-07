# Compute x86-specific values

# This one really needs a little love! :-(

CT_DoArchTupleValues() {
    # Override the architecture part of the tuple:
    if [ "${CT_ARCH_64}" = "y" ]; then
        CT_TARGET_ARCH=x86_64
    else
        arch="${CT_ARCH_ARCH}"
        [ -z "${arch}" ] && arch="${CT_ARCH_TUNE}"
        case "${arch}" in
            "")
                CT_DoLog WARN "Architecture level is not specified for 32-bit x86; defaulting to i386."
                CT_DoLog WARN "This may not be supported by the C library."
                CT_TARGET_ARCH=i386
            ;;
            i386|i486|i586|i686)          CT_TARGET_ARCH="${arch}";;
            winchip*)                     CT_TARGET_ARCH=i486;;
            pentium|pentium-mmx|c3*)      CT_TARGET_ARCH=i586;;
            pentiumpro|pentium*|athlon*)  CT_TARGET_ARCH=i686;;
            core2|atom)                   CT_TARGET_ARCH=i686;;
            prescott|nocona)              CT_TARGET_ARCH=i686;;
            k8*|opteron*)                 CT_TARGET_ARCH=i686;;
            *)                            CT_TARGET_ARCH=i586;;
        esac
    fi
    CT_TARGET_ARCH="${CT_TARGET_ARCH}${CT_ARCH_SUFFIX}"

    # Shouldn't be possible to specify this (CT_TARGET_SYS is not specified by the user,
    # it is computed by scripts/functions from libc choices). But trap if such invalid
    # values ever come from the caller:
    case "${CT_TARGET_ARCH}-${CT_TARGET_SYS}" in
        i[34567]86-gnux32)
            CT_DoLog ERROR "Invalid CT_TARGET: i[34567]86-<vendor>-<os>-gnux32 is invalid."
            CT_DoLog ERROR "CT_TARGET: ${CT_TARGET}"
            CT_Abort "Go read: https://wiki.debian.org/Multiarch/Tuples"
            ;;
    esac
}

#------------------------------------------------------------------------------
# Get multilib architecture-specific target
# Usage: CT_DoArchMultilibTarget "target variable" "multilib flags"
CT_DoArchMultilibTarget ()
{
    local target_var="${1}"; shift
    local -a multi_flags=( "$@" )
    local target_

    local bit32=false
    local bit64=false
    local abi_dflt=false
    local abi_x32=false

    for m in "${multi_flags[@]}"; do
        case "$m" in
            -m32)  bit32=true; abi_dflt=true;;
            -m64)  bit64=true; abi_dflt=true;;
            -mx32) bit64=true; abi_x32=true;;
        esac
    done

    eval target_=\"\${${target_var}}\"

    # Fix up architecture.
    case "${target_}" in
        x86_64-*)      $bit32 && target_=${target_/#x86_64-/i386-} ;;
        i[34567]86-*)  $bit64 && target_=${target_/#i[34567]86-/x86_64-} ;;
    esac

    # Fix up the ABI part.
    case "${target_}" in
        *x32) $abi_dflt && target_=${target_/%x32} ;;
        *)    $abi_x32  && target_=${target_}x32 ;;
    esac

    # Set the target variable
    eval ${target_var}=\"${target_}\"
}

# Adjust target tuple for GLIBC
CT_DoArchGlibcAdjustTuple() {
    local target_var="${1}"
    local target_

    eval target_=\"\${${target_var}}\"

    case "${target_}" in
        # x86 quirk: architecture name is i386, but glibc expects i[4567]86 - to
        # indicate the desired optimization. If it was a multilib variant of x86_64,
        # then it targets at least NetBurst a.k.a. i786, but we'll follow the model
        # above and set the optimization to i686. Otherwise, replace with the most
        # conservative choice, i486.
        i386-*)
            if [ "${CT_TARGET_ARCH}" = "x86_64" ]; then
                target_=${target_/#i386-/i686-}
            elif [ "${CT_TARGET_ARCH}" != "i386" ]; then
                target_=${target_/#i386-/${CT_TARGET_ARCH}-}
            else
                target_=${target_/#i386-/i486-}
            fi
            ;;
    esac

    # Set the target variable
    eval ${target_var}=\"${target_}\"
}

CT_DoArchUClibcConfig() {
    local cfg="${1}"

    if [ "${CT_ARCH_BITNESS}" = 64 ]; then
        CT_DoArchUClibcSelectArch "${cfg}" "x86_64"
    else
        CT_DoArchUClibcSelectArch "${cfg}" "i386"
    fi

    # FIXME This doesn't cover all cases of x86_32 on uClibc (!ng)
    CT_KconfigDisableOption "CONFIG_386" "${cfg}"
    CT_KconfigDisableOption "CONFIG_486" "${cfg}"
    CT_KconfigDisableOption "CONFIG_586" "${cfg}"
    CT_KconfigDisableOption "CONFIG_686" "${cfg}"
    case ${CT_TARGET_ARCH} in
        i386)
            CT_KconfigEnableOption "CONFIG_386" "${cfg}"
            ;;
        i486)
            CT_KconfigEnableOption "CONFIG_486" "${cfg}"
            ;;
        i586)
            CT_KconfigEnableOption "CONFIG_586" "${cfg}"
            ;;
        i686)
            CT_KconfigEnableOption "CONFIG_686" "${cfg}"
            ;;
    esac
}

CT_DoArchUClibcCflags() {
    local cfg="${1}"
    local cflags="${2}"
    local f

    for f in ${cflags}; do
        case "${f}" in
            -m64)
                CT_DoArchUClibcSelectArch "${cfg}" "x86_64"
                ;;
            -m32)
                # Since it's a part of multilib with 64-bit flavor, default
                # to new architecture (i686).
                CT_DoArchUClibcSelectArch "${cfg}" "i386"
                CT_KconfigDisableOption "CONFIG_386" "${cfg}"
                CT_KconfigDisableOption "CONFIG_486" "${cfg}"
                CT_KconfigDisableOption "CONFIG_586" "${cfg}"
                CT_KconfigEnableOption "CONFIG_686" "${cfg}"
                ;;
            -mx32)
                CT_Abort "uClibc does not support x32 ABI"
                ;;
        esac
    done
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

CT_DoArchMUSLHeaderDir() {
    local dir_var="${1}"
    local cflags="${2}"

    # If it is non-default multilib, add a suffix with architecture (reported by gcc)
    # to the headers installation path.
    if [ -n "${cflags}" ]; then
        eval "${dir_var}="$( ${CT_TARGET}-${CT_CC} -print-multiarch ${cflags} )
    fi
}
