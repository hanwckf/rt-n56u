/**
 * security.c - Handling security/ACLs in NTFS.
 *              Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2004 Anton Altaparmakov
 * Copyright (c) 2005-2006 Szabolcs Szakacsits
 * Copyright (c) 2006 Yura Pakhuchiy
 * Copyright (c) 2007-2015 Jean-Pierre Andre
 * Copyright (C) 2019 AVM GmbH
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

#include "antfs.h"
#include "attrib.h"
#include "inode.h"
#include "dir.h"
#include "layout.h"
#include "misc.h"
#include "security.h"
#include "types.h"
#include "volume.h"

/*
 *	Create a default security descriptor for files whose descriptor
 *	cannot be inherited
 */

struct default_sd {
	struct SECURITY_DESCRIPTOR_RELATIVE sd;
	struct SID sid1;
	le32 sid1_sub_authority1;
	struct SID sid2;
	le32 sid2_sub_authority1;
	struct ACL acl;
	struct ACCESS_ALLOWED_ACE ace;
} __attribute__ ((__packed__));


static const struct default_sd default_sd = {
	.sd = {
		.revision = SECURITY_DESCRIPTOR_REVISION,
		.control = SE_DACL_PRESENT | SE_SELF_RELATIVE,
		.owner = cpu_to_le32(offsetof(struct default_sd, sid1)),
		.group = cpu_to_le32(offsetof(struct default_sd, sid2)),
		.dacl = cpu_to_le32(offsetof(struct default_sd, acl)),
	},
	.sid1 = {
		.revision = SID_REVISION,
		.sub_authority_count = 2,
		.identifier_authority.value[5] = 5,
		.sub_authority[0] = const_cpu_to_le32(SECURITY_BUILTIN_DOMAIN_RID),
	},
	.sid1_sub_authority1 = const_cpu_to_le32(DOMAIN_ALIAS_RID_ADMINS),
	.sid2 = {
		.revision = SID_REVISION,
		.sub_authority_count = 2,
		.identifier_authority.value[5] = 5,
		.sub_authority[0] = const_cpu_to_le32(SECURITY_BUILTIN_DOMAIN_RID),
	},
	.sid2_sub_authority1 = const_cpu_to_le32(DOMAIN_ALIAS_RID_ADMINS),
	.acl = {
		.revision = ACL_REVISION,
		.size = const_cpu_to_le16(sizeof(struct ACL) +
					  sizeof(struct ACCESS_ALLOWED_ACE)),
		.ace_count = const_cpu_to_le16(1),
	},
	.ace = {
		.type = ACCESS_ALLOWED_ACE_TYPE,
		.flags = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE,
		.size = const_cpu_to_le16(sizeof(struct ACCESS_ALLOWED_ACE)),
		.mask = const_cpu_to_le32(0x1f01ff), /* all permission flags set */
		// https://en.wikipedia.org/wiki/Security_descriptor#Permissions_in_NTFS
		.sid = {
			.revision = SID_REVISION,
			.sub_authority_count = 1,
			.identifier_authority.value[5] = 1,
			.sub_authority[0] = const_cpu_to_le32(0),
		},
	},
};

int ntfs_sd_add_everyone(struct ntfs_inode *ni)
{
	int ret;

	ret = ntfs_attr_add(ni, AT_SECURITY_DESCRIPTOR, AT_UNNAMED, 0,
			    (u8 *)&default_sd, sizeof(default_sd));
	if (ret)
		antfs_log_error("Failed to add SECURITY_DESCRIPTOR. ret = %d",
				ret);

	return ret;
}
