
# Copyright (C) 2006, 2007 Junjiro Okajima
# Copyright (C) 2006, 2007 Tomas Matejicek, slax.org
#
# LICENSE follows the described ones in lzma and squashfs.

# $Id: Makefile,v 1.8 2008-10-07 12:25:54 steven Exp $

# paths
Sqlzma = ${CURDIR}
LzmaVer = lzma443
Lzma = ${Sqlzma}/${LzmaVer}
SqVer = squashfs3.2-r2
Squashfs = ${Sqlzma}/${SqVer}

LzmaC = ${Lzma}/C/7zip/Compress/LZMA_C
LzmaAlone = ${Lzma}/C/7zip/Compress/LZMA_Alone
SqTools = ${Squashfs}/squashfs-tools

# enable it if you want to add -g option when compiling
UseDebugFlags =
# disable it if you don't want to compile squashfs kernel module here
BuildSquashfs = 1

export

all:
	${MAKE} -C ${LzmaC} -f sqlzma.mk $@
	${MAKE} -C ${LzmaAlone} -f sqlzma.mk $@
	${MAKE} -C ${SqTools} $@

clean:
	${MAKE} -C ${LzmaC} -f sqlzma.mk $@
	${MAKE} -C ${LzmaAlone} -f sqlzma.mk $@
	${MAKE} -C ${SqTools} $@
	${RM} *~

install: all
	-cp -fa squashfs3.2-r2/squashfs-tools/mksquashfs ../bin/mksquashfs_lzma-3.2
	-cp -fa ${LzmaAlone}/lzma_alone ../bin/lzma_alone

########################################



