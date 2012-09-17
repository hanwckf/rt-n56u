#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <crypt.h>

int
main (int argc, char *argv[])
{
  const char salt[] = "$1$saltstring";
  char *cp;

  cp = crypt ("Hello world!", salt);
  if (strcmp ("$1$saltstri$YMyguxXMBpd2TEZ.vS/3q1", cp)) {
      fprintf(stderr, "Failed md5 crypt test!\n");
      return EXIT_FAILURE;
  }
  fprintf(stderr, "Passed md5 crypt test!\n");
  return EXIT_SUCCESS;
}
