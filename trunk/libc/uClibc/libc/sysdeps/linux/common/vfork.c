/* Trivial implementation for arches that lack vfork */
#include <unistd.h>
#include <sys/types.h>

pid_t vfork(void)
{
    return fork();
}
