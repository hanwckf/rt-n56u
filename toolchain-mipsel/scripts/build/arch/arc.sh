# Compute ARC-specific values

CT_DoArchTupleValues()
{
    # The architecture part of the tuple:
    CT_TARGET_ARCH="${CT_ARCH}${CT_ARCH_SUFFIX:-${target_endian_eb}}"
}

CT_DoArchUClibcConfig()
{
    local cfg="${1}"

    CT_DoArchUClibcSelectArch "${cfg}" "arc"
}

CT_DoArchUClibcCflags()
{
    local cfg="${1}"
    local cflags="${2}"
    local f

    CT_KconfigDisableOption "CONFIG_ARC_HAS_ATOMICS" "${cfg}"

    for f in ${cflags}; do
        case "${f}" in
            -matomic)
                CT_KconfigEnableOption "CONFIG_ARC_HAS_ATOMICS" "${cfg}"
                ;;
        esac
    done
}
