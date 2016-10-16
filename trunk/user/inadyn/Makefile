VERSION       = 1.99.15
NAME          = inadyn
EXEC          = src/$(NAME)
RM           ?= rm -f
SHAREDIR      = $(ROOTDIR)/user/shared

PLUGIN_OBJS   = src/plugin.o		plugins/common.o	\
		plugins/asuscomm.o	plugins/duckdns.o	\
		plugins/dyndns.o	plugins/changeip.o	\
		plugins/dnsexit.o	plugins/easydns.o	\
		plugins/freedns.o	plugins/generic.o	\
		plugins/sitelutions.o				\
		plugins/tzo.o		plugins/zoneedit.o	\
		plugins/zerigo.o	plugins/dhis.o		\
		plugins/dynv6.o		plugins/dynv6-ipv4.o	\
		plugins/dtdns.o		plugins/giradns.o	\
		plugins/duiadns.o	plugins/netassist.o	\
		plugins/ddnss.o

BASE_OBJS     = src/main.o	src/ddns.o	src/cache.o	\
		src/cmd.o	src/os.o	src/error.o	\
		src/http.o	src/ssl.o	src/tcp.o	\
		src/ip.o	src/sha1.o	src/base64.o	\
		src/md5.o

OBJS          = $(BASE_OBJS) $(PLUGIN_OBJS)
SRCS          = $(OBJS:.o=.c)
DEPS          = $(SRCS:.c=.d)

CFLAGS       += -W -Wall
CFLAGS       += -D_BSD_SOURCE -D_GNU_SOURCE
CFLAGS       += -Iinclude -DVERSION=\"$(VERSION)\"
CFLAGS       += -Iinclude -I$(SHAREDIR)
CFLAGS       += -ffunction-sections -fdata-sections

LDFLAGS      += -L$(SHAREDIR) -lshared
LDFLAGS      += -Wl,--gc-sections

LDLIBS       += -ldl

ifeq ($(CONFIG_FIRMWARE_INCLUDE_DDNS_SSL),y)
CFLAGS       += -DENABLE_SSL -DCONFIG_OPENSSL
LDLIBS       += -lssl -lcrypto
endif

# Pattern rules
.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

# Build rules
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

clean:
	-@$(RM) $(OBJS) $(DEPS) $(EXEC)

romfs:
	$(ROMFSINST) $(EXEC) /bin/inadyn


