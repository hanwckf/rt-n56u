/* Test for inet_network.
   Copyright (C) 2000, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 2000.

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

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct
{
  const char *network;
  uint32_t number;
} tests [] =
{
  {"1.0.0.0", 0x1000000},
  {"1.0.0", 0x10000},
  {"1.0", 0x100},
  {"1", 0x1},
  {"192.168.0.0", 0xC0A80000},
  /* Now some invalid addresses.  */
  {"141.30.225.2800", INADDR_NONE},
  {"141.76.1.1.1", INADDR_NONE},
  {"141.76.1.11.", INADDR_NONE},
  {"1410", INADDR_NONE},
  {"1.1410", INADDR_NONE},
  {"1.1410.", INADDR_NONE},
  {"1.1410", INADDR_NONE},
  {"141.76.1111", INADDR_NONE},
  {"141.76.1111.", INADDR_NONE},
  {"1.1.1.257", INADDR_NONE},
  /* Now some from BSD */
  {"0x12", 0x00000012},
  {"127.1", 0x00007f01},
  {"127.1.2.3", 0x7f010203},
  {"0x123456", INADDR_NONE},
  {"0x12.0x34", 0x00001234},
  {"0x12.0x345", INADDR_NONE},
  {"1.2.3.4.5", INADDR_NONE},
  {"1..3.4", INADDR_NONE},
  {".", INADDR_NONE},
  {"1.", INADDR_NONE},
  {".1", INADDR_NONE},
  {"x", INADDR_NONE},
  {"0x", INADDR_NONE},
  {"0", 0x00000000},
  {"0x0", 0x00000000},
  {"01.02.07.077", 0x0102073f},
  {"0x1.23.045.0", 0x01172500},
  {"", INADDR_NONE},
  {" ", INADDR_NONE},
  {"bar", INADDR_NONE},
  {"1.2bar", INADDR_NONE},
  {"1.", INADDR_NONE},
  {"йцукен", INADDR_NONE},
  {"255.255.255.255", INADDR_NONE},
  {"x", INADDR_NONE},
  {"0X12", 0x00000012},
  {"078", INADDR_NONE},
  {"1 bar", INADDR_NONE},
  {"127.0xfff", INADDR_NONE},
};


int
main (void)
{
  int errors = 0;
  size_t i;
  uint32_t res;

  for (i = 0; i < sizeof (tests) / sizeof (tests[0]); ++i)
    {
      printf ("Testing: %s\n", tests[i].network);
      res = inet_network (tests[i].network);

      if (res != tests[i].number)
	{
	  ++errors;
	  printf ("Test failed for inet_network (\"%s\"):\n",
		  tests[i].network);
	  printf ("Expected return value %u (0x%x) but got %u (0x%x).\n",
		  tests[i].number, tests[i].number, res, res);
	}

    }

  return errors != 0;
}
