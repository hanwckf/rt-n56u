TOPDIR=../../

CFLAGS+=-DHAVE_CONFIG_H -Iinc

LIBTHREADUTIL=libthreadutil.a
LIBTHREADUTIL_VERS=1.3.1
LIBTHREADUTIL_SHARED=libthreadutil.so.$(LIBTHREADUTIL_VERS)

OBJS    := src/FreeList.o  src/iasnprintf.o  src/LinkedList.o  src/ThreadPool.o  src/TimerThread.o

all: $(LIBTHREADUTIL)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

shared: all
	$(LD) $(LDFLAGS) -soname=$(LIBTHREADUTIL_SHARED) \
		-o $(LIBTHREADUTIL_SHARED) --whole-archive $(LIBTHREADUTIL) \
		-lc -lpthread $(LDADD_LIBFLOAT);

$(LIBTHREADUTIL) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBTHREADUTIL) $(OBJS)

$(OBJS): Makefile

clean:
	$(RM) src/*.[oa] $(LIBTHREADUTIL_SHARED)* 

