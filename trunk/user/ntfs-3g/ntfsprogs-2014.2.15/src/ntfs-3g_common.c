/**
 * ntfs-3g_common.c - Common definitions for ntfs-3g and lowntfs-3g.
 *
 * Copyright (c) 2010-2012 Jean-Pierre Andre
 * Copyright (c) 2010      Erik Larsson
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the NTFS-3G
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include <getopt.h>
#include <fuse.h>

#include "inode.h"
#include "security.h"
#include "xattrs.h"
#include "ntfs-3g_common.h"
#include "realpath.h"
#include "misc.h"

const char xattr_ntfs_3g[] = "ntfs-3g.";

const char nf_ns_user_prefix[] = "user.";
const int nf_ns_user_prefix_len = sizeof(nf_ns_user_prefix) - 1;
const char nf_ns_system_prefix[] = "system.";
const int nf_ns_system_prefix_len = sizeof(nf_ns_system_prefix) - 1;
const char nf_ns_security_prefix[] = "security.";
const int nf_ns_security_prefix_len = sizeof(nf_ns_security_prefix) - 1;
const char nf_ns_trusted_prefix[] = "trusted.";
const int nf_ns_trusted_prefix_len = sizeof(nf_ns_trusted_prefix) - 1;

static const char nf_ns_alt_xattr_efsinfo[] = "user.ntfs.efsinfo";

static const char def_opts[] = "allow_other,nonempty,";

	/*
	 *	 Table of recognized options
	 * Their order may be significant
	 * The options invalid in some configuration should still
	 * be present, so that an error can be returned
	 */
const struct DEFOPTION optionlist[] = {
	{ "ro", OPT_RO, FLGOPT_APPEND | FLGOPT_BOGUS },
	{ "noatime", OPT_NOATIME, FLGOPT_BOGUS },
	{ "atime", OPT_ATIME, FLGOPT_BOGUS },
	{ "relatime", OPT_RELATIME, FLGOPT_BOGUS },
	{ "delay_mtime", OPT_DMTIME, FLGOPT_DECIMAL | FLGOPT_OPTIONAL },
	{ "fake_rw", OPT_FAKE_RW, FLGOPT_BOGUS },
	{ "fsname", OPT_FSNAME, FLGOPT_NOSUPPORT },
	{ "no_def_opts", OPT_NO_DEF_OPTS, FLGOPT_BOGUS },
	{ "default_permissions", OPT_DEFAULT_PERMISSIONS, FLGOPT_BOGUS },
	{ "permissions", OPT_PERMISSIONS, FLGOPT_BOGUS },
	{ "acl", OPT_ACL, FLGOPT_BOGUS },
	{ "umask", OPT_UMASK, FLGOPT_OCTAL },
	{ "fmask", OPT_FMASK, FLGOPT_OCTAL },
	{ "dmask", OPT_DMASK, FLGOPT_OCTAL },
	{ "uid", OPT_UID, FLGOPT_DECIMAL },
	{ "gid", OPT_GID, FLGOPT_DECIMAL },
	{ "show_sys_files", OPT_SHOW_SYS_FILES, FLGOPT_BOGUS },
	{ "hide_hid_files", OPT_HIDE_HID_FILES, FLGOPT_BOGUS },
	{ "hide_dot_files", OPT_HIDE_DOT_FILES, FLGOPT_BOGUS },
	{ "ignore_case", OPT_IGNORE_CASE, FLGOPT_BOGUS },
	{ "windows_names", OPT_WINDOWS_NAMES, FLGOPT_BOGUS },
	{ "compression", OPT_COMPRESSION, FLGOPT_BOGUS },
	{ "nocompression", OPT_NOCOMPRESSION, FLGOPT_BOGUS },
	{ "silent", OPT_SILENT, FLGOPT_BOGUS },
	{ "recover", OPT_RECOVER, FLGOPT_BOGUS },
	{ "norecover", OPT_NORECOVER, FLGOPT_BOGUS },
	{ "remove_hiberfile", OPT_REMOVE_HIBERFILE, FLGOPT_BOGUS },
	{ "sync", OPT_SYNC, FLGOPT_BOGUS | FLGOPT_APPEND },
	{ "big_writes", OPT_BIG_WRITES, FLGOPT_BOGUS },
	{ "locale", OPT_LOCALE, FLGOPT_STRING },
	{ "nfconv", OPT_NFCONV, FLGOPT_BOGUS },
	{ "nonfconv", OPT_NONFCONV, FLGOPT_BOGUS },
	{ "streams_interface", OPT_STREAMS_INTERFACE, FLGOPT_STRING },
	{ "user_xattr", OPT_USER_XATTR, FLGOPT_BOGUS },
	{ "noauto", OPT_NOAUTO, FLGOPT_BOGUS },
	{ "debug", OPT_DEBUG, FLGOPT_BOGUS },
	{ "no_detach", OPT_NO_DETACH, FLGOPT_BOGUS },
	{ "remount", OPT_REMOUNT, FLGOPT_BOGUS },
	{ "blksize", OPT_BLKSIZE, FLGOPT_STRING },
	{ "inherit", OPT_INHERIT, FLGOPT_BOGUS },
	{ "addsecurids", OPT_ADDSECURIDS, FLGOPT_BOGUS },
	{ "staticgrps", OPT_STATICGRPS, FLGOPT_BOGUS },
	{ "usermapping", OPT_USERMAPPING, FLGOPT_STRING },
	{ "xattrmapping", OPT_XATTRMAPPING, FLGOPT_STRING },
	{ "efs_raw", OPT_EFS_RAW, FLGOPT_BOGUS },
	{ (const char*)NULL, 0, 0 } /* end marker */
} ;

#define STRAPPEND_MAX_INSIZE   8192
#define strappend_is_large(x) ((x) > STRAPPEND_MAX_INSIZE)

int ntfs_strappend(char **dest, const char *append)
{
	char *p;
	size_t size_append, size_dest = 0;
	
	if (!dest)
		return -1;
	if (!append)
		return 0;

	size_append = strlen(append);
	if (*dest)
		size_dest = strlen(*dest);
	
	if (strappend_is_large(size_dest) || strappend_is_large(size_append)) {
		errno = EOVERFLOW;
		ntfs_log_perror("%s: Too large input buffer", EXEC_NAME);
		return -1;
	}
	
	p = (char*)realloc(*dest, size_dest + size_append + 1);
    	if (!p) {
		ntfs_log_perror("%s: Memory realloction failed", EXEC_NAME);
		return -1;
	}
	
	*dest = p;
	strcpy(*dest + size_dest, append);
	
	return 0;
}

/*
 *		Insert an option before ",fsname="
 *	This is for keeping "fsname" as the last option, because on
 *	Solaris device names may contain commas.
 */

int ntfs_strinsert(char **dest, const char *append)
{
	char *p, *q;
	size_t size_append, size_dest = 0;
	
	if (!dest)
		return -1;
	if (!append)
		return 0;

	size_append = strlen(append);
	if (*dest)
		size_dest = strlen(*dest);
	
	if (strappend_is_large(size_dest) || strappend_is_large(size_append)) {
		errno = EOVERFLOW;
		ntfs_log_perror("%s: Too large input buffer", EXEC_NAME);
		return -1;
	}
	
	p = (char*)malloc(size_dest + size_append + 1);
	if (!p) {
		ntfs_log_perror("%s: Memory reallocation failed", EXEC_NAME);
		return -1;
	}
	strcpy(p, *dest);
	q = strstr(p, ",fsname=");
	if (q) {
		strcpy(q, append);
		q = strstr(*dest, ",fsname=");
		if (q)
			strcat(p, q);
		free(*dest);
		*dest = p;
	} else {
		free(*dest);
		*dest = p;
		strcpy(*dest + size_dest, append);
	}
	return 0;
}

static int bogus_option_value(char *val, const char *s)
{
	if (val) {
		ntfs_log_error("'%s' option shouldn't have value.\n", s);
		return -1;
	}
	return 0;
}

static int missing_option_value(char *val, const char *s)
{
	if (!val) {
		ntfs_log_error("'%s' option should have a value.\n", s);
		return -1;
	}
	return 0;
}

char *parse_mount_options(ntfs_fuse_context_t *ctx,
			const struct ntfs_options *popts, BOOL low_fuse)
{
	char *options, *s, *opt, *val, *ret = NULL;
	const char *orig_opts = popts->options;
	BOOL no_def_opts = FALSE;
	int default_permissions = 0;
	int permissions = 0;
	int acl = 0;
	int want_permissions = 0;
	int intarg;
	const struct DEFOPTION *poptl;

	ctx->secure_flags = 0;
#ifdef HAVE_SETXATTR	/* extended attributes interface required */
	ctx->efs_raw = FALSE;
#endif /* HAVE_SETXATTR */
	ctx->compression = DEFAULT_COMPRESSION;
	options = strdup(orig_opts ? orig_opts : "");
	if (!options) {
		ntfs_log_perror("%s: strdup failed", EXEC_NAME);
		return NULL;
	}
	
	s = options;
	while (s && *s && (val = strsep(&s, ","))) {
		opt = strsep(&val, "=");
		poptl = optionlist;
		while (poptl->name && strcmp(poptl->name,opt))
			poptl++;
		if (poptl->name) {
			if ((poptl->flags & FLGOPT_BOGUS)
			    && bogus_option_value(val, opt))
				goto err_exit;
			if ((poptl->flags & FLGOPT_OCTAL)
			    && (!val
				|| !sscanf(val, "%o", &intarg))) {
				ntfs_log_error("'%s' option needs an octal value\n",
					opt);
				goto err_exit;
			}
			if (poptl->flags & FLGOPT_DECIMAL) {
				if ((poptl->flags & FLGOPT_OPTIONAL) && !val)
					intarg = 0;
				else
					if (!val
					    || !sscanf(val, "%i", &intarg)) {
						ntfs_log_error("'%s' option "
						     "needs a decimal value\n",
							opt);
						goto err_exit;
					}
			}
			if ((poptl->flags & FLGOPT_STRING)
			    && missing_option_value(val, opt))
				goto err_exit;

			switch (poptl->type) {
			case OPT_RO :
			case OPT_FAKE_RW :
				ctx->ro = TRUE;
				break;
			case OPT_NOATIME :
				ctx->atime = ATIME_DISABLED;
				break;
			case OPT_ATIME :
				ctx->atime = ATIME_ENABLED;
				break;
			case OPT_RELATIME :
				ctx->atime = ATIME_RELATIVE;
				break;
			case OPT_DMTIME :
				if (!intarg)
					intarg = DEFAULT_DMTIME;
				ctx->dmtime = intarg*10000000LL;
				break;
			case OPT_NO_DEF_OPTS :
				no_def_opts = TRUE; /* Don't add default options. */
				ctx->silent = FALSE; /* cancel default silent */
				break;
			case OPT_DEFAULT_PERMISSIONS :
				default_permissions = 1;
				break;
			case OPT_PERMISSIONS :
				permissions = 1;
				break;
#if POSIXACLS
			case OPT_ACL :
				acl = 1;
				break;
#endif
			case OPT_UMASK :
				ctx->dmask = ctx->fmask = intarg;
				want_permissions = 1;
				break;
			case OPT_FMASK :
				ctx->fmask = intarg;
			       	want_permissions = 1;
				break;
			case OPT_DMASK :
				ctx->dmask = intarg;
			       	want_permissions = 1;
				break;
			case OPT_UID :
				ctx->uid = intarg;
			       	want_permissions = 1;
				break;
			case OPT_GID :
				ctx->gid = intarg;
				want_permissions = 1;
				break;
			case OPT_SHOW_SYS_FILES :
				ctx->show_sys_files = TRUE;
				break;
			case OPT_HIDE_HID_FILES :
				ctx->hide_hid_files = TRUE;
				break;
			case OPT_HIDE_DOT_FILES :
				ctx->hide_dot_files = TRUE;
				break;
			case OPT_WINDOWS_NAMES :
				ctx->windows_names = TRUE;
				break;
			case OPT_IGNORE_CASE :
				if (low_fuse)
					ctx->ignore_case = TRUE;
				else {
					ntfs_log_error("'%s' is an unsupported option.\n",
						poptl->name);
					goto err_exit;
				}
				break;
			case OPT_COMPRESSION :
				ctx->compression = TRUE;
				break;
			case OPT_NOCOMPRESSION :
				ctx->compression = FALSE;
				break;
			case OPT_SILENT :
				ctx->silent = TRUE;
				break;
			case OPT_RECOVER :
				ctx->recover = TRUE;
				break;
			case OPT_NORECOVER :
				ctx->recover = FALSE;
				break;
			case OPT_REMOVE_HIBERFILE :
				ctx->hiberfile = TRUE;
				break;
			case OPT_SYNC :
				ctx->sync = TRUE;
				break;
#ifdef FUSE_CAP_BIG_WRITES
			case OPT_BIG_WRITES :
				ctx->big_writes = TRUE;
				break;
#endif
			case OPT_LOCALE :
				ntfs_set_char_encoding(val);
				break;
#if defined(__APPLE__) || defined(__DARWIN__)
#ifdef ENABLE_NFCONV
			case OPT_NFCONV :
				if (ntfs_macosx_normalize_filenames(1)) {
					ntfs_log_error("ntfs_macosx_normalize_filenames(1) failed!\n");
					goto err_exit;
				}
				break;
			case OPT_NONFCONV :
				if (ntfs_macosx_normalize_filenames(0)) {
					ntfs_log_error("ntfs_macosx_normalize_filenames(0) failed!\n");
					goto err_exit;
				}
				break;
#endif /* ENABLE_NFCONV */
#endif /* defined(__APPLE__) || defined(__DARWIN__) */
			case OPT_STREAMS_INTERFACE :
				if (!strcmp(val, "none"))
					ctx->streams = NF_STREAMS_INTERFACE_NONE;
				else if (!strcmp(val, "xattr"))
					ctx->streams = NF_STREAMS_INTERFACE_XATTR;
				else if (!strcmp(val, "openxattr"))
					ctx->streams = NF_STREAMS_INTERFACE_OPENXATTR;
				else if (!low_fuse && !strcmp(val, "windows"))
					ctx->streams = NF_STREAMS_INTERFACE_WINDOWS;
				else {
					ntfs_log_error("Invalid named data streams "
						"access interface.\n");
					goto err_exit;
				}
				break;
			case OPT_USER_XATTR :
				ctx->streams = NF_STREAMS_INTERFACE_XATTR;
				break;
			case OPT_NOAUTO :
				/* Don't pass noauto option to fuse. */
				break;
			case OPT_DEBUG :
				ctx->debug = TRUE;
				ntfs_log_set_levels(NTFS_LOG_LEVEL_DEBUG);
				ntfs_log_set_levels(NTFS_LOG_LEVEL_TRACE);
				break;
			case OPT_NO_DETACH :
				ctx->no_detach = TRUE;
				break;
			case OPT_REMOUNT :
				ntfs_log_error("Remounting is not supported at present."
					" You have to umount volume and then "
					"mount it once again.\n");
				goto err_exit;
			case OPT_BLKSIZE :
				ntfs_log_info("WARNING: blksize option is ignored "
				      "because ntfs-3g must calculate it.\n");
				break;
			case OPT_INHERIT :
				/*
				 * do not overwrite inherited permissions
				 * in create()
				 */
				ctx->inherit = TRUE;
				break;
			case OPT_ADDSECURIDS :
				/*
				 * create security ids for files being read
				 * with an individual security attribute
				 */
				ctx->secure_flags |= (1 << SECURITY_ADDSECURIDS);
				break;
			case OPT_STATICGRPS :
				/*
				 * use static definition of groups
				 * for file access control
				 */
				ctx->secure_flags |= (1 << SECURITY_STATICGRPS);
				break;
			case OPT_USERMAPPING :
				ctx->usermap_path = strdup(val);
				if (!ctx->usermap_path) {
					ntfs_log_error("no more memory to store "
						"'usermapping' option.\n");
					goto err_exit;
				}
				break;
#ifdef HAVE_SETXATTR	/* extended attributes interface required */
#ifdef XATTR_MAPPINGS
			case OPT_XATTRMAPPING :
				ctx->xattrmap_path = strdup(val);
				if (!ctx->xattrmap_path) {
					ntfs_log_error("no more memory to store "
						"'xattrmapping' option.\n");
					goto err_exit;
				}
				break;
#endif /* XATTR_MAPPINGS */
			case OPT_EFS_RAW :
				ctx->efs_raw = TRUE;
				break;
#endif /* HAVE_SETXATTR */
			case OPT_FSNAME : /* Filesystem name. */
			/*
			 * We need this to be able to check whether filesystem
			 * mounted or not.
			 *      (falling through to default)
			 */
			default :
				ntfs_log_error("'%s' is an unsupported option.\n",
					poptl->name);
				goto err_exit;
			}
			if ((poptl->flags & FLGOPT_APPEND)
			    && (ntfs_strappend(&ret, poptl->name)
				    || ntfs_strappend(&ret, ",")))
				goto err_exit;
		} else { /* Probably FUSE option. */
			if (ntfs_strappend(&ret, opt))
				goto err_exit;
			if (val) {
				if (ntfs_strappend(&ret, "="))
					goto err_exit;
				if (ntfs_strappend(&ret, val))
					goto err_exit;
			}
			if (ntfs_strappend(&ret, ","))
				goto err_exit;
		}
	}
	if (!no_def_opts && ntfs_strappend(&ret, def_opts))
		goto err_exit;
	if ((default_permissions || (permissions && !acl))
			&& ntfs_strappend(&ret, "default_permissions,"))
		goto err_exit;
			/* The atime options exclude each other */
	if (ctx->atime == ATIME_RELATIVE && ntfs_strappend(&ret, "relatime,"))
		goto err_exit;
	else if (ctx->atime == ATIME_ENABLED && ntfs_strappend(&ret, "atime,"))
		goto err_exit;
	else if (ctx->atime == ATIME_DISABLED && ntfs_strappend(&ret, "noatime,"))
		goto err_exit;
	
	if (ntfs_strappend(&ret, "fsname="))
		goto err_exit;
	if (ntfs_strappend(&ret, popts->device))
		goto err_exit;
	if (permissions && !acl)
		ctx->secure_flags |= (1 << SECURITY_DEFAULT);
	if (acl)
		ctx->secure_flags |= (1 << SECURITY_ACL);
	if (want_permissions)
		ctx->secure_flags |= (1 << SECURITY_WANTED);
	if (ctx->ro)
		ctx->secure_flags &= ~(1 << SECURITY_ADDSECURIDS);
exit:
	free(options);
	return ret;
err_exit:
	free(ret);
	ret = NULL;
	goto exit;
}

/**
 * parse_options - Read and validate the programs command line
 * Read the command line, verify the syntax and parse the options.
 *
 * Return:   0 success, -1 error.
 */
int ntfs_parse_options(struct ntfs_options *popts, void (*usage)(void),
			int argc, char *argv[])
{
	int c;

	static const char *sopt = "-o:hnvV";
	static const struct option lopt[] = {
		{ "options",	 required_argument,	NULL, 'o' },
		{ "help",	 no_argument,		NULL, 'h' },
		{ "no-mtab",	 no_argument,		NULL, 'n' },
		{ "verbose",	 no_argument,		NULL, 'v' },
		{ "version",	 no_argument,		NULL, 'V' },
		{ NULL,		 0,			NULL,  0  }
	};

	opterr = 0; /* We'll handle the errors, thank you. */

	while ((c = getopt_long(argc, argv, sopt, lopt, NULL)) != -1) {
		switch (c) {
		case 1:	/* A non-option argument */
			if (!popts->device) {
				popts->device = ntfs_malloc(PATH_MAX + 1);
				if (!popts->device)
					return -1;
				
				/* Canonicalize device name (mtab, etc) */
				popts->arg_device = optarg;
				if (!ntfs_realpath_canonicalize(optarg,
						popts->device)) {
					ntfs_log_perror("%s: Failed to access "
					     "volume '%s'", EXEC_NAME, optarg);
					free(popts->device);
					popts->device = NULL;
					return -1;
				}
			} else if (!popts->mnt_point) {
				popts->mnt_point = optarg;
			} else {
				ntfs_log_error("%s: You must specify exactly one "
						"device and exactly one mount "
						"point.\n", EXEC_NAME);
				return -1;
			}
			break;
		case 'o':
			if (popts->options)
				if (ntfs_strappend(&popts->options, ","))
					return -1;
			if (ntfs_strappend(&popts->options, optarg))
				return -1;
			break;
		case 'h':
			usage();
			exit(9);
		case 'n':
			/*
			 * no effect - automount passes it, meaning 'no-mtab'
			 */
			break;
		case 'v':
			/*
			 * We must handle the 'verbose' option even if
			 * we don't use it because mount(8) passes it.
			 */
			break;
		case 'V':
			ntfs_log_info("%s %s %s %d\n", EXEC_NAME, VERSION, 
				      FUSE_TYPE, fuse_version());
			exit(0);
		default:
			ntfs_log_error("%s: Unknown option '%s'.\n", EXEC_NAME,
				       argv[optind - 1]);
			return -1;
		}
	}

	if (!popts->device) {
		ntfs_log_error("%s: No device is specified.\n", EXEC_NAME);
		return -1;
	}
	if (!popts->mnt_point) {
		ntfs_log_error("%s: No mountpoint is specified.\n", EXEC_NAME);
		return -1;
	}

	return 0;
}

#ifdef HAVE_SETXATTR

int ntfs_fuse_listxattr_common(ntfs_inode *ni, ntfs_attr_search_ctx *actx,
			char *list, size_t size, BOOL prefixing)
{
	int ret = 0;
	char *to = list;
#ifdef XATTR_MAPPINGS
	BOOL accepted;
	const struct XATTRMAPPING *item;
#endif /* XATTR_MAPPINGS */

		/* first list the regular user attributes (ADS) */
	while (!ntfs_attr_lookup(AT_DATA, NULL, 0, CASE_SENSITIVE,
				0, NULL, 0, actx)) {
		char *tmp_name = NULL;
		int tmp_name_len;

		if (!actx->attr->name_length)
			continue;
		tmp_name_len = ntfs_ucstombs(
			(ntfschar *)((u8*)actx->attr +
				le16_to_cpu(actx->attr->name_offset)),
			actx->attr->name_length, &tmp_name, 0);
		if (tmp_name_len < 0) {
			ret = -errno;
			goto exit;
		}
				/*
				 * When using name spaces, do not return
				 * security, trusted or system attributes
				 * (filtered elsewhere anyway)
				 * otherwise insert "user." prefix
				 */
		if (prefixing) {
			if ((strlen(tmp_name) > sizeof(xattr_ntfs_3g))
			  && !strncmp(tmp_name,xattr_ntfs_3g,
				sizeof(xattr_ntfs_3g)-1))
				tmp_name_len = 0;
			else
				ret += tmp_name_len
					 + nf_ns_user_prefix_len + 1;
		} else
			ret += tmp_name_len + 1;
		if (size && tmp_name_len) {
			if ((size_t)ret <= size) {
				if (prefixing) {
					strcpy(to, nf_ns_user_prefix);
					to += nf_ns_user_prefix_len;
				}
				strncpy(to, tmp_name, tmp_name_len);
				to += tmp_name_len;
				*to = 0;
				to++;
			} else {
				free(tmp_name);
				ret = -ERANGE;
				goto exit;
			}
		}
		free(tmp_name);
	}
#ifdef XATTR_MAPPINGS
		/* now append the system attributes mapped to user space */
	for (item=ni->vol->xattr_mapping; item; item=item->next) {
		switch (item->xattr) {
		case XATTR_NTFS_EFSINFO :
			accepted = ni->vol->efs_raw
				&& (ni->flags & FILE_ATTR_ENCRYPTED);
			break;
		case XATTR_NTFS_REPARSE_DATA :
			accepted = (ni->flags & FILE_ATTR_REPARSE_POINT)
					!= const_cpu_to_le32(0);
			break;
// TODO : we are supposed to only return xattrs which are set
// this is more complex for OBJECT_ID and DOS_NAME
		default : accepted = TRUE;
			break;
		}
		if (accepted) {
			ret += strlen(item->name) + 1;
			if (size) {
				if ((size_t)ret <= size) {
					strcpy(to, item->name);
					to += strlen(item->name);
					*to++ = 0;
				} else {
					ret = -ERANGE;
					goto exit;
				}
			}
#else /* XATTR_MAPPINGS */
		/* List efs info xattr for encrypted files */
	if (ni->vol->efs_raw && (ni->flags & FILE_ATTR_ENCRYPTED)) {
		ret += sizeof(nf_ns_alt_xattr_efsinfo);
		if ((size_t)ret <= size) {
			memcpy(to, nf_ns_alt_xattr_efsinfo,
				sizeof(nf_ns_alt_xattr_efsinfo));
			to += sizeof(nf_ns_alt_xattr_efsinfo);
#endif /* XATTR_MAPPINGS */
		}
	}
exit :
	return (ret);
}

#endif /* HAVE_SETXATTR */
