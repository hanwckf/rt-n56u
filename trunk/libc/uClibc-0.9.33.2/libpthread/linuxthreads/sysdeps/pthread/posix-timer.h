/* Definitions for POSIX timer implementation on top of LinuxThreads.
   Copyright (C) 2000, 2002, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Kaz Kylheku <kaz@ashi.footprints.net>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <limits.h>
#include <signal.h>

/* Double linked list.  */
struct list_links
{
  struct list_links *next;
  struct list_links *prev;
};


/* Forward declaration.  */
struct timer_node;


/* Definitions for an internal thread of the POSIX timer implementation.  */
struct thread_node
{
  struct list_links links;
  pthread_attr_t attr;
  pthread_t id;
  unsigned int exists;
  struct list_links timer_queue;
  pthread_cond_t cond;
  struct timer_node *current_timer;
  pthread_t captured;
  clockid_t clock_id;
};


/* Internal representation of a timer.  */
struct timer_node
{
  struct list_links links;
  struct sigevent event;
  clockid_t clock;
  struct itimerspec value;
  struct timespec expirytime;
  pthread_attr_t attr;
  unsigned int abstime;
  unsigned int armed;
  enum {
    TIMER_FREE, TIMER_INUSE, TIMER_DELETED
  } inuse;
  struct thread_node *thread;
  pid_t creator_pid;
  int refcount;
  int overrun_count;
};


/* Static array with the structures for all the timers.  */
extern struct timer_node __timer_array[TIMER_MAX];

/* Global lock to protect operation on the lists.  */
extern pthread_mutex_t __timer_mutex;

/* Variable to protext initialization.  */
extern pthread_once_t __timer_init_once_control;

/* Nonzero if initialization of timer implementation failed.  */
extern int __timer_init_failed;

/* Nodes for the threads used to deliver signals.  */
/* A distinct thread is used for each clock type.  */

extern struct thread_node __timer_signal_thread_rclk;


/* Return pointer to timer structure corresponding to ID.  */
static __inline__ struct timer_node *
timer_id2ptr (timer_t timerid)
{
  if (timerid >= 0 && timerid < TIMER_MAX)
    return &__timer_array[timerid];

  return NULL;
}

/* Return ID of TIMER.  */
static __inline__ int
timer_ptr2id (struct timer_node *timer)
{
  return timer - __timer_array;
}

/* Check whether timer is valid; global mutex must be held. */
static __inline__ int
timer_valid (struct timer_node *timer)
{
  return timer && timer->inuse == TIMER_INUSE;
}

/* Timer refcount functions; need global mutex. */
extern void __timer_dealloc (struct timer_node *timer);

static __inline__ void
timer_addref (struct timer_node *timer)
{
  timer->refcount++;
}

static __inline__ void
timer_delref (struct timer_node *timer)
{
  if (--timer->refcount == 0)
    __timer_dealloc (timer);
}

/* Timespec helper routines.  */
static __inline__ int
timespec_compare (const struct timespec *left, const struct timespec *right)
{
  if (left->tv_sec < right->tv_sec)
    return -1;
  if (left->tv_sec > right->tv_sec)
    return 1;

  if (left->tv_nsec < right->tv_nsec)
    return -1;
  if (left->tv_nsec > right->tv_nsec)
    return 1;

  return 0;
}

static __inline__ void
timespec_add (struct timespec *sum, const struct timespec *left,
	      const struct timespec *right)
{
  sum->tv_sec = left->tv_sec + right->tv_sec;
  sum->tv_nsec = left->tv_nsec + right->tv_nsec;

  if (sum->tv_nsec >= 1000000000)
    {
      ++sum->tv_sec;
      sum->tv_nsec -= 1000000000;
    }
}

static __inline__ void
timespec_sub (struct timespec *diff, const struct timespec *left,
	      const struct timespec *right)
{
  diff->tv_sec = left->tv_sec - right->tv_sec;
  diff->tv_nsec = left->tv_nsec - right->tv_nsec;

  if (diff->tv_nsec < 0)
    {
      --diff->tv_sec;
      diff->tv_nsec += 1000000000;
    }
}


/* We need one of the list functions in the other modules.  */
static __inline__ void
list_unlink_ip (struct list_links *list)
{
  struct list_links *lnext = list->next, *lprev = list->prev;

  lnext->prev = lprev;
  lprev->next = lnext;

  /* The suffix ip means idempotent; list_unlink_ip can be called
   * two or more times on the same node.
   */

  list->next = list;
  list->prev = list;
}


/* Functions in the helper file.  */
extern void __timer_mutex_cancel_handler (void *arg);
extern void __timer_init_once (void);
extern struct timer_node *__timer_alloc (void);
extern int __timer_thread_start (struct thread_node *thread);
extern struct thread_node *__timer_thread_find_matching (const pthread_attr_t *desired_attr, clockid_t);
extern struct thread_node *__timer_thread_alloc (const pthread_attr_t *desired_attr, clockid_t);
extern void __timer_thread_dealloc (struct thread_node *thread);
extern int __timer_thread_queue_timer (struct thread_node *thread,
				       struct timer_node *insert);
extern void __timer_thread_wakeup (struct thread_node *thread);
