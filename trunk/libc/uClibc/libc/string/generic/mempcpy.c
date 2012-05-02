/* Copy memory to memory until the specified number of bytes
   has been copied, return pointer to following byte.
   Overlap is NOT handled correctly.
*/

/* Ditch the glibc version and just wrap memcpy. */

#include <string.h>

#undef mempcpy
#undef __mempcpy

void *__mempcpy (void *dstpp, const void *srcpp, size_t len)
{
  memcpy(dstpp, srcpp, len);
  return (void *)(((char *)dstpp) + len);
}
weak_alias (__mempcpy, mempcpy)
