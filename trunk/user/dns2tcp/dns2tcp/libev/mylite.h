#ifndef MY_LITE_H
#define MY_LITE_H

/* libev-4.33 */
#define EV_STANDALONE 1 /* manual configuration */
#define EV_COMPAT3 0 /* remove compatible code */
#define EV_VERIFY 0 /* remove verification code */
#define EV_USE_FLOOR 1 /* use libm.floor() function */
#define EV_NO_SMP 1 /* disable multi-threads support */
#define EV_NO_THREADS 1 /* disable multi-threads support */
#define EV_PERIODIC_ENABLE 0 /* disable ev_periodic watcher */
#define EV_SIGNAL_ENABLE 0 /* disable ev_signal watcher */
#define EV_CHILD_ENABLE 0 /* disable ev_child watcher */
#define EV_STAT_ENABLE 0 /* disable ev_stat watcher */
#define EV_IDLE_ENABLE 0 /* disable ev_idle watcher */
#define EV_PREPARE_ENABLE 0 /* disable ev_prepare watcher */
#define EV_CHECK_ENABLE 0 /* disable ev_check watcher */
#define EV_EMBED_ENABLE 0 /* disable ev_embed watcher */
#define EV_FORK_ENABLE 0 /* disable ev_fork watcher */
#define EV_CLEANUP_ENABLE 0 /* disable ev_cleanup watcher */
#define EV_ASYNC_ENABLE 0 /* disbale ev_async watcher */

/* typedef struct */
typedef struct ev_loop  evloop_t;
typedef struct ev_io    evio_t;
typedef struct ev_timer evtimer_t;

/* typedef callback */
typedef void (*evio_cb_t)(evloop_t *evloop, evio_t *watcher, int revents);
typedef void (*evtimer_cb_t)(evloop_t *evloop, evtimer_t *watcher, int revents);

#endif
