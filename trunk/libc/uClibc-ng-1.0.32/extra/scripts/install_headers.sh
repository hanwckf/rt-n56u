#!/bin/sh
# Parameters:
# $1 = source dir
# $2 = dst dir
# $top_builddir = well you guessed it

srcdir=${1:-include}
dstdir=${2:-`. ./.config 2>/dev/null && echo ${DEVEL_PREFIX}/include`}
: ${top_builddir:=.}

die_if_not_dir()
{
	for dir in "$@"; do
		test -d "$dir" && continue
		echo "Error: '$dir' is not a directory"
		exit 1
	done
}


# Ensure that created dirs/files have 755/644 perms
umask 022


# Sanity tests
die_if_not_dir "${srcdir}"
mkdir -p "${dstdir}" 2>/dev/null
die_if_not_dir "${dstdir}"
die_if_not_dir "$top_builddir"
if ! test -x "$top_builddir/extra/scripts/unifdef"; then
	echo "Error: need '$top_builddir/extra/scripts/unifdef' executable"
	exit 1
fi

# Sanitize and copy uclibc headers
(
# We must cd, or else we will prepend "${srcdir}" to filenames!
cd "${srcdir}" || exit 1
find . ! -name '.' -a ! -path '*/.*' | sed -e 's/^\.\///' -e '/^config\//d' \
	-e '/^config$/d'
) | \
(
IFS=''
while read -r filename; do
	if test -d "${srcdir}/$filename"; then
		mkdir -p "${dstdir}/$filename" 2>/dev/null
		continue
	fi
	if test x"${filename##libc-*.h}" = x""; then
		# Do not install libc-XXXX.h files
		continue
	fi
	# Do not abort the script if unifdef "fails"!
	# NB2: careful with sed command arguments, they contain tab character
	"$top_builddir/extra/scripts/unifdef" \
		-B \
		-t \
		-x 2 \
		-f "$top_builddir/include/generated/unifdef_config.h" \
		-U_LIBC \
		-U__UCLIBC_GEN_LOCALE \
		-U__NO_CTYPE \
		"${srcdir}/$filename" \
	| sed -e '/^rtld_hidden_proto[ 	]*([a-zA-Z0-9_]*)$/d' \
	| sed -e '/^lib\(c\|m\|resolv\|dl\|intl\|rt\|nsl\|util\|crypt\|pthread\)_hidden_proto[ 	]*([a-zA-Z0-9_]*)$/d' \
	> "${dstdir}/$filename"
done
)


# Fix mode/owner bits
cd "${dstdir}" || exit 1
chmod -R u=rwX,go=rX . >/dev/null 2>&1
chown -R `id | sed 's/^uid=\([0-9]*\).*gid=\([0-9]*\).*$/\1:\2/'` . >/dev/null 2>&1

# ignore errors on unrelated files
exit 0
