BUILT_SOURCES =

sbin_PROGRAMS = parted

AM_CFLAGS = $(WARN_CFLAGS) $(WERROR_CFLAGS)
partedincludedir = \
  -I$(top_srcdir)/lib -I$(top_builddir)/include -I$(top_srcdir)/include

parted_SOURCES = command.c	\
		 command.h	\
		 parted.c	\
		 strlist.c	\
		 strlist.h	\
		 ui.c		\
		 ui.h		\
		 table.c	\
		 table.h

noinst_LIBRARIES = libver.a
nodist_libver_a_SOURCES = version.c version.h

BUILT_SOURCES += version.c
version.c: Makefile
	$(AM_V_GEN)rm -f $@
	$(AM_V_at)printf '#include <config.h>\n' > $@t
	$(AM_V_at)printf 'char const *Version = "$(PACKAGE_VERSION)";\n' >> $@t
	$(AM_V_at)chmod a-w $@t
	$(AM_V_at)mv $@t $@

BUILT_SOURCES += version.h
version.h: Makefile
	$(AM_V_GEN)rm -f $@
	$(AM_V_at)printf 'extern char const *Version;\n' > $@t
	$(AM_V_at)chmod a-w $@t
	$(AM_V_at)mv $@t $@

DISTCLEANFILES = version.c version.h
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

parted_LDADD = \
  libver.a \
  $(top_builddir)/libparted/libparted.la \
  $(INTLLIBS) $(LIBS) \
  $(PARTED_LIBS)

parted_LDFLAGS = $(PARTEDLDFLAGS)

# Tell the linker to omit references to unused shared libraries.
parted_LDFLAGS += $(IGNORE_UNUSED_LIBRARIES_CFLAGS)

INCLUDES	= $(partedincludedir) $(INTLINCS)

MAINTAINERCLEANFILES += Makefile.in
