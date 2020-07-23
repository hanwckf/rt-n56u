CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2
LIBS = -lm
SRCS = dns2tcp.c
OBJS = $(SRCS:.c=.o)
MAIN = dns2tcp
DESTDIR = /usr/local/bin

EVCFLAGS = -w -O2
EVSRCFILE = libev/ev.c
EVOBJFILE = ev.o

.PHONY: all install clean

all: $(MAIN)

install: $(MAIN)
	mkdir -p $(DESTDIR)
	install -m 0755 $(MAIN) $(DESTDIR)

clean:
	$(RM) *.o $(MAIN)

$(MAIN): $(EVOBJFILE) $(OBJS)
	$(CC) $(CFLAGS) -s -o $(MAIN) $(OBJS) $(EVOBJFILE) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(EVOBJFILE): $(EVSRCFILE)
	$(CC) $(EVCFLAGS) -c $(EVSRCFILE) -o $(EVOBJFILE)
