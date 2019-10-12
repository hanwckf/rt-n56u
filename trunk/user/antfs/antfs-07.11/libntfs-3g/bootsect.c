/**
 * bootsect.c - Boot sector handling code.
 * Originated from the Linux-NTFS project.
 *
 * Copyright (c) 2000-2006 Anton Altaparmakov
 * Copyright (c) 2003-2008 Szabolcs Szakacsits
 * Copyright (c)      2005 Yura Pakhuchiy
 * Copyright (c)      2016 Martin Pommerenke, Jens Krieg, Arwed Meyer,
 *		           Christian René Sechting
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

#include <linux/fs.h>

#include "antfs.h"
#include "bootsect.h"
#include "debug.h"

/**
 * ntfs_boot_sector_is_ntfs - check if buffer contains a valid ntfs boot sector
 *
 * @param b       buffer containing putative boot sector to analyze
 * @param silent  if zero, output progress messages to stderr
 *
 * Check if the buffer @b contains a valid ntfs boot sector. The buffer @b
 * must be at least 512 bytes in size.
 *
 * If @silent is zero, output progress messages to stderr. Otherwise, do not
 * output any messages (except when configured with --enable-debug in which
 * case warning/debug messages may be displayed).
 *
 * @retval @TRUE if @b contains a valid ntfs boot sector
 * @retval @FALSE if not.
 */
bool ntfs_boot_sector_is_ntfs(struct NTFS_BOOT_SECTOR *b)
{
	u32 i;
	bool ret = FALSE;

	antfs_log_debug("Beginning bootsector check.");

	antfs_log_debug("Checking OEMid, NTFS signature.");
	if (b->oem_id != const_cpu_to_le64(0x202020205346544eULL)) {/* "NTFS" */
		antfs_log_error("NTFS signature is missing.");
		goto not_ntfs;
	}

	antfs_log_debug("Checking bytes per sector.");
	if (le16_to_cpu(b->bpb.bytes_per_sector) < 256 ||
	    le16_to_cpu(b->bpb.bytes_per_sector) > 4096) {
		antfs_log_error("Unexpected bytes per sector value (%d).",
				le16_to_cpu(b->bpb.bytes_per_sector));
		goto not_ntfs;
	}

	antfs_log_debug("Checking sectors per cluster.");
	switch (b->bpb.sectors_per_cluster) {
	case 1:
	case 2:
	case 4:
	case 8:
	case 16:
	case 32:
	case 64:
	case 128:
		break;
	default:
		antfs_log_error("Unexpected sectors per cluster value (%d).",
				b->bpb.sectors_per_cluster);
		goto not_ntfs;
	}

	antfs_log_debug("Checking cluster size.");
	i = (u32) le16_to_cpu(b->bpb.bytes_per_sector) *
	    b->bpb.sectors_per_cluster;
	if (i > 65536) {
		antfs_log_error("Unexpected cluster size (%d).", i);
		goto not_ntfs;
	}

	antfs_log_debug("Checking reserved fields are zero.");
	if (le16_to_cpu(b->bpb.reserved_sectors) ||
	    le16_to_cpu(b->bpb.root_entries) ||
	    le16_to_cpu(b->bpb.sectors) ||
	    le16_to_cpu(b->bpb.sectors_per_fat) ||
	    le32_to_cpu(b->bpb.large_sectors) || b->bpb.fats) {
		antfs_log_error("Reserved fields aren't zero "
				"(%d, %d, %d, %d, %d, %d).",
				le16_to_cpu(b->bpb.reserved_sectors),
				le16_to_cpu(b->bpb.root_entries),
				le16_to_cpu(b->bpb.sectors),
				le16_to_cpu(b->bpb.sectors_per_fat),
				le32_to_cpu(b->bpb.large_sectors), b->bpb.fats);
		goto not_ntfs;
	}

	antfs_log_debug("Checking clusters per mft record.");
	if ((u8) b->clusters_per_mft_record < 0xe1 ||
	    (u8) b->clusters_per_mft_record > 0xf7) {
		switch (b->clusters_per_mft_record) {
		case 1:
		case 2:
		case 4:
		case 8:
		case 0x10:
		case 0x20:
		case 0x40:
			break;
		default:
			antfs_log_error("Unexpected clusters per mft record "
					"(%d).", b->clusters_per_mft_record);
			goto not_ntfs;
		}
	}

	antfs_log_debug("Checking clusters per index block.");
	if ((u8) b->clusters_per_index_record < 0xe1 ||
	    (u8) b->clusters_per_index_record > 0xf7) {
		switch (b->clusters_per_index_record) {
		case 1:
		case 2:
		case 4:
		case 8:
		case 0x10:
		case 0x20:
		case 0x40:
			break;
		default:
			antfs_log_error("Unexpected clusters per index record "
					"(%d).",
					b->clusters_per_index_record);
			goto not_ntfs;
		}
	}

	if (b->end_of_sector_marker != const_cpu_to_le16(0xaa55)) {
		antfs_log_debug("Warning: Bootsector has invalid end of sector "
				"marker.");
	}

	antfs_log_debug("Bootsector check completed successfully.");

	ret = TRUE;
not_ntfs:
	return ret;
}

static const char *last_sector_error =
	"HINTS: Either the volume is a RAID/LDM but it wasn't setup yet,\n"
	"   or it was not setup correctly (e.g. by not using mdadm --build ...)"
	",\n   or a wrong device is tried to be mounted,\n"
	"   or the partition table is corrupt (partition is smaller than NTFS),"
	"\n   or the NTFS boot sector is corrupt (NTFS size is not valid).\n";

/**
 * ntfs_boot_sector_parse - setup an ntfs volume from an ntfs boot sector
 *
 * @param vol  ntfs_volume to setup
 * @param bs   buffer containing ntfs boot sector to parse
 *
 * Parse the ntfs bootsector @bs and setup the ntfs volume @vol with the
 * obtained values.
 *
 * @retval 0 on success
 * @retval negative error code on failure
 */
int ntfs_boot_sector_parse(struct ntfs_volume *vol,
			   const struct NTFS_BOOT_SECTOR *bs)
{
	s64 sectors, tmp_sectors;
	u8 sectors_per_cluster;
	s8 c;
	int err;

	vol->sector_size = le16_to_cpu(bs->bpb.bytes_per_sector);
	vol->sector_size_bits = ffs(vol->sector_size) - 1;
	antfs_log_debug("SectorSize = 0x%x; SectorSizeBits = %u",
			vol->sector_size, vol->sector_size_bits);
	/*
	 * The bounds checks on mft_lcn and mft_mirr_lcn (i.e. them being
	 * below or equal the number_of_clusters) really belong in the
	 * ntfs_boot_sector_is_ntfs but in this way we can just do this once.
	 */
	sectors_per_cluster = bs->bpb.sectors_per_cluster;
	antfs_log_debug("SectorsPerCluster = 0x%x", sectors_per_cluster);
	if (sectors_per_cluster & (sectors_per_cluster - 1)) {
		antfs_log_error("sectors_per_cluster (%d) is not a power of 2."
				"", sectors_per_cluster);
		return -EINVAL;
	}

	sectors = sle64_to_cpu(bs->number_of_sectors);
	antfs_log_debug("NumberOfSectors = %lld", (long long)sectors);
	if (!sectors) {
		antfs_log_error("Volume size is set to zero.");
		return -EINVAL;
	}

	tmp_sectors = vol->dev->d_ops->seek(vol->dev,
					    (sectors -
					     1) << vol->sector_size_bits,
					    SEEK_SET);
	if (tmp_sectors < 0) {
		err = (int)tmp_sectors;
		antfs_log_error("Failed to read last sector (%lld) (err=%d)",
				(long long)(sectors - 1), err);
		antfs_log_error("%s", last_sector_error);
		return err;
	}

	vol->nr_clusters = sectors >> (ffs(sectors_per_cluster) - 1);

	vol->mft_lcn = sle64_to_cpu(bs->mft_lcn);
	vol->mftmirr_lcn = sle64_to_cpu(bs->mftmirr_lcn);
	antfs_log_debug("MFT LCN = %lld; MFTMirr LCN = %lld",
			(long long)vol->mft_lcn, (long long)vol->mftmirr_lcn);
	if ((vol->mft_lcn < 0 || vol->mft_lcn > vol->nr_clusters) ||
	    (vol->mftmirr_lcn < 0 || vol->mftmirr_lcn > vol->nr_clusters)) {
		antfs_log_error("$MFT LCN (%lld) or $MFTMirr LCN (%lld) is "
				"greater than the number of clusters (%lld).",
				(long long)vol->mft_lcn,
				(long long)vol->mftmirr_lcn,
				(long long)vol->nr_clusters);
		return -EINVAL;
	}

	vol->cluster_size = sectors_per_cluster * vol->sector_size;
	if (vol->cluster_size & (vol->cluster_size - 1)) {
		antfs_log_error("cluster_size (%d) is not a power of 2.",
				vol->cluster_size);
		return -EINVAL;
	}
	vol->cluster_size_bits = ffs(vol->cluster_size) - 1;
	/*
	 * Need to get the clusters per mft record and handle it if it is
	 * negative. Then calculate the mft_record_size. A value of 0x80 is
	 * illegal, thus signed char is actually ok!
	 */
	c = bs->clusters_per_mft_record;
	antfs_log_debug("ClusterSize = 0x%x; ClusterSizeBits = %u; "
			"ClustersPerMftRecord = 0x%x",
			(unsigned)vol->cluster_size, vol->cluster_size_bits, c);
	/*
	 * When clusters_per_mft_record is negative, it means that it is to
	 * be taken to be the negative base 2 logarithm of the mft_record_size
	 * min bytes. Then:
	 *       mft_record_size = 2^(-clusters_per_mft_record) bytes.
	 */
	if (c < 0)
		vol->mft_record_size = 1 << -c;
	else
		vol->mft_record_size = c << vol->cluster_size_bits;
	if (vol->mft_record_size & (vol->mft_record_size - 1)) {
		antfs_log_error("mft_record_size (%d) is not a power of 2.",
				vol->mft_record_size);
		return -EINVAL;
	}
	vol->mft_record_size_bits = ffs(vol->mft_record_size) - 1;
	antfs_log_debug("MftRecordSize = 0x%x; MftRecordSizeBits = %u",
			vol->mft_record_size_bits,
			(unsigned)vol->mft_record_size);
	/* Same as above for INDX record. */
	c = bs->clusters_per_index_record;
	antfs_log_debug("ClustersPerINDXRecord = 0x%x", c);
	if (c < 0)
		vol->indx_record_size = 1 << -c;
	else
		vol->indx_record_size = c << vol->cluster_size_bits;
	vol->indx_record_size_bits = ffs(vol->indx_record_size) - 1;
	antfs_log_debug("INDXRecordSize = 0x%x; INDXRecordSizeBits = %u",
			(unsigned)vol->indx_record_size,
			vol->indx_record_size_bits);
	vol->serial_no = sle64_to_cpu(bs->volume_serial_number);
	/*
	 * Work out the size of the MFT mirror in number of mft records. If the
	 * cluster size is less than or equal to the size taken by four mft
	 * records, the mft mirror stores the first four mft records. If the
	 * cluster size is bigger than the size taken by four mft records, the
	 * mft mirror contains as many mft records as will fit into one
	 * cluster.
	 */
	if (vol->cluster_size <= 4 * vol->mft_record_size)
		vol->mftmirr_size = 4;
	else
		vol->mftmirr_size = vol->cluster_size / vol->mft_record_size;
	return 0;
}
