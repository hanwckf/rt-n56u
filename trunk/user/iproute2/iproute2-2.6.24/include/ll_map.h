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
#ifndef __LL_MAP_H__
#define __LL_MAP_H__ 1

extern int ll_remember_index(const struct sockaddr_nl *who,
			     struct nlmsghdr *n, void *arg);
extern int ll_init_map(struct rtnl_handle *rth);
extern unsigned ll_name_to_index(const char *name);
extern const char *ll_index_to_name(unsigned idx);
extern const char *ll_idx_n2a(unsigned idx, char *buf);
extern int ll_index_to_type(unsigned idx);
extern unsigned ll_index_to_flags(unsigned idx);

#endif /* __LL_MAP_H__ */
