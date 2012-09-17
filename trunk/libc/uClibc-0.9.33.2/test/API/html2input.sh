#!/bin/sh
# vi: ft=awk :
#
# Script to extract functions and external variables off SUS html docs
#
# Copyright (C) 2010 Bernhard Reutner-Fischer
# Public Domain

# Usage:
# wget http://www.opengroup.org/onlinepubs/9699919799/download/susv4.tgz
# tar xzf susv4.tgz
# SUS=susv4 html2input.sh -vFULL_DECLARATIONS=1
# or
# SUS=susv4 html2input.sh -vFULL_DECLARATIONS=0 -vSTDNAME=SUSv4
# 
# Bug in time.h.html of SUSv4:
# It inconsistently reads "as variables" instead of "external variables" that
# is used everywhere except in time.h.html

test "x$SUS" = "x" && SUS="susv4"
test "x$AWK" = "x" && AWK="AWK"
test "x$GREP" = "x" && GREP="GREP"
for h in \
  $($GREP -l "shall be declared as functions" $SUS/basedefs/*.h.html) \
  $($GREP -l "shall declare the following as variables" $SUS/basedefs/*.h.html) \
  $($GREP -l "shall declare the following external variables" $SUS/basedefs/*.h.html)
do
$AWK $* '
function get_filename () {
	if (NR == 1) {
		x=FILENAME
		sub(".*/", "", x)
		split(x, f , ".")
		fname=f[1]
		if (STDNAME)
			fname=fname "." STDNAME
		fname=fname ".in"
		printf "" > fname
	}
}
function unhtml (l) {
	sub("<tt>", "", l)
	sub("</tt>", "", l)
	sub("<sup>", "", l)
	sub("</sup>", "", l)
	sub("<a [^>]*>", "", l)
	sub("</a>", "", l)
	if (l ~ /<img[^>]*Option[[:space:]][[:space:]]*Start[^>]*>/) {
		sub("<img[^>]*>", "[Option Start]", l)
	} else if (l ~ /<img[^>]*Option[[:space:]][[:space:]]*End[^>]*>/) {
		sub("<img[^>]*>", "[Option End]", l)
	}
	sub("<.*>", "", l)
	return l
}
function get_funcname (l) {
	if (FULL_DECLARATIONS)
		return l
	if (l !~ /;$/)
		return l
	cnt = split(l, foo, " ")
	if (cnt >= 2 && foo[2] ~ /^\(\*/) {
		cnt = split(l, foo, "(")
		# good enough for signal() and sigset()
		if (cnt >= 2)
			l=foo[2]
	} else {
		sub("\\(.*", "", l)
	}
	gsub("[[\\]\\*]", "", l)
	i = split(l, a, " ")
	if (i)
		l = a[i]
	return l
}
function get_varname (l) {
	if (FULL_DECLARATIONS)
		return l
	if (l !~ /;$/)
		return l
	gsub(",[[:space:]][[:space:]]*", ",", l)
	sub(";$", "", l)
	i = split(l, a, " ")
	if (i)
		l = a[i]
	gsub("[[\\]\\*]", "", l)
	gsub(",", "\n", l)
	return l
}
BEGIN{data=0;l=""}
get_filename()
/shall be declared as functions/{data=1;isvar=0;next;}
/shall declare the following as variables/{data=1;isvar=1;next;}
/shall declare the following external variables/{data=1;isvar=1;next;}
/<pre>/{data++;next;}
/<\/pre>/{data=0;next;}
/.*/{
	if (data == 2 && fname) {
		tmp = $0
		sub("^[[:space:]][[:space:]]*", " ", tmp)
		l = l tmp
		tmp = unhtml(l)
		if (!tmp)
			next
		l = tmp
		if (tmp !~ /;$/ && tmp !~ />$/ &&
			tmp !~ /Option Start\]$/ && tmp !~ /Option End\]$/)
			next
		if (!isvar)
			l = get_funcname(l)
		else
			l = get_varname(l)
		if (l)
			print l >> fname
		l=""
	}
}
' $h
done
