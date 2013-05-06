all: tcpcheck

tcpcheck: tcpcheck.o
	$(CC) $(CFLAGS) tcpcheck.o -o tcpcheck

clean:
	rm -f tcpcheck *~ *.o

romfs:
	$(STRIP) tcpcheck
	$(ROMFSINST) /usr/sbin/tcpcheck
