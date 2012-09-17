/* Copyright (C) 1997,98,99,2000,02 Free Software Foundation, Inc.
 * This file is part of the GNU C Library.
 * 
 * The GNU C Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.  
 *
 * The GNU C Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.  
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the GNU C Library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA.  
 *
 * Reworked Dec 2002 by Erik Andersen <andersen@codepoet.org>
 */

#define __FORCE_GLIBC
#include <features.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

static int __opensock(void)
{
    int fd;
#ifdef __UCLIBC_HAS_IPV6__
    fd=socket(AF_INET6,SOCK_DGRAM,0);
    if (fd<0)
#endif /* __UCLIBC_HAS_IPV6__ */
	fd=socket(AF_INET,SOCK_DGRAM,0);
    return(fd);
}

unsigned int if_nametoindex(const char* ifname) 
{
#ifndef SIOCGIFINDEX
    __set_errno (ENOSYS);
    return 0;
#else
    int fd;
    struct ifreq ifr;

    fd = __opensock();
    if (fd < 0)
	return 0;
    strncpy (ifr.ifr_name, ifname, sizeof (ifr.ifr_name));
    if (ioctl(fd,SIOCGIFINDEX,&ifr) < 0) {
	int saved_errno = errno;
	close(fd);
	if (saved_errno == EINVAL)
	    __set_errno(ENOSYS);
	return 0;
    }
    close(fd);
    return ifr.ifr_ifindex;

#endif /* SIOCGIFINDEX */
}

void if_freenameindex (struct if_nameindex *ifn)
{
    struct if_nameindex *ptr = ifn;
    while (ptr->if_name || ptr->if_index) {
	if (ptr->if_name) {
	    free (ptr->if_name);
	}
	++ptr;
    }
    free (ifn);
}

struct if_nameindex * if_nameindex (void)
{
#ifndef SIOCGIFINDEX
    __set_errno (ENOSYS);
    return NULL;
#else
    int fd;
    struct ifconf ifc;
    unsigned int nifs, i;
    int rq_len;
    struct if_nameindex *idx = NULL;
# define RQ_IFS	4

    fd = __opensock();
    if (fd < 0)
	return 0;

    ifc.ifc_buf = NULL;

    /* Guess on the correct buffer size... */
    rq_len = RQ_IFS * sizeof (struct ifreq);

    /* Read all the interfaces out of the kernel.  */
    do {
	ifc.ifc_buf = realloc(ifc.ifc_buf, ifc.ifc_len = rq_len);
	if (ifc.ifc_buf == NULL || ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
	    close(fd);
	    return NULL;
	}
	rq_len *= 2;
    } while (ifc.ifc_len == rq_len);

    nifs = ifc.ifc_len / sizeof(struct ifreq);

    idx = malloc ((nifs + 1) * sizeof(struct if_nameindex));
    if (idx == NULL) {
	close(fd);
	__set_errno(ENOBUFS);
	return NULL;
    }

    for (i = 0; i < nifs; ++i) {
	struct ifreq *ifr = &ifc.ifc_req[i];
	idx[i].if_name = strdup (ifr->ifr_name);
	if (idx[i].if_name == NULL || ioctl(fd,SIOCGIFINDEX,ifr) < 0) {
	    int saved_errno = errno;
	    unsigned int j;
	    for (j =  0; j < i; ++j)
		free (idx[j].if_name);
	    free(idx);
	    close(fd);
	    if (saved_errno == EINVAL)
		saved_errno = ENOSYS;
	    else if (saved_errno == ENOMEM)
		saved_errno = ENOBUFS;
	    __set_errno (saved_errno);
	    return NULL;
	}
	idx[i].if_index = ifr->ifr_ifindex;
    }

    idx[i].if_index = 0;
    idx[i].if_name = NULL;

    close(fd);
    return idx;
#endif
}

char * if_indextoname (unsigned int ifindex, char *ifname)
{
#ifdef SIOCGIFNAME
  /* Use ioctl to avoid searching the list. */
  struct ifreq ifr;
  int fd, saved_errno;

  fd = __opensock ();
  
  if (fd < 0)
      return NULL;

  ifr.ifr_ifindex = ifindex;
  if (ioctl (fd, SIOCGIFNAME, &ifr) < 0) {
      saved_errno = errno;
      close (fd);
      __set_errno (saved_errno);
      return NULL;
  }
  close (fd);

  return strncpy (ifname, ifr.ifr_name, IFNAMSIZ);
#else
    struct if_nameindex *idx;
    struct if_nameindex *p;
    char *result = NULL;

    idx = if_nameindex();
    if (idx != NULL) {
	for (p = idx; p->if_index || p->if_name; ++p) {
	    if (p->if_index == ifindex) {
		result = strncpy (ifname, p->if_name, IFNAMSIZ);
		break;
	    }
	}
	if_freenameindex (idx);
    }
    return result;
#endif
}

