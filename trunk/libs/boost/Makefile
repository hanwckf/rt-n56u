SRC_NAME = boost-1.72.0

all: 
	@echo "boost build ok"

clean:
	@echo "boost clean ok"

install:
	mkdir -p $(STAGEDIR)/include $(STAGEDIR)/lib ; \
	tar -xf $(SRC_NAME)/include/boost-headers.tar.xz -C $(STAGEDIR)/include ; \
	cp -rf $(SRC_NAME)/lib/*.a $(SRC_NAME)/lib/cmake $(STAGEDIR)/lib

romfs:
	

