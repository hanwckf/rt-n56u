/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2000-2005 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Lesser General Public License version 2.1 or later.
 */

/*
 * Environment variable to be removed for SUID programs.  The names are all
 * stuffed in a single string which means they have to be terminated with a
 * '\0' explicitly.
 */

#define UNSECURE_ENVVARS \
	"LD_PRELOAD\0" \
	"LD_LIBRARY_PATH\0" \
	"LD_DEBUG\0" \
	"LD_DEBUG_OUTPUT\0" \
	"LD_TRACE_LOADED_OBJECTS\0" \
	"TMPDIR\0"

/*
 * LD_TRACE_LOADED_OBJECTS is not in glibc-2.3.5's unsecvars.h
 * though used by ldd
 *
 * These environment variables are defined by glibc but ignored in
 * uClibc, but may very well have an equivalent in uClibc.
 *
 * LD_ORIGIN_PATH, LD_PROFILE, LD_USE_LOAD_BIAS, LD_DYNAMIC_WEAK, LD_SHOW_AUXV,
 * GCONV_PATH, GETCONF_DIR, HOSTALIASES, LOCALDOMAIN, LOCPATH, MALLOC_TRACE,
 * NLSPATH, RESOLV_HOST_CONF, RES_OPTIONS, TZDIR
 */
