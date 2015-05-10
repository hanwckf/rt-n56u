sinclude $(TOPDIR)/.config

OBJS = add_magic.o
CC = gcc
CFLAGS = -g -O2 -Wall
CPPFLAGS =
LDFLAGS =
BIN = addmg

ifeq ($(UBOOT_STAGE1),y)
CFLAGS += -DFLSH_SIZE=131072
else ifeq ($(UBI_SUPPORT),y)
CFLAGS += -DFLSH_SIZE=262144
endif

all : $(BIN)

$(BIN) : $(OBJS) $(TOPDIR)/.config
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean :
	rm -f *.o $(BIN)

.depend:
