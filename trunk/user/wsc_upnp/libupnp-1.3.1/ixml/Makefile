TOPDIR=../../

CFLAGS+=-DHAVE_CONFIG_H -Iinc -Isrc/inc

LIBIXML=libixml.a
LIBIXML_VERS=1.3.1
LIBIXML_SHARED=libixml.so.$(LIBIXML_VERS)

OBJS    := src/attr.o src/element.o  src/ixmlmembuf.o src/namedNodeMap.o src/nodeList.o \
	   src/document.o  src/ixml.o  src/ixmlparser.o  src/node.o


all: $(LIBIXML)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

shared: all
	$(LD) $(LDFLAGS) -soname=$(LIBIXML_SHARED) \
		-o $(LIBIXML_SHARED) --whole-archive $(LIBIXML) \
		-lc $(LDADD_LIBFLOAT);

$(LIBIXML) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBIXML) $(OBJS)

$(OBJS): Makefile

clean:
	$(RM) src/*.[oa] $(LIBIXML_SHARED)* 

