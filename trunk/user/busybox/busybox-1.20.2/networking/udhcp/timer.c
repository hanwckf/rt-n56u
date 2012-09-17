/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2002 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "common.h"
#include "dhcp6.h"
#include "config6.h"
#include "common6.h"
#include "timer.h"

void dhcp6_timer_init(void)
{
	LIST_INIT(&client6_config.timer_head);
	client6_config.tm_sentinel = ULLONG_MAX;
}

struct dhcp6_timer *dhcp6_timer_add(struct dhcp6_timer *(*timeout)(void *),
				    void *timeodata)
{
	struct dhcp6_timer *newtimer;

	if (timeout == NULL) {
		log1("timeout function unspecified");
		return NULL;
	}

	newtimer = xzalloc(sizeof(*newtimer));
	newtimer->expire = timeout;
	newtimer->expire_data = timeodata;
	newtimer->t = ULLONG_MAX;

	LIST_INSERT_HEAD(&client6_config.timer_head, newtimer, link);

	return newtimer;
}

void dhcp6_timer_remove(struct dhcp6_timer **timer)
{
	LIST_REMOVE(*timer, link);
	free(*timer);
	*timer = NULL;
}

void dhcp6_timer_set(unsigned long long t, struct dhcp6_timer *timer)
{
	timer->t = monotonic_ms() + t;

	/* update the next expiration time */
	if (timer->t < client6_config.tm_sentinel)
		client6_config.tm_sentinel = timer->t;
}

/*
 * Check expiration for each timer. If a timer is expired,
 * call the expire function for the timer and update the timer.
 * Return the next interval for select() call.
 */
struct timeval *dhcp6_timer_check(void)
{
	unsigned long long now = monotonic_ms();
	struct dhcp6_timer *tm, *tm_next;

	client6_config.tm_sentinel = ULLONG_MAX;
	for (tm = LIST_FIRST(&client6_config.timer_head); tm; tm = tm_next) {
		tm_next = LIST_NEXT(tm, link);

		if ((long long)(now - tm->t) >= 0) {
			if ((*tm->expire)(tm->expire_data) == NULL)
				continue; /* timer has been freed */
		}

		if (tm->t < client6_config.tm_sentinel)
			client6_config.tm_sentinel = tm->t;
	}

	if (ULLONG_MAX == client6_config.tm_sentinel) {
		/* no need to timeout */
		return NULL;
	} else if (client6_config.tm_sentinel < now) {
		/* this may occur when the interval is too small */
		client6_config.tm_check.tv_sec = client6_config.tm_check.tv_usec = 0;
	} else {
		client6_config.tm_check.tv_sec =
			(client6_config.tm_sentinel - now) / 1000ULL;
		client6_config.tm_check.tv_usec =
			((client6_config.tm_sentinel - now) % 1000ULL) * 1000ULL;
	}
	return (&client6_config.tm_check);
}

unsigned long long dhcp6_timer_rest(struct dhcp6_timer *timer)
{
	unsigned long long now = monotonic_ms();

	if (timer->t - now <= 0) {
		log2("a timer must be expired, but not yet");
		return 0;
	} else {
		return (timer->t - now);
	}
}
