/* Copyright (C) 1997, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Philip Blundell <pjb27@cam.ac.uk>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <netdb.h>

static const struct
  {
    int code;
    const char *msg;
  }
values[] =
  {
    { EAI_ADDRFAMILY, "Address family for hostname not supported" },
    { EAI_AGAIN, "Temporary failure in name resolution" },
    { EAI_BADFLAGS, "Bad value for ai_flags" },
    { EAI_FAIL, "Non-recoverable failure in name resolution" },
    { EAI_FAMILY, "ai_family not supported" },
    { EAI_MEMORY, "Memory allocation failure" },
    { EAI_NODATA, "No address associated with hostname" },
    { EAI_NONAME, "Name or service not known" },
    { EAI_SERVICE, "Servname not supported for ai_socktype" },
    { EAI_SOCKTYPE, "ai_socktype not supported" },
    { EAI_SYSTEM, "System error" },
    { EAI_INPROGRESS, "Processing request in progress" },
    { EAI_CANCELED, "Request canceled" },
    { EAI_NOTCANCELED, "Request not canceled" },
    { EAI_ALLDONE, "All requests done" },
    { EAI_INTR, "Interrupted by a signal" }
  };

const char *
gai_strerror (int code)
{
    size_t i;
    for (i = 0; i < sizeof (values) / sizeof (values[0]); ++i)
	if (values[i].code == code)
	    return values[i].msg;

    return "Unknown error";
}
