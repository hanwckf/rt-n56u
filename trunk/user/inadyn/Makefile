# Makefile for INADYN, a simple and small ddns client.          -*-Makefile-*-

VERSION      ?= 1.99.2
NAME          = inadyn
EXEC          = src/$(NAME)
OBJS          = src/base64.o src/md5.o src/dyndns.o src/errorcode.o src/get_cmd.o \
                src/http_client.o src/ip.o src/main.o src/os_unix.o src/os_windows.o   \
                src/os.o src/os_psos.o src/tcp.o src/inadyn_cmd.o src/sha1.o
SHAREDIR      = $(ROOTDIR)/user/shared
CFLAGS       += -W -Wall
CFLAGS       += -Iinclude -I$(SHAREDIR)/include -DVERSION_STRING=\"$(VERSION)\"
CFLAGS       += -ffunction-sections -fdata-sections
LDFLAGS       = -L$(SHAREDIR) -lshared
LDFLAGS      += -Wl,--gc-sections
LDLIBS       += -lresolv

# Pattern rules
.c.o:
	@printf "  CC      $@\n"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

# Build rules
all: $(EXEC)

$(EXEC): $(OBJS)
	@printf "  LINK    $@\n"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

clean:
	rm -f $(OBJS) core $(EXEC) *.o *.map .*.d *.out tags TAGS

romfs:
	$(ROMFSINST) $(EXEC) /bin/inadyn
