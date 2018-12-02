/*
 * Copyright (C) 2012 Felix Fietkau <nbd@nbd.name>
 * Copyright (C) 2008 John Crispin <blogic@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675
 * Mass Ave, Cambridge, MA 02139, USA.  */

#include "includes.h"
#include <endian.h>
#include <stdio.h>

static char buf[256];

static void md4hash(const char *passwd, uchar p16[16])
{
	int len;
	smb_ucs2_t wpwd[129];
	int i;

	len = strlen(passwd);
	for (i = 0; i < len; i++) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
		wpwd[i] = (unsigned char)passwd[i];
#else
		wpwd[i] = (unsigned char)passwd[i] << 8;
#endif
	}
	wpwd[i] = 0;

	len = len * sizeof(int16);
	mdfour(p16, (unsigned char *)wpwd, len);
	ZERO_STRUCT(wpwd);
}


static bool find_passwd_line(FILE *fp, const char *user, char **next)
{
	char *p1;

	while (!feof(fp)) {
		if(!fgets(buf, sizeof(buf) - 1, fp))
			continue;

		p1 = strchr(buf, ':');

		if (p1 - buf != strlen(user))
			continue;

		if (strncmp(buf, user, p1 - buf) != 0)
			continue;

		if (next)
			*next = p1;
		return true;
	}
	return false;
}

/* returns -1 if user is not present in /etc/passwd*/
static int find_uid_for_user(const char *user)
{
	FILE *fp;
	char *p1, *p2, *p3;
	int ret = -1;

	fp = fopen("/etc/passwd", "r");
	if (!fp) {
		printf("failed to open /etc/passwd");
		goto out;
	}

	if (!find_passwd_line(fp, user, &p1)) {
		printf("User %s not found or invalid in /etc/passwd\n", user);
		goto out;
	}

	p2 = strchr(p1 + 1, ':');
	if (!p2)
		goto out;

	p2++;
	p3 = strchr(p2, ':');
	if (!p1)
		goto out;

	*p3 = '\0';
	ret = atoi(p2);

out:
	if(fp)
		fclose(fp);
	return ret;
}

static void smbpasswd_write_user(FILE *fp, const char *user, int uid, const char *password)
{
	static uchar nt_p16[NT_HASH_LEN];
	int len = 0;
	int i;

	md4hash(strdup(password), nt_p16);

	len += snprintf(buf + len, sizeof(buf) - len, "%s:%u:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX:", user, uid);
	for(i = 0; i < NT_HASH_LEN; i++)
		len += snprintf(buf + len, sizeof(buf) - len, "%02X", nt_p16[i]);

	snprintf(buf + len, sizeof(buf) - len, ":[U          ]:LCT-00000001:\n");
	fputs(buf, fp);
}

int main(int argc, char **argv)
{
	const char *user, *pw;
	FILE *fp;
	bool found;
	int uid;

	TALLOC_CTX *frame = talloc_stackframe();

	if (argc < 3){
		fprintf(stderr, "usage for smbpasswd - \n\t%s USERNAME PASSWD\n", argv[0]);
		return 1;
	}

	user = argv[1];
	uid = find_uid_for_user(user);
	if (uid < 0) {
		fprintf(stderr, "Could not find user '%s' in /etc/passwd\n", user);
		return 2;
	}

	fp = fopen("/etc/samba/smbpasswd", "a+");
	if(!fp) {
		fprintf(stderr, "Failed to open /etc/samba/smbpasswd");
		return 3;
	}
	fseek(fp, 0, SEEK_SET);

	found = find_passwd_line(fp, user, NULL);

	if (found)
		fseek(fp, -strlen(buf), SEEK_CUR);
	pw = argv[2];
	smbpasswd_write_user(fp, user, uid, pw);

	fclose(fp);
	TALLOC_FREE(frame);

	return 0;
}
