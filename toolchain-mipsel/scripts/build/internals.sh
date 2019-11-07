# This file contains crosstool-NG internal steps

create_ldso_conf()
{
    local multi_dir multi_os_dir multi_os_dir_gcc multi_root multi_flags multi_index multi_count multi_target
    local b d

    for arg in "$@"; do
        eval "${arg// /\\ }"
    done

    CT_DoExecLog ALL mkdir -p "${multi_root}/etc"
    for b in /lib /usr/lib "${CT_LDSO_CONF_EXTRA_DIRS_ARRAY[@]}"; do
        d="${b}/${multi_os_dir}"
        CT_SanitizeVarDir d
        echo "${d}" >> "${multi_root}/etc/ld.so.conf"
        if [ "${multi_os_dir}" != "${multi_os_dir_gcc}" ]; then
            d="${b}/${multi_os_dir_gcc}"
            CT_SanitizeVarDir d
            echo "${d}" >> "${multi_root}/etc/ld.so.conf"
        fi
    done
}

# This step is called once all components were built, to remove
# un-wanted files, to add tuple aliases, and to add the final
# crosstool-NG-provided files.
do_finish() {
    local _t
    local _type
    local strip_args
    local gcc_version
    local exe_suffix

    CT_DoStep INFO "Finalizing the toolchain's directory"

    if [ "${CT_CREATE_LDSO_CONF}" = "y" ]; then
        # Create /etc/ld.so.conf
        CT_mkdir_pushd "${CT_BUILD_DIR}/build-create-ldso"
        CT_IterateMultilibs create_ldso_conf create-ldso
        CT_Popd
    fi

    if [ "${CT_STRIP_HOST_TOOLCHAIN_EXECUTABLES}" = "y" ]; then
        case "$CT_HOST" in
            *darwin*)
                strip_args=""
                ;;
            *freebsd*)
                strip_args="--strip-all"
                ;;
            *)
                strip_args="--strip-all -v"
                ;;
        esac
        case "$CT_TARGET" in
            *mingw*)
                exe_suffix=".exe"
                ;;
            *)
                exe_suffix=""
                ;;
        esac
        CT_DoLog INFO "Stripping all toolchain executables"
        CT_Pushd "${CT_PREFIX_DIR}"

        # Strip gdbserver
        if [ "${CT_GDB_GDBSERVER}" = "y" ]; then
            CT_DoExecLog ALL "${CT_TARGET}-strip" ${strip_args}         \
                             "${CT_TARGET}/debug-root/usr/bin/gdbserver${exe_suffix}"
        fi
        if [ "${CT_CC_GCC}" = "y" ]; then
            # We can not use the version in CT_GCC_VERSION because
            # of the Linaro stuff. So, harvest the version string
            # directly from the gcc sources...
            gcc_version=$( cat "${CT_SRC_DIR}/gcc/gcc/BASE-VER" )
            for _t in "bin/${CT_TARGET}-"*                                      \
                      "${CT_TARGET}/bin/"*                                      \
                      "libexec/gcc/${CT_TARGET}/${gcc_version}/"*               \
                      "libexec/gcc/${CT_TARGET}/${gcc_version}/install-tools/"* \
            ; do
                _type="$( file "${_t}" |cut -d ' ' -f 2- )"
                case "${_type}" in
                    *script*executable*)
                        ;;
                    *executable*)
                        CT_DoExecLog ALL ${CT_HOST}-strip ${strip_args} "${_t}"
                        ;;
                esac
            done
        fi
        CT_Popd
    fi

    if [ "${CT_BARE_METAL}" != "y" ]; then
        CT_DoLog EXTRA "Installing the populate helper"
        sed -r -e 's|@@CT_TARGET@@|'"${CT_TARGET}"'|g;' \
               -e 's|@@CT_install@@|'"install"'|g;'     \
               -e 's|@@CT_awk@@|'"awk"'|g;'             \
               -e 's|@@CT_bash@@|'"${bash}"'|g;'           \
               -e 's|@@CT_grep@@|'"grep"'|g;'           \
               -e 's|@@CT_make@@|'"make"'|g;'           \
               -e 's|@@CT_sed@@|'"sed"'|g;'             \
               "${CT_LIB_DIR}/scripts/populate.in"         \
               >"${CT_PREFIX_DIR}/bin/${CT_TARGET}-populate"
        CT_DoExecLog ALL chmod 755 "${CT_PREFIX_DIR}/bin/${CT_TARGET}-populate"
    fi

    if [ "${CT_LIBC_XLDD}" = "y" ]; then
        CT_DoLog EXTRA "Installing a cross-ldd helper"
        sed -r -e 's|@@CT_VERSION@@|'"${CT_VERSION}"'|g;' \
               -e 's|@@CT_TARGET@@|'"${CT_TARGET}"'|g;'      \
               -e 's|@@CT_BITS@@|'"${CT_ARCH_BITNESS}"'|g;'  \
               -e 's|@@CT_install@@|'"install"'|g;'       \
               -e 's|@@CT_bash@@|'"${bash}"'|g;'             \
               -e 's|@@CT_grep@@|'"grep"'|g;'             \
               -e 's|@@CT_make@@|'"make"'|g;'             \
               -e 's|@@CT_sed@@|'"sed"'|g;'               \
               "${CT_LIB_DIR}/scripts/xldd.in"               \
               >"${CT_PREFIX_DIR}/bin/${CT_TARGET}-ldd"
        CT_DoExecLog ALL chmod 755 "${CT_PREFIX_DIR}/bin/${CT_TARGET}-ldd"
    fi

    # Create the aliases to the target tools
    CT_DoLog EXTRA "Creating toolchain aliases"
    CT_SymlinkTools "${CT_PREFIX_DIR}/bin" "${CT_PREFIX_DIR}/bin" \
            "${CT_TARGET_ALIAS}" "${CT_TARGET_ALIAS_SED_EXPR}"

    # Remove the generated documentation files
    if [ "${CT_REMOVE_DOCS}" = "y" ]; then
        CT_DoLog EXTRA "Removing installed documentation"
        CT_DoForceRmdir "${CT_PREFIX_DIR}/"{,usr/}{,share/}{man,info}
        CT_DoForceRmdir "${CT_SYSROOT_DIR}/"{,usr/}{,share/}{man,info}
        CT_DoForceRmdir "${CT_DEBUGROOT_DIR}/"{,usr/}{,share/}{man,info}
    fi

    if [ "${CT_INSTALL_LICENSES}" = y ]; then
        CT_InstallCopyingInformation
    fi

    CT_EndStep
}
