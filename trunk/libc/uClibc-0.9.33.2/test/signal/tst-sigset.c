/* Test sig*set functions.  */

#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define TEST_FUNCTION do_test ()
static int
do_test (void)
{
  int result = 0;
  int sig = -1;

#define TRY(call)							      \
  if (call)								      \
    {									      \
      printf ("%s (sig = %d): %s\n", #call, sig, strerror(errno));			      \
      result = 1;							      \
    }									      \
  else


  sigset_t set;
  TRY (sigemptyset (&set) != 0);

#ifdef SIGRTMAX
  int max_sig = SIGRTMAX;
#else
  int max_sig = NSIG - 1;
#endif

  for (sig = 1; sig <= max_sig; ++sig)
    {
      TRY (sigismember (&set, sig) != 0);
      TRY (sigaddset (&set, sig) != 0);
      TRY (sigismember (&set, sig) == 0);
      TRY (sigdelset (&set, sig) != 0);
      TRY (sigismember (&set, sig) != 0);
    }

  return result;
}

#include "../test-skeleton.c"
