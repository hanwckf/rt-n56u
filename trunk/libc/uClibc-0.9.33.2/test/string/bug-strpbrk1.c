/* Test case by Joseph S. Myers <jsm28@cam.ac.uk>.  */
#undef __USE_STRING_INLINES
#define __USE_STRING_INLINES
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int
main (void)
{
  const char *a = "abc";
  const char *b = a;

  strpbrk (b++, "");
  if (b != a + 1)
    return 1;

  return 0;
}
