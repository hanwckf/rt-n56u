## Makefile.am for Oniguruma

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src test sample

EXTRA_DIST = oniguruma.pc.in HISTORY README.ja README.md \
	index.html index_ja.html \
	CMakeLists.txt dist.info oniguruma.pc.cmake.in cmake/dist.cmake \
	src/config.h.cmake.in \
	doc/API doc/API.ja doc/RE doc/RE.ja doc/FAQ doc/FAQ.ja \
	doc/UNICODE_PROPERTIES \
	src/Makefile.windows src/config.h.win32 src/config.h.win64 \
	windows/testc.c contributed/libfuzzer-onig.cpp

bin_SCRIPTS = onig-config

onig-config: onig-config.in

do_subst = sed                                          \
      -e 's,[@]datadir[@],$(datadir),g'                 \
      -e 's,[@]datarootdir[@],$(datarootdir),g'         \
      -e 's,[@]PACKAGE_VERSION[@],$(PACKAGE_VERSION),g' \
      -e 's,[@]prefix[@],$(prefix),g'                   \
      -e 's,[@]exec_prefix[@],$(exec_prefix),g'         \
      -e 's,[@]libdir[@],$(libdir),g'                   \
      -e 's,[@]includedir[@],$(includedir),g'

oniguruma.pc: $(srcdir)/oniguruma.pc.in Makefile
	$(do_subst) < $(srcdir)/oniguruma.pc.in > $(@)

pkgconfigdir   = $(libdir)/pkgconfig
pkgconfig_DATA = oniguruma.pc
