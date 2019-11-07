# This file declares functions to install the kernel headers for linux
# Copyright 2007 Yann E. MORIN
# Licensed under the GPL v2. See COPYING in the root of this package

CT_DoKernelTupleValues()
{
    if [ -z "${CT_ARCH_USE_MMU}" ]; then
        # Some no-mmu linux targets requires a -uclinux tuple (like m68k/cf),
        # while others must have a -linux tuple.  Other targets
        # should be added here when someone starts to care about them.
        case "${CT_ARCH}" in
            arm*)               CT_TARGET_KERNEL="linux" ;;
            m68k|xtensa*)       CT_TARGET_KERNEL="uclinux" ;;
            *)                  CT_Abort "Unsupported no-mmu arch '${CT_ARCH}'"
        esac
    fi
}

# Download the kernel
do_kernel_get()
{
    CT_Fetch LINUX
}

# Disable building relocs application - it needs <linux/types.h>
# on the host, which may not be present on Cygwin or MacOS; it
# needs <elf.h>, which again is not present on MacOS; and most
# important, we don't need it to install the headers.
# This is not done as a patch, since it varies from Linux version
# to version - patching each particular Linux version would be
# too cumbersome.
linux_disable_build_relocs()
{
    sed -i -r 's/(\$\(MAKE\) .* relocs)$/:/' arch/*/Makefile
}

# Extract kernel
do_kernel_extract()
{
    # TBD verify linux_disable_build_relocs is run
    CT_ExtractPatch LINUX linux_disable_build_relocs
}

# Install kernel headers using headers_install from kernel sources.
do_kernel_headers()
{
    local kernel_path
    local kernel_arch

    CT_DoStep INFO "Installing kernel headers"

    mkdir -p "${CT_BUILD_DIR}/build-kernel-headers"

    kernel_path="${CT_SRC_DIR}/linux"
    V_OPT="V=${CT_KERNEL_LINUX_VERBOSE_LEVEL}"

    kernel_arch="${CT_ARCH}"
    case "${CT_ARCH}:${CT_ARCH_BITNESS}" in
        # ARM 64 (aka AArch64) is special
        arm:64) kernel_arch="arm64";;
    esac

    CT_DoLog EXTRA "Installing kernel headers"
    CT_DoExecLog ALL                                         \
    make -C "${kernel_path}"                                 \
         BASH="$(which bash)"                                \
         HOSTCC="${CT_BUILD}-gcc"                            \
         CROSS_COMPILE="${CT_TARGET}-"                       \
         O="${CT_BUILD_DIR}/build-kernel-headers"            \
         ARCH=${kernel_arch}                                 \
         INSTALL_HDR_PATH="${CT_SYSROOT_DIR}/usr"            \
         ${V_OPT}                                            \
         headers_install

    if [ "${CT_KERNEL_LINUX_INSTALL_CHECK}" = "y" ]; then
        CT_DoLog EXTRA "Checking installed headers"
        CT_DoExecLog ALL                                         \
        make -C "${kernel_path}"                                 \
             BASH="$(which bash)"                                \
             HOSTCC="${CT_BUILD}-gcc"                            \
             CROSS_COMPILE="${CT_TARGET}-"                       \
             O="${CT_BUILD_DIR}/build-kernel-headers"            \
             ARCH=${kernel_arch}                                 \
             INSTALL_HDR_PATH="${CT_SYSROOT_DIR}/usr"            \
             ${V_OPT}                                            \
             headers_check
    fi

    # Cleanup
    find "${CT_SYSROOT_DIR}" -type f                        \
                             \(    -name '.install'         \
                                -o -name '..install.cmd'    \
                                -o -name '.check'           \
                                -o -name '..check.cmd'      \
                             \)                             \
                             -exec rm {} \;

    CT_EndStep
}
