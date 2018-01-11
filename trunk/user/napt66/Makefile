SRC_NAME = napt66-v1.1

all:
	$(MAKE) -C $(SRC_NAME)

romfs:
	${ROMFSINST} -d ./scripts/start_napt66 /bin/start_napt66
	${ROMFSINST} -d ./scripts/disable_ip6tables /bin/disable_ip6tables
	${ROMFSINST} -S -d ./$(SRC_NAME)/napt66.ko /etc_ro/napt66.ko

clean:
	$(MAKE) -C $(SRC_NAME) clean
