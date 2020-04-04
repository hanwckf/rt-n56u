/*
 * Copyright (C) 2006-2007 Axis Communications AB
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <string.h>

char *strcpy(char *dest, const char *src)
{
  char *ret = dest;
  unsigned long himagic = 0x80808080L;
  unsigned long lomagic = 0x01010101L;

  while ((unsigned long)src & (sizeof src - 1))
  {
    if (!(*dest++ = *src++))
    {
      return ret;
    }
  }

  while (1)
  {
    unsigned long value = *(unsigned long*)src;
    unsigned long magic;

    src += sizeof (unsigned long);

    if ((magic = (value - lomagic) & himagic))
    {
      if (magic & ~value)
      {
        break;
      }
    }

    *(unsigned long*)dest = value;
    dest += sizeof (unsigned long);
  }

  src -= sizeof (unsigned long);

  while ((*dest++ = *src++))
  {
  }

  return ret;
}
libc_hidden_def(strcpy)
