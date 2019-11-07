# Compute Alpha-specific values

CT_DoArchTupleValues () {
    # The architecture part of the tuple:
    CT_TARGET_ARCH="${CT_ARCH}${CT_ARCH_SUFFIX:-${CT_ARCH_ALPHA_VARIANT}}"
}
