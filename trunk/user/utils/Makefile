all:
	$(CC) $(CFLAGS) $(LDFLAGS) cpu.c -o cpu
	$(CC) $(CFLAGS) $(LDFLAGS) swapoff.c -o swapoff

clean:
	rm -f cpu

romfs:
	$(ROMFSINST) /usr/bin/cpu
