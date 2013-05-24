include $(top_srcdir)/config/include-Makefile.am

AM_CPPFLAGS += -I$(ul_libblkid_incdir) -I$(ul_libblkid_srcdir)
libblkid_partitions_la_LIBADD =

noinst_LTLIBRARIES = libblkid_partitions.la
libblkid_partitions_la_SOURCES = partitions.c \
				partitions.h \
				blkid_parttypes.h \
				bsd.c \
				mac.c \
				dos.c \
				dos.h \
				gpt.c
