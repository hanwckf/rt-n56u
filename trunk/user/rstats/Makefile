OBJS = rstats.o traffic.o
BIN = rstats

CFLAGS += -Wall -s
CFLAGS += -DASUS_NVRAM
CFLAGS += -I../shared/include

LDFLAGS += -L../shared -lshared -s
LDFLAGS += -L../netconf 

all: ${BIN}
	${STRIP} ${BIN}

install: all
	install -D ${BIN} ${INSTALLDIR}/usr/sbin/${BIN}

romfs:
	$(ROMFSINST) /usr/sbin/rstats

${BIN}: ${OBJS}
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o ${BIN}


