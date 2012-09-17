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
#ifndef _IP6T_POLICY_H
#define _IP6T_POLICY_H

#define IP6T_POLICY_MAX_ELEM	4

enum ip6t_policy_flags
{
	IP6T_POLICY_MATCH_IN		= 0x1,
	IP6T_POLICY_MATCH_OUT		= 0x2,
	IP6T_POLICY_MATCH_NONE		= 0x4,
	IP6T_POLICY_MATCH_STRICT	= 0x8,
};

enum ip6t_policy_modes
{
	IP6T_POLICY_MODE_TRANSPORT,
	IP6T_POLICY_MODE_TUNNEL
};

struct ip6t_policy_spec
{
	u_int8_t	saddr:1,
			daddr:1,
			proto:1,
			mode:1,
			spi:1,
			reqid:1;
};

union ip6t_policy_addr
{
	struct in_addr	a4;
	struct in6_addr	a6;
};

struct ip6t_policy_elem
{
	union ip6t_policy_addr	saddr;
	union ip6t_policy_addr	smask;
	union ip6t_policy_addr	daddr;
	union ip6t_policy_addr	dmask;
	u_int32_t		spi;
	u_int32_t		reqid;
	u_int8_t		proto;
	u_int8_t		mode;

	struct ip6t_policy_spec	match;
	struct ip6t_policy_spec	invert;
};

struct ip6t_policy_info
{
	struct ip6t_policy_elem pol[IP6T_POLICY_MAX_ELEM];
	u_int16_t flags;
	u_int16_t len;
};

#endif /* _IP6T_POLICY_H */
