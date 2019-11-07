#!/bin/sh
#
# Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
#
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
#

# Creates the necessary Makefiles to build w/ the Makefile.{arch,in} files

DIRS="ldso libc libcrypt libintl libm libnsl libpthread libresolv librt libutil"

if [ ! -f Makerules ] ; then
	echo "Run this command in top_srcdir"
	exit 1
fi

if [ -z "${USE_CMD}" ] ; then
USE_CMD="cp"
fi

RM="rm -f"
${RM} Makefile
${USE_CMD} extra/scripts/Makefile.libs.lvl0 Makefile

#for x in ${DIRS} ; do
#	find ./${x} -name Makefile -exec rm -f {} \;
#done

for x in */Makefile.in ; do
	${RM} `dirname ${x}`/Makefile
	${USE_CMD} extra/scripts/Makefile.libs.lvl1 `dirname ${x}`/Makefile
done

for x in utils/Makefile.in ; do
	${RM} `dirname ${x}`/Makefile
	${USE_CMD} extra/scripts/Makefile.utils.lvl1 `dirname ${x}`/Makefile
done

for x in */*/Makefile.in ; do
	${RM} `dirname ${x}`/Makefile
	${USE_CMD} extra/scripts/Makefile.objs.lvl2 `dirname ${x}`/Makefile
done

# overwrites the earlier ones, we do not add arch specific to libm/arch
for x in ldso/*/Makefile.in libpthread/*/Makefile.in ; do
	${RM} `dirname ${x}`/Makefile
	${USE_CMD} extra/scripts/Makefile.libs.lvl2 `dirname ${x}`/Makefile
done

for x in */*/*/Makefile.in ; do
	${RM} `dirname ${x}`/Makefile
	${USE_CMD} extra/scripts/Makefile.objs.lvl3 `dirname ${x}`/Makefile
done

for x in libc/*/*/Makefile.arch ; do
	${RM} `dirname ${x}`/Makefile
	${USE_CMD} extra/scripts/Makefile.arch.lvl3 `dirname ${x}`/Makefile
done

for x in */*/*/*/Makefile.in ; do
	${RM} `dirname ${x}`/Makefile
	${USE_CMD} extra/scripts/Makefile.objs.lvl4 `dirname ${x}`/Makefile
done

# we do not add these to libpthread/PTNAME/sysdeps/arch
for x in libc/*/*/*/Makefile.arch ; do
	${RM} `dirname ${x}`/Makefile
	${USE_CMD} extra/scripts/Makefile.arch.lvl4 `dirname ${x}`/Makefile
done

exit 0
