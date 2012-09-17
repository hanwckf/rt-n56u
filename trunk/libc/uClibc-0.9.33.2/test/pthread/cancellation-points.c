/*
 * Make sure functions marked as cancellation points actually are.
 * http://www.opengroup.org/onlinepubs/009695399/functions/xsh_chap02_09.html#tag_02_09_05
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <features.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <mqueue.h>
#include <poll.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/* take care of optional things ... */
#define STUB(func, args) static void func args { sleep(0); }
#if defined(__UCLIBC_AIO__)
# include <aio.h>
#else
STUB(aio_suspend, (void *p, int n, const void *p2))
#endif
#if defined(__UCLIBC_STROPTS__)
# include <stropts.h>
#else
STUB(getmsg, (int f, void *p, void *p2, void *p3))
STUB(getpmsg, (int f, void *p, void *p2, void *p3, void *p4))
STUB(putmsg, (int f, void *p, void *p2, void *p3))
STUB(putpmsg, (int f, void *p, void *p2, void *p3, void *p4))
#endif
#if defined(__UCLIBC__)
STUB(clock_nanosleep, (int i, int f, const void *p, void *p2))
#endif

int cnt;
bool ready;

void cancel_timeout(int sig)
{
	ready = false;
}
void cancel_thread_cleanup(void *arg)
{
	ready = false;
}

/* some funcs need some help as they wont take NULL args ... */
const struct timespec zero_sec = { .tv_sec = 0, .tv_nsec = 0 };

sem_t sem;
void help_sem_setup(void)
{
	if (sem_init(&sem, 0, 1) == -1) {
		perror("sem_init() failed");
		exit(-1);
	}
}

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex;
void help_pthread_setup(void)
{
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);
}

/* the pthread function that will call the cancellable function over and over */
#define _MAKE_CANCEL_THREAD_FUNC_EX(func, sysfunc, args, setup) \
void *cancel_thread_##func(void *arg) \
{ \
	if (pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL)) { \
		perror("unable to set cancel type to deferred; something is seriously broken"); \
		exit(-1); \
	} \
	pthread_cleanup_push(cancel_thread_cleanup, NULL); \
	setup; \
	ready = true; \
	while (ready) \
		sysfunc args; \
	pthread_cleanup_pop(1); \
	return NULL; \
}
#define MAKE_CANCEL_THREAD_FUNC_RE(func, sysfunc, args) _MAKE_CANCEL_THREAD_FUNC_EX(func, sysfunc, args, (void)0)
#define MAKE_CANCEL_THREAD_FUNC_EX(func, args, setup)   _MAKE_CANCEL_THREAD_FUNC_EX(func, func, args, setup)
#define MAKE_CANCEL_THREAD_FUNC(func, args)             _MAKE_CANCEL_THREAD_FUNC_EX(func, func, args, (void)0)

MAKE_CANCEL_THREAD_FUNC(accept, (-1, NULL, NULL))
MAKE_CANCEL_THREAD_FUNC(aio_suspend, (NULL, 0, &zero_sec))
MAKE_CANCEL_THREAD_FUNC(clock_nanosleep, (0, 0, NULL, NULL))
MAKE_CANCEL_THREAD_FUNC(close, (-1))
MAKE_CANCEL_THREAD_FUNC(connect, (-1, NULL, 0))
MAKE_CANCEL_THREAD_FUNC(creat, ("", 0))
MAKE_CANCEL_THREAD_FUNC(fcntl, (0, F_SETLKW, NULL))
MAKE_CANCEL_THREAD_FUNC(fdatasync, (-1))
MAKE_CANCEL_THREAD_FUNC(fsync, (0))
MAKE_CANCEL_THREAD_FUNC(getmsg, (-1, NULL, NULL, NULL))
MAKE_CANCEL_THREAD_FUNC(getpmsg, (-1, NULL, NULL, NULL, NULL))
MAKE_CANCEL_THREAD_FUNC(lockf, (-1, F_TEST, 0))
MAKE_CANCEL_THREAD_FUNC(mq_receive, (0, NULL, 0, NULL))
MAKE_CANCEL_THREAD_FUNC(mq_send, (0, NULL, 0, 0))
MAKE_CANCEL_THREAD_FUNC(mq_timedreceive, (0, NULL, 0, NULL, NULL))
MAKE_CANCEL_THREAD_FUNC(mq_timedsend, (0, NULL, 0, 0, NULL))
MAKE_CANCEL_THREAD_FUNC(msgrcv, (-1, NULL, 0, 0, 0))
MAKE_CANCEL_THREAD_FUNC(msgsnd, (-1, NULL, 0, 0))
MAKE_CANCEL_THREAD_FUNC(msync, (NULL, 0, 0))
MAKE_CANCEL_THREAD_FUNC(nanosleep, (NULL, NULL))
MAKE_CANCEL_THREAD_FUNC(open, ("", 0))
MAKE_CANCEL_THREAD_FUNC(pause, ())
MAKE_CANCEL_THREAD_FUNC(poll, (NULL, 0, 0))
MAKE_CANCEL_THREAD_FUNC(pread, (-1, NULL, 0, 0))
MAKE_CANCEL_THREAD_FUNC(pselect, (0, NULL, NULL, NULL, NULL, NULL))
MAKE_CANCEL_THREAD_FUNC_EX(pthread_cond_timedwait, (&cond, &mutex, &zero_sec), help_pthread_setup())
MAKE_CANCEL_THREAD_FUNC_EX(pthread_cond_wait, (&cond, &mutex), help_pthread_setup())
/*MAKE_CANCEL_THREAD_FUNC_EX(pthread_join, (0, NULL))*/
MAKE_CANCEL_THREAD_FUNC(pthread_testcancel, ())
MAKE_CANCEL_THREAD_FUNC(putmsg, (-1, NULL, NULL, 0))
MAKE_CANCEL_THREAD_FUNC(putpmsg, (-1, NULL, NULL, 0, 0))
MAKE_CANCEL_THREAD_FUNC(pwrite, (-1, NULL, 0, 0))
MAKE_CANCEL_THREAD_FUNC(read, (-1, NULL, 0))
MAKE_CANCEL_THREAD_FUNC(readv, (-1, NULL, 0))
MAKE_CANCEL_THREAD_FUNC(recv, (-1, NULL, 0, 0))
MAKE_CANCEL_THREAD_FUNC(recvfrom, (-1, NULL, 0, 0, NULL, NULL))
MAKE_CANCEL_THREAD_FUNC(recvmsg, (-1, NULL, 0))
MAKE_CANCEL_THREAD_FUNC(select, (0, NULL, NULL, NULL, NULL))
MAKE_CANCEL_THREAD_FUNC_EX(sem_timedwait, (&sem, &zero_sec), help_sem_setup())
MAKE_CANCEL_THREAD_FUNC_EX(sem_wait, (&sem), help_sem_setup())
MAKE_CANCEL_THREAD_FUNC(send, (-1, NULL, 0, 0))
MAKE_CANCEL_THREAD_FUNC(sendmsg, (-1, NULL, 0))
MAKE_CANCEL_THREAD_FUNC(sendto, (-1, NULL, 0, 0, NULL, 0))
#ifdef __UCLIBC_SUSV4_LEGACY__
MAKE_CANCEL_THREAD_FUNC(sigpause, (0))
#endif
MAKE_CANCEL_THREAD_FUNC(sigsuspend, (NULL))
MAKE_CANCEL_THREAD_FUNC(sigtimedwait, (NULL, NULL, NULL))
MAKE_CANCEL_THREAD_FUNC(sigwait, (NULL, NULL))
MAKE_CANCEL_THREAD_FUNC(sigwaitinfo, (NULL, NULL))
MAKE_CANCEL_THREAD_FUNC(sleep, (0))
MAKE_CANCEL_THREAD_FUNC(system, (""))
MAKE_CANCEL_THREAD_FUNC(tcdrain, (-1))
#ifdef __UCLIBC_SUSV3_LEGACY__
MAKE_CANCEL_THREAD_FUNC(usleep, (0))
#endif
MAKE_CANCEL_THREAD_FUNC(wait, (NULL))
MAKE_CANCEL_THREAD_FUNC(waitid, (0, 0, NULL, 0))
MAKE_CANCEL_THREAD_FUNC(waitpid, (-1, NULL, 0))
MAKE_CANCEL_THREAD_FUNC(write, (-1, NULL, 0))
MAKE_CANCEL_THREAD_FUNC(writev, (-1, NULL, 0))

/* test a few variations that should not cancel ... */
MAKE_CANCEL_THREAD_FUNC_RE(fcntl_another, fcntl, (0, F_GETFD))

/* main test that creates thread, cancels it, etc... */
int _test_func(const char *func_name, void *(*func)(void*), const int should_cancel)
{
	int ret;
	pthread_t cancel_thread_id;

	++cnt;

	printf("testing %-30s ", func_name);

	printf(".");
	if (signal(SIGALRM, cancel_timeout) == SIG_ERR) {
		perror("unable to bind SIGALRM");
		exit(-1);
	}

	printf(".");
	ready = false;
	pthread_create(&cancel_thread_id, NULL, func, NULL);

	printf(".");
	while (!ready)
		sched_yield();

	printf(".");
	if (pthread_cancel(cancel_thread_id)) {
		perror("unable to cancel thread");
		exit(-1);
	}

	printf(".");
	alarm(5);
	while (ready)
		sched_yield();

	printf(".");
	ret = (!!!alarm(0) == should_cancel);

	if (ret)
		printf(" failed ;(\n");
	else
		printf(" OK!\n");

	return ret;
}
#define TEST_FUNC(f)    _test_func(#f, cancel_thread_##f, 1)
#define TEST_FUNC_RE(f) _test_func(#f, cancel_thread_##f, 0)

int main(int argc, char *argv[])
{
	int ret = 0;
	setbuf(stdout, NULL);
	cnt = 0;

	ret += TEST_FUNC(accept);
	ret += TEST_FUNC(aio_suspend);
	ret += TEST_FUNC(clock_nanosleep);
	ret += TEST_FUNC(close);
	ret += TEST_FUNC(connect);
	ret += TEST_FUNC(creat);
	ret += TEST_FUNC(fcntl);
	ret += TEST_FUNC(fdatasync);
	ret += TEST_FUNC(fsync);
	ret += TEST_FUNC(getmsg);
	ret += TEST_FUNC(getpmsg);
	ret += TEST_FUNC(lockf);
	ret += TEST_FUNC(mq_receive);
	ret += TEST_FUNC(mq_send);
	ret += TEST_FUNC(mq_timedreceive);
	ret += TEST_FUNC(mq_timedsend);
	ret += TEST_FUNC(msgrcv);
	ret += TEST_FUNC(msgsnd);
	ret += TEST_FUNC(msync);
	ret += TEST_FUNC(nanosleep);
	ret += TEST_FUNC(open);
	ret += TEST_FUNC(pause);
	ret += TEST_FUNC(poll);
	ret += TEST_FUNC(pread);
	ret += TEST_FUNC(pselect);
	ret += TEST_FUNC(pthread_cond_timedwait);
	ret += TEST_FUNC(pthread_cond_wait);
	/*ret += TEST_FUNC(pthread_join);*/
	ret += TEST_FUNC(pthread_testcancel);
	ret += TEST_FUNC(putmsg);
	ret += TEST_FUNC(putpmsg);
	ret += TEST_FUNC(pwrite);
	ret += TEST_FUNC(read);
	ret += TEST_FUNC(readv);
	ret += TEST_FUNC(recv);
	ret += TEST_FUNC(recvfrom);
	ret += TEST_FUNC(recvmsg);
	ret += TEST_FUNC(select);
	ret += TEST_FUNC(sem_timedwait);
	ret += TEST_FUNC(sem_wait);
	ret += TEST_FUNC(send);
	ret += TEST_FUNC(sendmsg);
	ret += TEST_FUNC(sendto);
	ret += TEST_FUNC(sigpause);
	ret += TEST_FUNC(sigsuspend);
	ret += TEST_FUNC(sigtimedwait);
	ret += TEST_FUNC(sigwait);
	ret += TEST_FUNC(sigwaitinfo);
	ret += TEST_FUNC(sleep);
	ret += TEST_FUNC(system);
	ret += TEST_FUNC(tcdrain);
#ifdef __UCLIBC_SUSV3_LEGACY__
	ret += TEST_FUNC(usleep);
#endif
	ret += TEST_FUNC(wait);
	ret += TEST_FUNC(waitid);
	ret += TEST_FUNC(waitpid);
	ret += TEST_FUNC(write);
	ret += TEST_FUNC(writev);

	ret += TEST_FUNC_RE(fcntl_another);

	if (ret)
		printf("!!! %i / %i tests failed\n", ret, cnt);

	return ret;
}
