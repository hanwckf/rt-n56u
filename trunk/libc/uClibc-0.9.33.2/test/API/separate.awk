#!/usr/bin/awk -f
# 
# Usage: awk -f separate.awk foo.SUSv4.in
# Input: http://www.opengroup.org/onlinepubs/9699919799/basedefs/unistd.h.html#tag_13_77_03_06
# Output: foo-$CODE.SUSv4.syms, foo.SUSv4.syms
#
# Copyright (C) 2010 Bernhard Reutner-Fischer
# Public domain

function get_code(line)
{
	sub("\\]\\[.*", "", line)
	sub("\\[", "", line)
	sub(" ", "", line)
	return line
}
BEGIN{
	code="";# feature set; XSI, OB XSI, CX, etc

}
/\[Option Start\]/{
	code = get_code($0)
	next
}
/\[Option End\]/{ code = ""; next; }
/.*/ {
	if (!hdrname) {
		split(FILENAME, fparts, ".")
		hdrname = fparts[1]
		stdname = fparts[2]
		if (fparts[3] != "in") {
			print "inputfilename may not be ok, exiting."
			exit(1)
		}
	}
	if (code) {
		fname = hdrname "-" code "." stdname ".syms"
	} else {
		fname = hdrname "." stdname ".syms"
	}
	sub("^*", "", $0)
	if (file[code]) {
		print $0 >> fname
	} else {
		print $0 > fname
		file[code] = 1
	}
}
