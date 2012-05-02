# Makefile for uClibc
#
# Copyright (C) 2000 by Lineo, inc.
# Copyright (C) 2000,2001,2002 Erik Andersen <andersen@uclibc.org>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Library General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
# details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
# Derived in part from the Linux-8086 C library, the GNU C Library, and several
# other sundry sources.  Files within this library are copyright by their
# respective copyright holders.

TOPDIR=../../../
include $(TOPDIR)Rules.mak
CFLAGS+=-I$(TOPDIR)libpthread/linuxthreads/sysdeps/pthread

ifeq ($(strip $(UCLIBC_HAS_FULL_RPC)),y)
CSRC :=auth_none.c auth_unix.c authunix_prot.c bindresvport.c \
	clnt_generic.c clnt_perror.c clnt_raw.c clnt_simple.c clnt_tcp.c \
	clnt_udp.c rpc_dtablesize.c get_myaddress.c getrpcent.c getrpcport.c \
	pmap_clnt.c pmap_getmaps.c pmap_getport.c pmap_prot.c \
	pmap_prot2.c pmap_rmt.c rpc_prot.c rpc_commondata.c rpc_callmsg.c \
	svc.c svc_auth.c svc_auth_unix.c svc_raw.c svc_run.c svc_simple.c \
	svc_tcp.c svc_udp.c xdr.c xdr_array.c xdr_float.c xdr_mem.c \
	xdr_rec.c xdr_reference.c xdr_stdio.c \
	rtime.c clnt_unix.c svc_unix.c create_xid.c xdr_intXX_t.c rcmd.c \
	rexec.c sa_len.c ruserpass.c rpc_thread.c
else
# For now, only compile the stuff needed to do an NFS mount....
CSRC:=create_xid.c pmap_clnt.c pmap_getmaps.c pmap_getport.c \
	pmap_prot.c pmap_prot2.c clnt_simple.c clnt_perror.c \
	clnt_tcp.c clnt_udp.c bindresvport.c authunix_prot.c \
	auth_none.c auth_unix.c xdr.c xdr_array.c xdr_rec.c \
	xdr_reference.c xdr_mem.c svc.c svc_auth.c svc_auth_unix.c \
	svc_simple.c svc_tcp.c svc_udp.c \
	rpc_callmsg.c rpc_prot.c rpc_dtablesize.c rpc_commondata.c \
	rpc_thread.c rcmd.c rexec.c sa_len.c ruserpass.c rtime.c \
	getrpcent.c get_myaddress.c clnt_generic.c
endif
COBJS=$(patsubst %.c,%.o, $(CSRC))
OBJS=$(COBJS)

OBJ_LIST=../../obj.inet.rpc

all: $(OBJ_LIST)

$(OBJ_LIST): $(OBJS)
	echo $(patsubst %, inet/rpc/%, $(OBJS)) > $(OBJ_LIST)

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
