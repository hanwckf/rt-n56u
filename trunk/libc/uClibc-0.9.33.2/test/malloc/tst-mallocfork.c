/* Derived from the test case in
   http://sourceware.org/bugzilla/show_bug.cgi?id=838.  */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static void
sig_handler (int signum)
{
  pid_t child = vfork ();
  if (child == 0)
    exit (0);
  TEMP_FAILURE_RETRY (waitpid (child, NULL, 0));
}

static int
do_test (void)
{
  pid_t parent = getpid ();

  struct sigaction action = { .sa_handler = sig_handler };
  sigemptyset (&action.sa_mask);

  malloc (sizeof (int));

  if (sigaction (SIGALRM, &action, NULL) != 0)
    {
      puts ("sigaction failed");
      return 1;
    }

  /* Create a child that sends the signal to be caught.  */
  pid_t child = vfork ();
  if (child == 0)
    {
      if (kill (parent, SIGALRM) == -1)
	perror ("kill");
      exit (0);
    }

  TEMP_FAILURE_RETRY (waitpid (child, NULL, 0));

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
