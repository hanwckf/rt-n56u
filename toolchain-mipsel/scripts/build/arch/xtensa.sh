# Compute Xtensa-specific values

CT_DoArchTupleValues()
{
    # The architecture part of the tuple:
    CT_TARGET_ARCH="${CT_ARCH}${CT_ARCH_SUFFIX}"
    CT_ARCH_ENDIAN_CFLAG=""
    CT_ARCH_ENDIAN_LDFLAG=""
}

CT_DoArchUClibcConfig()
{
    local cfg="${1}"

    CT_DoArchUClibcSelectArch "${cfg}" "xtensa"
}
