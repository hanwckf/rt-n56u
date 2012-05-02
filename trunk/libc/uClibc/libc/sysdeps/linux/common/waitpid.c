#include <stdlib.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>

__pid_t __libc_waitpid(__pid_t pid, int *wait_stat, int options)
{
    return wait4(pid, wait_stat, options, NULL);
}
weak_alias(__libc_waitpid, waitpid)
