THISDIR = $(shell pwd)
all:
	@echo "napt66 build done"

romfs:
	${ROMFSINST} -d $(THISDIR)/scripts/start_napt66 /bin/start_napt66
	${ROMFSINST} -d $(THISDIR)/scripts/disable_ip6tables /bin/disable_ip6tables
	${ROMFSINST} -S -d $(THISDIR)/mt7620/napt66.ko /etc_ro/napt66.ko

clean:
	@echo "napt66 clean done"
