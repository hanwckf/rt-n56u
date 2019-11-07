# This file provides the default implementations of arch-specific functions.

# Set up the target tuple
CT_DoArchTupleValues()
{
    :
}

# Adjust the list of multilibs for the target
CT_DoArchMultilibList()
{
    :
}

# Multilib: change the target triplet according to CFLAGS
# Usage: CT_DoArchGlibcAdjustTuple <variable-name> <CFLAGS>
CT_DoArchMultilibTarget()
{
    :
}

# Multilib: Adjust target tuple for GLIBC
# Usage: CT_DoArchGlibcAdjustTuple <variable-name>
CT_DoArchGlibcAdjustTuple()
{
    :
}

# Multilib: Adjust configure arguments for GLIBC
# Usage: CT_DoArchGlibcAdjustConfigure <configure-args-array-name> <cflags>
CT_DoArchGlibcAdjustConfigure()
{
    :
}

# Helper for uClibc configurators: select the architecture
# Usage: CT_DoArchUClibcSelectArch <config-file> <architecture>
CT_DoArchUClibcSelectArch() {
    local cfg="${1}"
    local arch="${2}"

    sed -i -r -e '/^TARGET_.*/d' "${cfg}"
    CT_KconfigEnableOption "TARGET_${arch}" "${cfg}"
    CT_KconfigSetOption "TARGET_ARCH" "${arch}" "${cfg}"
}

# uClibc: Adjust configuration file according to the CT-NG configuration
# Usage: CT_DoArchUClibcConfig <config-file>
CT_DoArchUClibcConfig()
{
    CT_DoLog WARN "Support for '${CT_ARCH}' is not implemented in uClibc config tweaker."
    CT_DoLog WARN "Exact configuration file must be provided."
}

# Multilib/uClibc: Adjust configuration file for given CFLAGS
# Usage: CT_DoArchUClibcCflags <config-file> <cflags>
CT_DoArchUClibcCflags()
{
    local cfg="${1}"
    local cflags="${2}"

    # Likely, any non-default cflags need to be reflected into the config.
    # It may work if we just pass them into EXTRA_CFLAGS, but we have no
    # idea how they might interact with the CFLAGS inferred by uClibc from
    # the configuration file.
    if [ "${cflags}" != "" ]; then
        CT_DoLog WARN "Multilib configuration not supported for uClibc/${CT_ARCH}"
    fi
}

# Multilib/uClibc: Adjust header installation path for given CFLAGS
# Usage: CT_DoArchUClibcHeaderDir <path-variable> <cflags>
CT_DoArchUClibcHeaderDir()
{
    # Only needed if a given architecture may select different uClibc architectures.
    :
}

# Multilib/MUSL: Adjust header installation path for given CFLAGS
# Usage: CT_DoArchMUSLHeaderDir <path-variable> <cflags>
CT_DoArchMUSLHeaderDir()
{
    # Only needed if a given architecture may select different MUSL architectures.
    :
}

# MUSL: Perform any final adjustments on the installed libc/headers
CT_DoArchMUSLPostInstall()
{
    :
}

# Override from the actual arch implementation as needed.
. "${CT_LIB_DIR}/scripts/build/arch/${CT_ARCH}.sh"
