#!/bin/sh

# Note: was using sed OPTS CMD -- FILES
# but users complain that many sed implementations
# are misinterpreting --.

test $# -ge 2 || { echo "Syntax: $0 SRCTREE OBJTREE"; exit 1; }

# cd to objtree
cd -- "$2" || { echo "Syntax: $0 SRCTREE OBJTREE"; exit 1; }
# In separate objtree build, include/ might not exist yet
mkdir include 2>/dev/null

srctree="$1"

status() { printf '  %-8s%s\n' "$1" "$2"; }
gen() { status "GEN" "$@"; }
chk() { status "CHK" "$@"; }

generate()
{
	local src="$1" dst="$2" header="$3" insert="$4"
	#chk "${dst}"
	(
		# Need to use printf: different shells have inconsistent
		# rules re handling of "\n" in echo params,
		# and ${insert} definitely contains "\n".
		# Therefore, echo "${header}" would not work:
		printf "%s\n" "${header}"
		if grep -qs '^INSERT$' "${src}"; then
			sed -n '1,/^INSERT$/p' "${src}"
			printf "%s\n" "${insert}"
			sed -n '/^INSERT$/,$p' "${src}"
		else
			if [ -n "${insert}" ]; then
				printf "%s\n" "ERROR: INSERT line missing in: ${src}" 1>&2
			fi
			cat "${src}"
		fi
	) | sed '/^INSERT$/d' > "${dst}.tmp"
	if ! cmp -s "${dst}" "${dst}.tmp"; then
		gen "${dst}"
		mv "${dst}.tmp" "${dst}"
	else
		rm -f "${dst}.tmp"
	fi
}

# (Re)generate include/applets.h
s=`sed -n 's@^//applet:@@p' "$srctree"/*/*.c "$srctree"/*/*/*.c`
generate \
	"$srctree/include/applets.src.h" \
	"include/applets.h" \
	"/* DO NOT EDIT. This file is generated from applets.src.h */" \
	"${s}"

# (Re)generate include/usage.h
# We add line continuation backslash after each line,
# and insert empty line before each line which doesn't start
# with space or tab
# (note: we need to use \\\\ because of ``)
s=`sed -n -e 's@^//usage:\([ \t].*\)$@\1 \\\\@p' -e 's@^//usage:\([^ \t].*\)$@\n\1 \\\\@p' "$srctree"/*/*.c "$srctree"/*/*/*.c`
generate \
	"$srctree/include/usage.src.h" \
	"include/usage.h" \
	"/* DO NOT EDIT. This file is generated from usage.src.h */" \
	"${s}"

# (Re)generate */Kbuild and */Config.in
{ cd -- "$srctree" && find . -type d; } | while read -r d; do
	d="${d#./}"

	src="$srctree/$d/Kbuild.src"
	dst="$d/Kbuild"
	if test -f "$src"; then
		mkdir -p -- "$d" 2>/dev/null

		s=`sed -n 's@^//kbuild:@@p' "$srctree/$d"/*.c`
		generate \
			"${src}" "${dst}" \
			"# DO NOT EDIT. This file is generated from Kbuild.src" \
			"${s}"
	fi

	src="$srctree/$d/Config.src"
	dst="$d/Config.in"
	if test -f "$src"; then
		mkdir -p -- "$d" 2>/dev/null

		s=`sed -n 's@^//config:@@p' "$srctree/$d"/*.c`
		generate \
			"${src}" "${dst}" \
			"# DO NOT EDIT. This file is generated from Config.src" \
			"${s}"
	fi
done

# Last read failed. This is normal. Don't exit with its error code:
exit 0
