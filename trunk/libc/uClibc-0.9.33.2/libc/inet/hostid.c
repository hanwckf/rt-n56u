/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define __FORCE_GLIBC
#include <features.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <not-cancel.h>

#define HOSTID "/etc/hostid"

#ifdef __USE_BSD
int sethostid(long int new_id)
{
	int fd;
	int ret;

	if (geteuid() || getuid())
		return __set_errno(EPERM);
	fd = open_not_cancel(HOSTID, O_CREAT|O_WRONLY, 0644);
	if (fd < 0)
		return fd;
	ret = write_not_cancel(fd, &new_id, sizeof(new_id)) == sizeof(new_id) ? 0 : -1;
	close_not_cancel_no_status (fd);
	return ret;
}
#endif

#define _addr(a) (((struct sockaddr_in*)a->ai_addr)->sin_addr.s_addr)
long int gethostid(void)
{
	char host[HOST_NAME_MAX + 1];
	int fd, id = 0;

	/* If hostid was already set then we can return that value.
	 * It is not an error if we cannot read this file. It is not even an
	 * error if we cannot read all the bytes, we just carry on trying...
	 */
	fd = open_not_cancel_2(HOSTID, O_RDONLY);
	if (fd >= 0) {
		int i = read_not_cancel(fd, &id, sizeof(id));
		close_not_cancel_no_status(fd);
		if (i > 0)
			return id;
	}
	/* Try some methods of returning a unique 32 bit id. Clearly IP
	 * numbers, if on the internet, will have a unique address. If they
	 * are not on the internet then we can return 0 which means they should
	 * really set this number via a sethostid() call. If their hostname
	 * returns the loopback number (i.e. if they have put their hostname
	 * in the /etc/hosts file with 127.0.0.1) then all such hosts will
	 * have a non-unique hostid, but it doesn't matter anyway and
	 * gethostid() will return a non zero number without the need for
	 * setting one anyway.
	 *						Mitch
	 */
	if (gethostname(host, HOST_NAME_MAX) >= 0 && *host) {
		struct addrinfo hints, *results, *addr;
		memset(&hints, 0, sizeof(struct addrinfo));
		if (!getaddrinfo(host, NULL, &hints, &results)) {
			for (addr = results; addr; addr = results->ai_next) {
				/* Just so it doesn't look exactly like the
				   IP addr */
				id = _addr(addr) << 16 | _addr(addr) >> 16;
				break;
			}
			freeaddrinfo(results);
		}
	}
	return id;
}
