/**
 * lcnalloc.c - Cluster (de)allocation code.
 * Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2002-2004 Anton Altaparmakov
 * Copyright (c) 2004 Yura Pakhuchiy
 * Copyright (c) 2004-2008 Szabolcs Szakacsits
 * Copyright (c) 2008-2009 Jean-Pierre Andre
 * Copyright (c) 2016 Martin Pommerenke, Jens Krieg, Arwed Meyer,
 *		      Christian René Sechting
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
#include "attrib.h"
#include "bitmap.h"
#include "debug.h"
#include "runlist.h"
#include "volume.h"
#include "lcnalloc.h"
#include "misc.h"

/*
 * Plenty possibilities for big optimizations all over in the cluster
 * allocation, however at the moment the dominant bottleneck (~ 90%) is
 * the update of the mapping pairs which converges to the cubic Faulhaber's
 * formula as the function of the number of extents (fragments, runs).
 */
#define NTFS_LCNALLOC_BSIZE 31
#define NTFS_LCNALLOC_SKIP  NTFS_LCNALLOC_BSIZE

enum {
	ZONE_MFT = 1,
	ZONE_DATA1 = 2,
	ZONE_DATA2 = 4
};

static const char __maybe_unused *get_zone_name(const int zone)
{
	switch (zone) {
	case ZONE_MFT:
		return "MFT";
	case ZONE_DATA1:
		return "DATA1";
	case ZONE_DATA2:
		return "DATA2";
	default:
		return "???";
	}
}

static void ntfs_cluster_set_zone_pos(LCN start, LCN end, LCN *pos, LCN tc)
{
	antfs_log_debug("pos: %lld  tc: %lld", (long long)*pos, (long long)tc);

	if (tc >= end)
		*pos = start;
	else if (tc >= start)
		*pos = tc;
}

static void ntfs_cluster_update_zone_pos(struct ntfs_volume *vol, u8 zone,
					 LCN lcn, s64 size)
{
	LCN size_in_clusters = size >> vol->cluster_size_bits;
	LCN tc = lcn;

	if (size_in_clusters < 0) {
		/* special case for mft record alloc, make space for another
		 * allocation of records
		 */
		unsigned int nr;

		nr = vol->mft_record_size << MFT_DATA_BURST_ALLOC_SHIFT >>
			vol->cluster_size_bits;
		tc += nr;
	} else if (size_in_clusters < 8) {
		tc += 1;
	} else if (size_in_clusters < 32) {
		tc += 31;
	} else if (size_in_clusters < 2048) {
		tc += 63;
	} else {
		tc += 255;
	}

	antfs_log_debug("tc = %lld, zone = %d", (long long)tc, zone);

	if (zone == ZONE_MFT) {
		tc = lcn + 1;
		ntfs_cluster_set_zone_pos(vol->mft_lcn, vol->mft_zone_end,
					  &vol->mft_zone_pos, tc);
	} else if (zone == ZONE_DATA1) {
		if (lcn < vol->data1_zone_pos)
			return;
		ntfs_cluster_set_zone_pos(vol->mft_zone_end, vol->nr_clusters,
					  &vol->data1_zone_pos, tc);
	} else {			/* zone == ZONE_DATA2 */
		if (lcn < vol->data2_zone_pos)
			return;
		ntfs_cluster_set_zone_pos(0, vol->mft_zone_start,
					  &vol->data2_zone_pos, tc);
	}
}

static void antfs_force_zone_pos(struct ntfs_volume *vol, u8 zone,
					 LCN lcn)
{
	antfs_log_enter("lcn = %lld, zone = %d", (long long)lcn, zone);

	if (zone == ZONE_MFT) {
		ntfs_cluster_set_zone_pos(vol->mft_lcn, vol->mft_zone_end,
					  &vol->mft_zone_pos, lcn);
	} else if (zone == ZONE_DATA1) {
		ntfs_cluster_set_zone_pos(vol->mft_zone_end, vol->nr_clusters,
					  &vol->data1_zone_pos, lcn);
	} else {			/* zone == ZONE_DATA2 */
		ntfs_cluster_set_zone_pos(0, vol->mft_zone_start,
					  &vol->data2_zone_pos, lcn);
	}
}
/*
 *		Unmark full zones when a cluster has been freed in a full zone
 *
 *	Next allocation will reuse the freed cluster
 */
void update_full_status(struct ntfs_volume *vol, LCN lcn)
{
	if (lcn >= vol->mft_zone_end) {
		if (vol->full_zones & ZONE_DATA1) {
			ntfs_cluster_set_zone_pos(vol->mft_zone_end,
					    vol->nr_clusters,
					    &vol->data1_zone_pos, lcn);
			vol->full_zones &= ~ZONE_DATA1;
		}
	} else if (lcn < vol->mft_zone_start) {
		if (vol->full_zones & ZONE_DATA2) {
			ntfs_cluster_set_zone_pos(0, vol->mft_zone_start,
					    &vol->data2_zone_pos, lcn);
			vol->full_zones &= ~ZONE_DATA2;
		}
	} else {
		if (vol->full_zones & ZONE_MFT) {
			ntfs_cluster_set_zone_pos(vol->mft_lcn,
					    vol->mft_zone_end,
					    &vol->mft_zone_pos, lcn);
			vol->full_zones &= ~ZONE_MFT;
		}
	}
}

/**
 * @brief Find largest range of 0-bits in @buf
 *
 * Walks through @buf starting at @starting_pos to find the longest
 * continuous range of 0-bits and returns this.
 *
 * @param buf           Pointer to bit field
 * @param size          Size of @buf in bytes
 * @param starting_pos  Bit-number to start the search
 *
 * @return Bit position in buf of longest 0bit range or -1.
 */
static ssize_t max_empty_bit_range(unsigned char *buf, size_t size,
				   size_t starting_pos)
{
	size_t i, j, run = 0;
	size_t max_range = 0;
	ssize_t start_pos = -1;	/* Ok to use with size_t -
				   can never overflow with ntfs for now. */

	antfs_log_enter("starting_pos(%lu) | byte(%lu) | bit(%d)",
			(unsigned long)starting_pos,
			(unsigned long)starting_pos >> 3,
			(int)starting_pos & 7);

	/* Advance to the first byte we are allowed to look at. */
	buf += starting_pos >> 3;
	i = starting_pos >> 3;

	if (i >= size)
		goto out;

	if (starting_pos & 7) {
		/* Modify the first byte according the bit position we are
		 * allowed to start allocating blocks. */
		switch (*buf) {
		case 0:
			run = 8 - (starting_pos & 7);
			break;
		case 255:
			break;
		default:
			for (j = starting_pos & 7; j < 8; j++) {
				int bit = *buf & (1 << j);
				if (bit) {
					/* Reset run, save the last one. */
					if (run > max_range) {
						max_range = run;
						/* start_pos is our result */
						start_pos = (i * 8) + (j - run);
					}
					run = 0;
				} else
					run++;
			}
			break;
		}
		buf++;
		i++;
	}

	while (i < size) {
		switch (*buf) {
		case 0:
			do {
				buf++;
				run += 8;
				i++;
			} while ((i < size) && !*buf);
			break;
		case 255:
			if (run > max_range) {
				max_range = run;
				start_pos = i * 8 - run;
			}
			run = 0;
			do {
				buf++;
				i++;
			} while ((i < size) && (*buf == 255));
			break;
		default:
			for (j = 0; j < 8; j++) {

				int bit = *buf & (1 << j);

				if (bit) {
					if (run > max_range) {
						max_range = run;
						start_pos = i * 8 + (j - run);
					}
					run = 0;
				} else {
					run++;
				}
			}
			i++;
			buf++;
		}
	}

	if (run > max_range)
		start_pos = i * 8 - run;

out:
	antfs_log_leave("start_pos(%lld)", (long long)start_pos);
	return start_pos;
}

static int antfs_check_bh(struct ntfs_volume *vol, LCN lcn, LCN zone_end)
{
	s64 curr_bh_pos;
	int block_to_lcnbits;
	int err;
	int buf_size;

	block_to_lcnbits = 3 + vol->dev->d_sb->s_blocksize_bits;
	/* lcn as block number in cluster bitmap */
	curr_bh_pos = lcn >> block_to_lcnbits;
	/* (Re-)Load buffer head for bitmap? */
	if (!vol->lcnbmp_bh || curr_bh_pos != vol->lcnbmp_start) {
		if (vol->lcnbmp_bh) {
			antfs_log_debug("'wrong bufferhead! lcnmbp_start(%lld) "
					"curr_bh_pos(%lld)", (long long)
					vol->lcnbmp_start,
					(long long)curr_bh_pos);

			/* it takes some time till the buffer_head gets written
			 * back to the NTFS device. In case of creating lots
			 * of small files in a short time we will accumulate
			 * a lot of bh's that need to be written back at some
			 * point. This write every bh back at once will block
			 * all allocation-operations for quite some time.
			 * Therefore as a quickfix write at least the lcnbmp_bh
			 * back at this point.
			 */
			write_dirty_buffer(vol->lcnbmp_bh, WRITE);
			brelse(vol->lcnbmp_bh);
		}
		vol->lcnbmp_bh = ntfs_load_bitmap_attr(vol, vol->lcnbmp_na,
						       lcn);
		if (IS_ERR_OR_NULL(vol->lcnbmp_bh)) {
			err = PTR_ERR(vol->lcnbmp_bh);
			if (err == 0)
				err = -EIO;
			antfs_log_debug("Reading $BITMAP failed: %d", err);
			vol->lcnbmp_bh = NULL;
			return err;
		}
		vol->lcnbmp_start = curr_bh_pos;
	} else {
		antfs_log_debug("'correct buffer_head! start(%lld) "
				"curr_bh(%lld)",
				(long long)vol->lcnbmp_start,
				(long long)curr_bh_pos);
	}

	/* buf size in bits
	 * Make sure we don't search past end of zone. */
	buf_size = min(zone_end + 1 - (curr_bh_pos << block_to_lcnbits),
		       (LCN) vol->lcnbmp_bh->b_size << 3);
	return buf_size;
}

static LCN antfs_get_zone_pos(struct ntfs_volume *vol, LCN lcn, u8 zone)
{
	switch (zone) {
	case ZONE_MFT:
		return vol->mft_zone_pos > lcn ? vol->mft_zone_pos : lcn;
	case ZONE_DATA1:
		return vol->data1_zone_pos > lcn ? vol->data1_zone_pos : lcn;
	case ZONE_DATA2:
		return vol->data2_zone_pos > lcn ? vol->data2_zone_pos : lcn;
	default:
		return lcn;
	}
}
/**
 * ntfs_cluster_alloc - allocate clusters on an ntfs volume
 * @vol:	mounted ntfs volume on which to allocate the clusters
 * @start_vcn:	vcn to use for the first allocated cluster
 * @count:	number of clusters to allocate
 * @start_lcn:	starting lcn at which to allocate the clusters (or -1 if none)
 * @zone:	zone from which to allocate the clusters
 *
 * Allocate @count clusters preferably starting at cluster @start_lcn or at the
 * current allocator position if @start_lcn is -1, on the mounted ntfs volume
 * @vol. @zone is either DATA_ZONE for allocation of normal clusters and
 * MFT_ZONE for allocation of clusters for the master file table, i.e. the
 * $MFT/$DATA attribute.
 *
 * On success return a runlist describing the allocated cluster(s).
 *
 * On error return NULL with errno set to the error code.
 *
 * Notes on the allocation algorithm
 * =================================
 *
 * There are two data zones. First is the area between the end of the mft zone
 * and the end of the volume, and second is the area between the start of the
 * volume and the start of the mft zone. On unmodified/standard NTFS 1.x
 * volumes, the second data zone doesn't exist due to the mft zone being
 * expanded to cover the start of the volume in order to reserve space for the
 * mft bitmap attribute.
 *
 * The complexity stems from the need of implementing the mft vs data zoned
 * approach and from the fact that we have access to the lcn bitmap via up to
 * @b_size (buffer_head) bytes at a time, so we need to cope with crossing over
 * boundaries of two buffers. Further, the fact that the allocator allows for
 * caller supplied hints as to the location of where allocation should begin
 * and the fact that the allocator keeps track of where in the data zones the
 * next natural allocation should occur, contribute to the complexity of the
 * function. But it should all be worthwhile, because this allocator:
 *   1) implements MFT zone reservation
 *   2) causes reduction in fragmentation.
 * The code is not optimized for speed.
 */
struct runlist_element *ntfs_cluster_alloc(struct ntfs_volume *vol,
					   VCN start_vcn, s64 count,
					   LCN start_lcn,
					   const enum
					   NTFS_CLUSTER_ALLOCATION_ZONES zone,
					   s64 file_size)
{
	struct runlist_element *rl = NULL, *trl;
	LCN zone_start, zone_end;	/* current search range */
	LCN lcn;
	LCN prev_lcn = 0, prev_run_len = 0;
	s64 clusters, reserved_clusters;
	int block_to_lcnbits;
	LCN block_to_lcnbits_mask;
	int rl_pos, rl_elements;
	int err = 0;
	int pass = 1;
	u8 search_zone;	/* 4: data2 (start) 1: mft (middle) 2: data1 (end) */
	u8 done_zones = 0;

	/* === initialization and checks === */
	antfs_log_enter("count = %lld, start_lcn = %lld, "
			"zone = %s_ZONE.", (long long)count, (long long)
			start_lcn, zone == MFT_ZONE ? "MFT" : "DATA");

	if (!vol || count < 0 || start_lcn < -1 || !vol->lcnbmp_na ||
	    (s8) zone < FIRST_ZONE || zone > LAST_ZONE) {
		antfs_log_error("EINVAL: vcn: %lld, count: %lld, lcn: %lld",
				(long long)start_vcn, (long long)count,
				(long long)start_lcn);
		err = -EINVAL;
		goto done_err_ret;
	}

	/* Return empty runlist if @count == 0 */
	if (!count) {
		rl = ntfs_malloc(ANTFS_RL_BUF_SIZE);
		if (rl) {
			rl[0].vcn = start_vcn;
			rl[0].lcn = LCN_RL_NOT_MAPPED;
			rl[0].length = 0;
		} else {
			rl = ERR_PTR(-ENOMEM);
		}
		goto out;
	}

	/* XXX: This is to prevent the volume from becoming completely filled
	 *      up. Windows and chkdsk really don't like this.
	 */
	reserved_clusters = antfs_reserved_clusters(vol);
	if (vol->free_clusters < reserved_clusters ||
	    vol->free_clusters - reserved_clusters < count) {
		err = -ENOSPC;
		goto done_err_ret;
	}
	/* initialize constant variables */
	block_to_lcnbits = 3 + vol->dev->d_sb->s_blocksize_bits;
	block_to_lcnbits_mask = (vol->dev->d_sb->s_blocksize << 3) - 1;

	/*
	 * If no @start_lcn was requested, use the current zone
	 * position otherwise use the requested @start_lcn.
	 */
	zone_start = start_lcn;

	if (zone_start < 0) {
		if (zone == DATA_ZONE)
			zone_start = vol->data1_zone_pos;
		else
			zone_start = vol->mft_zone_pos;
	}

	/* Fix zone_start if it doesn't lay inside the zone we should use to
	 * allocate the new cluster. Check if the preferred zone is full though!
	 */
	if (zone != MFT_ZONE && zone_start >= vol->mft_zone_start &&
	    zone_start < vol->mft_zone_end) {
		if (!(vol->full_zones & ZONE_DATA1))
			zone_start = vol->data1_zone_pos;
		else if (!(vol->full_zones & ZONE_DATA2))
			zone_start = vol->data2_zone_pos;
	}

	/* there is no need for a second pass if we start at the beginning of
	 * the current zone
	 */
	if (!zone_start || zone_start == vol->mft_zone_start ||
	    zone_start == vol->mft_zone_end)
		pass = 2;

	/* set the zone_end for the current search zone */
	if (zone_start < vol->mft_zone_start) {
		zone_end = vol->mft_zone_start;
		search_zone = ZONE_DATA2;
	} else if (zone_start < vol->mft_zone_end) {
		zone_end = vol->mft_zone_end;
		search_zone = ZONE_MFT;
	} else {
		zone_end = vol->nr_clusters;
		search_zone = ZONE_DATA1;
	}

	lcn = zone_start;

	/* === Loop until all clusters are allocated. === */
	clusters = count;
	rl_pos = rl_elements = 0;

	if (mutex_lock_interruptible(&vol->lcnbmp_lock)) {
		err = -EBUSY;
		goto done_err_ret;
	}
	while (1) {
		/* === go through bufferhead and find free bit === */
		/* === if free lcn found, add it to rl === */
		int bmp_offs;
		int buf_size;

		/* check whether we have exhausted the current zone */
		if (search_zone & vol->full_zones)
			goto skip_zone;

		buf_size = antfs_check_bh(vol, lcn, zone_end);
		if (buf_size < 0) {
			err = (int)buf_size;
			goto err_ret;
		}

		/* Offset in current bitmap part in bufferhead - in bits
		 * Note: Could as well use buf_size as mask, but this seems
		 * "more safe". */
		bmp_offs = lcn & block_to_lcnbits_mask;

		/* Walk the whole buffer using bmp_offs and also increment lcn
		 * with each turn. */
		/*FIXME: another while or new function?*/
		while (bmp_offs < buf_size) {
			char *byte;
			u8 bit;

			byte = vol->lcnbmp_bh->b_data + (bmp_offs >> 3);
			bit = 1 << (bmp_offs & 7);
			antfs_log_debug("lcn(%lld) buf_size(%d) [BITS] "
					"bmp_offs(%d) byte(0x%02x) bit(%d)",
					(long long)lcn, buf_size, bmp_offs,
					*byte, bmp_offs & 7);

			if (*byte & bit) {
				LCN tmp_lcn = antfs_get_zone_pos(vol, lcn,
								 search_zone);
				int next_bmp_offs;

				if (tmp_lcn != lcn) {
					lcn = tmp_lcn;
					bmp_offs = lcn - (vol->lcnbmp_start <<
						    block_to_lcnbits);
					continue;
				}

				/* We have no clue. Seek for a starting point */
				next_bmp_offs =
				    max_empty_bit_range(vol->lcnbmp_bh->b_data,
						buf_size >> 3, bmp_offs + 1);
				if (next_bmp_offs < 0 && bmp_offs) {
					/* Didn't find anything behind bmp_offs.
					 * Try again from buffer start.
					 */
					next_bmp_offs = max_empty_bit_range(
							vol->lcnbmp_bh->
							b_data,
							buf_size >> 3, 0);
				}
				if (next_bmp_offs < 0) {
					/* Didn't find clear bits in buffer */
					lcn = (vol->lcnbmp_start + 1) <<
						block_to_lcnbits;
					break;
				}
				bmp_offs = next_bmp_offs;
				lcn = bmp_offs + (vol->lcnbmp_start <<
						  block_to_lcnbits);
				/* For 2nd pass we search a block until we used
				 * up every little space. Even go backwards in
				 * zone_pos.
				 */
				if (pass == 2)
					antfs_force_zone_pos(vol, search_zone,
							lcn);
				continue;
			}
			/* Found a clear bit / free LCN here */

			/* Reallocate memory if necessary. */
			if (rl_pos + 2 >= rl_elements) {
				int new_rl_elements = rl_elements +
					ANTFS_RL_CHUNKS;

				trl = ntfs_rl_realloc(rl, rl_elements,
						new_rl_elements);
				if (!trl) {
					err = -ENOMEM;
					goto err_ret;
				}
				rl = trl;
				rl_elements = new_rl_elements;
			}

			/* Allocate the bitmap bit. */
			*byte |= bit;
			mark_buffer_dirty(vol->lcnbmp_bh);
			if (vol->free_clusters <= 0)
				antfs_log_error("Non-positive free clusters "
						"(%lld)!",
						(long long)vol->free_clusters);
			else
				vol->free_clusters--;

			/*
			 * Coalesce with previous run if adjacent LCNs.
			 * Otherwise, append a new run.
			 */
			antfs_log_debug
			    ("prev_lcn(%lld) prev_run_len(%lld), lcn(%lld)",
			     prev_lcn, prev_run_len, lcn);
			if (prev_lcn == lcn - prev_run_len && rl_pos) {
				antfs_log_debug("Cluster coalesce: prev_lcn("
						"%lld) lcn(%lld)"
						"prev_run_len(%lld)",
						(long long)prev_lcn,
						(long long)lcn,
						(long long)prev_run_len);
				rl[rl_pos - 1].length = ++prev_run_len;
			} else {
				if (rl_pos)
					rl[rl_pos].vcn = rl[rl_pos - 1].vcn +
					    prev_run_len;
				else {
					rl[rl_pos].vcn = start_vcn;
					antfs_log_debug("Start_vcn: %lld",
							(long long)start_vcn);
				}

				rl[rl_pos].lcn = prev_lcn = lcn;
				rl[rl_pos].length = prev_run_len = 1;
				antfs_log_debug("2) rl->lcn: %lld",
						rl[rl_pos].lcn);
				rl_pos++;
			}

			antfs_log_debug("RUN:   %-16lld %-16lld %-16lld",
					(long long)rl[rl_pos - 1].vcn,
					(long long)rl[rl_pos - 1].lcn,
					(long long)rl[rl_pos - 1].length);

			/* Done? */
			if (!--clusters) {
				ntfs_cluster_update_zone_pos(vol, search_zone,
							     lcn, file_size);
				goto done_ret;
			}

			bmp_offs++;
			/* Try harder in 2nd pass. If this buffer is really
			 * full, we break out if max_empty_bit_range above
			 * fails.
			 */
			if (pass == 2 && bmp_offs >= buf_size) {
				/* Start over. */
				lcn &= ~block_to_lcnbits_mask;
				if (lcn < zone_start)
					lcn = zone_start;
				bmp_offs = lcn & block_to_lcnbits_mask;
				antfs_force_zone_pos(vol, search_zone, lcn);
			} else {
				lcn++;
			}
		}

		/* === couldn't find free lcn goto next buffer === */
		/* === maybe switch to next zone === */

		/* lcn should always be valid if we get here! */
		/* go through the next bufferhead if we are still in the zone */
		if (lcn < zone_end)
			continue;

		/* Walked whole zone here, didn't find nuffin */
		antfs_log_debug("Finished current zone pass(%i).", pass);
		if (pass == 1) {
zone_pass_skip:
			/* We started in the middle of the zone.
			 * Start from the beginning of the same zone
			 * in the next pass. */
			pass = 2;

			/* zone_start may point to our "guess" here.
			 * Set it to the real zone start. zone_end is already
			 * set correctly here. */
			if (search_zone == ZONE_MFT) {
				zone_start = vol->mft_zone_start;
				vol->mft_zone_pos = zone_start;
			} else if (search_zone == ZONE_DATA1) {
				zone_start = vol->mft_zone_end;
				vol->data1_zone_pos = zone_start;
			} else {
				zone_start = 0;
				vol->data2_zone_pos = 0;
			}

			lcn = zone_start;

			continue;
		}
		/* pass == 2 */
		/* Walked the zone from start to end here:
		 * After that we can say it is full for sure. */
		vol->full_zones |= search_zone;
skip_zone:
		done_zones |= search_zone;
		if (done_zones < (ZONE_MFT + ZONE_DATA1 + ZONE_DATA2)) {
			LCN tmp_lcn;

			antfs_log_debug("Switching zone.");
			pass = 1;

			switch (search_zone) {
			case ZONE_MFT:
				antfs_log_debug("Zone switch: mft -> data1");
switch_to_data1_zone:		search_zone = ZONE_DATA1;
				zone_start = vol->data1_zone_pos;
				zone_end = vol->nr_clusters;
				if (zone_start == vol->mft_zone_end)
					pass = 2;
				break;
			case ZONE_DATA1:
				antfs_log_debug("Zone switch: data1 -> data2");
				search_zone = ZONE_DATA2;
				/* There is no DATA2 */
				if (!vol->mft_zone_start)
					goto skip_zone;
				zone_start = vol->data2_zone_pos;
				zone_end = vol->mft_zone_start;
				if (!zone_start)
					pass = 2;
				break;
			case ZONE_DATA2:
				if (!(done_zones & ZONE_DATA1)) {
					antfs_log_debug("data2 -> data1");
					goto switch_to_data1_zone;
				}
				/* We don't just go to MFT: We shrink MFT
				 * zone instead and try again with DATA1.
				 *
				 * Shrink MFT zone by half of free space.
				 */
				tmp_lcn = (vol->mft_zone_end -
					   vol->mft_zone_start -
					   (vol->mft_na->allocated_size >>
					    vol->cluster_size_bits)) >> 1;
				/* DEBUG */
				antfs_log_debug("shrinking MFT: old_end="
						"0x%llx; new_end=0x%llx",
						(long long)vol->mft_zone_end,
						(long long)(vol->mft_zone_end
							    - tmp_lcn));
				if (tmp_lcn) {
					vol->mft_zone_end -= tmp_lcn;
					done_zones &= ~ZONE_DATA1;
					update_full_status(vol,
							   vol->mft_zone_end);
					goto switch_to_data1_zone;
				}
				/* Shrinking the MFT any further at this point
				 * get's rediculous. MFT is probably full, but
				 * give it a try anyway.
				 */
				antfs_log_debug("Zone switch: data2 -> mft");
				search_zone = ZONE_MFT;
				zone_start = vol->mft_zone_pos;
				zone_end = vol->mft_zone_end;
				if (zone_start == vol->mft_zone_start)
					pass = 2;
				break;
			}

			lcn = zone_start;

			if (zone_start == zone_end) {
				/* If this happens we skip pass 1
				 * and start at the beginning. */
				antfs_log_debug(
					    "Pos at end of zone. Skip pass 1.");
				goto zone_pass_skip;
			}

			continue;
		}

		antfs_log_debug("All zones are finished, no space on device.");
		if (vol->free_clusters) {
			s64 nr_free = ntfs_attr_get_free_bits(vol->lcnbmp_na);

			antfs_log_error("No clusters found but still "
					"remember %lld free clusters?! Counted "
					"lcnbmp again and got %lld free "
					"clusters.\ndone_zones=0x%x; "
					"full_zones=0x%x; pass=%d; "
					"search_zone=%d; lcn=0x%llx; "
					"zone_start=0x%llx; zone_end=0x%llx",
					vol->free_clusters, nr_free,
					(int)done_zones, (int)vol->full_zones,
					(int)pass, (int)search_zone,
					(long long)lcn, (long long)zone_start,
					(long long)zone_end);
		}
		err = -ENOSPC;
		goto err_ret;
	}

	/* === finish up [rl / error code] and return === */
done_ret:
	antfs_log_debug("At done_ret.");
	/* Add runlist terminator element. */
	rl[rl_pos].vcn = rl[rl_pos - 1].vcn + rl[rl_pos - 1].length;
	rl[rl_pos].lcn = LCN_RL_NOT_MAPPED;
	rl[rl_pos].length = 0;
	mutex_unlock(&vol->lcnbmp_lock);
done_err_ret:
	if (err) {
		if (err != -ENOSPC)
			antfs_log_error("Failed to allocate %d clusters. "
					"err: %d", (int)count, err);
		rl = ERR_PTR(err);
	}

out:
	antfs_log_leave("rl->lcn/err(%lld)", IS_ERR_OR_NULL(rl) ?
			(long long)err : rl->lcn);
	return rl;

	/* === error occured, set everything up for error code return === */
err_ret:
	antfs_log_debug("At err_ret.");
	mutex_unlock(&vol->lcnbmp_lock);
	if (rl) {
		/* Add runlist terminator element. */
		rl[rl_pos].vcn = rl[rl_pos - 1].vcn + rl[rl_pos - 1].length;
		rl[rl_pos].lcn = LCN_RL_NOT_MAPPED;
		rl[rl_pos].length = 0;
		ntfs_debug_runlist_dump(rl);
		ntfs_cluster_free_from_rl(vol, rl);
		ntfs_free(rl);
		rl = NULL;
	}
	goto done_err_ret;
}

/**
 * ntfs_cluster_free_from_rl - free clusters from runlist
 * @vol:	mounted ntfs volume on which to free the clusters
 * @rl:		runlist from which deallocate clusters
 *
 * On success return 0 and on error return -1 with errno set to the error code.
 */
int ntfs_cluster_free_from_rl(struct ntfs_volume *vol,
			      struct runlist_element *rl)
{
	s64 nr_freed = 0;
	int ret = 0;

	antfs_log_enter();

	for (; rl->length; rl++) {

		antfs_log_debug("Dealloc lcn 0x%llx, len 0x%llx.",
				(long long)rl->lcn, (long long)rl->length);

		if (rl->lcn >= 0) {
			ret = ntfs_lcn_bitmap_clear_run(vol, rl->lcn,
							rl->length);
			if (ret) {
				antfs_log_error("Cluster deallocation failed "
						"(%lld, %lld)",
						(long long)rl->lcn,
						(long long)rl->length);
				goto out;
			}
			nr_freed += rl->length;
			antfs_log_debug("Inc clusters: lcn: %lld; len: %lld",
					(long long)rl->lcn,
					(long long)rl->length);
		}
	}
out:
	vol->free_clusters += nr_freed;
	if (vol->free_clusters > vol->nr_clusters) {
		antfs_logger(vol->dev->d_sb->s_id,
				"Too many free clusters (%lld > %lld)!",
				(long long)vol->free_clusters,
				(long long)vol->nr_clusters);
	}
	return ret;
}

/**
 * @brief Clear lcn in $Bitmap
 *
 * Clears bit in $Bitsmap corresponding to lcn. Used mostly with data attributes
 *
 * @param vol NTFS volume
 * @param lcn LCN to clear
 * @param count number of contiguous lcns to clear
 *
 * @return 0 on success or negative error code.
 */
int ntfs_lcn_bitmap_clear_run(struct ntfs_volume *vol, LCN lcn, int count)
{
	size_t b_size_left;
	LCN curr_bh_pos = lcn >> (vol->dev->d_sb->s_blocksize_bits + 3);
	int bit;
	u8 *byte;
	int i, rwnd, err = 0;

	antfs_log_enter("lcn(%lld), count(%d)", (long long)lcn, count);
	if (mutex_lock_interruptible(&vol->lcnbmp_lock)) {
		err = -EBUSY;
		goto out;
	}
	if (!vol->lcnbmp_bh || (curr_bh_pos != vol->lcnbmp_start)) {
		if (vol->lcnbmp_bh)
			brelse(vol->lcnbmp_bh);
		vol->lcnbmp_bh = ntfs_load_bitmap_attr(vol,
						       vol->lcnbmp_na, lcn);
		if (IS_ERR_OR_NULL(vol->lcnbmp_bh)) {
			err = PTR_ERR(vol->lcnbmp_bh);
			if (err == 0)
				err = -EIO;
			antfs_log_error("Reading LCN bitmap failed: %d", err);
			vol->lcnbmp_bh = NULL;
			goto out_locked;
		}
		vol->lcnbmp_start = curr_bh_pos;
	}

	bit = lcn & 7;
	/* Byte offset into buffer */
	b_size_left = (lcn >> 3) & (vol->dev->d_sb->s_blocksize - 1);
	byte = vol->lcnbmp_bh->b_data + b_size_left;
	/* Bytes left in buffer */
	b_size_left = vol->lcnbmp_bh->b_size - b_size_left;
	for (i = 0; i < count; i++, bit++) {
		if (bit == 8) {
			bit = 0;
			byte++;
			b_size_left--;
		}
		/* in case we exceed the current buffer_head we need to
		 * switch to the next buffer_head!
		 */
		if (!b_size_left) {
			mark_buffer_dirty(vol->lcnbmp_bh);
			brelse(vol->lcnbmp_bh);
			vol->lcnbmp_bh = ntfs_load_bitmap_attr(vol,
							       vol->lcnbmp_na,
							       lcn + i);
			if (IS_ERR_OR_NULL(vol->lcnbmp_bh)) {
				err = PTR_ERR(vol->lcnbmp_bh);
				if (err == 0)
					err = -EIO;
				antfs_log_error("Failed to read the LCN bitmap"
						"while deleting! -> rewinding: "
						"%d", err);
				vol->lcnbmp_bh = NULL;
				goto rewind;
			}
			vol->lcnbmp_start++;
			byte = vol->lcnbmp_bh->b_data;
			b_size_left = vol->lcnbmp_bh->b_size;
		}
		*byte &= ~(1 << bit);
	}
	update_full_status(vol, lcn);
out_locked:
	mutex_unlock(&vol->lcnbmp_lock);
out:
	if (vol->lcnbmp_bh)
		mark_buffer_dirty(vol->lcnbmp_bh);

	if (vol->free_clusters > vol->nr_clusters) {
		antfs_logger(vol->dev->d_sb->s_id,
				"Too many free clusters (%lld > %lld)!",
				(long long)vol->free_clusters,
				(long long)vol->nr_clusters);
	}

	antfs_log_leave("err: %d", err);
	return err;

rewind:
	rwnd = i + 1;
	if (curr_bh_pos != vol->lcnbmp_start) {
		brelse(vol->lcnbmp_bh);
		vol->lcnbmp_bh = ntfs_load_bitmap_attr(vol,
						       vol->lcnbmp_na, lcn);
		if (IS_ERR_OR_NULL(vol->lcnbmp_bh)) {
			err = PTR_ERR(vol->lcnbmp_bh);
			if (err == 0)
				err = -EIO;
			antfs_logger(vol->dev->d_sb->s_id,
				"Rewind: Reading LCN bitmap failed: %d", err);
			vol->lcnbmp_bh = NULL;
			goto out_locked;
		}
		vol->lcnbmp_start = curr_bh_pos;
	}

	if (!vol->lcnbmp_bh)
		goto out_locked;

	bit = lcn & 7;
	b_size_left = (lcn >> 3) & (vol->dev->d_sb->s_blocksize - 1);
	byte = vol->lcnbmp_bh->b_data + b_size_left;
	b_size_left = vol->lcnbmp_bh->b_size - b_size_left;
	for (i = 0; i < rwnd; i++, bit++) {
		if (bit == 8) {
			bit = 0;
			byte++;
			b_size_left--;
		}
		/* in case we exceed the current buffer_head we need to
		 * switch to the next buffer_head!
		 */
		if (!b_size_left) {
			mark_buffer_dirty(vol->lcnbmp_bh);
			brelse(vol->lcnbmp_bh);
			vol->lcnbmp_bh = ntfs_load_bitmap_attr(vol,
							       vol->lcnbmp_na,
							       lcn + i);
			if (IS_ERR_OR_NULL(vol->lcnbmp_bh)) {
				err = PTR_ERR(vol->lcnbmp_bh);
				if (err == 0)
					err = -EIO;
				antfs_logger(vol->dev->d_sb->s_id,
					"Failed to read the bitmap while "
					"rewinding! %d", err);
				vol->lcnbmp_bh = NULL;
				goto out_locked;
			}
			vol->lcnbmp_start++;
			byte = vol->lcnbmp_bh->b_data;
			b_size_left = vol->lcnbmp_bh->b_size;
		}
		*byte |= (1 << bit);
	}
	goto out_locked;
}

/*
 *		Basic cluster run free
 *	Returns 0 if successful
 */

int ntfs_cluster_free_basic(struct ntfs_volume *vol, s64 lcn, s64 count)
{
	s64 nr_freed = 0;
	int ret = 0;

	antfs_log_enter("Dealloc lcn 0x%llx, len 0x%llx.",
			(long long)lcn, (long long)count);

	if (lcn >= 0) {
		ret = ntfs_lcn_bitmap_clear_run(vol, lcn, count);
		if (ret) {
			antfs_log_error("Cluster deallocation failed "
					"(%lld, %lld)",
					(long long)lcn, (long long)count);
			goto out;
		}
		nr_freed += count;
	}
out:
	vol->free_clusters += nr_freed;
	if (vol->free_clusters > vol->nr_clusters) {
		antfs_logger(vol->dev->d_sb->s_id,
				"Too many free clusters (%lld > %lld)!",
				(long long) vol->free_clusters,
				(long long) vol->nr_clusters);
	}
	return ret;
}

/**
 * ntfs_cluster_free - free clusters on an ntfs volume
 * @vol:	mounted ntfs volume on which to free the clusters
 * @na:		attribute whose runlist describes the clusters to free
 * @start_vcn:	vcn in @rl at which to start freeing clusters
 * @count:	number of clusters to free or -1 for all clusters
 *
 * Free @count clusters starting at the cluster @start_vcn in the runlist
 * described by the attribute @na from the mounted ntfs volume @vol.
 *
 * If @count is -1, all clusters from @start_vcn to the end of the runlist
 * are deallocated.
 *
 * On success return the number of deallocated clusters (not counting sparse
 * clusters) and on error return -1 with errno set to the error code.
 */
int ntfs_cluster_free(struct ntfs_volume *vol, struct ntfs_attr *na,
		      VCN start_vcn, s64 count)
{
	struct runlist_element *rl;
	s64 delta, to_free, nr_freed = 0;
	int err;

	if (!vol || !vol->lcnbmp_na || !na || start_vcn < 0 ||
	    (count < 0 && count != -1)) {
		antfs_log_debug("Invalid arguments!");
		return -EINVAL;
	}

	antfs_log_enter("Entering for inode 0x%llx, attr 0x%x, count 0x%lld, "
			"vcn 0x%llx.", (unsigned long long)na->ni->mft_no,
			le32_to_cpu(na->type), (long long)count,
			(long long)start_vcn);

	rl = ntfs_attr_find_vcn(na, start_vcn);
	if (IS_ERR_OR_NULL(rl)) {
		err = PTR_ERR(rl);
		if (err == -ENOENT)
			err = 0;
		else
			antfs_log_error("Could not find run: %d", err);
		goto leave;
	}

	if (rl->lcn < 0 && rl->lcn != LCN_HOLE) {
		antfs_log_error("Unexpected lcn (%lld)", (long long)rl->lcn);
		err = -EIO;
		goto leave;
	}

	/* Find the starting cluster inside the run that needs freeing. */
	delta = start_vcn - rl->vcn;

	/* The number of clusters in this run that need freeing. */
	to_free = rl->length - delta;
	if (count >= 0 && to_free > count)
		to_free = count;

	if (rl->lcn != LCN_HOLE) {
		/* Do the actual freeing of the clusters in this run. */
		err = ntfs_lcn_bitmap_clear_run(vol, rl->lcn + delta, to_free);
		if (err)
			goto leave;
		nr_freed = to_free;
	}

	/* Go to the next run and adjust the number of clusters left to free. */
	++rl;
	if (count >= 0)
		count -= to_free;

	/*
	 * Loop over the remaining runs, using @count as a capping value, and
	 * free them.
	 */
	for (; rl->length && count != 0; ++rl) {
		/* FIXME: Need to try ntfs_attr_map_runlist() for attribute
		 *        list support! (AIA) */
		if (rl->lcn < 0 && rl->lcn != LCN_HOLE) {
			/* FIXME: Eeek! We need rollback! (AIA) */
			antfs_log_error("Invalid lcn (%lli)",
					(long long)rl->lcn);
			err = -EIO;
			goto out;
		}

		/* The number of clusters in this run that need freeing. */
		to_free = rl->length;
		if (count >= 0 && to_free > count)
			to_free = count;

		if (rl->lcn != LCN_HOLE) {
			err = ntfs_lcn_bitmap_clear_run(vol, rl->lcn, to_free);
			if (err) {
				/* FIXME: Eeek! We need rollback! (AIA) */
				antfs_log_error("Clearing bitmap run failed");
				goto out;
			}
			nr_freed += to_free;
		}

		if (count >= 0)
			count -= to_free;
	}

	if (count != -1 && count != 0) {
		/* FIXME: Eeek! BUG() */
		antfs_log_error("Count still not zero(%lld)", (long long)count);
		err = -EIO;
		goto out;
	}

	err = nr_freed;
out:
	vol->free_clusters += nr_freed;
	if (vol->free_clusters > vol->nr_clusters) {
		antfs_logger(vol->dev->d_sb->s_id,
			"Too many free clusters (%lld > %lld)!",
			(long long)vol->free_clusters,
			(long long)vol->nr_clusters);
	}
leave:
	antfs_log_leave();
	return err;
}
