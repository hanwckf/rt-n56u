# Compute s390-specific values

CT_DoArchTupleValues() {
    # That's the only thing to override
    if [ "${CT_ARCH_64}" = "y" ]; then
        CT_TARGET_ARCH="s390x${CT_ARCH_SUFFIX}"
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

    local m31=false
    local m64=false

    for m in "${multi_flags[@]}"; do
        case "${multi_flags}" in
            -m64) m64=true ;;
            -m31) m31=true ;;
        esac
    done

    eval target_=\"\${${target_var}}\"

    # Fix bitness
    case "${target_}" in
        s390-*)   $m64 && target_=${target_/#s390-/s390x-} ;;
        s390x-*)  $m31 && target_=${target_/#s390x-/s390-} ;;
    esac

    # Set the target variable
    eval ${target_var}=\"${target_}\"
}
