#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

volatile int gotit = 0;

static void
alarm_handler (int signum)
{
    gotit = 1;
}


int
main (int argc, char ** argv)
{
  clock_t start, stop;

  if (signal(SIGALRM, alarm_handler) == SIG_ERR)
    {
      perror ("signal");
      exit (1);
    }
  alarm(1);
  start = clock ();
  while (!gotit);
  stop = clock ();

  printf ("%ld clock ticks per second (start=%ld,stop=%ld)\n",
	  stop - start, start, stop);
  printf ("CLOCKS_PER_SEC=%ld, sysconf(_SC_CLK_TCK)=%ld\n",
	  CLOCKS_PER_SEC, sysconf(_SC_CLK_TCK));
  return 0;
}
