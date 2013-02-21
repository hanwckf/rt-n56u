/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2004-2005, 2007, 2009-2012 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DISCOVER_ONLY

#include <config.h>

#include <parted/parted.h>
#include <parted/endian.h>
#include <parted/debug.h>
#include <stdint.h>

#if ENABLE_NLS
#  include <libintl.h>
#  define _(String) dgettext (PACKAGE, String)
#else
#  define _(String) (String)
#endif /* ENABLE_NLS */

#include "hfs.h"
#include "reloc_plus.h"

#include "journal.h"

static int hfsj_vh_replayed = 0;
static int is_le = 0;

static uint32_t
hfsj_calc_checksum(uint8_t *ptr, int len)
{
	int	 i;
	uint32_t cksum=0;

	for (i=0; i < len; i++, ptr++) {
		cksum = (cksum << 8) ^ (cksum + *ptr);
	}

	return (~cksum);
}

int
hfsj_update_jib(PedFileSystem* fs, uint32_t block)
{
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						    fs->type_specific;

	priv_data->vh->journal_info_block = PED_CPU_TO_BE32(block);

	if (!hfsplus_update_vh (fs))
		return 0;

	priv_data->jib_start_block = block;
	return 1;
}

int
hfsj_update_jl(PedFileSystem* fs, uint32_t block)
{
	uint8_t			buf[PED_SECTOR_SIZE_DEFAULT];
	PedSector		sector;
	uint64_t		offset;
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						    fs->type_specific;
	HfsJJournalInfoBlock*	jib;
	int			binsect;

	binsect = HFS_32_TO_CPU(priv_data->vh->block_size, is_le) / PED_SECTOR_SIZE_DEFAULT;
	sector = (PedSector) priv_data->jib_start_block * binsect;
	if (!ped_geometry_read(priv_data->plus_geom, buf, sector, 1))
		return 0;
	jib = (HfsJJournalInfoBlock*) buf;

	offset = (uint64_t)block * PED_SECTOR_SIZE_DEFAULT * binsect;
	jib->offset = HFS_CPU_TO_64(offset, is_le);

	if (!ped_geometry_write(priv_data->plus_geom, buf, sector, 1)
	    || !ped_geometry_sync(priv_data->plus_geom))
		return 0;

	priv_data->jl_start_block = block;
	return 1;
}

/* Return the sector in the journal that is after the area read */
/* or 0 on error */
static PedSector
hfsj_journal_read(PedGeometry* geom, HfsJJournalHeader* jh,
		  PedSector journ_sect, PedSector journ_length,
		  PedSector read_sect, unsigned int nb_sect,
		  void* buf)
{
	int r;

	while (nb_sect--) {
		r = ped_geometry_read(geom, buf, journ_sect + read_sect, 1);
		if (!r) return 0;

		buf = ((uint8_t*)buf) + PED_SECTOR_SIZE_DEFAULT;
		read_sect++;
		if (read_sect == journ_length)
			read_sect = 1; /* skip journal header
					  which is asserted to be
					  1 sector long */
	}

	return read_sect;
}

static int
hfsj_replay_transaction(PedFileSystem* fs, HfsJJournalHeader* jh,
			PedSector jsector, PedSector jlength)
{
	PedSector		start, sector;
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						fs->type_specific;
	HfsJBlockListHeader*	blhdr;
	uint8_t*		block;
	unsigned int		blhdr_nbsect;
	int			i, r;
	uint32_t		cksum, size;

	blhdr_nbsect = HFS_32_TO_CPU(jh->blhdr_size, is_le) / PED_SECTOR_SIZE_DEFAULT;
	blhdr = (HfsJBlockListHeader*)
		  ped_malloc (blhdr_nbsect * PED_SECTOR_SIZE_DEFAULT);
	if (!blhdr) return 0;

	start = HFS_64_TO_CPU(jh->start, is_le) / PED_SECTOR_SIZE_DEFAULT;
	do {
		start = hfsj_journal_read(priv_data->plus_geom, jh, jsector,
					  jlength, start, blhdr_nbsect, blhdr);
		if (!start) goto err_replay;

		cksum = HFS_32_TO_CPU(blhdr->checksum, is_le);
		blhdr->checksum = 0;
		if (cksum!=hfsj_calc_checksum((uint8_t*)blhdr, sizeof(*blhdr))){
			ped_exception_throw (
				PED_EXCEPTION_ERROR,
				PED_EXCEPTION_CANCEL,
				_("Bad block list header checksum."));
			goto err_replay;
		}
		blhdr->checksum = HFS_CPU_TO_32(cksum, is_le);

		for (i=1; i < HFS_16_TO_CPU(blhdr->num_blocks, is_le); ++i) {
			size = HFS_32_TO_CPU(blhdr->binfo[i].bsize, is_le);
			sector = HFS_64_TO_CPU(blhdr->binfo[i].bnum, is_le);
			if (!size) continue;
			if (size % PED_SECTOR_SIZE_DEFAULT) {
				ped_exception_throw(
					PED_EXCEPTION_ERROR,
					PED_EXCEPTION_CANCEL,
					_("Invalid size of a transaction "
					  "block while replaying the journal "
					  "(%i bytes)."),
					size);
				goto err_replay;
			}
			block = (uint8_t*) ped_malloc(size);
			if (!block) goto err_replay;
			start = hfsj_journal_read(priv_data->plus_geom, jh,
						  jsector, jlength, start,
						  size / PED_SECTOR_SIZE_DEFAULT,
						  block);
			if (!start) {
				free (block);
				goto err_replay;
			}
			/* the sector stored in the journal seems to be
			   relative to the begin of the block device which
			   contains the hfs+ journaled volume */
			if (sector != ~0LL)
				r = ped_geometry_write (fs->geom, block, sector,
							size / PED_SECTOR_SIZE_DEFAULT);
			else
				r = 1;
			free (block);
			/* check if wrapper mdb or vh with no wrapper has
			   changed */
			if (   (sector != ~0LL)
			    && (2 >= sector)
			    && (2 < sector + size / PED_SECTOR_SIZE_DEFAULT) )
				hfsj_vh_replayed = 1;
			/* check if vh of embedded hfs+ has changed */
			if (   (sector != ~0LL)
			    && (priv_data->plus_geom != fs->geom)
			    && (sector
			        + fs->geom->start
				- priv_data->plus_geom->start <= 2)
			    && (sector
			        + size / PED_SECTOR_SIZE_DEFAULT
				+ fs->geom->start
				- priv_data->plus_geom->start > 2) )
				hfsj_vh_replayed = 1;
			if (!r) goto err_replay;
		}
	} while (blhdr->binfo[0].next);

	jh->start = HFS_CPU_TO_64(start * PED_SECTOR_SIZE_DEFAULT, is_le);

	free (blhdr);
	return (ped_geometry_sync (fs->geom));

err_replay:
	free (blhdr);
	return 0;
}

/* 0 => Failure, don't continue to open ! */
/* 1 => Success, the journal has been completly replayed, or don't need to */
int
hfsj_replay_journal(PedFileSystem* fs)
{
	uint8_t			buf[PED_SECTOR_SIZE_DEFAULT];
	PedSector		sector, length;
	HfsPPrivateFSData* 	priv_data = (HfsPPrivateFSData*)
						    fs->type_specific;
	HfsJJournalInfoBlock*	jib;
	HfsJJournalHeader*	jh;
	int			binsect;
	uint32_t		cksum;

	binsect = PED_BE32_TO_CPU(priv_data->vh->block_size) / PED_SECTOR_SIZE_DEFAULT;
	priv_data->jib_start_block =
		PED_BE32_TO_CPU(priv_data->vh->journal_info_block);
	sector  = (PedSector) priv_data->jib_start_block * binsect;
	if (!ped_geometry_read(priv_data->plus_geom, buf, sector, 1))
		return 0;
	jib = (HfsJJournalInfoBlock*) buf;

	if (    (jib->flags & PED_CPU_TO_BE32(1 << HFSJ_JOURN_IN_FS))
	    && !(jib->flags & PED_CPU_TO_BE32(1 << HFSJ_JOURN_OTHER_DEV)) ) {
		priv_data->jl_start_block = HFS_64_TO_CPU(jib->offset, is_le)
					    / ( PED_SECTOR_SIZE_DEFAULT * binsect );
	}

	if (jib->flags & PED_CPU_TO_BE32(1 << HFSJ_JOURN_NEED_INIT))
		return 1;

	if (  !(jib->flags & PED_CPU_TO_BE32(1 << HFSJ_JOURN_IN_FS))
	    || (jib->flags & PED_CPU_TO_BE32(1 << HFSJ_JOURN_OTHER_DEV)) ) {
		ped_exception_throw (
			PED_EXCEPTION_NO_FEATURE,
			PED_EXCEPTION_CANCEL,
			_("Journal stored outside of the volume are "
			  "not supported.  Try to desactivate the "
			  "journal and run Parted again."));
		return 0;
	}

	if (   (PED_BE64_TO_CPU(jib->offset) % PED_SECTOR_SIZE_DEFAULT)
	    || (PED_BE64_TO_CPU(jib->size)   % PED_SECTOR_SIZE_DEFAULT) ) {
		ped_exception_throw (
			PED_EXCEPTION_NO_FEATURE,
			PED_EXCEPTION_CANCEL,
			_("Journal offset or size is not multiple of "
			  "the sector size."));
		return 0;
	}

	sector = PED_BE64_TO_CPU(jib->offset) / PED_SECTOR_SIZE_DEFAULT;
	length = PED_BE64_TO_CPU(jib->size)   / PED_SECTOR_SIZE_DEFAULT;

	jib = NULL;
	if (!ped_geometry_read(priv_data->plus_geom, buf, sector, 1))
		return 0;
	jh = (HfsJJournalHeader*) buf;

	if (jh->endian == PED_LE32_TO_CPU(HFSJ_ENDIAN_MAGIC))
	    is_le = 1;

	if (   (jh->magic  != HFS_32_TO_CPU(HFSJ_HEADER_MAGIC, is_le))
	    || (jh->endian != HFS_32_TO_CPU(HFSJ_ENDIAN_MAGIC, is_le)) ) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Incorrect magic values in the journal header."));
		return 0;
	}

	if ( (HFS_64_TO_CPU(jh->size, is_le)%PED_SECTOR_SIZE_DEFAULT)
	  || (HFS_64_TO_CPU(jh->size, is_le)/PED_SECTOR_SIZE_DEFAULT
		  != (uint64_t)length) ) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Journal size mismatch between journal info block "
			  "and journal header."));
		return 0;
	}

	if (   (HFS_64_TO_CPU(jh->start, is_le) % PED_SECTOR_SIZE_DEFAULT)
	    || (HFS_64_TO_CPU(jh->end, is_le)   % PED_SECTOR_SIZE_DEFAULT)
	    || (HFS_32_TO_CPU(jh->blhdr_size, is_le) % PED_SECTOR_SIZE_DEFAULT)
	    || (HFS_32_TO_CPU(jh->jhdr_size, is_le)  % PED_SECTOR_SIZE_DEFAULT) ) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Some header fields are not multiple of the sector "
			  "size."));
		return 0;
	}

	if (HFS_32_TO_CPU(jh->jhdr_size, is_le) != PED_SECTOR_SIZE_DEFAULT) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("The sector size stored in the journal is not 512 "
			  "bytes.  Parted only supports 512 bytes length "
			  "sectors."));
		return 0;
	}

	cksum = HFS_32_TO_CPU(jh->checksum, is_le);
	jh->checksum = 0;
	if (cksum != hfsj_calc_checksum((uint8_t*)jh, sizeof(*jh))) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Bad journal checksum."));
		return 0;
	}
	jh->checksum = HFS_CPU_TO_32(cksum, is_le);

	/* The 2 following test are in the XNU Darwin source code */
	/* so I assume they're needed */
	if (jh->start == jh->size)
		jh->start = HFS_CPU_TO_64(PED_SECTOR_SIZE_DEFAULT, is_le);
	if (jh->end   == jh->size)
		jh->start = HFS_CPU_TO_64(PED_SECTOR_SIZE_DEFAULT, is_le);

	if (jh->start == jh->end)
		return 1;

	if (ped_exception_throw (
		PED_EXCEPTION_WARNING,
		PED_EXCEPTION_FIX | PED_EXCEPTION_CANCEL,
		_("The journal is not empty.  Parted must replay the "
		  "transactions before opening the file system.  This will "
		  "modify the file system."))
			!= PED_EXCEPTION_FIX)
		return 0;

	while (jh->start != jh->end) {
		/* Replay one complete transaction */
		if (!hfsj_replay_transaction(fs, jh, sector, length))
			return 0;

		/* Recalculate cksum of the journal header */
		jh->checksum = 0; /* need to be 0 while calculating the cksum */
		cksum = hfsj_calc_checksum((uint8_t*)jh, sizeof(*jh));
		jh->checksum = HFS_CPU_TO_32(cksum, is_le);

		/* Update the Journal Header */
		if (!ped_geometry_write(priv_data->plus_geom, buf, sector, 1)
		    || !ped_geometry_sync(priv_data->plus_geom))
			return 0;
	}

	if (hfsj_vh_replayed) {
		/* probe could have reported incorrect info ! */
		/* is there a way to ask parted to quit ? */
		ped_exception_throw(
			PED_EXCEPTION_WARNING,
			PED_EXCEPTION_OK,
			_("The volume header or the master directory block has "
			  "changed while replaying the journal.  You should "
			  "restart Parted."));
		return 0;
	}

	return 1;
}

#endif /* DISCOVER_ONLY */
