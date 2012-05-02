all: tcpcheck

tcpcheck: tcpcheck.o
	$(CC) tcpcheck.o -o tcpcheck

clean:
	rm -f tcpcheck *~ *.o

romfs:
	$(STRIP) tcpcheck
#	cp tcpcheck httpdcheck
	$(ROMFSINST) /usr/sbin/tcpcheck
#	$(ROMFSINST) /usr/sbin/httpdcheck
#	cd $(INSTALLDIR)/usr/sbin && ln -sf tcpcheck httpdcheck
