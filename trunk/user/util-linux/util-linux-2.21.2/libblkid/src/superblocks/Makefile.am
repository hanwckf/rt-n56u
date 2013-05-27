include $(top_srcdir)/config/include-Makefile.am

AM_CPPFLAGS += -I$(ul_libblkid_incdir) -I$(ul_libblkid_srcdir)

noinst_LTLIBRARIES = libblkid_superblocks.la
libblkid_superblocks_la_SOURCES = \
			superblocks.h \
			superblocks.c \
			swap.c \
			xfs.c \
			ext.c \
			ntfs.c \
			hfs.c \
			vfat.c \
			exfat.c
