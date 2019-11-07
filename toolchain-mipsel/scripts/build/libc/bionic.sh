# This file adds functions to extract the bionic C library from the Android NDK
# Copyright 2017 Howard Chu
# Licensed under the GPL v2. See COPYING in the root of this package

# Install Unified headers
bionic_start_files()
{
    CT_DoStep INFO "Installing C library headers"
    CT_DoExecLog ALL cp -r "${CT_SRC_DIR}/android-ndk/sysroot/usr" "${CT_SYSROOT_DIR}"
}

bionic_main()
{
    local arch="${CT_ARCH}"
    if [ "${CT_ARCH_64}" = "y" ]; then
        if [ "${CT_ARCH}" = "x86" ]; then
            arch="${arch}_"
        fi
        arch="${arch}64"
    fi
    CT_DoStep INFO "Installing C library binaries"
    CT_DoExecLog ALL cp -r "${CT_SRC_DIR}/android-ndk/platforms/android-${CT_ANDROID_API}/arch-${arch}/usr" "${CT_SYSROOT_DIR}"

    # NB: Modifying both CT_TARGET_CFLAGS and CT_ALL_TARGET_CFLAGS: the __ANDROID_API__
    # definition needs to be passed into GCC build, or the resulting libstdc++ gets
    # miscompiled (attempt to link against it results in unresolved symbols to stdout/...).
    # And since __ANDROID_API__ is a user config option, placing it with other user-supplied
    # options isn't completely out of character.
    # On the other hand, CT_ALL_TARGET_CFLAGS in non-multilib builds is already set and does
    # not get recalculated after GCC build, so setting CT_TARGET_CFLAGS is not reflected
    # on other libraries/apps, such as gdbserver.
    CT_EnvModify CT_TARGET_CFLAGS "${CT_TARGET_CFLAGS} -D__ANDROID_API__=${CT_ANDROID_API}"
    CT_EnvModify CT_ALL_TARGET_CFLAGS "${CT_ALL_TARGET_CFLAGS} -D__ANDROID_API__=${CT_ANDROID_API}"
}
