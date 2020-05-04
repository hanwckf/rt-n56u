/**
 * object_id.c - Processing of object ids
 *
 *	This module is part of ntfs-3g library
 *
 * Copyright (c) 2009 Jean-Pierre Andre
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
 */

#include "antfs.h"
#include "types.h"
#include "debug.h"
#include "attrib.h"
#include "dir.h"
#include "volume.h"
#include "mft.h"
#include "index.h"
#include "lcnalloc.h"
#include "object_id.h"
#include "misc.h"

/*
 *			Endianness considerations
 *
 *	According to RFC 4122, GUIDs should be printed with the most
 *	significant byte first, and the six fields be compared individually
 *	for ordering. RFC 4122 does not define the internal representation.
 *
 *	Here we always copy disk images with no endianness change,
 *	and, for indexing, GUIDs are compared as if they were a sequence
 *	of four unsigned 32 bit integers.
 *
 * --------------------- begin from RFC 4122 ----------------------
 * Consider each field of the UUID to be an unsigned integer as shown
 * in the table in section Section 4.1.2.  Then, to compare a pair of
 * UUIDs, arithmetically compare the corresponding fields from each
 * UUID in order of significance and according to their data type.
 * Two UUIDs are equal if and only if all the corresponding fields
 * are equal.
 *
 * UUIDs, as defined in this document, can also be ordered
 * lexicographically.  For a pair of UUIDs, the first one follows the
 * second if the most significant field in which the UUIDs differ is
 * greater for the first UUID.  The second precedes the first if the
 * most significant field in which the UUIDs differ is greater for
 * the second UUID.
 *
 * The fields are encoded as 16 octets, with the sizes and order of the
 * fields defined above, and with each field encoded with the Most
 * Significant Byte first (known as network byte order).  Note that the
 * field names, particularly for multiplexed fields, follow historical
 * practice.
 *
 * 0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          time_low                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |       time_mid                |         time_hi_and_version   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |clk_seq_hi_res |  clk_seq_low  |         node (0-1)            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         node (2-5)                            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * ---------------------- end from RFC 4122 -----------------------
 */

struct OBJECT_ID_INDEX_KEY {
	union {
		/* alignment may be needed to evaluate collations */
		u32 alignment;
		struct GUID guid;
	} object_id;
};

struct OBJECT_ID_INDEX_DATA {
	le64 file_id;
	struct GUID birth_volume_id;
	struct GUID birth_object_id;
	struct GUID domain_id;
};		/* known as OBJ_ID_INDEX_DATA */

struct OBJECT_ID_INDEX {	/* index entry in $Extend/$ObjId */
	struct INDEX_ENTRY_HEADER header;
	struct OBJECT_ID_INDEX_KEY key;
	struct OBJECT_ID_INDEX_DATA data;
};

static ntfschar objid_index_name[] = { const_cpu_to_le16('$'),
	const_cpu_to_le16('O')
};

#ifdef HAVE_SETXATTR		/* extended attributes interface required */

/*
 *			Set the index for a new object id
 *
 *	Returns 0 if success
 *		error code if failure
 */

static int set_object_id_index(struct ntfs_inode *ni,
			       struct ntfs_index_context *xo,
			       const struct OBJECT_ID_ATTR *object_id)
{
	struct OBJECT_ID_INDEX indx;
	u64 file_id_cpu;
	le64 file_id;
	le16 seqn;

	seqn = ni->mrec->sequence_number;
	file_id_cpu = MK_MREF(ni->mft_no, le16_to_cpu(seqn));
	file_id = cpu_to_le64(file_id_cpu);
	indx.header.data_offset =
		const_cpu_to_le16(sizeof(struct INDEX_ENTRY_HEADER) +
				    sizeof(struct OBJECT_ID_INDEX_KEY));
	indx.header.data_length =
	    const_cpu_to_le16(sizeof(struct OBJECT_ID_INDEX_DATA));
	indx.header.reservedV = const_cpu_to_le32(0);
	indx.header.length = const_cpu_to_le16(sizeof(struct OBJECT_ID_INDEX));
	indx.header.key_length =
		const_cpu_to_le16(sizeof(struct OBJECT_ID_INDEX_KEY));
	indx.header.flags = const_cpu_to_le16(0);
	indx.header.reserved = const_cpu_to_le16(0);

	memcpy(&indx.key.object_id, object_id, sizeof(struct GUID));

	indx.data.file_id = file_id;
	memcpy(&indx.data.birth_volume_id,
	       &object_id->birth_volume_id, sizeof(struct GUID));
	memcpy(&indx.data.birth_object_id,
	       &object_id->birth_object_id, sizeof(struct GUID));
	memcpy(&indx.data.domain_id, &object_id->domain_id,
	       sizeof(struct GUID));
	ntfs_index_ctx_reinit(xo);
	return ntfs_ie_add(xo, (INDEX_ENTRY *) &indx);
}

#endif /* HAVE_SETXATTR */

/*
 *		Open the $Extend/$ObjId file and its index
 *
 *	Return the index context if opened
 *		or error code if an error occurred
 *
 *	The index has to be freed and inode closed when not needed any more.
 */

static struct ntfs_index_context *open_object_id_index(struct ntfs_volume *vol)
{
	u64 inum;
	struct ntfs_inode *ni = NULL;
	struct ntfs_inode *dir_ni;
	struct ntfs_index_context *xo = ERR_PTR(-EIO);
	int err = 0;

	/* do not use path_name_to inode - could reopen root */
	dir_ni = ntfs_inode_open(vol, FILE_EXTEND, NULL);
	if (!IS_ERR(dir_ni)) {
		err = ntfs_inode_lookup_by_mbsname(dir_ni, "$ObjId", &inum);
		if (!err)
			ni = ntfs_inode_open(vol, inum, NULL);
		ntfs_inode_close(dir_ni);
	} else {
		err = PTR_ERR(dir_ni);
	}
	if (!IS_ERR_OR_NULL(ni)) {
		xo = ntfs_index_ctx_get(ni, objid_index_name, 2);
		if (IS_ERR(xo))
			ntfs_inode_close(ni);
	} else {
		err = PTR_ERR(ni);
	}
	if (err)
		xo = ERR_PTR(err);
	return xo;
}

#ifdef HAVE_SETXATTR		/* extended attributes interface required */

/*
 *		Merge object_id data stored in the index into
 *	a full object_id struct.
 *
 *	returns 0 if merging successful
 *		-1 if no data could be merged. This is generally not an error
 *		error code in case of an error
 */

static int merge_index_data(struct ntfs_inode *ni,
			    const struct OBJECT_ID_ATTR *objectid_attr,
			    struct OBJECT_ID_ATTR *full_objectid)
{
	struct OBJECT_ID_INDEX_KEY key;
	struct OBJECT_ID_INDEX *entry;
	struct ntfs_index_context *xo;
	struct ntfs_inode *xoni;
	int res;

	res = -1;
	xo = open_object_id_index(ni->vol);
	if (!IS_ERR(xo)) {
		memcpy(&key.object_id, objectid_attr, sizeof(struct GUID));
		if (!ntfs_index_lookup(&key,
		sizeof(struct OBJECT_ID_INDEX_KEY), xo)) {
			entry = (struct OBJECT_ID_INDEX *)xo->entry;
			/* make sure inode numbers match */
			if (entry && (MREF(le64_to_cpu(entry->data.file_id))
				      == ni->mft_no)) {
				memcpy(&full_objectid->birth_volume_id,
				       &entry->data.birth_volume_id,
				       sizeof(struct GUID));
				memcpy(&full_objectid->birth_object_id,
				       &entry->data.birth_object_id,
				       sizeof(struct GUID));
				memcpy(&full_objectid->domain_id,
				       &entry->data.domain_id,
				       sizeof(struct GUID));
				res = 0;
			}
		}
		xoni = xo->ni;
		ntfs_index_ctx_put(xo);
		ntfs_inode_close(xoni);
	} else {
		res = PTR_ERR(xo);
	}
	return res;
}

#endif /* HAVE_SETXATTR */

/*
 *		Remove an object id index entry if attribute present
 *
 *	Returns the size of existing object id
 *			(the existing object_d is returned)
 *		error code if failure
 */

static int remove_object_id_index(struct ntfs_attr *na,
				  struct ntfs_index_context *xo,
				  struct OBJECT_ID_ATTR *old_attr)
{
	struct OBJECT_ID_INDEX_KEY key;
	struct OBJECT_ID_INDEX *entry;
	s64 size;
	int ret;

	ret = na->data_size;
	if (ret) {
		/* read the existing object id attribute */
		size = ntfs_attr_pread(na, 0, sizeof(struct GUID), old_attr);
		if (size >= (s64) sizeof(struct GUID)) {
			memcpy(&key.object_id,
			       &old_attr->object_id, sizeof(struct GUID));
			ret = ntfs_index_lookup(&key,
				sizeof(struct OBJECT_ID_INDEX_KEY), xo);
			if (!ret) {
				entry = (struct OBJECT_ID_INDEX *)xo->entry;
				memcpy(&old_attr->birth_volume_id,
				       &entry->data.birth_volume_id,
				       sizeof(struct GUID));
				memcpy(&old_attr->birth_object_id,
				       &entry->data.birth_object_id,
				       sizeof(struct GUID));
				memcpy(&old_attr->domain_id,
				       &entry->data.domain_id,
				       sizeof(struct GUID));
				ret = ntfs_index_rm(xo);
			}
		} else {
			ret = -ENODATA;
		}
	}
	return ret;
}

#ifdef HAVE_SETXATTR		/* extended attributes interface required */

/*
 *		Update the object id and index
 *
 *	The object_id attribute should have been created and the
 *	non-duplication of the GUID should have been checked before.
 *
 *	Returns 0 if success
 *		error code if failure
 *	If could not remove the existing index, nothing is done,
 *	If could not write the new data, no index entry is inserted
 *	If failed to insert the index, data is removed
 */

static int update_object_id(struct ntfs_inode *ni,
			    struct ntfs_index_context *xo,
			    const struct OBJECT_ID_ATTR *value, size_t size)
{
	struct OBJECT_ID_ATTR old_attr;
	struct ntfs_attr *na;
	int oldsize;
	int written;
	int res = 0;

	na = ntfs_attr_open(ni, AT_OBJECT_ID, AT_UNNAMED, 0);
	if (!IS_ERR(na)) {

		/* remove the existing index entry */
		oldsize = remove_object_id_index(na, xo, &old_attr);
		if (oldsize < 0) {
			res = oldsize;
		} else {
			/* resize attribute */
			res = ntfs_attr_truncate(na, (s64) sizeof(struct GUID));
			/* write the object_id in attribute */
			if (!res && value) {
				written = (int)ntfs_attr_pwrite(na, (s64) 0,
						(s64) sizeof(struct GUID),
						&value->object_id);
				if (written != (s64) sizeof(struct GUID)) {
					antfs_log_error("Failed to update "
						       "object id");
					res = -EIO;
				}
			}
			/* write index part if provided */

			if (!res) {
				res = set_object_id_index(ni, xo, value)
				    if (res || (size <
					sizeof(struct OBJECT_ID_ATTR))) {
					/*
					 * If cannot index, try to remove the
					 * object id and log the error. There
					 * will be an inconsistency if removal
					 * fails.
					 */
					ntfs_attr_rm(na);
					antfs_log_error
					    ("Failed to index object id."
					     " Possible corruption.");
				}
			}
		}
		ntfs_attr_close(na);
		NInoSetDirty(ni);
	} else {
		res = PTR_ERR(na);
	}
	return res;
}

/*
 *		Add a (dummy) object id to an inode if it does not exist
 *
 *	returns 0 if attribute was inserted (or already present)
 *		error code if adding failed
 */

static int add_object_id(struct ntfs_inode *ni, int flags)
{
	int res;
	u8 dummy;

	if (!ntfs_attr_exist(ni, AT_OBJECT_ID, AT_UNNAMED, 0)) {
		if (!(flags & XATTR_REPLACE)) {
			/*
			 * no object id attribute : add one,
			 * apparently, this does not feed the new value in
			 * Note : NTFS version must be >= 3
			 */
			if (ni->vol->major_ver >= 3) {
				res = ntfs_attr_add(ni, AT_OBJECT_ID,
						    AT_UNNAMED, 0, &dummy,
						    (s64) 0);
				NInoSetDirty(ni);
			} else {
				res = -EOPNOTSUPP;
			}
		} else {
			res = -ENODATA;
		}
	} else {
		if (flags & XATTR_CREATE)
			res = -EEXIST;
		else
			res = 0;
	}

	return res;
}

#endif /* HAVE_SETXATTR */

/*
 *		Delete an object_id index entry
 *
 *	Returns 0 if success
 *		error code if failure
 */

int ntfs_delete_object_id_index(struct ntfs_inode *ni)
{
	struct ntfs_index_context *xo;
	struct ntfs_inode *xoni;
	struct ntfs_attr *na;
	struct OBJECT_ID_ATTR old_attr;
	int res = 0;

	na = ntfs_attr_open(ni, AT_OBJECT_ID, AT_UNNAMED, 0);
	if (!IS_ERR(na)) {
		/*
		 * read the existing object id
		 * and un-index it
		 */
		xo = open_object_id_index(ni->vol);
		if (!IS_ERR(xo)) {
			res = remove_object_id_index(na, xo, &old_attr);
			xoni = xo->ni;
			ntfs_index_entry_mark_dirty(xo);
			NInoSetDirty(xoni);
			ntfs_index_ctx_put(xo);
			ntfs_inode_close(xoni);
		} else {
			res = PTR_ERR(xo);
		}
		ntfs_attr_close(na);
	} else {
		res = PTR_ERR(na);
	}
	return res;
}

#ifdef HAVE_SETXATTR		/* extended attributes interface required */

/*
 *		Get the ntfs object id into an extended attribute
 *
 *	If present, the object_id from the attribute and the GUIDs
 *	from the index are returned (formatted as OBJECT_ID_ATTR)
 *
 *	Returns the global size (can be 0, 16 or 64)
 *		and the buffer is updated if it is long enough
 */

int ntfs_get_ntfs_object_id(struct ntfs_inode *ni, char *value, size_t size)
{
	struct OBJECT_ID_ATTR full_objectid;
	struct OBJECT_ID_ATTR *objectid_attr;
	s64 attr_size;
	int full_size = 0;/* default to no data and some error to be defined */
	int err = 0;

	if (ni) {
		objectid_attr = (struct OBJECT_ID_ATTR *) ntfs_attr_readall(ni,
				    AT_OBJECT_ID, NULL, 0,
				    &attr_size);
		if (!IS_ERR(objectid_attr)) {
			/* restrict to only GUID present in attr */
			if (attr_size == sizeof(struct GUID)) {
				memcpy(&full_objectid.object_id,
				       objectid_attr, sizeof(struct GUID));
				full_size = sizeof(struct GUID);
				/* get data from index, if any */
				if (!merge_index_data(ni, objectid_attr,
						      &full_objectid)) {
					full_size =
						sizeof(struct OBJECT_ID_ATTR);
				}
				if (full_size <= (s64) size) {
					if (value)
						memcpy(value, &full_objectid,
						       full_size);
					else
						err = -EINVAL;
				}
			} else {
				/* unexpected size, better return unsupported */
				err = -EOPNOTSUPP;
				full_size = 0;
			}
			ntfs_free(objectid_attr);
		} else
			err = -ENODATA;
	} else {
		err = -EINVAL;
	}
	if (err)
		full_size = err;
	return full_size;
}

/*
 *		Set the object id from an extended attribute
 *
 *	If the size is 64, the attribute and index are set.
 *	else if the size is not less than 16 only the attribute is set.
 *	The object id index is set accordingly.
 *
 *	Returns 0, or -1 if there is a problem
 */

int ntfs_set_ntfs_object_id(struct ntfs_inode *ni,
			    const char *value, size_t size, int flags)
{
	struct OBJECT_ID_INDEX_KEY key;
	struct ntfs_inode *xoni;
	struct ntfs_index_context *xo;
	int res;

	res = 0;
	if (ni && value && (size >= sizeof(struct GUID))) {
		xo = open_object_id_index(ni->vol);
		if (!IS_ERR(xo)) {
			/* make sure the GUID was not used somewhere */
			memcpy(&key.object_id, value, sizeof(struct GUID));
			if (ntfs_index_lookup(&key,
			sizeof(struct OBJECT_ID_INDEX_KEY), xo)) {
				ntfs_index_ctx_reinit(xo);
				res = add_object_id(ni, flags);
				if (!res) {
					/* update value and index */
					res = update_object_id(ni, xo,
						(const struct OBJECT_ID_ATTR *)
						value, size);
				}
			} else {
				/* GUID is present elsewhere */
				res = -EEXIST;
			}
			xoni = xo->ni;
			ntfs_index_entry_mark_dirty(xo);
			NInoSetDirty(xoni);
			ntfs_index_ctx_put(xo);
			ntfs_inode_close(xoni);
		} else {
			res = PTR_ERR(xo);
		}
	} else {
		return -EINVAL;
	}
	return res;
}

/*
 *		Remove the object id
 *
 *	Returns 0, or error code if there is a problem
 */

int ntfs_remove_ntfs_object_id(struct ntfs_inode *ni)
{
	int res = 0;
	int err;
	struct ntfs_attr *na;
	struct ntfs_inode *xoni;
	struct ntfs_index_context *xo;
	int oldsize;
	struct OBJECT_ID_ATTR old_attr;

	if (IS_ERR_OR_NULL(ni))
		return -EINVAL;
	/*
	 * open and delete the object id
	 */
	na = ntfs_attr_open(ni, AT_OBJECT_ID, AT_UNNAMED, 0);
	if (IS_ERR(na)) {
		res = -ENODATA;
		goto out;
	}

	/* first remove index (old object id needed) */
	xo = open_object_id_index(ni->vol);
	if (IS_ERR(xo)) {
		res = PTR_ERR(xo);
		goto close_na;
	}

	oldsize = remove_object_id_index(na, xo, &old_attr);
	if (oldsize < 0) {
		res = oldsize;
	} else {
		/* now remove attribute */
		res = ntfs_attr_rm(na);
		if (res && (oldsize > (int)sizeof(struct GUID))) {
			/*
			 * If we could not remove the
			 * attribute, try to restore the
			 * index and log the error. There
			 * will be an inconsistency if
			 * the reindexing fails.
			 */
			set_object_id_index(ni, xo, &old_attr);
			ntfs_log_error("Failed to remove object id."
				       " Possible corruption.");
		}
	}

	xoni = xo->ni;
	ntfs_index_entry_mark_dirty(xo);
	NInoSetDirty(xoni);
	ntfs_index_ctx_put(xo);
	ntfs_inode_close(xoni);

close_na:
	err = ntfs_attr_close(na);
	if (err != -ENOENT)
		res = err;
out:
	NInoSetDirty(ni);
	return res;
}

#endif /* HAVE_SETXATTR */
