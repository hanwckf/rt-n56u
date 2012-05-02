CC=gcc
STRIP=strip

all :	install

install :
	rm -rf ./bin
	mkdir bin
	make -C mkimage
	make -C sstrip
	make -C mksquash_lzma-3.2 install


clean:
	make -C sstrip clean
	make -C mkimage clean
	make -C mksquash_lzma-3.2 clean
	rm -rf ./bin
	rm -rf ./headers

mrproper:
	echo "OK"