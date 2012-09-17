#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int usleep (__useconds_t usec)
{
    const struct timespec ts = {
	.tv_sec = (long int) (usec / 1000000),
	.tv_nsec = (long int) (usec % 1000000) * 1000ul
    };
    return(nanosleep(&ts, NULL));
}
