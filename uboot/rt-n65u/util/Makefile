OBJS = add_magic.o
CC = gcc
FLAGS = -g -O2 -Wall
LDFLAGS = 
BIN = addmg

all : ${BIN}

${BIN} : ${OBJS}
	${CC} $(FLAGS) -o $@  ${OBJS} ${LDFLAGS}

clean :
	rm -f *.o ${BIN}
	rm -f uboot.bin
