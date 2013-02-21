partedincludedir = \
  -I$(top_srcdir)/lib -I$(top_builddir)/include -I$(top_srcdir)/include

AM_CFLAGS = $(WARN_CFLAGS) $(WERROR_CFLAGS)
sbin_PROGRAMS = partprobe

partprobe_SOURCES = partprobe.c

partprobe_LDADD = \
  $(top_builddir)/libparted/libparted.la \
  $(INTLLIBS) $(LIBS) \
  $(PARTED_LIBS)

partprobe_LDFLAGS = $(PARTEDLDFLAGS)

INCLUDES	= $(partedincludedir) $(INTLINCS)
