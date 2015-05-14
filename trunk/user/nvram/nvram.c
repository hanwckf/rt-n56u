/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * Frontend command-line utility for Linux NVRAM layer
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: nvram.c,v 1.1 2007/06/08 10:22:42 arthur Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <bcmnvram.h>

#include <nvram_linux.h>

#define PROFILE_HEADER		"HDR1"
#define PROFILE_HEADER_NEW	"HDR2"

static void usage(void)
{
	printf("Usage: nvram [get name] [set name=value] [settmp name=value] [unset name]\n"
	       "             [show] [showall] [clear] [save fn] [restore fn] [commit]\n");
	exit(1);
}

static unsigned char get_rand()
{
	unsigned char buf[1];
	FILE *fp;

	fp = fopen("/dev/urandom", "r");
	if (fp == NULL) {
		return 0;
	}
	fread(buf, 1, 1, fp);
	fclose(fp);

	return buf[0];
}

static int is_sys_param(char *p)
{
	struct nvram_pair *np;
	extern struct nvram_pair router_defaults[];
	extern struct nvram_pair tables_defaults[];

	for (np = router_defaults; np->name; np++)
	{
		if (strcmp(p, np->name)==0)
			return 1;
	}

	for (np = tables_defaults; np->name; np++)
	{
		if (strstr(p, np->name))
			return 1;
	}

	return 0;
}

static int nvram_save(char *file)
{
	FILE *fp;
	char *name, *buf;
	unsigned long count, filelen, i;
	unsigned char rand = 0, temp;

	if ((fp = fopen(file, "w")) == NULL)
		return -1;

	buf = malloc(NVRAM_SPACE);
	if (!buf) {
		fclose(fp);
		perror ("Out of memory!\n");
		return -1;
	}

	buf[0] = 0;
	nvram_getall(buf, NVRAM_SPACE, 0);

	count = 0;
	for (name = buf; *name; name += strlen(name) + 1)
	{
		count = count + strlen(name) + 1;
	}

	filelen = count + (1024 - count % 1024);
	rand = get_rand() % 30;

	fwrite(PROFILE_HEADER_NEW, 1, 4, fp);
	fwrite(&filelen, 1, 3, fp);
	fwrite(&rand, 1, 1, fp);

	for (i = 0; i < count; i++)
	{
		if (buf[i] == 0x0)
			buf[i] = 0xfd + get_rand() % 3;
		else
			buf[i] = 0xff - buf[i] + rand;
	}

	fwrite(buf, 1, count, fp);
	for (i = count; i < filelen; i++)
	{
		temp = 0xfd + get_rand() % 3;
		fwrite(&temp, 1, 1, fp);
	}

	fclose(fp);
	free(buf);
	return 0;
}

static int nvram_restore(char *file)
{
	FILE *fp;
	char header[8], *p, *v, *buf;
	unsigned long count, filelen, *filelenptr, i;
	unsigned char rand, *randptr;

	if ((fp = fopen(file, "r+")) == NULL)
		return -1;

	buf = malloc(NVRAM_SPACE);
	if (!buf) {
		fclose(fp);
		perror ("Out of memory!\n");
		return -1;
	}

	buf[0] = 0;

	count = fread(header, 1, 8, fp);
	if (count>=8 && strncmp(header, PROFILE_HEADER, 4) == 0)
	{
		filelenptr = (unsigned long *)(header + 4);
		fread(buf, 1, *filelenptr, fp);
	}
	else if (count>=8 && strncmp(header, PROFILE_HEADER_NEW, 4) == 0)
	{
		filelenptr = (unsigned long *)(header + 4);
		filelen = *filelenptr & 0xffffff;
		randptr = (unsigned char *)(header + 7);
		rand = *randptr;
		if (filelen > NVRAM_SPACE)
			filelen = NVRAM_SPACE;
		count = fread(buf, 1, filelen, fp);
		for (i = 0; i < count; i++)
		{
			if ((unsigned char) buf[i] > ( 0xfd - 0x1))
				buf[i] = 0x0;
			else
				buf[i] = 0xff + rand - buf[i];
		}
	}
	else
	{
		fclose(fp);
		free(buf);
		return 1;
	}

	fclose(fp);

	nvram_clear();

	p = buf;

	while (*p)
	{
		v = strchr(p, '=');
		if (v != NULL)
		{
			*v++ = 0;
			if (is_sys_param(p))
				nvram_set(p, v);
			p = v + strlen(v) + 1;
		}
		else
		{
			nvram_unset(p);
			p = p + 1;
		}
	}

	free(buf);

	return 0;
}

static int nvram_show(int show_all)
{
	char *name, *buf;
	int buf_len, size;

	buf_len = NVRAM_SPACE;
	if (show_all)
		buf_len *= 2;

	buf = malloc(buf_len);
	if (!buf) {
		perror ("Out of memory!\n");
		return -1;
	}

	buf[0] = 0;
	nvram_getall(buf, buf_len, show_all);

	for (name = buf; *name; name += strlen(name) + 1)
		puts(name);

	if (!show_all) {
		size = sizeof(struct nvram_header) + (int) name - (int) buf;
		printf("\nsize: %d bytes (%d left)\n", size, NVRAM_SPACE - size);
	}

	free(buf);

	return 0;
}

/* NVRAM utility */
int
main(int argc, char **argv)
{
	char *name, *value;
	int ret = 0;

	/* Skip program name */
	--argc;
	++argv;

	if (!*argv)
		usage();

	/* Process the remaining arguments. */
	for (; *argv; argv++) {
		if (!strcmp(*argv, "get")) {
			if (*++argv) {
				if ((value = nvram_get(*argv)))
					puts(value);
			}
		}
		else if (!strcmp(*argv, "settmp")) {
			if (*++argv) {
				char buf[1024];
				strncpy(value = buf, *argv, sizeof(buf)-1);
				name = strsep(&value, "=");
				nvram_set_temp(name, value);
			}
		}
		else if (!strcmp(*argv, "set")) {
			if (*++argv) {
				char buf[1024];
				strncpy(value = buf, *argv, sizeof(buf)-1);
				name = strsep(&value, "=");
				nvram_set(name, value);
			}
		}
		else if (!strcmp(*argv, "unset")) {
			if (*++argv)
				nvram_unset(*argv);
		}
		else if (!strcmp(*argv, "clear")) {
			nvram_clear();
		}
		else if (!strcmp(*argv, "commit")) {
			nvram_commit();
			break;
		}
		else if (!strcmp(*argv, "save")) {
			if (*++argv)
				ret |= nvram_save(*argv);
			break;
		}
		else if (!strcmp(*argv, "restore")) {
			if (*++argv)
				ret |= nvram_restore(*argv);
			break;
		}
		else if (!strcmp(*argv, "show")) {
			ret |= nvram_show(0);
			break;
		}
		else if (!strcmp(*argv, "showall")) {
			ret |= nvram_show(1);
			break;
		}
		if (!*argv)
			break;
	}

	return ret;
}
