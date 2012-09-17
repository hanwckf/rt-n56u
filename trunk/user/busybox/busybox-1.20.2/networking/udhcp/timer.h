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

#ifndef __DHCP6_TIMER_H
#define __DHCP6_TIMER_H 1

PUSH_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

struct dhcp6_timer {
	LIST_ENTRY(dhcp6_timer) link;

	unsigned long long t;

	struct dhcp6_timer *(*expire)(void *);
	void *expire_data;
};

void dhcp6_timer_init(void);
struct dhcp6_timer *dhcp6_timer_add(struct dhcp6_timer *(*)(void *), void *);
void dhcp6_timer_set(unsigned long long , struct dhcp6_timer *);
void dhcp6_timer_remove(struct dhcp6_timer **);
struct timeval *dhcp6_timer_check(void);
unsigned long long dhcp6_timer_rest(struct dhcp6_timer *);

POP_SAVED_FUNCTION_VISIBILITY

#endif /* __DHCP6_TIMER_H */
