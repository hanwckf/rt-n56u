OBJS = add_magic.o
CC = gcc
FLAGS = -g -O2 -Wall
LDFLAGS = 
BIN = addmg

all : ${BIN}
	cp -f ../uboot.bin .
	./${BIN} uboot.bin

${BIN} : ${OBJS}
	${CC} $(FLAGS) -o $@  ${OBJS} ${LDFLAGS}

clean :
	rm -f *.o ${BIN}
	rm -f uboot.bin
