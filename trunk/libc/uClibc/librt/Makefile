#
# Makefile for librt
#

TOPDIR=../
include $(TOPDIR)Rules.mak

LIBRT=librt.a
LIBRT_SHARED=librt.so
LIBRT_SHARED_FULLNAME=librt-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so

# uClibc's librt lacks all aio routines, all clock routines,
# and all shm routines
CSRC=mq_open.c mq_close.c mq_unlink.c mq_getsetattr.c \
     mq_send.c mq_receive.c mq_notify.c \
     timer_create.c timer_delete.c \
     timer_settime.c timer_gettime.c timer_getoverr.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

all: $(LIBRT)

$(LIBRT) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBRT) $(OBJS)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBRT)
	$(INSTALL) -m 644 $(LIBRT) $(TOPDIR)lib/

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

shared: all
	$(LD) $(LDFLAGS) -soname=$(LIBRT_SHARED).$(MAJOR_VERSION) \
		-o $(LIBRT_SHARED_FULLNAME) --whole-archive $(LIBRT) \
		--no-whole-archive $(TOPDIR)libc/misc/internals/interp.o \
		-L$(TOPDIR)lib -lc $(LDADD_LIBFLOAT) $(LIBGCC);
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBRT_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBRT_SHARED).$(MAJOR_VERSION)
	$(INSTALL) -m 644 $(LIBRT_SHARED_FULLNAME) $(TOPDIR)lib
	$(LN) -sf $(LIBRT_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBRT_SHARED)
	$(LN) -sf $(LIBRT_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBRT_SHARED).$(MAJOR_VERSION)

clean:
	$(RM) *.[oa] *~ core $(LIBRT_SHARED)* $(LIBRT_SHARED_FULLNAME)*
