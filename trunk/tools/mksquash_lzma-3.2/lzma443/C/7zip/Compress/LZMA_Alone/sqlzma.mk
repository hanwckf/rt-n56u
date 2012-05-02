
# Copyright (C) 2006 Junjiro Okajima
# Copyright (C) 2006 Tomas Matejicek, slax.org
#
# LICENSE follows the described one in lzma.txt.

# $Id: sqlzma.mk,v 1.2 2008-07-16 05:30:07 winfred Exp $

ifndef Sqlzma
$(error Sqlzma is not defined)
endif

include makefile

ifdef UseDebugFlags
DebugFlags = -Wall -O0 -g -UNDEBUG
endif
# -pthread
CXXFLAGS = ${CFLAGS} -D_REENTRANT -include pthread.h -DNDEBUG ${DebugFlags}
Tgt = liblzma_r.a

all: ${Tgt}

RObjs = LZMAEncoder_r.o Alloc_r.o LZInWindow_r.o CRC_r.o StreamUtils_r.o \
	OutBuffer_r.o RangeCoderBit_r.o
%_r.cc: ../LZMA/%.cpp
	ln -f $< $@
%_r.cc: ../LZ/%.cpp
	ln -f $< $@
%_r.cc: ../RangeCoder/%.cpp
	ln -f $< $@
%_r.cc: ../../Common/%.cpp
	ln -f $< $@
%_r.cc: ../../../Common/%.cpp
	ln -f $< $@
LZMAEncoder_r.o: CXXFLAGS += -I../LZMA
LZInWindow_r.o: CXXFLAGS += -I../LZ
RangeCoderBit_r.o: CXXFLAGS += -I../RangeCoder
OutBuffer_r.o StreamUtils_r.o: CXXFLAGS += -I../../Common
Alloc_r.o CRC_r.o: CXXFLAGS += -I../../../Common

comp.o: CXXFLAGS += -I${Sqlzma}
comp.o: comp.cc ${Sqlzma}/sqlzma.h

liblzma_r.a: ${RObjs} comp.o
	${AR} cr $@ $^

clean: clean_sqlzma
clean_sqlzma:
	$(RM) comp.o *_r.o ${Tgt} *~
	$(RM) Alloc_r.cc CRC_r.cc LZMAEncoder_r.cc RangeCoderBit_r.cc \
	LZInWindow_r.cc OutBuffer_r.cc StreamUtils_r.cc

# Local variables: ;
# compile-command: (concat "make Sqlzma=../../../../.. -f " (file-name-nondirectory (buffer-file-name)));
# End: ;
