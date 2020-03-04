#CC = mipsel-linux-uclibc-gcc
CFLAGS = -std=c99 -Wall -Wextra -O3 -pthread
INCLUDES =
LDFLAGS =
#LIBS = -luv
LIBS = -l:libuv.a
SRCS = ipt2socks.c lrucache.c netutils.c
OBJS = $(SRCS:.c=.o)
MAIN = ipt2socks
DESTDIR = /usr

.PHONY: all install clean

all: $(MAIN)

install: $(MAIN)
	mkdir -p $(DESTDIR)
	install -m 0755 $(MAIN) $(DESTDIR)

clean:
	$(RM) *.o $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -s -o $(MAIN) $(OBJS) $(LDFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
