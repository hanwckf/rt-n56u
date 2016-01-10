/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2003 WIDE Project.
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

/* XXX: bsdi4 does not have TAILQ_EMPTY */
#ifndef TAILQ_EMPTY
#define	TAILQ_EMPTY(head) ((head)->tqh_first == NULL)
#endif

/* and linux *_FIRST and *_NEXT */
#ifndef LIST_EMPTY
#define	LIST_EMPTY(head)	((head)->lh_first == NULL)
#endif
#ifndef LIST_FIRST
#define	LIST_FIRST(head)	((head)->lh_first)
#endif
#ifndef LIST_NEXT
#define	LIST_NEXT(elm, field)	((elm)->field.le_next)
#endif
#ifndef TAILQ_FIRST
#define	TAILQ_FIRST(head)	((head)->tqh_first)
#endif
#ifndef TAILQ_LAST
#define	TAILQ_LAST(head, headname)					\
	(*(((struct headname *)((head)->tqh_last))->tqh_last))
#endif
#ifndef TAILQ_PREV
#define	TAILQ_PREV(elm, headname, field)				\
	(*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))
#endif
#ifndef TAILQ_NEXT
#define	TAILQ_NEXT(elm, field) ((elm)->field.tqe_next)
#endif
#ifndef TAILQ_FOREACH
#define	TAILQ_FOREACH(var, head, field)					\
	for ((var) = TAILQ_FIRST((head));				\
	    (var);							\
	    (var) = TAILQ_NEXT((var), field))
#endif

#define xstr(s)		_xstr(s)
#define	_xstr(s)	#s

#include "common.h"
#include "dhcp6.h"
#include "dhcp6c_config.h"

PUSH_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

/* common6.c */
struct dhcp6_option_typelen_t {
	uint8_t	lvtype;
	uint8_t	minlen;
};

extern const struct dhcp6_option_typelen_t dhcp6_option_typelens[D6_OPT__MAX+1];

/* daemon state code */
enum {
	DHCP6S_INIT		= 0,
	DHCP6S_SOLICIT	= DH6_SOLICIT,
	DHCP6S_REQUEST	= DH6_REQUEST,
	DHCP6S_RENEW	= DH6_RENEW,
	DHCP6S_REBIND	= DH6_REBIND,
	DHCP6S_RELEASE	= DH6_RELEASE,
	DHCP6S_INFOREQ	= DH6_INFORM_REQ,
};

/* search option for dhcp6_find_listval() */
enum {
	MATCHLIST_PREFIXLEN	= (1 << 0),			/* Match prefix length only */
	MATCHLIST_DH6OPTYPE	= (1 << 1),			/* Match by option type rather than listvalue type */
};

int dhcp6_copy_list(struct dhcp6_list *dst, const struct dhcp6_list *src);
void dhcp6_move_list(struct dhcp6_list *dst, struct dhcp6_list *src);
void dhcp6_clear_list(struct dhcp6_list *);
void dhcp6_clear_listval(struct dhcp6_listval *);
struct dhcp6_listval *dhcp6_find_listval(struct dhcp6_list *head,
    int type, const void *val, int options);
struct dhcp6_listval *dhcp6_add_listval(struct dhcp6_list *head,
    int dh6optype, dhcp6_listval_type_t type,
    const void *val, struct dhcp6_list *sublist);
struct authparam *new_authparam(int proto, int alg, int rdm) RETURNS_MALLOC;
int prefix6_mask(struct in6_addr *in6, int plen);
int get_duid(const char *, struct dhcp6_vbuf *, int);
void dhcp6_init_options(struct dhcp6_optinfo *);
void dhcp6_clear_options(struct dhcp6_optinfo *);
void dhcp6_copy_options(struct dhcp6_optinfo *,
			   struct dhcp6_optinfo *);
int dhcp6_get_options(struct dhcp6opt *, struct dhcp6opt *,
			struct dhcp6_optinfo *);
int dhcp6_set_options(int, struct dhcp6opt *, struct dhcp6opt *,
			struct dhcp6_optinfo *);
void dhcp6_set_timeoparam(struct dhcp6_event *);
void dhcp6_reset_timer(struct dhcp6_event *);

const char *dhcp6optstr(int);
const char *dhcp6msgstr(int);
const char *dhcp6stcodestr(uint16_t);
char *duidstr(struct dhcp6_vbuf *);
const char *dhcp6_event_statestr(struct dhcp6_event *);
int get_rdvalue(int, void *);

void dhcp6_vbuf_copy(struct dhcp6_vbuf *, struct dhcp6_vbuf *);
void dhcp6_vbuf_free(struct dhcp6_vbuf *);
static int ALWAYS_INLINE dhcp6_vbuf_cmp(struct dhcp6_vbuf *v1, struct dhcp6_vbuf *v2)
{
	if (v1->dv_len != v2->dv_len)
		return (v1->dv_len - v2->dv_len);

	return memcmp(v1->dv_buf, v2->dv_buf, v1->dv_len);
}

/* timer functions */
struct dhcp6_timer {
	LIST_ENTRY(dhcp6_timer) link;

	unsigned long long t;

	struct dhcp6_timer *(*expire)(void *);
	void *expire_data;
};

struct dhcp6_timer *dhcp6_timer_add(struct dhcp6_timer *(*)(void *), void *);
void dhcp6_timer_set(unsigned long long , struct dhcp6_timer *);
void dhcp6_timer_remove(struct dhcp6_timer **);
struct timeval *dhcp6_timer_check(void);
unsigned long long dhcp6_timer_rest(struct dhcp6_timer *);

/* dhcp6c_socket.c */
typedef enum { IFADDRCONF_ADD, IFADDRCONF_REMOVE } ifaddrconf_cmd_t;

int sock6_init(const char *boundaddr, const char *port);

int ifaddrconf(ifaddrconf_cmd_t, const char *ifname, struct in6_addr *,
			int plen, int pltime, int vltime);

/* dhcp6c_script.c */
char **fill_envp_client6(struct dhcp6_optinfo *optinfo, const char *ifname);
char **fill_envp_relay6(struct dhcp6_optinfo *optinfo, const char *clientaddr);
int dhcp6_script(const char *scriptpath, char **envp);

/* dhcp6c_ia.c */
void update_ia(struct dhcp6_list *iahead, struct dhcp6_if *ifp,
		struct dhcp6_vbuf *serverid
		IF_FEATURE_DHCP6_AUTH(, struct authparam *));
void release_all_ia(struct dhcp6_if *);

/* dhcp6c.c */
#define MAX_ELAPSED_TIME 0xffff

struct dhcp6_event *dhcp6_create_event(const struct dhcp6_if *, int) RETURNS_MALLOC;
void dhcp6_remove_event(struct dhcp6_event *);

int client6_start(struct dhcp6_if *);
void client6_send(struct dhcp6_event *);

POP_SAVED_FUNCTION_VISIBILITY
