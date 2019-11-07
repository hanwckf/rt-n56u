## Process this file with automake to produce Makefile.in

man8_MANS	= nfsdcltrack.man
EXTRA_DIST	= $(man8_MANS)

AM_CFLAGS	+= -D_LARGEFILE64_SOURCE
sbin_PROGRAMS	= nfsdcltrack

nfsdcltrack_SOURCES = nfsdcltrack.c sqlite.c
nfsdcltrack_LDADD = ../../support/nfs/libnfs.a $(LIBSQLITE) $(LIBCAP)

MAINTAINERCLEANFILES = Makefile.in

