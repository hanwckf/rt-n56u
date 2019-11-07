#!/bin/bash

# Usage:
#  download-docs.sh TOP-LEVEL-DIR MANUAL-FILES...
distdir=${1}
shift

# Configurable portions
docs_git=https://github.com/crosstool-ng/crosstool-ng.github.io.git
docs_subdir=_pages/docs

# Clone a repository for docs. Github does not support 'git archive --remote='.
set -ex
git clone --depth=1 "${docs_git}" "${distdir}/site-docs"

# Copy the docs instead of the MANUAL_ONLINE placeholder
mkdir -p "${distdir}/docs/manual"
while [ -n "${1}" ]; do
    case "${1}" in
        docs/manual/*) ;;
        *) echo "Expected file not in docs/manual/: $1" >&2; exit 1;;
    esac
    input="${distdir}/site-docs/${docs_subdir}/${1#docs/manual/}"
    if [ ! -r "${input}" ]; then
        echo "Not found: ${1}" >&2
        exit 1
    fi
    awk '
BEGIN   { skip=0; }
        {
            if ($0=="---") {
                if (NR==1) {
                    skip=1
                    next
                }
                else if (skip) {
                    skip=0
                    next
                }
            }
            if (!skip) {
                print $0
            }
        }
    ' < "${input}" > "${distdir}/${1}"
    rm -f "${input}"
    shift
done
extra_md_pages=false
for i in "${distdir}/site-docs/${docs_subdir}/"*.md; do
    if [ -r "${i}" ]; then
        echo "Unpackaged page in the manual: ${i#${distdir}/site-docs/${docs_subdir}/}"
        extra_md_files=true
    fi
done
if [ "${extra_md_files}" = "true" ]; then
    exit 1
fi
rm -rf "${distdir}/site-docs"
