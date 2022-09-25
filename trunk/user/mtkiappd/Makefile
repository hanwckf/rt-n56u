THISDIR = $(shell pwd)
EXEC = mtkiappd
OBJS = rtmpiapp.o mt_iapp_util.o

# If you want to debug daemon, add following line
CFLAGS += -I./ -DDBG 
#-DSYSLOG
$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /sbin/$(EXEC)
	$(ROMFSINST) -p +x $(THISDIR)/iappd.sh /usr/bin/iappd.sh
	

clean:
	rm -f core *.o mtkiappd
