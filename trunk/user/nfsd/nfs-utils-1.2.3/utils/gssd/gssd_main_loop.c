/*
  Copyright (c) 2004 The Regents of the University of Michigan.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of the University nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif	/* HAVE_CONFIG_H */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>

#include "gssd.h"
#include "err_util.h"

extern struct pollfd *pollarray;
extern int pollsize;

#define POLL_MILLISECS	500

static volatile int dir_changed = 1;

static void dir_notify_handler(int sig, siginfo_t *si, void *data)
{
	printerr(2, "dir_notify_handler: sig %d si %p data %p\n", sig, si, data);

	dir_changed = 1;
}

static void
scan_poll_results(int ret)
{
	int			i;
	struct clnt_info	*clp;

	for (clp = clnt_list.tqh_first; clp != NULL; clp = clp->list.tqe_next)
	{
		i = clp->gssd_poll_index;
		if (i >= 0 && pollarray[i].revents) {
			if (pollarray[i].revents & POLLHUP)
				dir_changed = 1;
			if (pollarray[i].revents & POLLIN)
				handle_gssd_upcall(clp);
			pollarray[clp->gssd_poll_index].revents = 0;
			ret--;
			if (!ret)
				break;
		}
		i = clp->krb5_poll_index;
		if (i >= 0 && pollarray[i].revents) {
			if (pollarray[i].revents & POLLHUP)
				dir_changed = 1;
			if (pollarray[i].revents & POLLIN)
				handle_krb5_upcall(clp);
			pollarray[clp->krb5_poll_index].revents = 0;
			ret--;
			if (!ret)
				break;
		}
		i = clp->spkm3_poll_index;
		if (i >= 0 && pollarray[i].revents) {
			if (pollarray[i].revents & POLLHUP)
				dir_changed = 1;
			if (pollarray[i].revents & POLLIN)
				handle_spkm3_upcall(clp);
			pollarray[clp->spkm3_poll_index].revents = 0;
			ret--;
			if (!ret)
				break;
		}
	}
};

static int
topdirs_add_entry(struct dirent *dent)
{
	struct topdirs_info *tdi;

	tdi = calloc(sizeof(struct topdirs_info), 1);
	if (tdi == NULL) {
		printerr(0, "ERROR: Couldn't allocate struct topdirs_info\n");
		return -1;
	}
	tdi->dirname = malloc(PATH_MAX);
	if (tdi->dirname == NULL) {
		printerr(0, "ERROR: Couldn't allocate directory name\n");
		free(tdi);
		return -1;
	}
	snprintf(tdi->dirname, PATH_MAX, "%s/%s", pipefs_dir, dent->d_name);
	tdi->fd = open(tdi->dirname, O_RDONLY);
	if (tdi->fd != -1) {
		fcntl(tdi->fd, F_SETSIG, DNOTIFY_SIGNAL);
		fcntl(tdi->fd, F_NOTIFY,
		      DN_CREATE|DN_DELETE|DN_MODIFY|DN_MULTISHOT);
	}

	TAILQ_INSERT_HEAD(&topdirs_list, tdi, list);
	return 0;
}

static void
topdirs_free_list(void)
{
	struct topdirs_info *tdi;

	TAILQ_FOREACH(tdi, &topdirs_list, list) {
		free(tdi->dirname);
		if (tdi->fd != -1)
			close(tdi->fd);
		TAILQ_REMOVE(&topdirs_list, tdi, list);
		free(tdi);
	}
}

static int
topdirs_init_list(void)
{
	DIR		*pipedir;
	struct dirent	*dent;
	int		ret;

	TAILQ_INIT(&topdirs_list);

	pipedir = opendir(pipefs_dir);
	if (pipedir == NULL) {
		printerr(0, "ERROR: could not open rpc_pipefs directory '%s': "
			 "%s\n", pipefs_dir, strerror(errno));
		return -1;
	}
	for (dent = readdir(pipedir); dent != NULL; dent = readdir(pipedir)) {
		if (dent->d_type != DT_DIR ||
		    strcmp(dent->d_name, ".") == 0  ||
		    strcmp(dent->d_name, "..") == 0) {
			continue;
		}
		ret = topdirs_add_entry(dent);
		if (ret)
			goto out_err;
	}
	closedir(pipedir);
	return 0;
out_err:
	topdirs_free_list();
	return -1;
}

void
gssd_run()
{
	int			ret;
	struct sigaction	dn_act;
	sigset_t		set;

	/* Taken from linux/Documentation/dnotify.txt: */
	dn_act.sa_sigaction = dir_notify_handler;
	sigemptyset(&dn_act.sa_mask);
	dn_act.sa_flags = SA_SIGINFO;
	sigaction(DNOTIFY_SIGNAL, &dn_act, NULL);

	/* just in case the signal is blocked... */
	sigemptyset(&set);
	sigaddset(&set, DNOTIFY_SIGNAL);
	sigprocmask(SIG_UNBLOCK, &set, NULL);

	if (topdirs_init_list() != 0)
		return;

	init_client_list();

	printerr(1, "beginning poll\n");
	while (1) {
		while (dir_changed) {
			dir_changed = 0;
			if (update_client_list()) {
				/* Error msg is already printed */
				exit(1);
			}
		}
		/* race condition here: dir_changed could be set before we
		 * enter the poll, and we'd never notice if it weren't for the
		 * timeout. */
		ret = poll(pollarray, pollsize, POLL_MILLISECS);
		if (ret < 0) {
			if (errno != EINTR)
				printerr(0,
					 "WARNING: error return from poll\n");
		} else if (ret == 0) {
			/* timeout */
		} else { /* ret > 0 */
			scan_poll_results(ret);
		}
	}
	topdirs_free_list();

	return;
}
