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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <httpd.h>

#define MAX_PATTERN_LENGTH	1024

static const char *asp_mark1 = "<%", *asp_mark2 = "%>";
static const char *kw_mark1 = "<#", *kw_mark2 = "#>";

/* Look for unquoted character within a string */
static char *
unqstrstr(char *haystack, char *needle)
{
	char *cur;
	int q;

	for (cur = haystack, q = 0;
	     cur < &haystack[strlen(haystack)] && !(!q && !strncmp(needle, cur, strlen(needle)));
	     cur++) {
		if (*cur == '"')
			q ? q-- : q++;
	}
	return (cur < &haystack[strlen(haystack)]) ? cur : NULL;
}

static char *
get_arg(char *args, char **next)
{
	char *arg, *end;

	/* Parse out arg, ... */
	if (!(end = unqstrstr(args, ","))) {
		end = args + strlen(args);
		*next = NULL;
	} else
		*next = end + 1;

	/* Skip whitespace and quotation marks on either end of arg */
	for (arg = args; isspace((int)*arg) || *arg == '"'; arg++);
	for (*end-- = '\0'; isspace((int)*end) || *end == '"'; end--)
		*end = '\0';

	return arg;
}

static int
call_asp(char *func, FILE *stream)
{
	int argc;
	char *args, *end, *next, *argv[16] = {NULL};
	struct ej_handler *handler;
	int success = 0;

	/* Parse out ( args ) */
	if (!(args = strchr(func, '(')))
		return 0;
	if (!(end = unqstrstr(func, ")")))
		return 0;
	*args++ = *end = '\0';

	/* Set up argv list */
	for (argc = 0; argc < 16 && args && *args; argc++, args = next) {
		if (!(argv[argc] = get_arg(args, &next)))
			break;
	}

	/* Call handler */
	for (handler = &ej_handlers[0]; handler->pattern; handler++) {
		if (strcmp(handler->pattern, func) == 0){
			handler->output(0, stream, argc, argv);
			success = 1;
			break;
		}
	}

	return success;
}

// Call this function if and only if we can read whole <%....%> pattern.
static char *
process_asp(char *s, char *e, FILE *stream)
{
	char *func, *end;
	int asp_res = 0;

	if (!s || !e || s >= e)
		return NULL;

	func = s;

	/* Skip initial whitespace */
	while (func < e && isspace((int)*func))
		func++;

	/* find end of function '<% f(); %>' */
	end = unqstrstr(func, ";");
	if (end && end < e && (end-func) > 2) {
		int asp_flen = end-func + 1;
		char asp_func[asp_flen];
		
		/* copy func */
		memcpy(asp_func, func, asp_flen-1);
		asp_func[asp_flen-1] = '\0';
		
		/* Call function (1: success, 0: pattern not found) */
		asp_res = call_asp(asp_func, stream);
	}

	if (!asp_res) {
		/* write pattern unchanged */
		fwrite(s - strlen(asp_mark1), 1, e - s + strlen(asp_mark1) + strlen(asp_mark2), stream);
	}

	/* skip asp_mark2 */
	return e + strlen(asp_mark2);
}

static char*
search_desc(pkw_t pkw, char *name)
{
	int i, len;
	char *p, *ret = NULL;

	if (!pkw)
		return NULL;

	len = strlen(name);

	for (i = 0; i < pkw->len; ++i)  {
		p = pkw->idx[i];
		if (strncmp(name, p, len) == 0) {
			ret = p + len;
			break;
		}
	}

	return ret;
}

// Call this function if and only if we can read whole <#....#> pattern.
static char *
translate_lang(char *s, char *e, FILE *stream, kw_t *pkw)
{
	char *name, *end, *desc = NULL;

	if (!s || !e || s >= e)
		return NULL;

	name = s;

	/* Skip initial whitespace */
	while (name < e && isspace((int)*name))
		name++;

	/* find end of name '<# NNN #>' */
	end = strstr(name, kw_mark2);
	if (end && end <= e && (end-name) > 0) {
		*end++ = '=';	// '#' --> '=', search_desc() need '='
		*end = '\0';	// '>' --> '\0'
		
		desc = search_desc(pkw, name);
		if (!desc && pkw != &kw_EN)
			desc = search_desc(&kw_EN, name);
		
		if (desc) {
			/* write translation from dictionary */
			fputs(desc, stream);
		} else {
			/* restore pattern body */
			*end-- = '>';
			*end = '#';
		}
	}

	if (!desc) {
		/* write pattern unchanged */
		fwrite(s - strlen(kw_mark1), 1, e - s + strlen(kw_mark1) + strlen(kw_mark2), stream);
	}

	/* skip kw_mark2 */
	return e + strlen(kw_mark2);
}

char *
get_alert_msg_from_dict(const char *msg_id)
{
	pkw_t pkw = &kw_EN;
	char msg_name[32], *desc, *lang;

	lang = nvram_safe_get("preferred_lang");
	if (strlen(lang) > 1 && strcmp(lang, kw_XX.dict) == 0)
		pkw = &kw_XX;

	snprintf(msg_name, sizeof(msg_name), "ALERT_OF_ERROR_%s=", msg_id);

	desc = search_desc(pkw, msg_name);
	if (!desc && pkw != &kw_EN)
		desc = search_desc(&kw_EN, msg_name);

	return desc;
}

void
release_dictionary(pkw_t pkw)
{
	if (!pkw)
		return;

	if (pkw->idx)
		free(pkw->idx);

	if (pkw->buf)
		free(pkw->buf);

	memset(pkw, 0, sizeof(kw_t));
}

int
load_dictionary(char *lang, pkw_t pkw)
{
	FILE *dfp;
	char dfn[16];
	char *p, *q;
	int res, dict_size = 0;

	if (!pkw)
		return 0;

	snprintf(dfn, sizeof (dfn), "%s.dict", lang);
	dfp = fopen(dfn, "r");
	if (!dfp)
		return 0;

	memset(pkw, 0, sizeof(kw_t));

	fseek(dfp, 0L, SEEK_END);
	dict_size = ftell(dfp) + 128;

	pkw->buf = malloc(dict_size);
	if (!pkw->buf) {
		fclose(dfp);
		return 0;
	}

	snprintf(pkw->dict, sizeof(pkw->dict), "%s", lang);
	REALLOC_VECTOR (pkw->idx, pkw->len, pkw->tlen, sizeof (unsigned char*));

	fseek(dfp, 0L, SEEK_SET);

	q = pkw->buf;

	while ((res = fscanf(dfp, "%[^\n]", q)) != EOF) {
		fgetc(dfp);
		
		if (res < 1)
			continue;
		
		p = strchr(q, '=');
		if (!p)
			continue;
		
		REALLOC_VECTOR (pkw->idx, pkw->len, pkw->tlen, sizeof (unsigned char*));
		pkw->idx[pkw->len] = q;
		pkw->len++;
		q = p + strlen(p);
		*q = '\0';
		q++;
	}

	fclose(dfp);

	return 1;
}

// This translation engine can not process <%...%> interlace with <#...#>
void
do_ej(const char *url, FILE *stream)
{
#define FRAG_SIZE	128
#define RESERVE_SIZE	4
	FILE *fp;
	int frag_size = FRAG_SIZE;
	int pattern_size = MAX_PATTERN_LENGTH - RESERVE_SIZE;
	char pat_buf[MAX_PATTERN_LENGTH];
	char *pattern = pat_buf, *asp = NULL, *asp_end = NULL, *key = NULL, *key_end = NULL;
	char *start_pat, *end_pat, *lang;
	pkw_t pkw = &kw_EN;
	int conn_break = 0;
	size_t ret, read_len, len;
	int no_translate = 1;

	if (!(fp = fopen(url, "r")))
		return;

	// Load dictionary file
	lang = nvram_safe_get("preferred_lang");
	if (strlen(lang) > 1 && strcmp(lang, "EN") != 0) {
		if (strcmp(lang, kw_XX.dict) != 0) {
			release_dictionary(&kw_XX);
			if (load_dictionary(lang, &kw_XX))
				pkw = &kw_XX;
		} else
			pkw = &kw_XX;
	} else {
		if (kw_XX.buf)
			release_dictionary(&kw_XX);
	}

	if (pkw->buf)
		no_translate = 0;

	start_pat = end_pat = pattern;
	memset (pattern + pattern_size, 0, 4);
	while (conn_break == 0)
	{
		int special;

		// Arrange pattern[] if available buffer length (end_pat~pattern[pattern_size]) is smaller than frag_size
		if (((pattern + pattern_size) - end_pat) < frag_size)
		{
			len = end_pat - start_pat;
			memcpy (pattern, start_pat, len);
			start_pat = pattern;
			end_pat = start_pat + len;
			*end_pat = '\0';
		}

		read_len = (pattern + pattern_size) - end_pat;
		len = fread (end_pat, 1, read_len, fp);
		if (len == 0)   {
			if (start_pat < end_pat){
				fwrite (start_pat, 1, (size_t) (end_pat - start_pat), stream);
			}
			break;
		}
		end_pat += len;
		*end_pat = '\0';

		asp = strstr (start_pat, asp_mark1);
		key = NULL;
		if (no_translate == 0)  {
			key = strstr (start_pat, kw_mark1);
		}
		special = 0;
		while ((start_pat < end_pat) && special == 0)
		{
			int postproc = 0;       /* 0: need more data; 1: translate; 2: execute asp; 3: write only; */
			char *s, *e, *p;

			/*				 asp      asp_end
			 *				 ^	^
			 *      +------------------------------<%.......%>-------------------------------+
			 *  |	 XXXXXXXXXXXXXXXXXXXXX<#.......#>YYYYYYYYYYYYYYYYYY0	    |0000
			 *  +------------------------------------------------------------------------+
			 *  ^	 ^		    ^	^ ^		 ^	     ^
			 *  |	 |		    |	| p		 |	     |
			 *  pattern   start_pat,s	  key,e(2) key_end	     end_pat,e(1)  pattern + pattern_size
			 *				     ^				|
			 *				     +--------------------------------+
			 *
			 */

			// If <%...%> and <#...#> do not exist in pattern[], write whole pattern[].
			s = start_pat;
			e = end_pat;

			if (key != NULL && asp == NULL) {
				e = key;						// Write start_pat ~ (key - 1)
				key_end = strstr (key, kw_mark2);
				if (key_end != NULL)    {	       // We do have <#...#> in pattern[].
					postproc = 1;
				}
			} else if (key != NULL && asp != NULL)  {
				// We have <%...%> and <#...#> in pattern[], process first occurrence
				if (asp < key)  {
					e = asp;					// Write start_pat ~ (asp - 1)
					asp_end = strstr (asp, asp_mark2);
					if (asp_end != NULL)    {       // We do have whole <%...%>.
						postproc = 2;
					}
				} else {
					e = key;					// Write start_pat ~ (key - 1)
					key_end = strstr (key, kw_mark2);
					if (key_end != NULL)    {       // We do have whole <#...#>.
						postproc = 1;
					}
				}
			} else if (key == NULL && asp != NULL)  {
				e = asp;						// Write start_pat ~ (asp - 1)
				asp_end = strstr (asp, asp_mark2);
				if (asp_end != NULL)    {	       // We do have whole <%...%>.
					postproc = 2;
				}
			} else {
				// Special case. If last character is '<'
				// DO NOT write this character due to next one may be % or #.
				if (*(e-1) == *asp_mark1 || *(e-1) == *kw_mark1)	{
					special = 1;
					e--;
				}
				postproc = 3;
			}

			// process text preceeding <# or <%
			if (e > s)      {
				ret = fwrite (s, 1, (size_t) (e - s), stream);
				if (ret == 0 || ret < (e - s))  {
					/* the connection had been damaged. DO NOT process another data. */
					/* (reduce response time of httpd) */
//					cprintf ("fwrite() ret %d, s %p e %p len %d, break do_ej()'s while loop\n", ret, s, e, e-s);
					conn_break = 1;
					break;
				} else {
					start_pat = e;
				}
			}
			// post process
			p = NULL;
			if (postproc == 1) {			    // translate
				p = translate_lang (key + strlen (kw_mark1), key_end, stream, pkw);
				if (no_translate == 0 && p != NULL)     {
					key = strstr (p, kw_mark1);
				}
			} else if (postproc == 2)       {	       // execute asp
				p = process_asp (asp + strlen (asp_mark1), asp_end, stream);
				if (p != NULL)  {
					asp = strstr (p, asp_mark1);
				}
			} else if (postproc == 3)       {	       // no <%...%> or <#...#>
				p = e;
			} else if (postproc == 0)       {	       // read more data
				break;
			}

			if (p != NULL)  {
				start_pat = p;
			}

		}       /* while ((start_pat < end_pat) && special == 0) */
	}	       /* while (conn_break == 0) */

	fclose(fp);

	if (pattern != pat_buf)
		free (pattern);

	fflush(stream);
}

int
ejArgs(int argc, char **argv, char *fmt, ...)
{
	va_list	ap;
	int arg;
	char *c;

	if (!argv)
		return 0;

	va_start(ap, fmt);
	for (arg = 0, c = fmt; c && *c && arg < argc;) {
		if (*c++ != '%')
			continue;
		switch (*c) {
		case 'd':
			*(va_arg(ap, int *)) = atoi(argv[arg]);
			break;
		case 's':
			*(va_arg(ap, char **)) = argv[arg];
			break;
		}
		arg++;
	}
	va_end(ap);

	return arg;
}
