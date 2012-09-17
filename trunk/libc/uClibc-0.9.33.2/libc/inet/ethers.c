/*
 * libc/inet/ethers.c
 *
 * Programmatic interface for the /etc/ethers file
 *
 * Copyright 2007 by Matthew Wilcox <matthew@wil.cx>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <netinet/ether.h>

#define ETHER_LINE_LEN	256

/*
 * Internal function which returns a pointer to the part of the line
 * with the start of the hostname, or NULL if we couldn't parse the line.
 * Note that this line may have a comment symbol on it somewhere; if so
 * it will return NULL if the # is before or within the ether_addr, and
 * succeed if the # is before or within the host.  It's up to the callers
 * to be aware of this.
 *
 * I would have preferred to write a NUL to the location of the comment
 * character, but ether_line takes a const argument.  See __ether_line_w.
 */
static const char *__ether_line(const char *line, struct ether_addr *addr)
{
	struct ether_addr *res = ether_aton_r(line, addr);
	if (!res)
		return NULL;

	while (*line && (*line != ' ') && (*line != '\t'))
		line++;
	while (*line && ((*line == ' ')	|| (*line == '\t')))
		line++;
	return (*line) ? line : NULL;
}

/*
 * Strips out the comment before calling __ether_line.  We can do this,
 * since we know the buffer is writable.
 */
static const char *__ether_line_w(char *line, struct ether_addr *addr)
{
	char *end = strchr(line, '#');
	if (!end)
		end = strchr(line, '\n');
	if (end)
		*end = '\0';
	return __ether_line(line, addr);
}

int ether_line(const char *line, struct ether_addr *addr, char *hostname)
{
	const char *name = __ether_line(line, addr);
	if (!name)
		return -1;

	while (*name) {
		if ((*name == '#') || isspace(*name))
			break;
		*hostname++ = *name++;
	}
	*hostname = '\0';

	return 0;
}

int ether_ntohost(char *hostname, const struct ether_addr *addr)
{
	int res = -1;
	FILE *fp;
	char buf[ETHER_LINE_LEN];

	fp = fopen(ETHER_FILE_NAME, "r");
	if (!fp)
		return -1;

	while (fgets(buf, sizeof(buf), fp)) {
		struct ether_addr tmp_addr;
		const char *cp = __ether_line_w(buf, &tmp_addr);
		if (!cp)
			continue;
		if (memcmp(addr, &tmp_addr, sizeof(tmp_addr)))
			continue;

		strcpy(hostname, cp);
		res = 0;
		break;
	}

	fclose(fp);
	return res;
}

int ether_hostton(const char *hostname, struct ether_addr *addr)
{
	int res = -1;
	FILE *fp;
	char buf[ETHER_LINE_LEN];

	fp = fopen(ETHER_FILE_NAME, "r");
	if (!fp)
		return -1;

	while (fgets(buf, sizeof(buf), fp)) {
		const char *cp = __ether_line_w(buf, addr);
		if (!cp)
			continue;
		if (strcasecmp(hostname, cp))
			continue;

		res = 0;
		break;
	}

	fclose(fp);
	return res;
}
