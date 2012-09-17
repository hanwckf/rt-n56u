/* Copyright (C) 2000 Free Software Foundation, Inc.
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
   02111-1307 USA.  */

/* Linux version.  */

#ifndef _BITS_ERRQUEUE_H
#define _BITS_ERRQUEUE_H  1

#include <sys/types.h>
#include <sys/socket.h>

struct sock_extended_err
  {
    u_int32_t ee_errno;
    u_int8_t ee_origin;
    u_int8_t ee_type;
    u_int8_t ee_code;
    u_int8_t ee_pad;
    u_int32_t ee_info;
    u_int32_t ee_data;
  };

#define SO_EE_ORIGIN_NONE  0
#define SO_EE_ORIGIN_LOCAL 1
#define SO_EE_ORIGIN_ICMP  2
#define SO_EE_ORIGIN_ICMP6 3

#define SO_EE_OFFENDER(see)	\
  ((struct sockaddr *)(((struct sock_extended_err)(see))+1))

#endif /* bits/errqueue.h */
