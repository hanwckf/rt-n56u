#!/bin/sh
#
# A tool to simplify Makefiles that need to put something
# into the ROMFS
#
# Copyright (C) David McCullough, 2002,2003
#
#############################################################################

# Provide a default PATH setting to avoid potential problems...
PATH="/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:$PATH"

usage()
{
	cat >&2 <<EOF
$0: [options] [src] dst
    -v          : output actions performed.
    -e env-var  : only take action if env-var is set to "y".
    -o option   : only take action if option is set to "y".
    -p perms    : chmod style permissions for dst.
    -d          : make dst directory if it doesn't exist
    -S          : don't strip after installing
    -a text     : append text to dst.
    -A pattern  : only append text if pattern doesn't exist in file
    -l link     : dst is a link to 'link'.
    -s sym-link : dst is a sym-link to 'sym-link'.

    if "src" is not provided,  basename is run on dst to determine the
    source in the current directory.

    multiple -e and -o options are ANDed together.  To achieve an OR affect
    use a single -e/-o with 1 or more y/n/"" chars in the condition.

    if src is a directory,  everything in it is copied recursively to dst
    with special files removed (currently CVS and Subversion dirs).
EOF
	exit 1
}

#############################################################################

setperm()
{
	rc=0
	# Always start with write access for the user so that files can be
	# updated/overwritten during make romfs
	chmod u+w ${ROMFSDIR}${dst}
	if [ "$perm" ]
	then
		[ "$v" ] && echo "chmod ${perm} ${ROMFSDIR}${dst}"
		chmod ${perm} ${ROMFSDIR}${dst}
		rc=$?
	fi
	return $rc
}

#############################################################################

file_copy()
{
	rc=0
	if [ -d "${src}" ]
	then
		[ "$v" ] && echo "CopyDir ${src} ${ROMFSDIR}${dst}"
		(
			cd ${src} || return 1
			V=
			[ "$v" ] && V=v
			find . -print | grep -E -v '/CVS|/\.svn' | cpio -p${V}dumL ${ROMFSDIR}${dst}
			rc=$?
			# And make sure these files are still writable
			find . -print | grep -E -v '/CVS|/\.svn' | ( cd ${ROMFSDIR}${dst}; xargs chmod u+w )
		)
	else
		if [ -d ${dst} ]; then
			dstfile=${ROMFSDIR}${dst}/`basename ${src}`
		else
			dstfile=${ROMFSDIR}${dst}
		fi
		rm -f ${dstfile}
		[ "$v" ] && echo "cp -f ${src} ${dstfile}"
		cp -f ${src} ${dstfile} && setperm ${dstfile}
		rc=$?
		if [ $rc -eq 0 -a -n "$strip" ]; then
			${STRIPTOOL} ${dstfile} 2>/dev/null
			${STRIPTOOL} -R .comment -R .note --strip-debug --strip-unneeded ${dstfile} 2>/dev/null
			if [ $? -eq 0 ] && [ -x "${SSTRIP_TOOL}" ] ; then
				${SSTRIP_TOOL} ${dstfile}
			fi
		fi
	fi
	return $rc
}

#############################################################################

file_append()
{
	touch ${ROMFSDIR}${dst} || return 1
	if [ -z "${pattern}" ] && grep -F "${src}" ${ROMFSDIR}${dst} > /dev/null
	then
		[ "$v" ] && echo "File entry already installed."
	elif [ "${pattern}" ] && egrep "${pattern}" ${ROMFSDIR}${dst} > /dev/null
	then
		[ "$v" ] && echo "File pattern already installed."
	else
		[ "$v" ] && echo "Installing entry into ${ROMFSDIR}${dst}."
		echo "${src}" >> ${ROMFSDIR}${dst} || return 1
	fi
	setperm ${ROMFSDIR}${dst}
	return $?
}

#############################################################################

hard_link()
{
	rm -f ${ROMFSDIR}${dst}
	[ "$v" ] && echo "ln -f ${src} ${ROMFSDIR}${dst}"
	ln -f ${ROMFSDIR}${src} ${ROMFSDIR}${dst}
	return $?
}

#############################################################################

sym_link()
{
	rm -f ${ROMFSDIR}${dst}
	[ "$v" ] && echo "ln -sf ${src} ${ROMFSDIR}${dst}"
	ln -sf ${src} ${ROMFSDIR}${dst}
	return $?
}

#############################################################################
#
# main program entry point
#

if [ -z "$ROMFSDIR" ]; then
	echo "ROMFSDIR is not set" >&2
	usage
	exit 1
fi

if [ -z "$ROOTDIR" ]; then
	echo "ROOTDIR is not set" >&2
	usage
	exit 1
fi

v=
option=y
pattern=
perm=
func=file_copy
mdir=
src=
dst=
strip=1
sstrip=0


while getopts 'dSve:o:A:p:a:l:s:' opt "$@"
do
	case "$opt" in
	v) v="1";                           ;;
	d) mdir="1";                        ;;
	S) strip=;                          ;;
	o) option="$OPTARG";                ;;
	e) eval option=\"\$$OPTARG\";       ;;
	p) perm="$OPTARG";                  ;;
	a) src="$OPTARG"; func=file_append; ;;
	A) pattern="$OPTARG";               ;;
	l) src="$OPTARG"; func=hard_link;   ;;
	s) src="$OPTARG"; func=sym_link;    ;;

	*)  break ;;
	esac
#
#	process option here to get an ANDing effect
#
	case "$option" in
	*[mMyY]*) # this gives OR effect, ie., nYn
		;;
	*)
		[ "$v" ] && echo "Condition not satisfied."
		exit 0
		;;
	esac
done

shift `expr $OPTIND - 1`

case $# in
1)
	dst="$1"
	if [ -z "$src" ]
	then
		src="`basename $dst`"
	fi
	;;
2)
	if [ ! -z "$src" ]
	then
		echo "Source file already provided" >&2
		exit 1
	fi
	src="$1"
	dst="$2"
	;;
*)
	usage
	;;
esac

if [ "$mdir" -a ! -d "`dirname ${ROMFSDIR}${dst}`/." ]
then
	mkdir -p "`dirname ${ROMFSDIR}${dst}`/." || exit 1
fi

$func || exit 1

exit 0

#############################################################################
