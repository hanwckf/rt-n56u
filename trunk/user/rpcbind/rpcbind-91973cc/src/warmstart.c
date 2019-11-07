/*
 * Copyright (c) 2009, Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Sun Microsystems, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * warmstart.c
 * Allows for gathering of registrations from an earlier dumped file.
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <rpc/rpc.h>
#include <rpc/rpcb_prot.h>
#include <rpc/xdr.h>
#ifdef PORTMAP
#include <netinet/in.h>
#include <rpc/pmap_prot.h>
#endif
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "rpcbind.h"

/* These files keep the pmap_list and rpcb_list in XDR format */
#define	RPCBFILE	RPCBIND_STATEDIR "/rpcbind.xdr"
#ifdef PORTMAP
#define	PMAPFILE	RPCBIND_STATEDIR "/portmap.xdr"
#endif

#ifndef O_DIRECTORY
#define O_DIRECTORY 0
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

static bool_t write_struct(char *, xdrproc_t, void *);
static bool_t read_struct(char *, xdrproc_t, void *);

static bool_t
write_struct(char *filename, xdrproc_t structproc, void *list)
{
	FILE *fp;
	XDR xdrs;
	mode_t omask;

	omask = umask(077);
	fp = fopen(filename, "w");
	if (fp == NULL) {
		int i;

		for (i = 0; i < 10; i++)
			close(i);
		fp = fopen(filename, "w");
		if (fp == NULL) {
			syslog(LOG_ERR,
				"cannot open file = %s for writing", filename);
			syslog(LOG_ERR, "cannot save any registration");
			return (FALSE);
		}
	}
	(void) umask(omask);
	xdrstdio_create(&xdrs, fp, XDR_ENCODE);

	if (structproc(&xdrs, list) == FALSE) {
		syslog(LOG_ERR, "xdr_%s: failed", filename);
		fclose(fp);
		return (FALSE);
	}
	XDR_DESTROY(&xdrs);
	fclose(fp);
	return (TRUE);
}

static bool_t
read_struct(char *filename, xdrproc_t structproc, void *list)
{
	FILE *fp;
	XDR xdrs;

	if (debugging)
		fprintf(stderr, "rpcbind: using '%s' startup file\n", filename);

	if ((fp = fopen(filename, "r")) == NULL) {
		if (errno != ENOENT)
			syslog(LOG_ERR,
				"Cannot open '%s' file for reading, errno %d (%s)",
				filename, errno, strerror(errno));
		goto error;
	}

	xdrstdio_create(&xdrs, fp, XDR_DECODE);
	if (structproc(&xdrs, list) == FALSE) {
		fprintf(stderr, "rpcbind: xdr_%s: failed\n", filename);
		fclose(fp);
		goto error;
	}
	XDR_DESTROY(&xdrs);

	fclose(fp);
	if (unlink(filename) < 0) {
		syslog(LOG_ERR, "Cannot unlink '%s', errno %d (%s)", 
			filename, errno, strerror(errno));
	}
	return (TRUE);

error:	
	if (errno != ENOENT && unlink(filename) < 0) {
		syslog(LOG_ERR, "Cannot unlink '%s', errno %d (%s)", 
			filename, errno, strerror(errno));
	}
	if (debugging)
		fprintf(stderr, "rpcbind: will start from scratch\n");
	return (FALSE);
}

void
mkdir_warmstart(int uid)
{
	/* Already exists? */
	if (access(RPCBIND_STATEDIR, X_OK) == 0)
		return;

	if (mkdir(RPCBIND_STATEDIR, 0770) == 0) {
		int fd = open(RPCBIND_STATEDIR, O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
		if (fd >= 0) {
			if (fchown(fd, uid, -1) < 0) {
				syslog(LOG_ERR, 
					"mkdir_warmstart: open failed '%s', errno %d (%s)", 
					RPCBIND_STATEDIR, errno, strerror(errno));
			}
			close(fd);
		} else
			syslog(LOG_ERR, "mkdir_warmstart: open failed '%s', errno %d (%s)", 
				RPCBIND_STATEDIR, errno, strerror(errno));
	} else
		syslog(LOG_ERR, "mkdir_warmstart: mkdir failed '%s', errno %d (%s)", 
			RPCBIND_STATEDIR, errno, strerror(errno));
}

void
write_warmstart()
{
	(void) mkdir(RPCBIND_STATEDIR, 0770);
	(void) write_struct(RPCBFILE, (xdrproc_t)xdr_rpcblist_ptr, &list_rbl);
#ifdef PORTMAP
	(void) write_struct(PMAPFILE, (xdrproc_t)xdr_pmaplist_ptr, &list_pml);
#endif

}

void
read_warmstart()
{
	rpcblist_ptr tmp_rpcbl = NULL;
#ifdef PORTMAP
	struct pmaplist *tmp_pmapl = NULL;
#endif
	int rc;

	rc = read_struct(RPCBFILE, (xdrproc_t)xdr_rpcblist_ptr, &tmp_rpcbl);
	if (rc == TRUE) {
		rpcblist *pos, **tail;

		/* The current rpcblist contains only the registrations
		 * for rpcbind and portmap. We keep those, since the
		 * info from the warm start file may be stale if the
		 * netconfig file was changed in the meantime.
		 * From the warm start file, we weed out any rpcbind info.
		 */
		for (tail = &list_rbl; *tail; tail = &(*tail)->rpcb_next)
			;
		while ((pos = tmp_rpcbl) != NULL) {
			tmp_rpcbl = pos->rpcb_next;
			if (pos->rpcb_map.r_prog != RPCBPROG) {
				*tail = pos;
				tail = &pos->rpcb_next;
			} else {
				free(pos);
			}
		}
		*tail = NULL;
	}
#ifdef PORTMAP
	rc = read_struct(PMAPFILE, (xdrproc_t)xdr_pmaplist_ptr, &tmp_pmapl);
	if (rc == TRUE) {
		struct pmaplist *pos, **tail;

		/* The current pmaplist contains only the registrations
		 * for rpcbind and portmap. We keep those, since the
		 * info from the warm start file may be stale if the
		 * netconfig file was changed in the meantime.
		 * From the warm start file, we weed out any rpcbind info.
		 */
		for (tail = &list_pml; *tail; tail = &(*tail)->pml_next)
			;
		while ((pos = tmp_pmapl) != NULL) {
			tmp_pmapl = pos->pml_next;
			if (pos->pml_map.pm_prog != PMAPPROG) {
				*tail = pos;
				tail = &pos->pml_next;
			} else {
				free(pos);
			}
		}
		*tail = NULL;
	}
#endif

	return;
}
