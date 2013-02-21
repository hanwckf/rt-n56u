# This file is part of GNU Parted
# Copyright (C) 1999-2001, 2007, 2009-2012 Free Software Foundation, Inc.
#
# This file may be modified and/or distributed without restriction.

AUTOMAKE_OPTIONS = subdir-objects

partedincludedir = -I$(top_builddir)/include -I$(top_srcdir)/include

AM_CFLAGS = $(WARN_CFLAGS)

noinst_LTLIBRARIES    =	libfs.la

libfs_la_LIBADD   = $(UUID_LIBS)		\
		    $(INTLLIBS)			\
		    $(OS_LIBS)

libfs_la_SOURCES =		\
  amiga/affs.c			\
  amiga/affs.h			\
  amiga/amiga.c			\
  amiga/amiga.h			\
  amiga/apfs.c			\
  amiga/apfs.h			\
  amiga/asfs.c			\
  amiga/asfs.h			\
  amiga/a-interface.c		\
  ext2/ext2.h			\
  ext2/ext2_fs.h		\
  ext2/interface.c		\
  fat/bootsector.c		\
  fat/bootsector.h		\
  fat/count.h			\
  fat/fat.c			\
  fat/fat.h			\
  hfs/hfs.c			\
  hfs/hfs.h			\
  hfs/probe.c			\
  hfs/probe.h			\
  jfs/jfs.c			\
  jfs/jfs_superblock.h		\
  jfs/jfs_types.h		\
  linux_swap/linux_swap.c	\
  nilfs2/nilfs2.c		\
  ntfs/ntfs.c			\
  reiserfs/reiserfs.c		\
  reiserfs/reiserfs.h		\
  ufs/ufs.c			\
  xfs/platform_defs.h		\
  xfs/xfs.c			\
  xfs/xfs_sb.h			\
  xfs/xfs_types.h

lib_LTLIBRARIES = libparted-fs-resize.la

EXTRA_DIST =			\
  hfs/DOC			\
  hfs/HISTORY			\
  hfs/TODO

# Set the shared library version, per Libtool's guidelines.
# For details, see the "Updating library version information" section of
# "info libtool".
CURRENT = 0
REVISION = 0
AGE = 0

sym_file = $(srcdir)/fsresize.sym
libparted_fs_resize_la_LDFLAGS =		\
  -Wl,--version-script=$(sym_file)		\
  -version-info $(CURRENT):$(REVISION):$(AGE)
EXTRA_DIST += fsresize.sym
libparted_fs_resize_la_DEPENDENCIES = $(sym_file)

libparted_fs_resize_la_SOURCES = \
  r/filesys.c			\
  r/fat/bootsector.c		\
  r/fat/bootsector.h		\
  r/fat/calc.c			\
  r/fat/calc.h			\
  r/fat/clstdup.c		\
  r/fat/clstdup.h		\
  r/fat/context.c		\
  r/fat/context.h		\
  r/fat/count.c			\
  r/fat/count.h			\
  r/fat/fat.c			\
  r/fat/fat.h			\
  r/fat/fatio.c			\
  r/fat/fatio.h			\
  r/fat/resize.c		\
  r/fat/table.c			\
  r/fat/table.h			\
  r/fat/traverse.c		\
  r/fat/traverse.h		\
  r/hfs/advfs.c			\
  r/hfs/advfs.h			\
  r/hfs/advfs_plus.c		\
  r/hfs/advfs_plus.h		\
  r/hfs/cache.c			\
  r/hfs/cache.h			\
  r/hfs/file.c			\
  r/hfs/file.h			\
  r/hfs/file_plus.c		\
  r/hfs/file_plus.h		\
  r/hfs/hfs.c			\
  r/hfs/hfs.h			\
  r/hfs/journal.c		\
  r/hfs/journal.h		\
  r/hfs/probe.c			\
  r/hfs/probe.h			\
  r/hfs/reloc.c			\
  r/hfs/reloc.h			\
  r/hfs/reloc_plus.c		\
  r/hfs/reloc_plus.h

INCLUDES = \
  -I$(top_srcdir)/libparted/labels	\
  $(partedincludedir)			\
  $(INTLINCS)
