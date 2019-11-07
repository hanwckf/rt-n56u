# C library build routines. We don't invoke the corresponding functions directly
# because some of them build on top of another. E.g. moxiebox runtime requires
# newlib as a prerequisite.

# Define default hooks - download/unpack just the main package; no-op build hooks.
# The actual implementation can override just what it needs then.
eval "${CT_LIBC//[^A-Za-z0-9]/_}_get() { CT_Fetch \"\${CT_LIBC_${CT_LIBC_CHOICE_KSYM}_PKG_KSYM}\"; }"
eval "${CT_LIBC//[^A-Za-z0-9]/_}_extract() { CT_ExtractPatch \"\${CT_LIBC_${CT_LIBC_CHOICE_KSYM}_PKG_KSYM}\"; }"
for _m in start_files main post_cc; do
    eval "${CT_LIBC//[^A-Za-z0-9]/_}_${_m}() { :; }"
done

# Source the selected libc.
. "${CT_LIB_DIR}/scripts/build/libc/${CT_LIBC}.sh"

do_libc_get()
{
    eval "${CT_LIBC//[^A-Za-z0-9]/_}_get"
}

do_libc_extract()
{
    eval "${CT_LIBC//[^A-Za-z0-9]/_}_extract"
}

do_libc_start_files()
{
    eval "${CT_LIBC//[^A-Za-z0-9]/_}_start_files"
}

do_libc_main()
{
    eval "${CT_LIBC//[^A-Za-z0-9]/_}_main"
}

do_libc_post_cc()
{
    eval "${CT_LIBC//[^A-Za-z0-9]/_}_post_cc"
}
