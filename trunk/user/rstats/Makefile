OBJS = rstats.o traffic.o
BIN = rstats

CFLAGS += -Wall -s
CFLAGS += -I../shared -I../shared/include
CFLAGS += $(if $(CONFIG_RAETH_GMAC2),,-DUSE_SINGLE_MAC)

LDFLAGS += -L../shared -lshared -s

all: ${BIN}
	${STRIP} ${BIN}

romfs:
	$(ROMFSINST) /usr/sbin/rstats

${BIN}: ${OBJS}
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o ${BIN}


