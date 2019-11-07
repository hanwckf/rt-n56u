# This script is responsible for saving the current configuration into a
# sample to be used later on as a pre-configured target.

# What we need to save:
#  - the .config file
#  - the uClibc .config file if uClibc selected
#  - info about who reported the sample

# Parse the tools' paths configuration
. "${CT_LIB_DIR}/scripts/functions"

CT_LoadConfig

# We can not reliably save a sample which either uses local patches
# and/or custom Linux kernel headers. Warn the user about this issue
# and continue if he/she confirms sving the sample.
if [ "${CT_CUSTOM_PATCH}" = "y" ]; then
    echo "You are using local patches."
    echo "You will not be able to (easily) share this sample in this case."
    read -p "Press Ctrl-C to stop now, or Enter to continue..."
fi

# Do not use a progress bar
unset CT_LOG_PROGRESS_BAR

# Override log options
unset CT_LOG_PROGRESS_BAR CT_LOG_ERROR CT_LOG_INFO CT_LOG_EXTRA CT_LOG_DEBUG LOG_ALL
CT_LOG_WARN=y
CT_LOG_LEVEL_MAX="WARN"

# Compute the name of the sample directory
case "${CT_TOOLCHAIN_TYPE}" in
    cross)      samp_name="${CT_TARGET}";;
    canadian)   samp_name="${CT_HOST},${CT_TARGET}";;
    *)          CT_Abort "Unsupported toolchain type '${CT_TOOLCHAIN_TYPE}'";;
esac
samp_dir="samples/${samp_name}"
mkdir -p "${samp_dir}"

# Tweak the .config file: remove the options that we want to keep
# at default setting in samples.
force_default_opts=( \
    PREFIX_DIR LOG_TO_FILE LOG_FILE_COMPRESS \
    LOCAL_TARBALLS_DIR SAVE_TARBALLS \
    LOG_ERROR LOG_WARN LOG_INFO LOG_EXTRA LOG_ALL LOG_DEBUG \
    LOG_PROGRESS_BAR
    )
regexp=${force_default_opts[*]}
regexp=${regexp// /|}
${grep} -v -E '^(# )?CT_('"${regexp}"')' .config > .defconfig

# Function to copy a file to the sample directory
# Needed in case the file is already there (think of a previously available sample)
# Usage: CT_DoAddFileToSample <source> <dest>
CT_DoAddFileToSample() {
    source="$1"
    dest="$2"
    inode_s=$(ls -i "${source}" | ${awk} '{ print $1; }')
    inode_d=$(ls -i "${dest}" 2>/dev/null | ${awk} '{ print $1; }' || true)
    if [ "${inode_s}" != "${inode_d}" ]; then
        cp "${source}" "${dest}"
    fi
}

if [ "${CT_TOP_DIR}" = "${CT_LIB_DIR}" ]; then
    samp_top_dir="\${CT_LIB_DIR}"
else
    samp_top_dir="\${CT_TOP_DIR}"
fi

# Save the uClibc .config file
if [ -n "${CT_LIBC_UCLIBC_CONFIG_FILE}" ]; then
    # We save the file, and then point the saved sample to this file
    CT_DoAddFileToSample "${CT_LIBC_UCLIBC_CONFIG_FILE}" "${samp_dir}/${CT_LIBC}.config"
    "${sed}" -r -i -e 's|^(CT_LIBC_UCLIBC_CONFIG_FILE)=.+$|\1="'"${samp_top_dir}"'/samples/${CT_TARGET}/${CT_LIBC}.config"|;' \
             .defconfig
else
    # remove any dangling files
    for f in "${samp_dir}/${CT_LIBC}-"*.config; do
        if [ -f "${f}" ]; then rm -f "${f}"; fi
    done
fi

# Now, actually save the defconfig
export KCONFIG_CONFIG="$(pwd)/.defconfig"
srctree="${CT_LIB_DIR}" ${CONF} --savedefconfig="${samp_dir}/crosstool.config" "${KCONFIG_TOP}"
rm -f .defconfig

# Fill-in the reported-by info
[ -f "${samp_dir}/reported.by" ] && . "${samp_dir}/reported.by"
old_name="${reporter_name}"
old_url="${reporter_url}"
old_comment="${reporter_comment}"
read -p "Reporter name [${reporter_name}]: " reporter_name
read -p "Reporter URL [${reporter_url}]: " reporter_url
if [ -n "${reporter_comment}" ]; then
  echo "Old comment:"
  printf "${reporter_comment}\n" | ${sed} -r -e 's/^/  > /;'
fi
echo "Reporter comment (Ctrl-D to finish, '.' to use previous):"
reporter_comment=$(cat)
if [ "${reporter_comment}" = "." ]; then
  reporter_comment="${old_comment}"
fi

( echo "reporter_name=\"${reporter_name:=${old_name}}\""
  echo "reporter_url=\"${reporter_url:=${old_url}}\""
  printf "reporter_comment=\"${reporter_comment}\"\n"
) >"${samp_dir}/reported.by"
