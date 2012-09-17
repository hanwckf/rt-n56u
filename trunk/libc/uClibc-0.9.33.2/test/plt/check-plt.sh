#!/bin/sh
allowed="
calloc
free
malloc
memalign
realloc
"

${OBJDUMP:-objdump} -d ${top_builddir:-../..}/lib/libc.so.? | \
gawk -v allowed="${allowed}" '
BEGIN {
	COUNT = split(" " allowed, ALLOWED);
}

# Strip away the noise.  The name will be like:
# <brk>:
# <foo@plt>
function symstrip(name) {
	return gensub(/.*<([^>@]*).*/, "\\1", "", name);
}

{
# Match the start of the symbol disassembly
# 00009720 <brk>:
if ($2 ~ />:$/) {
	f = symstrip($2);

} else if ($NF ~ /@plt>/) {
	rf = symstrip($NF);
	for (a in ALLOWED) {
		a = ALLOWED[a];
		if (a == rf)
			next;
	}
	print "Func " f " references " rf;
}
}' | sort -u
