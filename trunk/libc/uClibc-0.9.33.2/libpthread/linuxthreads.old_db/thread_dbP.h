/* Private header for thread debug library.  */
#ifndef _THREAD_DBP_H
#define _THREAD_DBP_H	1

#define __FORCE_GLIBC
#include <features.h>
#include <string.h>
#include <unistd.h>
#include "proc_service.h"
#include "thread_db.h"
#include "internals.h"


/* Indices for the symbol names.  */
enum
  {
    PTHREAD_THREADS_EVENTS = 0,
    PTHREAD_LAST_EVENT,
    PTHREAD_HANDLES_NUM,
    PTHREAD_HANDLES,
    PTHREAD_KEYS,
    LINUXTHREADS_PTHREAD_THREADS_MAX,
    LINUXTHREADS_PTHREAD_KEYS_MAX,
    LINUXTHREADS_PTHREAD_SIZEOF_DESCR,
    LINUXTHREADS_CREATE_EVENT,
    LINUXTHREADS_DEATH_EVENT,
    LINUXTHREADS_REAP_EVENT,
    LINUXTHREADS_INITIAL_REPORT_EVENTS,
    LINUXTHREADS_VERSION,
    NUM_MESSAGES
  };


/* Comment out the following for less verbose output.  */
#ifndef NDEBUG
# define LOG(c) if (__td_debug) write (2, c "\n", strlen (c "\n"))
extern int __td_debug attribute_hidden;
#else
# define LOG(c)
#endif


/* Handle for a process.  This type is opaque.  */
struct td_thragent
{
  /* Delivered by the debugger and we have to pass it back in the
     proc callbacks.  */
  struct ps_prochandle *ph;

  /* Some cached information.  */

  /* Address of the `__pthread_handles' array.  */
  struct pthread_handle_struct *handles;

  /* Address of the `pthread_kyes' array.  */
  struct pthread_key_struct *keys;

  /* Maximum number of threads.  */
  int pthread_threads_max;

  /* Maximum number of thread-local data keys.  */
  int pthread_keys_max;

  /* Size of 2nd level array for thread-local data keys.  */
  int pthread_key_2ndlevel_size;

  /* Sizeof struct _pthread_descr_struct.  */
  int sizeof_descr;

  /* Pointer to the `__pthread_threads_events' variable in the target.  */
  psaddr_t pthread_threads_eventsp;

  /* Pointer to the `__pthread_last_event' variable in the target.  */
  psaddr_t pthread_last_event;

  /* Pointer to the `__pthread_handles_num' variable.  */
  psaddr_t pthread_handles_num;
};


/* Type used internally to keep track of thread agent descriptors.  */
struct agent_list
{
  td_thragent_t *ta;
  struct agent_list *next;
};

/* List of all known descriptors.  */
extern struct agent_list *__td_agent_list attribute_hidden;

/* Function used to test for correct thread agent pointer.  */
static __inline__ int
ta_ok (const td_thragent_t *ta)
{
  struct agent_list *runp = __td_agent_list;

  if (ta == NULL)
    return 0;

  while (runp != NULL && runp->ta != ta)
    runp = runp->next;

  return runp != NULL;
}


/* Internal wrapper around ps_pglobal_lookup.  */
extern int td_lookup (struct ps_prochandle *ps, int idx, psaddr_t *sym_addr) attribute_hidden;

#endif /* thread_dbP.h */
