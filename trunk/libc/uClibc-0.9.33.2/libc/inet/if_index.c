/* Copyright (C) 1997, 1998, 1999, 2000, 2002, 2003, 2004, 2005
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.

   Reworked Dec 2002 by Erik Andersen <andersen@codepoet.org>
 */

#define __FORCE_GLIBC
#include <features.h>
#include <string.h>
#include <alloca.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <libc-internal.h>
#include <not-cancel.h>

#include "netlinkaccess.h"

extern int __opensock(void) attribute_hidden;

unsigned int
if_nametoindex(const char* ifname)
{
#ifndef SIOCGIFINDEX
  __set_errno (ENOSYS);
  return 0;
#else
  struct ifreq ifr;
  int fd = __opensock();

  if (fd < 0)
    return 0;

  strncpy (ifr.ifr_name, ifname, sizeof (ifr.ifr_name));
  if (ioctl (fd, SIOCGIFINDEX, &ifr) < 0)
    {
      /* close never fails here, fd is just a unconnected socket.
       *int saved_errno = errno; */
      close_not_cancel_no_status(fd);
      /*if (saved_errno == EINVAL)
       *  __set_errno(ENOSYS); */
      return 0;
    }

  close_not_cancel_no_status(fd);
  return ifr.ifr_ifindex;
#endif
}
libc_hidden_def(if_nametoindex)

void
if_freenameindex (struct if_nameindex *ifn)
{
  struct if_nameindex *ptr = ifn;
  while (ptr->if_name || ptr->if_index)
    {
      free (ptr->if_name);
      ++ptr;
    }
  free (ifn);
}
libc_hidden_def(if_freenameindex)

#if !__ASSUME_NETLINK_SUPPORT
struct if_nameindex *
if_nameindex (void)
{
#ifndef SIOCGIFINDEX
  __set_errno (ENOSYS);
  return NULL;
#else
  int fd = __opensock ();
  struct ifconf ifc;
  unsigned int nifs, i;
  int rq_len;
  struct if_nameindex *idx = NULL;
# define RQ_IFS	4

  if (fd < 0)
    return NULL;

  ifc.ifc_buf = NULL;

  /* Guess on the correct buffer size... */
  rq_len = RQ_IFS * sizeof (struct ifreq);

  /* Read all the interfaces out of the kernel.  */
  /* Note: alloca's in this loop are diff from glibc because it's smaller */
  do
    {
      ifc.ifc_buf = extend_alloca (ifc.ifc_buf, rq_len, 2 * rq_len);
      ifc.ifc_len = rq_len;

      if (ioctl (fd, SIOCGIFCONF, &ifc) < 0)
	{
	  close_not_cancel_no_status (fd);
	  return NULL;
	}
    }
  while (ifc.ifc_len == rq_len);

  nifs = ifc.ifc_len / sizeof(struct ifreq);

  idx = malloc ((nifs + 1) * sizeof (struct if_nameindex));
  if (idx == NULL)
    {
      close_not_cancel_no_status (fd);
      __set_errno(ENOBUFS);
      return NULL;
    }

  for (i = 0; i < nifs; ++i)
    {
      struct ifreq *ifr = &ifc.ifc_req[i];
      idx[i].if_name = strdup (ifr->ifr_name);
      if (idx[i].if_name == NULL
	  || ioctl (fd, SIOCGIFINDEX, ifr) < 0)
	{
	  int saved_errno = errno;
	  unsigned int j;

	  for (j =  0; j < i; ++j)
	    free (idx[j].if_name);
	  free(idx);
	  close_not_cancel_no_status (fd);
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

  close_not_cancel_no_status (fd);
  return idx;
#endif
}
#else
struct if_nameindex *
if_nameindex (void)
{
  unsigned int nifs = 0;
  struct netlink_handle nh = { 0, 0, 0, NULL, NULL };
  struct if_nameindex *idx = NULL;
  struct netlink_res *nlp;

  if (__netlink_open (&nh) < 0)
    return NULL;


  /* Tell the kernel that we wish to get a list of all
     active interfaces.  Collect all data for every interface.  */
  if (__netlink_request (&nh, RTM_GETLINK) < 0)
    goto exit_free;

  /* Count the interfaces.  */
  for (nlp = nh.nlm_list; nlp; nlp = nlp->next)
    {
      struct nlmsghdr *nlh;
      size_t size = nlp->size;

      if (nlp->nlh == NULL)
	continue;

      /* Walk through all entries we got from the kernel and look, which
         message type they contain.  */
      for (nlh = nlp->nlh; NLMSG_OK (nlh, size); nlh = NLMSG_NEXT (nlh, size))
	{
	  /* Check if the message is what we want.  */
	  if ((pid_t) nlh->nlmsg_pid != nh.pid || nlh->nlmsg_seq != nlp->seq)
	    continue;

	  if (nlh->nlmsg_type == NLMSG_DONE)
	    break;		/* ok */

	  if (nlh->nlmsg_type == RTM_NEWLINK)
	    ++nifs;
	}
    }

  idx = malloc ((nifs + 1) * sizeof (struct if_nameindex));
  if (idx == NULL)
    {
    nomem:
      __set_errno (ENOBUFS);
      goto exit_free;
    }

  /* Add the interfaces.  */
  nifs = 0;
  for (nlp = nh.nlm_list; nlp; nlp = nlp->next)
    {
      struct nlmsghdr *nlh;
      size_t size = nlp->size;

      if (nlp->nlh == NULL)
	continue;

      /* Walk through all entries we got from the kernel and look, which
         message type they contain.  */
      for (nlh = nlp->nlh; NLMSG_OK (nlh, size); nlh = NLMSG_NEXT (nlh, size))
	{
	  /* Check if the message is what we want.  */
	  if ((pid_t) nlh->nlmsg_pid != nh.pid || nlh->nlmsg_seq != nlp->seq)
	    continue;

	  if (nlh->nlmsg_type == NLMSG_DONE)
	    break;		/* ok */

	  if (nlh->nlmsg_type == RTM_NEWLINK)
	    {
	      struct ifinfomsg *ifim = (struct ifinfomsg *) NLMSG_DATA (nlh);
	      struct rtattr *rta = IFLA_RTA (ifim);
	      size_t rtasize = IFLA_PAYLOAD (nlh);

	      idx[nifs].if_index = ifim->ifi_index;

	      while (RTA_OK (rta, rtasize))
		{
		  char *rta_data = RTA_DATA (rta);
		  size_t rta_payload = RTA_PAYLOAD (rta);

		  if (rta->rta_type == IFLA_IFNAME)
		    {
		      idx[nifs].if_name = strndup (rta_data, rta_payload);
		      if (idx[nifs].if_name == NULL)
			{
			  idx[nifs].if_index = 0;
			  if_freenameindex (idx);
			  idx = NULL;
			  goto nomem;
			}
		      break;
		    }

		  rta = RTA_NEXT (rta, rtasize);
		}

	      ++nifs;
	    }
	}
    }

  idx[nifs].if_index = 0;
  idx[nifs].if_name = NULL;

 exit_free:
  __netlink_free_handle (&nh);
  __netlink_close (&nh);

  return idx;
}
#endif
libc_hidden_def(if_nameindex)

char *
if_indextoname (unsigned int ifindex, char *ifname)
{
#if !defined SIOCGIFINDEX
  __set_errno (ENOSYS);
  return NULL;
#else
# ifdef SIOCGIFNAME
  /* Use ioctl to avoid searching the list. */
  struct ifreq ifr;
  int fd;

  fd = __opensock ();

  if (fd < 0)
    return NULL;

  ifr.ifr_ifindex = ifindex;
  if (ioctl (fd, SIOCGIFNAME, &ifr) < 0)
    {
      int serrno = errno;
      close_not_cancel_no_status (fd);
      if (serrno == ENODEV)
	/* POSIX requires ENXIO.  */
	serrno = ENXIO;
      __set_errno (serrno);
      return NULL;
  }
  close_not_cancel_no_status (fd);

  return strncpy (ifname, ifr.ifr_name, IFNAMSIZ);
# else
  struct if_nameindex *idx;
  struct if_nameindex *p;
  char *result = NULL;

  idx = if_nameindex();

  if (idx != NULL)
    {
      for (p = idx; p->if_index || p->if_name; ++p)
	if (p->if_index == ifindex)
	  {
	    result = strncpy (ifname, p->if_name, IFNAMSIZ);
	    break;
	  }

      if_freenameindex (idx);

      if (result == NULL)
	__set_errno (ENXIO);
    }
  return result;
# endif
#endif
}

