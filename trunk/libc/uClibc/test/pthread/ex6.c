#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void *
test_thread (void *v_param)
{
  return NULL;
}

int
main (void)
{
  unsigned long count;

  setvbuf (stdout, NULL, _IONBF, 0);

  for (count = 0; count < 2000; ++count)
    {
      pthread_t thread;
      int status;

      status = pthread_create (&thread, NULL, test_thread, NULL);
      if (status != 0)
	{
	  printf ("status = %d, count = %lu: %s\n", status, count,
		  strerror (errno));
	  return 1;
	}
      else
	{
	  printf ("count = %lu\n", count);
	}
      /* pthread_detach (thread); */
      pthread_join (thread, NULL);
      usleep (10);
    }
  return 0;
}
