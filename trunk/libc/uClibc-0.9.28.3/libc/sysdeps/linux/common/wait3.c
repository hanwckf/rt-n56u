#include <syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>


/* Wait for a child to exit.  When one does, put its status in *STAT_LOC and
 * return its process ID.  For errors return (pid_t) -1.  If USAGE is not nil,
 * store information about the child's resource usage (as a `struct rusage')
 * there.  If the WUNTRACED bit is set in OPTIONS, return status for stopped
 * children; otherwise don't.  */
pid_t wait3 (__WAIT_STATUS stat_loc, int options, struct rusage * usage)
{
      return wait4 (WAIT_ANY, stat_loc, options, usage);
}
