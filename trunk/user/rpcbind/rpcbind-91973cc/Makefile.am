AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = \
	-DCHECK_LOCAL \
	-DPORTMAP \
	-DFACILITY=LOG_MAIL \
	-DSEVERITY=LOG_INFO \
	-DRPCBIND_STATEDIR="\"$(statedir)\"" \
	-DRPCBIND_USER="\"$(rpcuser)\"" \
	-DNSS_MODULES="\"$(nss_modules)\"" \
	-D_GNU_SOURCE \
	$(TIRPC_CFLAGS)

if DEBUG
AM_CPPFLAGS +=	-DRPCBIND_DEBUG -DDEBUG_RMTCALL
AM_CPPFLAGS +=	-DND_DEBUG -DBIND_DEBUG
endif

if LIBSETDEBUG
AM_CPPFLAGS +=	-DLIB_SET_DEBUG
endif

if IPV6
AM_CPPFLAGS += -DINET6
endif

if WARMSTART
AM_CPPFLAGS +=	-DWARMSTART
endif

if LIBWRAP
AM_CPPFLAGS +=	-DLIBWRAP
endif

bin_PROGRAMS = rpcinfo
sbin_PROGRAMS = rpcbind

rpcbind_SOURCES = \
	src/check_bound.c \
	src/pmap_svc.c \
	src/rpcb_stat.c \
	src/rpcb_svc.c \
	src/rpcb_svc_4.c \
	src/rpcb_svc_com.c \
	src/rpcbind.c \
	src/rpcbind.h \
	src/security.c \
	src/util.c \
	src/warmstart.c \
	src/xlog.c
rpcbind_LDADD = $(TIRPC_LIBS)

if SYSTEMD
AM_CPPFLAGS += $(SYSTEMD_CFLAGS) -DSYSTEMD

rpcbind_LDADD += $(SYSTEMD_LIBS)

systemdsystemunit_DATA = \
	systemd/rpcbind.service \
	systemd/rpcbind.socket
endif

rpcinfo_SOURCES =       src/rpcinfo.c
rpcinfo_LDADD   =       $(TIRPC_LIBS)

dist_man8_MANS = man/rpcbind.8 man/rpcinfo.8
