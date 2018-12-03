SRC_NAME=lzma-4.65

all:
	make -j$(HOST_NCPU) -C $(SRC_NAME)/CPP/7zip/Compress/LZMA_Alone -f makefile.gcc ;\
	cp $(SRC_NAME)/CPP/7zip/Compress/LZMA_Alone/lzma ./lzma

clean:
	make -C $(SRC_NAME)/CPP/7zip/Compress/LZMA_Alone -f makefile.gcc clean
	rm -f lzma
