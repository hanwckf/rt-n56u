#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>


void test_handler(int signo) 
{
    write(1, "caught SIGCHLD\n", 15);
    return;
}


int main(void) 
{
    pid_t mypid;
    struct sigaction siga;
    static sigset_t sigset;

    /* Set up sighandling */
    sigfillset(&sigset);
    siga.sa_handler = test_handler;
    siga.sa_mask = sigset;
    siga.sa_flags = 0;
    if (sigaction(SIGCHLD, &siga, (struct sigaction *)NULL) != 0) {
	fprintf(stderr, "sigaction choked: %s!", strerror(errno));
	exit(EXIT_FAILURE);
    }
    

    /* Setup a child process to exercise the sig handling for us */
    mypid = getpid();
    if (fork() == 0) {
	int i;

	for (i=0; i < 3; i++) {
	    sleep(2);
	    kill(mypid, SIGCHLD);
	}
	_exit(EXIT_SUCCESS);
    }


    /* Wait for signals */
    write(1, "waiting for a SIGCHLD\n",22);
    for(;;) {
	sleep(10);
	if (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0)
	    break;
	write(1, "after sleep\n", 12); 
    }

    printf("Bye-bye!  All done!\n");
    return 0;
}

