/*
 * Copyright (C) 2006-2007 Axis Communications AB
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <string.h>


char *strncpy(char *dest, const char *src, size_t count)
{
  char *ret = dest;
  unsigned long himagic = 0x80808080L;
  unsigned long lomagic = 0x01010101L;

  while (count && (unsigned long)src & (sizeof src - 1))
  {
    count--;
    if (!(*dest++ = *src++))
    {
      goto finalize;
    }
  }

  while (count >= sizeof (unsigned long))
  {
    unsigned long value = *(unsigned long*)src;
    unsigned long magic;

    if ((magic = (value - lomagic) & himagic))
    {
      if (magic & ~value)
      {
        break;
      }
    }

    *(unsigned long*)dest = value;
    dest += sizeof (unsigned long);
    src += sizeof (unsigned long);
    count -= sizeof (unsigned long);
  }

  while (count)
  {
    count--;
    if (!(*dest++ = *src++))
      break;
  }

finalize:
  if (count)
  {
    memset(dest, '\0', count);
  }

  return ret;
}
libc_hidden_def(strncpy)
