#ifndef _LINUX_MOUNT__CONFIG_H
#define _LINUX_MOUNT_CONFIG__H
/*
 * mount_config.h -- mount configuration file routines 
 * Copyright (C) 2008 Red Hat, Inc <nfs@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

inline void mount_config_init(char *);

#ifdef MOUNT_CONFIG
#include "conffile.h"
#include "xlog.h"

extern char *conf_get_mntopts(char *, char *, char *);

inline void mount_config_init(char *program)
{
	xlog_open(program);
	/*
	 * Read the the default mount options
	 */
	conf_init();
}
inline char *mount_config_opts(char *spec, 
		char *mount_point, char *mount_opts)
{
	return conf_get_mntopts(spec, mount_point, mount_opts);
}
#else /* MOUNT_CONFIG */

inline void mount_config_init(char *program) { }

inline char *mount_config_opts(char *spec, 
		char *mount_point, char *mount_opts)
{
	return mount_opts;
}
#endif /* MOUNT_CONFIG */
#endif
