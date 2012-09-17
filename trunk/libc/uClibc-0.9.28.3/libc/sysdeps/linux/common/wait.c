#include <stdlib.h>
#include <syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>

/* Wait for a child to die.  When one does, put its status in *STAT_LOC
 * and return its process ID.  For errors, return (pid_t) -1.  */
__pid_t __libc_wait (__WAIT_STATUS_DEFN stat_loc)
{
      return wait4 (WAIT_ANY, stat_loc, 0, (struct rusage *) NULL);
}
weak_alias(__libc_wait, wait)
