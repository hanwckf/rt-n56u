EXTRA_DIST = \
	BSDCOPYING \
	SECURITY \
	build-aux/mtr.bat \
	img/mtr_icon.xpm
	$(TEST_FILES)

sbin_PROGRAMS = mtr mtr-packet
TESTS = \
	test/cmdparse.py \
	test/param.py \
	test/probe.py

TEST_FILES = \
	test/cmdparse.py \
	test/mtrpacket.py \
	test/param.py \
	test/probe.py \
	test/lint.sh
EXTRA_DIST += $(TEST_FILES)

PATHFILES =
CLEANFILES = $(PATHFILES)
EXTRA_DIST += $(PATHFILES:=.in)

#
#  We would use % pattern matching here, but that is a GNU make
#  extension and doesn't work on FreeBSD.
#
mtr-packet.8: $(srcdir)/man/mtr-packet.8.in
	$(AM_V_GEN) $(srcdir)/build-aux/mangen.sh "$(VERSION)" \
		$(srcdir)/man/mtr-packet.8.in $@

mtr.8: $(srcdir)/man/mtr.8.in
	$(AM_V_GEN) $(srcdir)/build-aux/mangen.sh "$(VERSION)" \
		$(srcdir)/man/mtr.8.in $@

$(PATHFILES): Makefile

dist_man_MANS = mtr.8 mtr-packet.8
PATHFILES += man/mtr.8 man/mtr-packet.8

install-exec-hook: 
	`setcap cap_net_raw+ep $(DESTDIR)$(sbindir)/mtr-packet` \
	|| chmod u+s $(DESTDIR)$(sbindir)/mtr-packet

mtr_SOURCES = ui/mtr.c ui/mtr.h \
              ui/net.c ui/net.h \
              ui/cmdpipe.c ui/cmdpipe.h \
              ui/dns.c ui/dns.h \
              ui/raw.c ui/raw.h \
              ui/split.c ui/split.h \
              ui/display.c ui/display.h \
              ui/report.c ui/report.h \
              ui/select.c ui/select.h \
              ui/utils.c ui/utils.h \
              packet/cmdparse.c packet/cmdparse.h \
              ui/mtr-curses.h \
              img/mtr_icon.xpm \
              ui/mtr-gtk.h

if WITH_ERROR
mtr_SOURCES += \
	portability/error.h \
	portability/error.c
endif

if WITH_GETOPT
mtr_SOURCES += \
	portability/getopt.h \
	portability/getopt.c \
	portability/getopt1.c
endif

if WITH_IPINFO
mtr_SOURCES += ui/asn.c ui/asn.h
endif

if WITH_CURSES
mtr_SOURCES += ui/curses.c
endif

if WITH_GTK
mtr_SOURCES += ui/gtk.c
endif

mtr_INCLUDES = $(GLIB_CFLAGS) -I$(top_builddir) -I$(top_srcdir)
mtr_CFLAGS = $(GTK_CFLAGS) $(NCURSES_CFLAGS)
mtr_LDADD = $(GTK_LIBS) $(NCURSES_LIBS) $(RESOLV_LIBS)


mtr_packet_SOURCES = \
	portability/queue.h \
	packet/packet.c \
	packet/cmdparse.c packet/cmdparse.h \
	packet/command.c packet/command.h \
	packet/platform.h \
	packet/probe.c packet/probe.h \
	packet/protocols.h \
	packet/timeval.c packet/timeval.h \
	packet/wait.h

mtr_packet_LDADD = $(CAP_LIBS)


if CYGWIN

mtr_packet_SOURCES += \
	packet/command_cygwin.c packet/command_cygwin.h \
	packet/probe_cygwin.c packet/probe_cygwin.h \
	packet/wait_cygwin.c
mtr_packet_LDADD += -lcygwin -liphlpapi -lws2_32

dist_windows_aux = \
	$(srcdir)/build-aux/mtr.bat \
	$(srcdir)/AUTHORS \
	$(srcdir)/BSDCOPYING \
	$(srcdir)/COPYING \
	$(srcdir)/README \
	$(srcdir)/NEWS

distwindir = $(distdir)-win-$(host_cpu)

#  Bundle necessary files for a Windows binary distribution
distdir-win: $(dist_windows_aux) mtr.exe mtr-packet.exe
	rm -fr $(distwindir)
	mkdir -p $(distwindir) $(distwindir)/bin $(distwindir)/terminfo
	cp $(dist_windows_aux) -t $(distwindir)
	cp mtr.exe mtr-packet.exe -t $(distwindir)/bin
	ldd mtr.exe | grep -v cygdrive | awk '{ print $$3 }' | xargs cp -t $(distwindir)/bin
	cp `find /usr/share/terminfo -name cygwin | xargs dirname` -r $(distwindir)/terminfo

#  Zip up a Windows binary distribution
dist-windows-bin: distdir-win
	rm -f $(distwindir).zip
	zip -rq $(distwindir).zip $(distwindir)
	rm -fr $(distwindir)

else  # if CYGWIN

check_PROGRAMS = mtr-packet-listen

mtr_packet_SOURCES += \
	packet/command_unix.c packet/command_unix.h \
	packet/construct_unix.c packet/construct_unix.h \
	packet/deconstruct_unix.c packet/deconstruct_unix.h \
	packet/probe_unix.c packet/probe_unix.h \
	packet/wait_unix.c

mtr_packet_listen_SOURCES = \
	test/packet_listen.c

endif  # if CYGWIN


if BUILD_BASH_COMPLETION
dist_bashcompletion_DATA = bash-completion/mtr
endif
