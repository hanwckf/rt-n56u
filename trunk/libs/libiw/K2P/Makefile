THISDIR = $(shell pwd)

all:
	@echo "libiw done"

clean:
	@echo "libiw clean done"

install:
	@echo "libiw install done"

romfs:
	cp -fP libiw.so* $(ROMFSDIR)/lib
	$(ROMFSINST) -S -d $(THISDIR)/iwpriv /bin/iwpriv
