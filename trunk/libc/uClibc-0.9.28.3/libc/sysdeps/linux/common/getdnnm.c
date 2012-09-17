#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#define __USE_GNU
#include <sys/utsname.h>


int
getdomainname(char *name, size_t len)
{
  struct utsname uts;

  if (name == NULL) {
    __set_errno(EINVAL);
    return -1;
  }

  if (uname(&uts) == -1) return -1;

  if (strlen(uts.domainname)+1 > len) {
    __set_errno(EINVAL);
    return -1;
  }
  strcpy(name, uts.domainname);
  return 0;
}
