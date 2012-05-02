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
#ifndef _TC_UTIL_H_
#define _TC_UTIL_H_ 1

#include "utils.h"
#include "linux/genetlink.h"

struct genl_util
{
	struct  genl_util *next;
	char	name[16];
	int	(*parse_genlopt)(struct genl_util *fu, int argc, char **argv);
	int	(*print_genlopt)(const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg);
};

extern int genl_ctrl_resolve_family(const char *family);

#endif
