/* Test case by Joseph S. Myers <jsm28@cam.ac.uk>.  */
#undef __USE_STRING_INLINES
#define __USE_STRING_INLINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char d[3] = "\0\1\2";

int
main (void)
{
  strncat (d, "\5\6", 1);
  if (d[0] != '\5')
    {
      puts ("d[0] != '\\5'");
      exit (1);
    }
  if (d[1] != '\0')
    {
      puts ("d[1] != '\\0'");
      exit (1);
    }
  if (d[2] != '\2')
    {
      puts ("d[2] != '\\2'");
      exit (1);
    }

  return 0;
}
