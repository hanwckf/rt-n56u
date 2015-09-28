CFLAGS = -I./ -DUSE_HOSTCC

all: mkimage

clean:
	rm -f mkimage *.o

.c.o:
	$(HOSTCC) $(CFLAGS) -c $^

.depend:

mkimage: mkimage.o crc32.o crc.o
	$(HOSTCC) -o $@ $^


