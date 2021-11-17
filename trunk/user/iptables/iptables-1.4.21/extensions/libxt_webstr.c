/* Shared library add-on to iptables to add string matching support..
 *.
 * Copyright (C) 2000 Emmanuel Roger  <winfield@freegates.be>
 *
 * ChangeLog
 *     27.01.2001: Gianni Tedesco <gianni@ecsc.co.uk>
 *             Changed --tos to --string in save(). Also
 *             updated to work with slightly modified
 *             xt_string_info.
 */

/* Shared library add-on to iptables to add webstr matching support. 
 *
 * Copyright (C) 2003, CyberTAN Corporation
 * All Rights Reserved.
 *
 * Description:
 *   This is shared library, added to iptables, for web content inspection. 
 *   It was derived from 'string' matching support, declared as above.
 *
 * ChangeLog
 *	2009-09-26: Leonid Lisovskiy <lly.dev@gmail.com>
 *			Fix save() method, minor parse() optimization.
 */


#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <xtables.h>

#define BM_MAX_NLEN 256

#define BLK_JAVA		0x01
#define BLK_ACTIVE		0x02
#define BLK_COOKIE		0x04
#define BLK_PROXY		0x08

struct xt_webstr_info {
    char string[BM_MAX_NLEN];
    u_int16_t invert;
    u_int16_t len;
    u_int8_t type;
};

enum xt_webstr_type
{
    XT_WEBSTR_HOST,
    XT_WEBSTR_URL,
    XT_WEBSTR_CONTENT
};


/* Function which prints out usage message. */
static void
webstr_help(void)
{
	printf(
"WEBSTR match v%s options:\n"
"--host [!] host               Match a http string in a packet\n"
"--url [!] url                 Match a http string in a packet\n"
"--content [!] content         Match a http string in a packet\n"
"\n",
XTABLES_VERSION);
}

static struct option webstr_opts[] = {
	{ "host",    1, 0, '1' },
	{ "url",     1, 0, '2' },
	{ "content", 1, 0, '3' },
	XT_GETOPT_TABLEEND,
};

/* Initialize the match. */
static void
webstr_init(struct xt_entry_match *m)
{
}

static void
parse_string(const unsigned char *s, struct xt_webstr_info *info)
{
	if (strlen(s) <= BM_MAX_NLEN) strcpy(info->string, s);
	else xtables_error(PARAMETER_PROBLEM, "WEBSTR too long `%s'", s);
}

static int
webstr_parse(int c, char **argv, int invert, unsigned int *flags,
             const void *entry, struct xt_entry_match **match)
{
	struct xt_webstr_info *stringinfo = (struct xt_webstr_info *)(*match)->data;

	switch (c) {
	case '1':
		stringinfo->type = XT_WEBSTR_HOST;
		break;
	case '2':
		stringinfo->type = XT_WEBSTR_URL;
		break;
	case '3':
		stringinfo->type = XT_WEBSTR_CONTENT;
		break;
	default:
		return 0;
	}

	if (invert)
		stringinfo->invert = 1;
	parse_string(argv[optind-1], stringinfo);
	stringinfo->len=strlen((char *)&stringinfo->string);

	*flags = 1;
	return 1;
}

static void
print_string(char string[], int invert, int numeric)
{
	if (invert)
		fputc('!', stdout);
	printf("%s ",string);
}

/* Final check; must have specified --string. */
static void
webstr_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			   "WEBSTR match: You must specify `--webstr'");
}

static void
print_type(unsigned type)
{
	switch (type) {
	case XT_WEBSTR_HOST:
		printf("%s ", webstr_opts[0].name);
		break;

	case XT_WEBSTR_URL:
		printf("%s ", webstr_opts[1].name);
		break;

	case XT_WEBSTR_CONTENT:
		printf("%s ", webstr_opts[2].name);
		break;

	default:
		printf("ERROR ");
		break;
	}
}

/* Prints out the matchinfo. */
static void
webstr_print(const void *ip,
      const struct xt_entry_match *match,
      int numeric)
{
	struct xt_webstr_info *stringinfo = (struct xt_webstr_info *)match->data;

	printf("webstr: ");
	print_type(stringinfo->type);
	print_string(stringinfo->string, stringinfo->invert, numeric);
}

/* Saves the union xt_matchinfo in parsable form to stdout. */
static void
webstr_save(const void *ip, const struct xt_entry_match *match)
{
	struct xt_webstr_info *stringinfo = (struct xt_webstr_info *)match->data;

	printf(" --"); print_type(stringinfo->type);
	print_string(stringinfo->string, stringinfo->invert, 0);
}

static struct xtables_match webstr = { 
	.family		= NFPROTO_UNSPEC,
	.name		= "webstr",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_webstr_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_webstr_info)),
	.help		= webstr_help,
	.init		= webstr_init,
	.print		= webstr_print,
	.save		= webstr_save,
	.parse		= webstr_parse,
	.final_check	= webstr_check,
	.extra_opts	= webstr_opts
};

void _init(void)
{
	xtables_register_match(&webstr);
}
