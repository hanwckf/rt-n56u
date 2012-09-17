#include <wchar.h>
#include <assert.h>
#include <stdlib.h>

/* bugs.uclibc.org/1471 : make sure output is 0  */
static int
do_test(void)
{
  wchar_t output;
  int result;

  output = L'A';		/* anything other than 0 will do... */
  result = mbrtowc (&output, "", 1, 0);

  assert (result == 0);
  assert (output == 0);

  return EXIT_SUCCESS;
}
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
