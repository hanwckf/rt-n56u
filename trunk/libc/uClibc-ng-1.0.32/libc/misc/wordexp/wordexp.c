/* POSIX.2 wordexp implementation.
   Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Tim Waugh <tim@cyberelk.demon.co.uk>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   see <http://www.gnu.org/licenses/>.  */

#include <features.h>
#include <bits/kernel-features.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <assert.h>
#include <fnmatch.h>
#include <glob.h>
#include <wordexp.h>

#ifndef __ARCH_USE_MMU__
# define fork vfork
#endif

#define __WORDEXP_FULL

/*
 * This is a recursive-descent-style word expansion routine.
 */

/* These variables are defined and initialized in the startup code.  */
/*extern int __libc_argc;*/
/*extern char **__libc_argv;*/

/* FIXME!!!! */
int attribute_hidden __libc_argc;
char attribute_hidden **__libc_argv;

/* Some forward declarations */
static int parse_dollars(char **word, size_t * word_length,
						 size_t * max_length, const char *words,
						 size_t * offset, int flags, wordexp_t * pwordexp,
						 const char *ifs, const char *ifs_white,
						 int quoted);
static int parse_backtick(char **word, size_t * word_length,
						  size_t * max_length, const char *words,
						  size_t * offset, int flags, wordexp_t * pwordexp,
						  const char *ifs, const char *ifs_white);
static int parse_dquote(char **word, size_t * word_length,
						size_t * max_length, const char *words,
						size_t * offset, int flags, wordexp_t * pwordexp,
						const char *ifs, const char *ifs_white);



/* The w_*() functions manipulate word lists. */
#define W_CHUNK	(100)

/* Result of w_newword will be ignored if it's the last word. */
static __inline__ char *w_newword(size_t * actlen, size_t * maxlen)
{
	*actlen = *maxlen = 0;
	return NULL;
}

/* Add a character to the buffer, allocating room for it if needed.  */
static __inline__ char *w_addchar(char *buffer, size_t * actlen,
							  size_t * maxlen, char ch)
	 /* (lengths exclude trailing zero) */
{

	if (*actlen == *maxlen) {
		char *old_buffer = buffer;
		assert(buffer == NULL || *maxlen != 0);
		*maxlen += W_CHUNK;
		buffer = realloc(buffer, 1 + *maxlen);
		if (buffer == NULL)
			free(old_buffer);
	}

	if (buffer != NULL) {
		buffer[*actlen] = ch;
		buffer[++(*actlen)] = '\0';
	}

	return buffer;
}
#ifndef MAX
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#endif
static char *w_addmem(char *buffer, size_t * actlen, size_t * maxlen,
					  const char *str, size_t len)
{
	/* Add a string to the buffer, allocating room for it if needed.
	 */
	if (*actlen + len > *maxlen) {
		char *old_buffer = buffer;
		assert(buffer == NULL || *maxlen != 0);
		*maxlen += MAX(2 * len, W_CHUNK);
		buffer = realloc(old_buffer, 1 + *maxlen);
		if (buffer == NULL)
			free(old_buffer);
	}

	if (buffer != NULL) {
		*((char *) mempcpy(&buffer[*actlen], str, len)) = '\0';
		*actlen += len;
	}
	return buffer;
}

/* Add a string to the buffer, allocating room for it if needed.  */
static char *w_addstr(char *buffer, size_t * actlen, size_t * maxlen,
					  const char *str)
	 /* (lengths exclude trailing zero) */
{
	size_t len;
	assert(str != NULL);		/* w_addstr only called from this file */
	len = strlen(str);

	return w_addmem(buffer, actlen, maxlen, str, len);
}

/* Add a word to the wordlist */
static int w_addword(wordexp_t * pwordexp, char *word)
{
	size_t num_p;
	char **new_wordv;

	/* Internally, NULL acts like "".  Convert NULLs to "" before
	 * the caller sees them.
	 */
	if (word == NULL) {
		word = strdup("");
		if (word == NULL)
			goto no_space;
	}

	num_p = 2 + pwordexp->we_wordc + pwordexp->we_offs;
	new_wordv = realloc(pwordexp->we_wordv, sizeof(char *) * num_p);
	if (new_wordv != NULL) {
		pwordexp->we_wordv = new_wordv;
		pwordexp->we_wordv[pwordexp->we_offs + pwordexp->we_wordc++] = word;
		pwordexp->we_wordv[pwordexp->we_offs + pwordexp->we_wordc] = NULL;
		return 0;
	}

  no_space:
	return WRDE_NOSPACE;
}

/* The parse_*() functions should leave *offset being the offset in 'words'
 * to the last character processed.
 */
static int
parse_backslash(char **word, size_t * word_length, size_t * max_length,
				const char *words, size_t * offset)
{
	/* We are poised _at_ a backslash, not in quotes */

	switch (words[1 + *offset]) {
	case 0:
		/* Backslash is last character of input words */
		return WRDE_SYNTAX;

	case '\n':
		++(*offset);
		break;

	default:
		*word = w_addchar(*word, word_length, max_length, words[1 + *offset]);
		if (*word == NULL)
			return WRDE_NOSPACE;

		++(*offset);
		break;
	}

	return 0;
}

static int
parse_qtd_backslash(char **word, size_t * word_length, size_t * max_length,
					const char *words, size_t * offset)
{
	/* We are poised _at_ a backslash, inside quotes */

	switch (words[1 + *offset]) {
	case 0:
		/* Backslash is last character of input words */
		return WRDE_SYNTAX;

	case '\n':
		++(*offset);
		break;

	case '$':
	case '`':
	case '"':
	case '\\':
		*word =
			w_addchar(*word, word_length, max_length, words[1 + *offset]);
		if (*word == NULL)
			return WRDE_NOSPACE;

		++(*offset);
		break;

	default:
		*word = w_addchar(*word, word_length, max_length, words[*offset]);
		if (*word != NULL)
			*word =
				w_addchar(*word, word_length, max_length,
						  words[1 + *offset]);

		if (*word == NULL)
			return WRDE_NOSPACE;

		++(*offset);
		break;
	}

	return 0;
}

static int
parse_tilde(char **word, size_t * word_length, size_t * max_length,
			const char *words, size_t * offset, size_t wordc)
{
	/* We are poised _at_ a tilde */
	size_t i;

	if (*word_length != 0) {
		if (!((*word)[*word_length - 1] == '=' && wordc == 0)) {
			if (!((*word)[*word_length - 1] == ':'
				  && strchr(*word, '=') && wordc == 0)) {
				*word = w_addchar(*word, word_length, max_length, '~');
				return *word ? 0 : WRDE_NOSPACE;
			}
		}
	}

	for (i = 1 + *offset; words[i]; i++) {
		if (words[i] == ':' || words[i] == '/' || words[i] == ' ' ||
			words[i] == '\t' || words[i] == 0)
			break;

		if (words[i] == '\\') {
			*word = w_addchar(*word, word_length, max_length, '~');
			return *word ? 0 : WRDE_NOSPACE;
		}
	}

	if (i == 1 + *offset) {
		/* Tilde appears on its own */
		uid_t uid;
		struct passwd pwd, *tpwd;
		int buflen = 1000;
		char *home;
		char *buffer;
		int result;

		/* POSIX.2 says ~ expands to $HOME and if HOME is unset the
		   results are unspecified.  We do a lookup on the uid if
		   HOME is unset. */

		home = getenv("HOME");
		if (home != NULL) {
			*word = w_addstr(*word, word_length, max_length, home);
			if (*word == NULL)
				return WRDE_NOSPACE;
		} else {
			uid = getuid();
			buffer = alloca(buflen);

			while ((result = getpwuid_r(uid, &pwd, buffer, buflen, &tpwd))
					!= 0 && errno == ERANGE)
			{
				buflen += 1000;
				buffer = alloca(buflen);
			}

			if (result == 0 && tpwd != NULL && pwd.pw_dir != NULL) {
				*word = w_addstr(*word, word_length, max_length, pwd.pw_dir);
				if (*word == NULL)
					return WRDE_NOSPACE;
			} else {
				*word = w_addchar(*word, word_length, max_length, '~');
				if (*word == NULL)
					return WRDE_NOSPACE;
			}
		}
	} else {
		/* Look up user name in database to get home directory */
		char *user = strndup(&words[1 + *offset], i - (1 + *offset));
		struct passwd pwd, *tpwd;
		int buflen = 1000;
		char *buffer = alloca(buflen);
		int result;

		while ((result = getpwnam_r(user, &pwd, buffer, buflen, &tpwd)) != 0
			   && errno == ERANGE) {
			buflen += 1000;
			buffer = alloca(buflen);
		}

		if (result == 0 && tpwd != NULL && pwd.pw_dir)
			*word = w_addstr(*word, word_length, max_length, pwd.pw_dir);
		else {
			/* (invalid login name) */
			*word = w_addchar(*word, word_length, max_length, '~');
			if (*word != NULL)
				*word = w_addstr(*word, word_length, max_length, user);
		}

		*offset = i - 1;
	}
	return *word ? 0 : WRDE_NOSPACE;
}


static int
do_parse_glob(const char *glob_word, char **word, size_t * word_length,
			  size_t * max_length, wordexp_t * pwordexp, const char *ifs
			  /*, const char *ifs_white*/)
{
	int error;
	int match;
	glob_t globbuf;

	error = glob(glob_word, GLOB_NOCHECK, NULL, &globbuf);

	if (error != 0) {
		/* We can only run into memory problems.  */
		assert(error == GLOB_NOSPACE);
		return WRDE_NOSPACE;
	}

	if (ifs && !*ifs) {
		/* No field splitting allowed. */
		assert(globbuf.gl_pathv[0] != NULL);
		*word = w_addstr(*word, word_length, max_length, globbuf.gl_pathv[0]);
		for (match = 1; match < globbuf.gl_pathc && *word != NULL; ++match) {
			*word = w_addchar(*word, word_length, max_length, ' ');
			if (*word != NULL)
				*word = w_addstr(*word, word_length, max_length,
								 globbuf.gl_pathv[match]);
		}

		globfree(&globbuf);
		return *word ? 0 : WRDE_NOSPACE;
	}

	assert(ifs == NULL || *ifs != '\0');
	if (*word != NULL) {
		free(*word);
		*word = w_newword(word_length, max_length);
	}

	for (match = 0; match < globbuf.gl_pathc; ++match) {
		char *matching_word = strdup(globbuf.gl_pathv[match]);

		if (matching_word == NULL || w_addword(pwordexp, matching_word)) {
			globfree(&globbuf);
			return WRDE_NOSPACE;
		}
	}

	globfree(&globbuf);
	return 0;
}

static int
parse_glob(char **word, size_t * word_length, size_t * max_length,
		   const char *words, size_t * offset, int flags,
		   wordexp_t * pwordexp, const char *ifs, const char *ifs_white)
{
	/* We are poised just after a '*', a '[' or a '?'. */
	int error = WRDE_NOSPACE;
	int quoted = 0;				/* 1 if singly-quoted, 2 if doubly */
	int i;
	wordexp_t glob_list;		/* List of words to glob */

	glob_list.we_wordc = 0;
	glob_list.we_wordv = NULL;
	glob_list.we_offs = 0;
	for (; words[*offset] != '\0'; ++*offset) {
		if ((ifs && strchr(ifs, words[*offset])) ||
			(!ifs && strchr(" \t\n", words[*offset])))
			/* Reached IFS */
			break;

		/* Sort out quoting */
		if (words[*offset] == '\'') {
			if (quoted == 0) {
				quoted = 1;
				continue;
			} else if (quoted == 1) {
				quoted = 0;
				continue;
			}
		} else if (words[*offset] == '"') {
			if (quoted == 0) {
				quoted = 2;
				continue;
			} else if (quoted == 2) {
				quoted = 0;
				continue;
			}
		}

		/* Sort out other special characters */
		if (quoted != 1 && words[*offset] == '$') {
			error = parse_dollars(word, word_length, max_length, words,
								  offset, flags, &glob_list, ifs,
								  ifs_white, quoted == 2);
			if (error)
				goto tidy_up;

			continue;
		} else if (words[*offset] == '\\') {
			if (quoted)
				error = parse_qtd_backslash(word, word_length, max_length,
											words, offset);
			else
				error = parse_backslash(word, word_length, max_length,
										words, offset);

			if (error)
				goto tidy_up;

			continue;
		}

		*word = w_addchar(*word, word_length, max_length, words[*offset]);
		if (*word == NULL)
			goto tidy_up;
	}

	/* Don't forget to re-parse the character we stopped at. */
	--*offset;

	/* Glob the words */
	error = w_addword(&glob_list, *word);
	*word = w_newword(word_length, max_length);
	for (i = 0; error == 0 && i < glob_list.we_wordc; i++)
		error = do_parse_glob(glob_list.we_wordv[i], word, word_length,
				max_length, pwordexp, ifs /*, ifs_white*/);

	/* Now tidy up */
  tidy_up:
	wordfree(&glob_list);
	return error;
}

static int
parse_squote(char **word, size_t * word_length, size_t * max_length,
			 const char *words, size_t * offset)
{
	/* We are poised just after a single quote */
	for (; words[*offset]; ++(*offset)) {
		if (words[*offset] != '\'') {
			*word = w_addchar(*word, word_length, max_length, words[*offset]);
			if (*word == NULL)
				return WRDE_NOSPACE;
		} else
			return 0;
	}

	/* Unterminated string */
	return WRDE_SYNTAX;
}

#ifdef __WORDEXP_FULL
static int eval_expr(char *expr, long int *result);

static char *_itoa(unsigned long long int value, char *buf)
{
	sprintf(buf, "%llu", value);
	return buf;
}

/* Functions to evaluate an arithmetic expression */
static int eval_expr_val(char **expr, long int *result)
{
	int sgn = +1;
	char *digit;

	/* Skip white space */
	for (digit = *expr; digit && *digit && isspace(*digit); ++digit);

	switch (*digit) {
	case '(':

		/* Scan for closing paren */
		for (++digit; **expr && **expr != ')'; ++(*expr));

		/* Is there one? */
		if (!**expr)
			return WRDE_SYNTAX;

		*(*expr)++ = 0;

		if (eval_expr(digit, result))
			return WRDE_SYNTAX;

		return 0;

	case '+':					/* Positive value */
		++digit;
		break;

	case '-':					/* Negative value */
		++digit;
		sgn = -1;
		break;

	default:
		if (!isdigit(*digit))
			return WRDE_SYNTAX;
	}

	*result = 0;
	for (; *digit && isdigit(*digit); ++digit)
		*result = (*result * 10) + (*digit - '0');

	*expr = digit;
	*result *= sgn;
	return 0;
}

static int eval_expr_multdiv(char **expr, long int *result)
{
	long int arg;

	/* Read a Value */
	if (eval_expr_val(expr, result) != 0)
		return WRDE_SYNTAX;

	while (**expr) {
		/* Skip white space */
		for (; *expr && **expr && isspace(**expr); ++(*expr));

		if (**expr == '*') {
			++(*expr);
			if (eval_expr_val(expr, &arg) != 0)
				return WRDE_SYNTAX;

			*result *= arg;
		} else if (**expr == '/') {
			++(*expr);
			if (eval_expr_val(expr, &arg) != 0)
				return WRDE_SYNTAX;

			*result /= arg;
		} else
			break;
	}

	return 0;
}

static int eval_expr(char *expr, long int *result)
{
	long int arg;

	/* Read a Multdiv */
	if (eval_expr_multdiv(&expr, result) != 0)
		return WRDE_SYNTAX;

	while (*expr) {
		/* Skip white space */
		for (; expr && *expr && isspace(*expr); ++expr);

		if (*expr == '+') {
			++expr;
			if (eval_expr_multdiv(&expr, &arg) != 0)
				return WRDE_SYNTAX;

			*result += arg;
		} else if (*expr == '-') {
			++expr;
			if (eval_expr_multdiv(&expr, &arg) != 0)
				return WRDE_SYNTAX;

			*result -= arg;
		} else
			break;
	}

	return 0;
}

static int
parse_arith(char **word, size_t * word_length, size_t * max_length,
			const char *words, size_t * offset, int flags, int bracket)
{
	/* We are poised just after "$((" or "$[" */
	int error;
	int paren_depth = 1;
	size_t expr_length;
	size_t expr_maxlen;
	char *expr;

	expr = w_newword(&expr_length, &expr_maxlen);
	for (; words[*offset]; ++(*offset)) {
		switch (words[*offset]) {
		case '$':
			error = parse_dollars(&expr, &expr_length, &expr_maxlen,
								  words, offset, flags, NULL, NULL, NULL,
								  1);
			/* The ``1'' here is to tell parse_dollars not to
			 * split the fields.
			 */
			if (error) {
				free(expr);
				return error;
			}
			break;

		case '`':
			(*offset)++;
			error = parse_backtick(&expr, &expr_length, &expr_maxlen,
								   words, offset, flags, NULL, NULL, NULL);
			/* The first NULL here is to tell parse_backtick not to
			 * split the fields.
			 */
			if (error) {
				free(expr);
				return error;
			}
			break;

		case '\\':
			error = parse_qtd_backslash(&expr, &expr_length, &expr_maxlen,
										words, offset);
			if (error) {
				free(expr);
				return error;
			}
			/* I think that a backslash within an
			 * arithmetic expansion is bound to
			 * cause an error sooner or later anyway though.
			 */
			break;

		case ')':
			if (--paren_depth == 0) {
				char result[21];	/* 21 = ceil(log10(2^64)) + 1 */
				long int numresult = 0;
				long long int convertme;

				if (bracket || words[1 + *offset] != ')') {
					free(expr);
					return WRDE_SYNTAX;
				}

				++(*offset);

				/* Go - evaluate. */
				if (*expr && eval_expr(expr, &numresult) != 0) {
					free(expr);
					return WRDE_SYNTAX;
				}

				if (numresult < 0) {
					convertme = -numresult;
					*word = w_addchar(*word, word_length, max_length, '-');
					if (!*word) {
						free(expr);
						return WRDE_NOSPACE;
					}
				} else
					convertme = numresult;

				result[20] = '\0';
				*word = w_addstr(*word, word_length, max_length,
								 _itoa(convertme, result));
				free(expr);
				return *word ? 0 : WRDE_NOSPACE;
			}
			expr =
				w_addchar(expr, &expr_length, &expr_maxlen,
						  words[*offset]);
			if (expr == NULL)
				return WRDE_NOSPACE;

			break;

		case ']':
			if (bracket && paren_depth == 1) {
				char result[21];	/* 21 = ceil(log10(2^64)) + 1 */
				long int numresult = 0;

				/* Go - evaluate. */
				if (*expr && eval_expr(expr, &numresult) != 0) {
					free(expr);
					return WRDE_SYNTAX;
				}

				result[20] = '\0';
				*word = w_addstr(*word, word_length, max_length,
								 _itoa(numresult, result));
				free(expr);
				return *word ? 0 : WRDE_NOSPACE;
			}

			free(expr);
			return WRDE_SYNTAX;

		case '\n':
		case ';':
		case '{':
		case '}':
			free(expr);
			return WRDE_BADCHAR;

		case '(':
			++paren_depth;
		default:
			expr =
				w_addchar(expr, &expr_length, &expr_maxlen,
						  words[*offset]);
			if (expr == NULL)
				return WRDE_NOSPACE;
		}
	}

	/* Premature end */
	free(expr);
	return WRDE_SYNTAX;
}

/* Function called by child process in exec_comm() */
static void attribute_noreturn
exec_comm_child(char *comm, int *fildes, int showerr, int noexec)
{
	int fd;
	const char *args[4] = { _PATH_BSHELL, "-c", comm, NULL };

	/* Execute the command, or just check syntax? */
	if (noexec)
		args[1] = "-nc";

	/* Redirect output.  */
	fd = fildes[1];
	if (likely(fd != STDOUT_FILENO)) {
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}
#if defined O_CLOEXEC && defined __UCLIBC_LINUX_SPECIFIC__ && defined __ASSUME_PIPE2
	else {
		/* Reset the close-on-exec flag (if necessary).  */
		fcntl (fd, F_SETFD, 0);
	}
#endif
	/* Redirect stderr to /dev/null if we have to.  */
	if (showerr == 0) {
		close(STDERR_FILENO);
		fd = open(_PATH_DEVNULL, O_WRONLY);
		if (fd >= 0 && fd != STDERR_FILENO) {
			dup2(fd, 2);
			close(fd);
		}
	}

	/* Make sure the subshell doesn't field-split on our behalf. */
	unsetenv("IFS");

	if (fildes[0] != 1)
		close(fildes[0]);
	execve(_PATH_BSHELL, (char *const *) args, __environ);

	/* Bad.  What now?  */
	abort();
}

/* Function to execute a command and retrieve the results */
/* pwordexp contains NULL if field-splitting is forbidden */
static int
exec_comm(char *comm, char **word, size_t * word_length,
		  size_t * max_length, int flags, wordexp_t * pwordexp,
		  const char *ifs, const char *ifs_white)
{
	int fildes[2];
	int bufsize = 128;
	int buflen;
	int i;
	int status = 0;
	size_t maxnewlines = 0;
	char *buffer;
	pid_t pid;

	/* Don't fork() unless necessary */
	if (!comm || !*comm)
		return 0;
#if defined O_CLOEXEC && defined __UCLIBC_LINUX_SPECIFIC__ && defined __ASSUME_PIPE2
	if (pipe2(fildes, O_CLOEXEC) < 0)
		/* Bad */
		return WRDE_NOSPACE;
#else
	if (pipe(fildes) < 0)
		/* Bad */
		return WRDE_NOSPACE;
#endif

	if ((pid = fork()) < 0) {
		/* Bad */
		close(fildes[0]);
		close(fildes[1]);
		return WRDE_NOSPACE;
	}

	if (pid == 0)
		exec_comm_child(comm, fildes, (flags & WRDE_SHOWERR), 0);

	/* Parent */

	close(fildes[1]);
	buffer = alloca(bufsize);

	if (!pwordexp)
		/* Quoted - no field splitting */
	{
		while (1) {
			if ((buflen = read(fildes[0], buffer, bufsize)) < 1) {
				if (waitpid(pid, &status, WNOHANG) == 0)
					continue;
				if ((buflen = read(fildes[0], buffer, bufsize)) < 1)
					break;
			}

			maxnewlines += buflen;

			*word = w_addmem(*word, word_length, max_length, buffer, buflen);
			if (*word == NULL)
				goto no_space;
		}
	} else
		/* Not quoted - split fields */
	{
		int copying = 0;

		/* 'copying' is:
		 *  0 when searching for first character in a field not IFS white space
		 *  1 when copying the text of a field
		 *  2 when searching for possible non-whitespace IFS
		 *  3 when searching for non-newline after copying field
		 */

		while (1) {
			if ((buflen = read(fildes[0], buffer, bufsize)) < 1) {
				if (waitpid(pid, &status, WNOHANG) == 0)
					continue;
				if ((buflen = read(fildes[0], buffer, bufsize)) < 1)
					break;
			}

			for (i = 0; i < buflen; ++i) {
				if (strchr(ifs, buffer[i]) != NULL) {
					/* Current character is IFS */
					if (strchr(ifs_white, buffer[i]) == NULL) {
						/* Current character is IFS but not whitespace */
						if (copying == 2) {
							/*            current character
							 *                   |
							 *                   V
							 * eg: text<space><comma><space>moretext
							 *
							 * So, strip whitespace IFS (like at the start)
							 */
							copying = 0;
							continue;
						}

						copying = 0;
						/* fall through and delimit field.. */
					} else {
						if (buffer[i] == '\n') {
							/* Current character is (IFS) newline */

							/* If copying a field, this is the end of it,
							   but maybe all that's left is trailing newlines.
							   So start searching for a non-newline. */
							if (copying == 1)
								copying = 3;

							continue;
						} else {
							/* Current character is IFS white space, but
							   not a newline */

							/* If not either copying a field or searching
							   for non-newline after a field, ignore it */
							if (copying != 1 && copying != 3)
								continue;

							/* End of field (search for non-ws IFS afterwards) */
							copying = 2;
						}
					}

					/* First IFS white space (non-newline), or IFS non-whitespace.
					 * Delimit the field.  Nulls are converted by w_addword. */
					if (w_addword(pwordexp, *word) == WRDE_NOSPACE)
						goto no_space;

					*word = w_newword(word_length, max_length);

					maxnewlines = 0;
					/* fall back round the loop.. */
				} else {
					/* Not IFS character */

					if (copying == 3) {
						/* Nothing but (IFS) newlines since the last field,
						   so delimit it here before starting new word */
						if (w_addword(pwordexp, *word) == WRDE_NOSPACE)
							goto no_space;

						*word = w_newword(word_length, max_length);
					}

					copying = 1;

					if (buffer[i] == '\n')	/* happens if newline not in IFS */
						maxnewlines++;
					else
						maxnewlines = 0;

					*word = w_addchar(*word, word_length, max_length,
									  buffer[i]);
					if (*word == NULL)
						goto no_space;
				}
			}
		}
	}

	/* Chop off trailing newlines (required by POSIX.2)  */
	/* Ensure we don't go back further than the beginning of the
	   substitution (i.e. remove maxnewlines bytes at most) */
	while (maxnewlines-- != 0 &&
		   *word_length > 0 && (*word)[*word_length - 1] == '\n') {
		(*word)[--*word_length] = '\0';

		/* If the last word was entirely newlines, turn it into a new word
		 * which can be ignored if there's nothing following it. */
		if (*word_length == 0) {
			free(*word);
			*word = w_newword(word_length, max_length);
			break;
		}
	}

	close(fildes[0]);

	/* Check for syntax error (re-execute but with "-n" flag) */
	if (buflen < 1 && status != 0) {
		if ((pid = fork()) < 0) {
			/* Bad */
			return WRDE_NOSPACE;
		}

		if (pid == 0) {
			fildes[0] = fildes[1] = -1;
			exec_comm_child(comm, fildes, 0, 1);
		}

		if (waitpid(pid, &status, 0) == pid && status != 0)
			return WRDE_SYNTAX;
	}

	return 0;

  no_space:
	kill(pid, SIGKILL);
	waitpid(pid, NULL, 0);
	close(fildes[0]);
	return WRDE_NOSPACE;
}

static int
parse_comm(char **word, size_t * word_length, size_t * max_length,
		   const char *words, size_t * offset, int flags,
		   wordexp_t * pwordexp, const char *ifs, const char *ifs_white)
{
	/* We are poised just after "$(" */
	int paren_depth = 1;
	int error = 0;
	int quoted = 0;				/* 1 for singly-quoted, 2 for doubly-quoted */
	size_t comm_length;
	size_t comm_maxlen;
	char *comm = w_newword(&comm_length, &comm_maxlen);

	for (; words[*offset]; ++(*offset)) {
		switch (words[*offset]) {
		case '\'':
			if (quoted == 0)
				quoted = 1;
			else if (quoted == 1)
				quoted = 0;

			break;

		case '"':
			if (quoted == 0)
				quoted = 2;
			else if (quoted == 2)
				quoted = 0;

			break;

		case ')':
			if (!quoted && --paren_depth == 0) {
				/* Go -- give script to the shell */
				if (comm) {
					error = exec_comm(comm, word, word_length, max_length,
									  flags, pwordexp, ifs, ifs_white);
					free(comm);
				}

				return error;
			}

			/* This is just part of the script */
			break;

		case '(':
			if (!quoted)
				++paren_depth;
		}

		comm = w_addchar(comm, &comm_length, &comm_maxlen, words[*offset]);
		if (comm == NULL)
			return WRDE_NOSPACE;
	}

	/* Premature end */
	free(comm);

	return WRDE_SYNTAX;
}

static int
parse_backtick(char **word, size_t * word_length, size_t * max_length,
			   const char *words, size_t * offset, int flags,
			   wordexp_t * pwordexp, const char *ifs,
			   const char *ifs_white)
{
	/* We are poised just after "`" */
	int error;
	int squoting = 0;
	size_t comm_length;
	size_t comm_maxlen;
	char *comm = w_newword(&comm_length, &comm_maxlen);

	for (; words[*offset]; ++(*offset)) {
		switch (words[*offset]) {
		case '`':
			/* Go -- give the script to the shell */
			error = exec_comm(comm, word, word_length, max_length, flags,
							  pwordexp, ifs, ifs_white);
			free(comm);
			return error;

		case '\\':
			if (squoting) {
				error = parse_qtd_backslash(&comm, &comm_length, &comm_maxlen,
										words, offset);

				if (error) {
					free(comm);
					return error;
				}

				break;
			}

			++(*offset);
			error = parse_backslash(&comm, &comm_length, &comm_maxlen, words,
								offset);

			if (error) {
				free(comm);
				return error;
			}

			break;

		case '\'':
			squoting = 1 - squoting;
		default:
			comm = w_addchar(comm, &comm_length, &comm_maxlen,
						  words[*offset]);
			if (comm == NULL)
				return WRDE_NOSPACE;
		}
	}

	/* Premature end */
	free(comm);
	return WRDE_SYNTAX;
}

static int
parse_param(char **word, size_t * word_length, size_t * max_length,
			const char *words, size_t * offset, int flags,
			wordexp_t * pwordexp, const char *ifs, const char *ifs_white,
			int quoted)
{
	/* We are poised just after "$" */
	enum action {
		ACT_NONE,
		ACT_RP_SHORT_LEFT = '#',
		ACT_RP_LONG_LEFT = 'L',
		ACT_RP_SHORT_RIGHT = '%',
		ACT_RP_LONG_RIGHT = 'R',
		ACT_NULL_ERROR = '?',
		ACT_NULL_SUBST = '-',
		ACT_NONNULL_SUBST = '+',
		ACT_NULL_ASSIGN = '='
	};
	size_t env_length;
	size_t env_maxlen;
	size_t pat_length;
	size_t pat_maxlen;
	size_t start = *offset;
	char *env;
	char *pattern;
	char *value = NULL;
	enum action action = ACT_NONE;
	int depth = 0;
	int colon_seen = 0;
	int seen_hash = 0;
	int free_value = 0;
	int pattern_is_quoted = 0;	/* 1 for singly-quoted, 2 for doubly-quoted */
	int error;
	int special = 0;
	char buffer[21];
	int brace = words[*offset] == '{';

	env = w_newword(&env_length, &env_maxlen);
	pattern = w_newword(&pat_length, &pat_maxlen);

	if (brace)
		++ * offset;

	/* First collect the parameter name. */

	if (words[*offset] == '#') {
		seen_hash = 1;
		if (!brace)
			goto envsubst;
		++*offset;
	}

	if (isalpha(words[*offset]) || words[*offset] == '_') {
		/* Normal parameter name. */
		do {
			env = w_addchar(env, &env_length, &env_maxlen, words[*offset]);
			if (env == NULL)
				goto no_space;
		}
		while (isalnum(words[++*offset]) || words[*offset] == '_');
	} else if (isdigit(words[*offset])) {
		/* Numeric parameter name. */
		special = 1;
		do {
			env = w_addchar(env, &env_length, &env_maxlen, words[*offset]);
			if (env == NULL)
				goto no_space;
			if (!brace)
				goto envsubst;
		}
		while (isdigit(words[++*offset]));
	} else if (strchr("*@$", words[*offset]) != NULL) {
		/* Special parameter. */
		special = 1;
		env = w_addchar(env, &env_length, &env_maxlen, words[*offset]);
		if (env == NULL)
			goto no_space;
		++*offset;
	} else {
		if (brace)
			goto syntax;
	}

	if (brace) {
		/* Check for special action to be applied to the value. */
		switch (words[*offset]) {
		case '}':
			/* Evaluate. */
			goto envsubst;

		case '#':
			action = ACT_RP_SHORT_LEFT;
			if (words[1 + *offset] == '#') {
				++*offset;
				action = ACT_RP_LONG_LEFT;
			}
			break;

		case '%':
			action = ACT_RP_SHORT_RIGHT;
			if (words[1 + *offset] == '%') {
				++*offset;
				action = ACT_RP_LONG_RIGHT;
			}
			break;

		case ':':
			if (strchr("-=?+", words[1 + *offset]) == NULL)
				goto syntax;

			colon_seen = 1;
			action = words[++*offset];
			break;

		case '-':
		case '=':
		case '?':
		case '+':
			action = words[*offset];
			break;

		default:
			goto syntax;
		}

		/* Now collect the pattern, but don't expand it yet. */
		++*offset;
		for (; words[*offset]; ++(*offset)) {
			switch (words[*offset]) {
			case '{':
				if (!pattern_is_quoted)
					++depth;
				break;

			case '}':
				if (!pattern_is_quoted) {
					if (depth == 0)
						goto envsubst;
					--depth;
				}
				break;

			case '\\':
				if (pattern_is_quoted)
					/* Quoted; treat as normal character. */
					break;

				/* Otherwise, it's an escape: next character is literal. */
				if (words[++*offset] == '\0')
					goto syntax;

				pattern = w_addchar(pattern, &pat_length, &pat_maxlen, '\\');
				if (pattern == NULL)
					goto no_space;

				break;

			case '\'':
				if (pattern_is_quoted == 0)
					pattern_is_quoted = 1;
				else if (pattern_is_quoted == 1)
					pattern_is_quoted = 0;

				break;

			case '"':
				if (pattern_is_quoted == 0)
					pattern_is_quoted = 2;
				else if (pattern_is_quoted == 2)
					pattern_is_quoted = 0;

				break;
			}

			pattern = w_addchar(pattern, &pat_length, &pat_maxlen,
								words[*offset]);
			if (pattern == NULL)
				goto no_space;
		}
	}

	/* End of input string -- remember to reparse the character that we
	 * stopped at.  */
	--(*offset);

  envsubst:
	if (words[start] == '{' && words[*offset] != '}')
		goto syntax;

	if (env == NULL) {
		if (seen_hash) {
			/* $# expands to the number of positional parameters */
			buffer[20] = '\0';
			value = _itoa(__libc_argc - 1, buffer);
			seen_hash = 0;
		} else {
			/* Just $ on its own */
			*offset = start - 1;
			*word = w_addchar(*word, word_length, max_length, '$');
			return *word ? 0 : WRDE_NOSPACE;
		}
	}
	/* Is it a numeric parameter? */
	else if (isdigit(env[0])) {
		int n = atoi(env);

		if (n >= __libc_argc)
			/* Substitute NULL. */
			value = NULL;
		else
			/* Replace with appropriate positional parameter. */
			value = __libc_argv[n];
	}
	/* Is it a special parameter? */
	else if (special) {
		/* Is it `$$'? */
		if (*env == '$') {
			buffer[20] = '\0';
			value = _itoa(getpid(), buffer);
		}
		/* Is it `${#*}' or `${#@}'? */
		else if ((*env == '*' || *env == '@') && seen_hash) {
			buffer[20] = '\0';
			value = _itoa(__libc_argc > 0 ? __libc_argc - 1 : 0,
							   buffer);
			*word = w_addstr(*word, word_length, max_length, value);
			free(env);
			free(pattern);
			return *word ? 0 : WRDE_NOSPACE;
		}
		/* Is it `$*' or `$@' (unquoted) ? */
		else if (*env == '*' || (*env == '@' && !quoted)) {
			size_t plist_len = 0;
			int p;
			char *end;

			/* Build up value parameter by parameter (copy them) */
			for (p = 1; __libc_argv[p]; ++p)
				plist_len += strlen(__libc_argv[p]) + 1;	/* for space */
			value = malloc(plist_len);
			if (value == NULL)
				goto no_space;
			end = value;
			*end = 0;
			for (p = 1; __libc_argv[p]; ++p) {
				if (p > 1)
					*end++ = ' ';
				end = stpcpy(end, __libc_argv[p]);
			}

			free_value = 1;
		} else {
			/* Must be a quoted `$@' */
			assert(*env == '@' && quoted);

			/* Each parameter is a separate word ("$@") */
			if (__libc_argc == 2)
				value = __libc_argv[1];
			else if (__libc_argc > 2) {
				int p;

				/* Append first parameter to current word. */
				value = w_addstr(*word, word_length, max_length,
								 __libc_argv[1]);
				if (value == NULL || w_addword(pwordexp, value))
					goto no_space;

				for (p = 2; __libc_argv[p + 1]; p++) {
					char *newword = strdup(__libc_argv[p]);

					if (newword == NULL || w_addword(pwordexp, newword))
						goto no_space;
				}

				/* Start a new word with the last parameter. */
				*word = w_newword(word_length, max_length);
				value = __libc_argv[p];
			} else {
				free(env);
				free(pattern);
				return 0;
			}
		}
	} else
		value = getenv(env);

	if (value == NULL && (flags & WRDE_UNDEF)) {
		/* Variable not defined. */
		error = WRDE_BADVAL;
		goto do_error;
	}

	if (action != ACT_NONE) {
		int expand_pattern = 0;

		/* First, find out if we need to expand pattern (i.e. if we will
		 * use it). */
		switch (action) {
		case ACT_RP_SHORT_LEFT:
		case ACT_RP_LONG_LEFT:
		case ACT_RP_SHORT_RIGHT:
		case ACT_RP_LONG_RIGHT:
			/* Always expand for these. */
			expand_pattern = 1;
			break;

		case ACT_NULL_ERROR:
		case ACT_NULL_SUBST:
		case ACT_NULL_ASSIGN:
			if (!value || (!*value && colon_seen))
				/* If param is unset, or set but null and a colon has been seen,
				   the expansion of the pattern will be needed. */
				expand_pattern = 1;

			break;

		case ACT_NONNULL_SUBST:
			/* Expansion of word will be needed if parameter is set and not null,
			   or set null but no colon has been seen. */
			if (value && (*value || !colon_seen))
				expand_pattern = 1;

			break;

		default:
			assert(!"Unrecognised action!");
		}

		if (expand_pattern) {
			/* We need to perform tilde expansion, parameter expansion,
			   command substitution, and arithmetic expansion.  We also
			   have to be a bit careful with wildcard characters, as
			   pattern might be given to fnmatch soon.  To do this, we
			   convert quotes to escapes. */

			char *expanded;
			size_t exp_len;
			size_t exp_maxl;
			char *p;
			int quotes = 0;		/* 1: single quotes; 2: double */

			expanded = w_newword(&exp_len, &exp_maxl);
			for (p = pattern; p && *p; p++) {
				size_t _offset;

				switch (*p) {
				case '"':
					if (quotes == 2)
						quotes = 0;
					else if (quotes == 0)
						quotes = 2;
					else
						break;

					continue;

				case '\'':
					if (quotes == 1)
						quotes = 0;
					else if (quotes == 0)
						quotes = 1;
					else
						break;

					continue;

				case '*':
				case '?':
					if (quotes) {
						/* Convert quoted wildchar to escaped wildchar. */
						expanded = w_addchar(expanded, &exp_len,
											 &exp_maxl, '\\');

						if (expanded == NULL)
							goto no_space;
					}
					break;

				case '$':
					_offset = 0;
					error = parse_dollars(&expanded, &exp_len, &exp_maxl, p,
									  &_offset, flags, NULL, NULL, NULL, 1);
					if (error) {
						if (free_value)
							free(value);

						free(expanded);

						goto do_error;
					}

					p += _offset;
					continue;

				case '~':
					if (quotes || exp_len)
						break;

					_offset = 0;
					error = parse_tilde(&expanded, &exp_len, &exp_maxl, p,
										&_offset, 0);
					if (error) {
						if (free_value)
							free(value);

						free(expanded);

						goto do_error;
					}

					p += _offset;
					continue;

				case '\\':
					expanded = w_addchar(expanded, &exp_len, &exp_maxl, '\\');
					++p;
					assert(*p);	/* checked when extracted initially */
					if (expanded == NULL)
						goto no_space;
				}

				expanded = w_addchar(expanded, &exp_len, &exp_maxl, *p);

				if (expanded == NULL)
					goto no_space;
			}

			free(pattern);

			pattern = expanded;
		}

		switch (action) {
		case ACT_RP_SHORT_LEFT:
		case ACT_RP_LONG_LEFT:
		case ACT_RP_SHORT_RIGHT:
		case ACT_RP_LONG_RIGHT:
		{
			char *p;
			char c;
			char *end;

			if (value == NULL || pattern == NULL || *pattern == '\0')
				break;

			end = value + strlen(value);

			switch (action) {
			case ACT_RP_SHORT_LEFT:
				for (p = value; p <= end; ++p) {
					c = *p;
					*p = '\0';
					if (fnmatch(pattern, value, 0) != FNM_NOMATCH) {
						*p = c;
						if (free_value) {
							char *newval = strdup(p);

							if (newval == NULL) {
								free(value);
								goto no_space;
							}
							free(value);
							value = newval;
						} else
							value = p;
						break;
					}
					*p = c;
				}

				break;

			case ACT_RP_LONG_LEFT:
				for (p = end; p >= value; --p) {
					c = *p;
					*p = '\0';
					if (fnmatch(pattern, value, 0) != FNM_NOMATCH) {
						*p = c;
						if (free_value) {
							char *newval = strdup(p);

							if (newval == NULL) {
								free(value);
								goto no_space;
							}
							free(value);
							value = newval;
						} else
							value = p;
						break;
					}
					*p = c;
				}

				break;

			case ACT_RP_SHORT_RIGHT:
				for (p = end; p >= value; --p) {
					if (fnmatch(pattern, p, 0) != FNM_NOMATCH) {
						char *newval;

						newval = malloc(p - value + 1);

						if (newval == NULL) {
							if (free_value)
								free(value);
							goto no_space;
						}

						*(char *) mempcpy(newval, value, p - value) = '\0';
						if (free_value)
							free(value);
						value = newval;
						free_value = 1;
						break;
					}
				}

				break;

			case ACT_RP_LONG_RIGHT:
				for (p = value; p <= end; ++p) {
					if (fnmatch(pattern, p, 0) != FNM_NOMATCH) {
						char *newval;

						newval = malloc(p - value + 1);

						if (newval == NULL) {
							if (free_value)
								free(value);
							goto no_space;
						}

						*(char *) mempcpy(newval, value, p - value) = '\0';
						if (free_value)
							free(value);
						value = newval;
						free_value = 1;
						break;
					}
				}

				break;

			default:
				break;
			}

			break;
		}

		case ACT_NULL_ERROR:
			if (value && *value)
				/* Substitute parameter */
				break;

			error = 0;
			if (!colon_seen && value)
				/* Substitute NULL */
				;
			else if (*pattern)
				fprintf(stderr, "%s: %s\n", env, pattern);
			else {
				fprintf(stderr, "%s: parameter null or not set\n", env);
				error = WRDE_BADVAL;
			}

			if (free_value)
				free(value);
			goto do_error;

		case ACT_NULL_SUBST:
			if (value && *value)
				/* Substitute parameter */
				break;

			if (free_value)
				free(value);

			if (!colon_seen && value)
				/* Substitute NULL */
				goto success;

			value = pattern ? strdup(pattern) : pattern;
			free_value = 1;

			if (pattern && !value)
				goto no_space;

			break;

		case ACT_NONNULL_SUBST:
			if (value && (*value || !colon_seen)) {
				if (free_value)
					free(value);

				value = pattern ? strdup(pattern) : pattern;
				free_value = 1;

				if (pattern && !value)
					goto no_space;

				break;
			}

			/* Substitute NULL */
			if (free_value)
				free(value);
			goto success;

		case ACT_NULL_ASSIGN:
			if (value && *value)
				/* Substitute parameter */
				break;

			if (!colon_seen && value) {
				/* Substitute NULL */
				if (free_value)
					free(value);
				goto success;
			}

			if (free_value)
				free(value);

			value = pattern ? strdup(pattern) : pattern;
			free_value = 1;

			if (pattern && !value)
				goto no_space;

			setenv(env, value, 1);
			break;

		default:
			assert(!"Unrecognised action!");
		}
	}

	free(env);
	env = NULL;
	free(pattern);
	pattern = NULL;

	if (seen_hash) {
		char param_length[21];

		param_length[20] = '\0';
		*word = w_addstr(*word, word_length, max_length,
						 _itoa(value ? strlen(value) : 0,
									param_length));
		if (free_value) {
			assert(value != NULL);
			free(value);
		}

		return *word ? 0 : WRDE_NOSPACE;
	}

	if (value == NULL)
		return 0;

	if (quoted || !pwordexp) {
		/* Quoted - no field split */
		*word = w_addstr(*word, word_length, max_length, value);
		if (free_value)
			free(value);

		return *word ? 0 : WRDE_NOSPACE;
	} else {
		/* Need to field-split */
		char *value_copy = strdup(value);	/* Don't modify value */
		char *field_begin = value_copy;
		int seen_nonws_ifs = 0;

		if (free_value)
			free(value);

		if (value_copy == NULL)
			goto no_space;

		do {
			char *field_end = field_begin;
			char *next_field;

			/* If this isn't the first field, start a new word */
			if (field_begin != value_copy) {
				if (w_addword(pwordexp, *word) == WRDE_NOSPACE) {
					free(value_copy);
					goto no_space;
				}

				*word = w_newword(word_length, max_length);
			}

			/* Skip IFS whitespace before the field */
			field_begin += strspn(field_begin, ifs_white);

			if (!seen_nonws_ifs && *field_begin == 0)
				/* Nothing but whitespace */
				break;

			/* Search for the end of the field */
			field_end = field_begin + strcspn(field_begin, ifs);

			/* Set up pointer to the character after end of field and
			   skip whitespace IFS after it. */
			next_field = field_end + strspn(field_end, ifs_white);

			/* Skip at most one non-whitespace IFS character after the field */
			seen_nonws_ifs = 0;
			if (*next_field && strchr(ifs, *next_field)) {
				seen_nonws_ifs = 1;
				next_field++;
			}

			/* Null-terminate it */
			*field_end = 0;

			/* Tag a copy onto the current word */
			*word = w_addstr(*word, word_length, max_length, field_begin);

			if (*word == NULL && *field_begin != '\0') {
				free(value_copy);
				goto no_space;
			}

			field_begin = next_field;
		}
		while (seen_nonws_ifs || *field_begin);

		free(value_copy);
	}

	return 0;

  success:
	error = 0;
	goto do_error;

  no_space:
	error = WRDE_NOSPACE;
	goto do_error;

  syntax:
	error = WRDE_SYNTAX;

  do_error:
	free(env);

	free(pattern);

	return error;
}
#else
static __inline__ int
parse_backtick(char **word, size_t * word_length, size_t * max_length,
			   const char *words, size_t * offset, int flags,
			   wordexp_t * pwordexp, const char *ifs,
			   const char *ifs_white)
{
	return 0;
}
#endif

static int
parse_dollars(char **word, size_t * word_length, size_t * max_length,
			  const char *words, size_t * offset, int flags,
			  wordexp_t * pwordexp, const char *ifs, const char *ifs_white,
			  int quoted)
{
	/* We are poised _at_ "$" */
	switch (words[1 + *offset]) {
	case '"':
	case '\'':
	case 0:
		*word = w_addchar(*word, word_length, max_length, '$');
		return *word ? 0 : WRDE_NOSPACE;

#ifdef __WORDEXP_FULL
	case '(':
		if (words[2 + *offset] == '(') {
			/* Differentiate between $((1+3)) and $((echo);(ls)) */
			int i = 3 + *offset;
			int depth = 0;

			while (words[i] && !(depth == 0 && words[i] == ')')) {
				if (words[i] == '(')
					++depth;
				else if (words[i] == ')')
					--depth;

				++i;
			}

			if (words[i] == ')' && words[i + 1] == ')') {
				(*offset) += 3;
				/* Call parse_arith -- 0 is for "no brackets" */
				return parse_arith(word, word_length, max_length, words,
								   offset, flags, 0);
			}
		}

		if (flags & WRDE_NOCMD)
			return WRDE_CMDSUB;

		(*offset) += 2;
		return parse_comm(word, word_length, max_length, words, offset,
						  flags, quoted ? NULL : pwordexp, ifs, ifs_white);

	case '[':
		(*offset) += 2;
		/* Call parse_arith -- 1 is for "brackets" */
		return parse_arith(word, word_length, max_length, words, offset,
						   flags, 1);

	case '{':
	default:
		++(*offset);			/* parse_param needs to know if "{" is there */
		return parse_param(word, word_length, max_length, words, offset,
						   flags, pwordexp, ifs, ifs_white, quoted);
#else
	default:
		++(*offset);			/* parse_param needs to know if "{" is there */
		return 0;
#endif
	}
}

static int
parse_dquote(char **word, size_t * word_length, size_t * max_length,
			 const char *words, size_t * offset, int flags,
			 wordexp_t * pwordexp, const char *ifs, const char *ifs_white)
{
	/* We are poised just after a double-quote */
	int error;

	for (; words[*offset]; ++(*offset)) {
		switch (words[*offset]) {
		case '"':
			return 0;

		case '$':
			error = parse_dollars(word, word_length, max_length, words, offset,
							  flags, pwordexp, ifs, ifs_white, 1);
			/* The ``1'' here is to tell parse_dollars not to
			 * split the fields.  It may need to, however ("$@").
			 */
			if (error)
				return error;

			break;

		case '`':
			if (flags & WRDE_NOCMD)
				return WRDE_CMDSUB;

			++(*offset);
			error = parse_backtick(word, word_length, max_length, words,
								   offset, flags, NULL, NULL, NULL);
			/* The first NULL here is to tell parse_backtick not to
			 * split the fields.
			 */
			if (error)
				return error;

			break;

		case '\\':
			error = parse_qtd_backslash(word, word_length, max_length, words,
									offset);

			if (error)
				return error;

			break;

		default:
			*word = w_addchar(*word, word_length, max_length, words[*offset]);
			if (*word == NULL)
				return WRDE_NOSPACE;
		}
	}

	/* Unterminated string */
	return WRDE_SYNTAX;
}

/*
 * wordfree() is to be called after pwordexp is finished with.
 */

void wordfree(wordexp_t * pwordexp)
{

	/* wordexp can set pwordexp to NULL */
	if (pwordexp && pwordexp->we_wordv) {
		char **wordv = pwordexp->we_wordv;

		for (wordv += pwordexp->we_offs; *wordv; ++wordv)
			free(*wordv);

		free(pwordexp->we_wordv);
		pwordexp->we_wordv = NULL;
	}
}
libc_hidden_def(wordfree)

/*
 * wordexp()
 */

int wordexp(const char *words, wordexp_t * we, int flags)
{
	size_t words_offset;
	size_t word_length;
	size_t max_length;
	char *word = w_newword(&word_length, &max_length);
	int error;
	char *ifs;
	char ifs_white[4];
	wordexp_t old_word = *we;

	if (flags & WRDE_REUSE) {
		/* Minimal implementation of WRDE_REUSE for now */
		wordfree(we);
		old_word.we_wordv = NULL;
	}

	if ((flags & WRDE_APPEND) == 0) {
		we->we_wordc = 0;

		if (flags & WRDE_DOOFFS) {
			we->we_wordv = calloc(1 + we->we_offs, sizeof(char *));
			if (we->we_wordv == NULL) {
				error = WRDE_NOSPACE;
				goto do_error;
			}
		} else {
			we->we_wordv = calloc(1, sizeof(char *));
			if (we->we_wordv == NULL) {
				error = WRDE_NOSPACE;
				goto do_error;
			}

			we->we_offs = 0;
		}
	}

	/* Find out what the field separators are.
	 * There are two types: whitespace and non-whitespace.
	 */
	ifs = getenv("IFS");

	if (!ifs)
		/* IFS unset - use <space><tab><newline>. */
		ifs = strcpy(ifs_white, " \t\n");
	else {
		char *ifsch = ifs;
		char *whch = ifs_white;

		/* Start off with no whitespace IFS characters */
		ifs_white[0] = '\0';

		while (*ifsch != '\0') {
			if ((*ifsch == ' ') || (*ifsch == '\t') || (*ifsch == '\n')) {
				/* Whitespace IFS.  See first whether it is already in our
				   collection.  */
				char *runp = ifs_white;

				while (runp < whch && *runp != '\0' && *runp != *ifsch)
					++runp;

				if (runp == whch)
					*whch++ = *ifsch;
			}

			++ifsch;
		}
		*whch = '\0';
	}

	for (words_offset = 0; words[words_offset]; ++words_offset)
		switch (words[words_offset]) {
		case '\\':
			error = parse_backslash(&word, &word_length, &max_length, words,
								&words_offset);

			if (error)
				goto do_error;

			break;

		case '$':
			error = parse_dollars(&word, &word_length, &max_length, words,
								  &words_offset, flags, we, ifs, ifs_white,
								  0);

			if (error)
				goto do_error;

			break;

		case '`':
			if (flags & WRDE_NOCMD) {
				error = WRDE_CMDSUB;
				goto do_error;
			}

			++words_offset;
			error = parse_backtick(&word, &word_length, &max_length, words,
								   &words_offset, flags, we, ifs,
								   ifs_white);

			if (error)
				goto do_error;

			break;

		case '"':
			++words_offset;
			error = parse_dquote(&word, &word_length, &max_length, words,
								 &words_offset, flags, we, ifs, ifs_white);

			if (error)
				goto do_error;

			if (!word_length) {
				error = w_addword(we, NULL);

				if (error)
					return error;
			}

			break;

		case '\'':
			++words_offset;
			error = parse_squote(&word, &word_length, &max_length, words,
								 &words_offset);

			if (error)
				goto do_error;

			if (!word_length) {
				error = w_addword(we, NULL);

				if (error)
					return error;
			}

			break;

		case '~':
			error = parse_tilde(&word, &word_length, &max_length, words,
								&words_offset, we->we_wordc);

			if (error)
				goto do_error;

			break;

		case '*':
		case '[':
		case '?':
			error = parse_glob(&word, &word_length, &max_length, words,
							   &words_offset, flags, we, ifs, ifs_white);

			if (error)
				goto do_error;

			break;

		default:
			/* Is it a word separator? */
			if (strchr(" \t", words[words_offset]) == NULL) {
				char ch = words[words_offset];

				/* Not a word separator -- but is it a valid word char? */
				if (strchr("\n|&;<>(){}", ch)) {
					/* Fail */
					error = WRDE_BADCHAR;
					goto do_error;
				}

				/* "Ordinary" character -- add it to word */
				word = w_addchar(word, &word_length, &max_length, ch);
				if (word == NULL) {
					error = WRDE_NOSPACE;
					goto do_error;
				}

				break;
			}

			/* If a word has been delimited, add it to the list. */
			if (word != NULL) {
				error = w_addword(we, word);
				if (error)
					goto do_error;
			}

			word = w_newword(&word_length, &max_length);
		}

	/* End of string */

	/* There was a word separator at the end */
	if (word == NULL)			/* i.e. w_newword */
		return 0;

	/* There was no field separator at the end */
	return w_addword(we, word);

  do_error:
	/* Error:
	 *  free memory used (unless error is WRDE_NOSPACE), and
	 *  set we members back to what they were.
	 */

	free(word);

	if (error == WRDE_NOSPACE)
		return WRDE_NOSPACE;

	if ((flags & WRDE_APPEND) == 0)
		wordfree(we);

	*we = old_word;
	return error;
}
