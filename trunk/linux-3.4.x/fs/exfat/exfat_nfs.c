/*
 *  linux/fs/exfat/nfs.c
 *
 *  ExFAT extensions for NFS export opertions,
 *   originally imported from linux/fs/fat/inode.c (3.0.94)
 *
 */

#define FILEID_EXFAT_GEN	3
#define EXFAT_FID_SIZE		5

/*
 * a (ex)FAT file handle with fhtype 3 is
 *  0/  i_ino - for fast, reliable lookup if still in the cache
 *  1/  i_generation - to see if i_ino is still valid
 *          bit 0 == 0 iff directory
 *  2/  i_pos(8-39) - if ino has changed, but still in cache
 *  3/  i_pos(4-7)|fid.start_clu - to semi-verify inode found at i_pos
 *  4/  i_pos(0-3)|parent->fid.start_clu - maybe used to hunt for the file on disc
 *
 * Hack for NFSv2: Maximum FAT entry number is 28bits and maximum
 * i_pos is 40bits (blocknr(32) + dir offset(8)), so two 4bits
 * of fid.start_clu is used to store the directory entry offset.
 */
static struct dentry *exfat_fh_to_dentry(struct super_block *sb,
		struct fid *fid, int fh_len, int fh_type)
{
	struct inode *inode = NULL;
	struct dentry *result;
	u32 *fh = fid->raw;

	if (fh_len < EXFAT_FID_SIZE || fh_type != FILEID_EXFAT_GEN)
		return NULL;

	inode = ilookup(sb, fh[0]);
	if (!inode || inode->i_generation != fh[1]) {
		if (inode)
			iput(inode);
		inode = NULL;
	}
	if (!inode) {
		loff_t i_pos;
		int start_clu = fh[3] & 0x0fffffff;

		i_pos = (loff_t)fh[2] << 8;
		i_pos |= ((fh[3] >> 24) & 0xf0) | (fh[4] >> 28);

		/* try 2 - see if i_pos is in F-d-c
		 * require start_clu to be the same
		 * Will fail if you truncate and then re-write
		 */

		inode = exfat_iget(sb, i_pos);
		if (inode && EXFAT_I(inode)->fid.start_clu != start_clu) {
			iput(inode);
			inode = NULL;
		}
	}

	/*
	 * For now, do nothing if the inode is not found.
	 *
	 * What we could do is:
	 *
	 *	- follow the file starting at fh[4], and record the ".." entry,
	 *	  and the name of the fh[2] entry.
	 *	- then follow the ".." file finding the next step up.
	 *
	 * This way we build a path to the root of the tree. If this works, we
	 * lookup the path and so get this inode into the cache.  Finally try
	 * the exfat_iget lookup again.  If that fails, then we are totally out
	 * of luck.  But all that is for another day
	 */
	result = d_obtain_alias(inode);
	if (!IS_ERR(result))
		result->d_op = sb->s_root->d_op;
	return result;
}

static int
exfat_encode_fh(struct dentry *de, __u32 *fh, int *lenp, int connectable)
{
	int len = *lenp;
	struct inode *inode =  de->d_inode;
	u32 ipos_h, ipos_m, ipos_l;

	if (len < EXFAT_FID_SIZE) {
		*lenp = EXFAT_FID_SIZE;
		return 255; /* no room */
	}

	ipos_h = EXFAT_I(inode)->i_pos >> 8;
	ipos_m = (EXFAT_I(inode)->i_pos & 0xf0) << 24;
	ipos_l = (EXFAT_I(inode)->i_pos & 0x0f) << 28;
	*lenp = EXFAT_FID_SIZE;
	fh[0] = inode->i_ino;
	fh[1] = inode->i_generation;
	fh[2] = ipos_h;
	fh[3] = ipos_m | EXFAT_I(inode)->fid.start_clu;
	fh[4] = ipos_l;
	if (connectable) {
		spin_lock(&de->d_lock);
		fh[4] |= EXFAT_I(de->d_parent->d_inode)->fid.start_clu;
		spin_unlock(&de->d_lock);
	}

	return FILEID_EXFAT_GEN;
}

static struct dentry *exfat_get_parent(struct dentry *child)
{
	static const struct qstr dotdot = {
		.name = "..",
		.len = 2,
	};
	struct super_block *sb = child->d_sb;
	FILE_ID_T fid;
	loff_t i_pos;
	struct dentry *parent;
	int err;

	__lock_super(sb);
	err = exfat_find(child->d_inode, &dotdot, &fid);
	if (err) {
		parent = ERR_PTR(-ENOENT);
		goto out;
	}

	i_pos = ((loff_t) fid.dir.dir << 32) | (fid.entry & 0xffffffff);
	parent = d_obtain_alias(exfat_iget(child->d_inode->i_sb, i_pos));
out:
	__unlock_super(sb);

	return parent;
}

static const struct export_operations exfat_export_ops = {
	.encode_fh	= exfat_encode_fh,
	.fh_to_dentry	= exfat_fh_to_dentry,
	.get_parent	= exfat_get_parent,
};
