#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


static void *
tf (void *arg)
{
  while (1)
    sleep (100);

  /* NOTREACHED */
  return NULL;
}


static int
do_test (void)
{
  pthread_t th;

  int e = pthread_create (&th, NULL, tf, NULL);
  if (e != 0)
    {
      printf ("create failed: %s\n", strerror (e));
      return 1;
    }

  /* Terminate only this thread.  */
  pthread_exit (NULL);

  /* NOTREACHED */
  return 1;
}

#define EXPECTED_SIGNAL SIGALRM
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
