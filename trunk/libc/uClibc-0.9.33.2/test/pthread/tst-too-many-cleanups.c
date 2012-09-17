/*
 * This illustrates the bug where the cleanup function
 * of a thread may be called too many times.
 *
 * main thread:
 *  - grab mutex
 *  - spawn thread1
 *  - go to sleep
 * thread1:
 *  - register cleanup handler via pthread_cleanup_push()
 *  - try to grab mutex and sleep
 * main:
 *  - kill thread1
 *  - go to sleep
 * thread1 cleanup handler:
 *  - try to grab mutex and sleep
 * main:
 *  - kill thread1
 *  - go to sleep
 * thread1 cleanup handler:
 *  - wrongly called again
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

#define warn(fmt, args...) fprintf(stderr, "[%p] " fmt, (void*)pthread_self(), ## args)
#define warnf(fmt, args...) warn("%s:%i: " fmt, __FUNCTION__, __LINE__, ## args)

int ok_to_kill_thread;

static void thread_killed(void *arg);

static void *KillMeThread(void *thread_par)
{
	pthread_t pthread_id;

	warnf("Starting child thread\n");

	pthread_id = pthread_self();
	pthread_cleanup_push(thread_killed, (void *)pthread_id);

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	/* main code */
	warnf("please kill me now\n");
	while (1) {
		ok_to_kill_thread = 1;
		sleep(1);
	}

	pthread_cleanup_pop(0);

	return 0;
}

static void thread_killed(void *arg)
{
	static int num_times_called = 0;

	warnf("killing %p [cnt=%i]\n", arg, ++num_times_called);
	assert(num_times_called == 1);

	/* pick any cancellation endpoint, sleep() will do just fine */
	while (1) {
		warnf("sleeping in cancellation endpoint ...\n");
		sleep(1);
	}

	warnf("done cleaning up\n");
}

int main(int argc, char *argv[])
{
	int count = 3;
	pthread_t app_pthread_id;

	/* need to tweak this test a bit to play nice with signals and LT */
	return 0;

	ok_to_kill_thread = 0;

	pthread_create(&app_pthread_id, NULL, KillMeThread, NULL);

	warnf("waiting for thread to prepare itself\n");
	while (!ok_to_kill_thread)
		sleep(1);

	while (count--) {
		warnf("killing thread\n");
		pthread_cancel(app_pthread_id);
		sleep(3);
	}

	return 0;
}
