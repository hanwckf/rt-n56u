# Under Solaris, you need to 
#    CFLAGS += -xO2 -Xc
#    LDLIBS += -lnsl -lsocket
# Some versions of Linux may need
#    CFLAGS += -D_GNU_SOURCE
# To cross-compile
#    CC = arm-linux-gcc

CFLAGS += -Wall -O

SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user

CFLAGS += -DASUS -DASUS_NVRAM
CFLAGS += -I$(SHDIR)/include
LDFLAGS = -L$(SHDIR) -lshared

all: ntpclient

#test: ntpclient
#	./ntpclient -d -r <test.dat

ntpclient: ntpclient.o phaselock.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

romfs: ntpclient
	$(STRIP) ntpclient
	$(ROMFSINST) /usr/sbin/ntpclient 

clean:
	rm -f ntpclient *.o
