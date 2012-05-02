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
#ifndef _IPT_CLUSTERIP_H_target
#define _IPT_CLUSTERIP_H_target

enum clusterip_hashmode {
    CLUSTERIP_HASHMODE_SIP = 0,
    CLUSTERIP_HASHMODE_SIP_SPT,
    CLUSTERIP_HASHMODE_SIP_SPT_DPT,
};

#define CLUSTERIP_HASHMODE_MAX CLUSTERIP_HASHMODE_SIP_SPT_DPT

#define CLUSTERIP_MAX_NODES 16

#define CLUSTERIP_FLAG_NEW 0x00000001

struct clusterip_config;

struct ipt_clusterip_tgt_info {

	u_int32_t flags;
	
	/* only relevant for new ones */
	u_int8_t clustermac[6];
	u_int16_t num_total_nodes;
	u_int16_t num_local_nodes;
	u_int16_t local_nodes[CLUSTERIP_MAX_NODES];
	enum clusterip_hashmode hash_mode;
	u_int32_t hash_initval;
	
#ifdef KERNEL_64_USERSPACE_32
	u_int64_t config;
#else
	struct clusterip_config *config;
#endif
};

#endif /*_IPT_CLUSTERIP_H_target*/
