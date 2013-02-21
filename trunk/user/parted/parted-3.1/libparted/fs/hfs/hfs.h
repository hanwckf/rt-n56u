/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2003-2005, 2007, 2009-2012 Free Software Foundation, Inc.

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

#ifndef _HFS_H
#define _HFS_H

/* WARNING : bn is used 2 times in theses macro */
/* so _never_ use side effect operators when using them */
#define TST_BLOC_OCCUPATION(tab,bn) \
	(((tab)[(bn)/8])  &  (1<<(7-((bn)&7))))
#define SET_BLOC_OCCUPATION(tab,bn) \
	(((tab)[(bn)/8]) |=  (1<<(7-((bn)&7))))
#define CLR_BLOC_OCCUPATION(tab,bn) \
	(((tab)[(bn)/8]) &= ~(1<<(7-((bn)&7))))

/* Maximum number of blocks for the copy buffers */
#define BLOCK_MAX_BUFF	256
/* Maximum size of the copy buffers, in bytes */
#define BYTES_MAX_BUFF	8388608

/* Apple Creator Codes follow */
#define HFSP_IMPL_Shnk	0x53686e6b	/* in use */
#define HFSP_IMPL_Xpnd	0x58706e64	/* reserved */
#define HFSP_IMPL_Resz	0x5265737a	/* reserved */
#define HFSP_IMPL_PHpx	0x50482b78	/* reserved */
#define HFSP_IMPL_traP  0x74726150	/* reserved */
#define HFSP_IMPL_GnuP  0x476e7550	/* reserved */

#define HFS_SIGNATURE	0x4244		/* 'BD' */
#define HFSP_SIGNATURE	0x482B		/* 'H+' */
#define HFSX_SIGNATURE  0x4858		/* 'HX' */

#define HFSP_VERSION	 4
#define HFSX_VERSION	 5

#define HFS_HARD_LOCK	 7
#define HFS_UNMOUNTED	 8
#define HFS_BAD_SPARED	 9
#define HFS_SOFT_LOCK	15
#define HFSP_NO_CACHE	10
#define HFSP_INCONSISTENT 11
#define HFSP_REUSE_CNID	12
#define HFSP_JOURNALED	13

#define HFS_IDX_NODE	0x00
#define HFS_HDR_NODE	0x01
#define HFS_MAP_NODE	0x02
#define HFS_LEAF_NODE	0xFF

#define HFS_FIRST_REC	0x0E
#define HFS_NSD_HD_REC	0x78
#define HFS_MAP_REC	0xF8

#define HFS_DATA_FORK	0x00
#define HFS_RES_FORK	0xFF

#define HFS_CAT_DIR	0x01
#define HFS_CAT_FILE	0x02
#define HFS_CAT_DIR_TH	0x03
#define HFS_CAT_FILE_TH	0x04

#define HFSP_ATTR_INLINE	0x10
#define HFSP_ATTR_FORK		0x20
#define HFSP_ATTR_EXTENTS	0x30

#define HFS_ROOT_PAR_ID		0x01
#define HFS_ROOT_DIR_ID		0x02
#define HFS_XTENT_ID		0x03
#define HFS_CATALOG_ID		0x04
#define HFS_BAD_BLOCK_ID	0x05
#define HFSP_ALLOC_ID		0x06
#define HFSP_STARTUP_ID		0x07
#define HFSP_ATTRIB_ID		0x08
#define HFSP_BOGUS_ID		0x0F
#define HFSP_FIRST_AV_ID	0x10

#define HFSJ_JOURN_IN_FS	0x00
#define HFSJ_JOURN_OTHER_DEV	0x01
#define HFSJ_JOURN_NEED_INIT	0x02

#define HFSJ_HEADER_MAGIC	0x4a4e4c78
#define HFSJ_ENDIAN_MAGIC	0x12345678

#define HFSX_CASE_FOLDING	0xCF	/* case insensitive HFSX */
#define HFSX_BINARY_COMPARE	0xBC	/* case sensitive   HFSX */

#define HFS_EXT_NB	3
#define HFSP_EXT_NB	8

/* Define the filenames used by the FS extractor */
#ifdef HFS_EXTRACT_FS

#define HFS_MDB_FILENAME	"mdb.hfs"
#define HFS_CATALOG_FILENAME	"catalog.hfs"
#define HFS_EXTENTS_FILENAME	"extents.hfs"
#define HFS_BITMAP_FILENAME	"bitmap.hfs"

#define HFSP_VH_FILENAME	"vh.hfsplus"
#define HFSP_CATALOG_FILENAME	"catalog.hfsplus"
#define HFSP_EXTENTS_FILENAME	"extents.hfsplus"
#define HFSP_BITMAP_FILENAME	"bitmap.hfsplus"
#define HFSP_ATTRIB_FILENAME	"attributes.hfsplus"
#define HFSP_STARTUP_FILENAME	"startup.hfsplus"

#endif /* HFS_EXTRACT_FS */



/* ----------------------------------- */
/* --      HFS DATA STRUCTURES      -- */
/* ----------------------------------- */

/* Extent descriptor */
struct __attribute__ ((packed)) _HfsExtDescriptor {
        uint16_t	start_block;
        uint16_t	block_count;
};
typedef struct _HfsExtDescriptor HfsExtDescriptor;
typedef HfsExtDescriptor HfsExtDataRec[HFS_EXT_NB];

/* Volume header */
struct __attribute__ ((packed)) _HfsMasterDirectoryBlock {
        uint16_t         signature;
        uint32_t         create_date;
        uint32_t         modify_date;
        uint16_t         volume_attributes;
        uint16_t         files_in_root;
        uint16_t         volume_bitmap_block;       /* in sectors */
        uint16_t         next_allocation;
        uint16_t         total_blocks;
        uint32_t         block_size;                /* in bytes */
        uint32_t         def_clump_size;            /* in bytes */
        uint16_t         start_block;               /* in sectors */
        uint32_t         next_free_node;
        uint16_t         free_blocks;
        uint8_t          name_length;
        char             name[27];
        uint32_t         backup_date;
        uint16_t         backup_number;
        uint32_t         write_count;
        uint32_t         extents_clump;
        uint32_t         catalog_clump;
        uint16_t         dirs_in_root;
        uint32_t         file_count;
        uint32_t         dir_count;
        uint32_t         finder_info[8];
        union __attribute__ ((packed)) {
                struct __attribute__ ((packed)) {
                        uint16_t    volume_cache_size;    /* in blocks */
                        uint16_t    bitmap_cache_size;    /* in blocks */
                        uint16_t    common_cache_size;    /* in blocks */
                } legacy;
                struct __attribute__ ((packed)) {
                        uint16_t            signature;
                        HfsExtDescriptor    location;
                } embedded;
        } old_new;
        uint32_t         extents_file_size;  /* in bytes, block size multiple */
        HfsExtDataRec    extents_file_rec;
        uint32_t         catalog_file_size;  /* in bytes, block size multiple */
        HfsExtDataRec    catalog_file_rec;
};
typedef struct _HfsMasterDirectoryBlock HfsMasterDirectoryBlock;

/* B*-Tree Node Descriptor */
struct __attribute__ ((packed)) _HfsNodeDescriptor {
        uint32_t        next;
        uint32_t        previous;
        int8_t          type;
        uint8_t         height;
        uint16_t        rec_nb;
        uint16_t        reserved;
};
typedef struct _HfsNodeDescriptor HfsNodeDescriptor;

/* Header record of a whole B*-Tree */
struct __attribute__ ((packed)) _HfsHeaderRecord {
        uint16_t        depth;
        uint32_t        root_node;
        uint32_t        leaf_records;
        uint32_t        first_leaf_node;
        uint32_t        last_leaf_node;
        uint16_t        node_size;
        uint16_t        max_key_len;
        uint32_t        total_nodes;
        uint32_t        free_nodes;
        int8_t          reserved[76];
};
typedef struct _HfsHeaderRecord HfsHeaderRecord;

/* Catalog key for B*-Tree lookup in the catalog file */
struct __attribute__ ((packed)) _HfsCatalogKey {
        uint8_t         key_length; /* length of the key without key_length */
        uint8_t         reserved;
        uint32_t        parent_ID;
        uint8_t         name_length;
        char            name[31];   /* in fact physicaly 1 upto 31 */
};
typedef struct _HfsCatalogKey HfsCatalogKey;

/* Extents overflow key for B*-Tree lookup */
struct __attribute__ ((packed)) _HfsExtentKey {
        uint8_t         key_length; /* length of the key without key_length */
        uint8_t         type;       /* data or ressource fork */
        uint32_t        file_ID;
        uint16_t        start;
};
typedef struct _HfsExtentKey HfsExtentKey;

/* Catalog subdata case directory */
struct __attribute__ ((packed)) _HfsDir {
        uint16_t        flags;
        uint16_t        valence;        /* number of files in this directory */
        uint32_t        dir_ID;
        uint32_t        create_date;
        uint32_t        modify_date;
        uint32_t        backup_date;
        int8_t          DInfo[16];      /* used by Finder, handle as reserved */
        int8_t          DXInfo[16];     /* used by Finder, handle as reserved */
        uint32_t        reserved[4];
};
typedef struct _HfsDir HfsDir;

/* Catalog subdata case file */
struct __attribute__ ((packed)) _HfsFile {
        int8_t          flags;
        int8_t          type;           /* should be 0 */
        int8_t          FInfo[16];      /* used by Finder, handle as reserved */
        uint32_t        file_ID;
        uint16_t        data_start_block;
        uint32_t        data_sz_byte;
        uint32_t        data_sz_block;
        uint16_t        res_start_block;
        uint32_t        res_sz_byte;
        uint32_t        res_sz_block;
        uint32_t        create_date;
        uint32_t        modify_date;
        uint32_t        backup_date;
        int8_t          FXInfo[16];     /* used by Finder, handle as reserved */
        uint16_t        clump_size;
        HfsExtDataRec   extents_data;
        HfsExtDataRec   extents_res;
        uint32_t        reserved;
};
typedef struct _HfsFile HfsFile;

/* Catalog subdata case directory thread */
struct __attribute__ ((packed)) _HfsDirTh {
        uint32_t        reserved[2];
        uint32_t        parent_ID;
        int8_t          name_length;
        char            name[31];
};
typedef struct _HfsDirTh HfsDirTh;

/* Catalog subdata case file thread */
typedef struct _HfsDirTh HfsFileTh;        /* same as directory thread */

/* Catalog data */
struct __attribute__ ((packed)) _HfsCatalog {
        int8_t          type;
        int8_t          reserved;
        union {
                HfsDir       dir;
                HfsFile      file;
                HfsDirTh     dir_th;
                HfsFileTh    file_th;
        } sel;
};
typedef struct _HfsCatalog HfsCatalog;



/* ------------------------------------ */
/* --      HFS+ DATA STRUCTURES      -- */
/* ------------------------------------ */

/* documented since 2004 in tn1150 */
struct __attribute__ ((packed)) _HfsPPerms {
        uint32_t        owner_ID;
        uint32_t        group_ID;
        uint32_t        permissions;
        uint32_t        special_devices;
};
typedef struct _HfsPPerms HfsPPerms;

/* HFS+ extent descriptor*/
struct __attribute__ ((packed)) _HfsPExtDescriptor {
        uint32_t        start_block;
        uint32_t        block_count;
};
typedef struct _HfsPExtDescriptor HfsPExtDescriptor;
typedef HfsPExtDescriptor HfsPExtDataRec[HFSP_EXT_NB];

/* HFS+ fork data structure */
struct __attribute__ ((packed)) _HfsPForkData {
        uint64_t        logical_size;
        uint32_t        clump_size;
        uint32_t        total_blocks;
        HfsPExtDataRec  extents;
};
typedef struct _HfsPForkData HfsPForkData;

/* HFS+ catalog node ID */
typedef uint32_t HfsPNodeID;

/* HFS+ file names */
typedef uint16_t unichar;
struct __attribute__ ((packed)) _HfsPUniStr255 {
        uint16_t        length;
        unichar         unicode[255];        /* 1 upto 255 */
};
typedef struct _HfsPUniStr255 HfsPUniStr255;

/* HFS+ volume header */
struct __attribute__ ((packed)) _HfsPVolumeHeader {
        uint16_t        signature;
        uint16_t        version;
        uint32_t        attributes;
        uint32_t        last_mounted_version;
        uint32_t        journal_info_block;

        uint32_t        create_date;
        uint32_t        modify_date;
        uint32_t        backup_date;
        uint32_t        checked_date;

        uint32_t        file_count;
        uint32_t        dir_count;

        uint32_t        block_size;
        uint32_t        total_blocks;
        uint32_t        free_blocks;

        uint32_t        next_allocation;
        uint32_t        res_clump_size;
        uint32_t        data_clump_size;
        HfsPNodeID      next_catalog_ID;

        uint32_t        write_count;
        uint64_t        encodings_bitmap;

        uint8_t         finder_info[32];

        HfsPForkData    allocation_file;
        HfsPForkData    extents_file;
        HfsPForkData    catalog_file;
        HfsPForkData    attributes_file;
        HfsPForkData    startup_file;
};
typedef struct _HfsPVolumeHeader HfsPVolumeHeader;

/* HFS+ B-Tree Node Descriptor. Same as HFS btree. */
struct __attribute__ ((packed)) _HfsPNodeDescriptor {
        uint32_t        next;
        uint32_t        previous;
        int8_t          type;
        uint8_t         height;
        uint16_t        rec_nb;
        uint16_t        reserved;
};
typedef struct _HfsPNodeDescriptor HfsPNodeDescriptor;

/* Header record of a whole HFS+ B-Tree. */
struct __attribute__ ((packed)) _HfsPHeaderRecord {
        uint16_t        depth;
        uint32_t        root_node;
        uint32_t        leaf_records;
        uint32_t        first_leaf_node;
        uint32_t        last_leaf_node;
        uint16_t        node_size;
        uint16_t        max_key_len;
        uint32_t        total_nodes;
        uint32_t        free_nodes;        /* same as hfs btree until here */
        uint16_t        reserved1;

        uint32_t        clump_size;
        uint8_t         btree_type;        /* must be 0 for HFS+ B-Tree */
        uint8_t         key_compare_type; /* hfsx => 0xCF = case folding */
                                          /*         0xBC = binary compare */
                                          /* otherwise, reserved */
        uint32_t        attributes;
        uint32_t        reserved3[16];
};
typedef struct _HfsPHeaderRecord HfsPHeaderRecord;

/* Catalog key for B-Tree lookup in the HFS+ catalog file */
struct __attribute__ ((packed)) _HfsPCatalogKey {
        uint16_t        key_length;
        HfsPNodeID      parent_ID;
        HfsPUniStr255   node_name;
};
typedef struct _HfsPCatalogKey HfsPCatalogKey;

/* HFS+ catalog subdata case dir */
struct __attribute__ ((packed)) _HfsPDir {
        uint16_t        flags;
        uint32_t        valence;
        HfsPNodeID      dir_ID;
        uint32_t        create_date;
        uint32_t        modify_date;
        uint32_t        attrib_mod_date;
        uint32_t        access_date;
        uint32_t        backup_date;
        HfsPPerms       permissions;
        int8_t          DInfo[16];        /* used by Finder, handle as reserved */
        int8_t          DXInfo[16];       /* used by Finder, handle as reserved */
        uint32_t        text_encoding;
        uint32_t        reserved;
};
typedef struct _HfsPDir HfsPDir;

/* HFS+ catalog subdata case file */
struct __attribute__ ((packed)) _HfsPFile {
        uint16_t        flags;
        uint32_t        reserved1;
        HfsPNodeID      file_ID;
        uint32_t        create_date;
        uint32_t        modify_date;
        uint32_t        attrib_mod_date;
        uint32_t        access_date;
        uint32_t        backup_date;
        HfsPPerms       permissions;
        int8_t          FInfo[16];        /* used by Finder, handle as reserved */
        int8_t          FXInfo[16];       /* used by Finder, handle as reserved */
        uint32_t        text_encoding;
        uint32_t        reserved2;

        HfsPForkData    data_fork;
        HfsPForkData    res_fork;
};
typedef struct _HfsPFile HfsPFile;

/* HFS+ catalog subdata case thread */
struct __attribute__ ((packed)) _HfsPThread {
        int16_t         reserved;
        HfsPNodeID      parent_ID;
        HfsPUniStr255   node_name;
};
typedef struct _HfsPThread HfsPDirTh;
typedef struct _HfsPThread HfsPFileTh;

/* HFS+ Catalog leaf data */
struct __attribute__ ((packed)) _HfsPCatalog {
        int16_t         type;
        union {
                HfsPDir         dir;
                HfsPFile        file;
                HfsPDirTh       dir_th;
                HfsPFileTh      file_th;
        } sel;
};
typedef struct _HfsPCatalog HfsPCatalog;

/* HFS+ extents file key */
struct __attribute__ ((packed)) _HfsPExtentKey {
        uint16_t        key_length;
        uint8_t         type;
        uint8_t         pad;
        HfsPNodeID      file_ID;
        uint32_t        start;
};
typedef struct _HfsPExtentKey HfsPExtentKey;

/* extent file data is HfsPExtDataRec */

/* Fork data attribute file */
struct __attribute__ ((packed)) _HfsPForkDataAttr {
        uint32_t        record_type;
        uint32_t        reserved;
        union __attribute__ ((packed)) {
                HfsPForkData        fork;
                HfsPExtDataRec      extents;
        } fork_res;
};
typedef struct _HfsPForkDataAttr HfsPForkDataAttr;


/* ----------- Journal data structures ----------- */

/* Info block : stored in a block # defined in the VH */
struct __attribute__ ((packed)) _HfsJJournalInfoBlock {
        uint32_t        flags;
        uint32_t        device_signature[8];
        uint64_t        offset;
        uint64_t        size;
        uint32_t        reserved[32];
};
typedef struct _HfsJJournalInfoBlock HfsJJournalInfoBlock;

struct __attribute__ ((packed)) _HfsJJournalHeader {
        uint32_t        magic;
        uint32_t        endian;
        uint64_t        start;
        uint64_t        end;
        uint64_t        size;
        uint32_t        blhdr_size;
        uint32_t        checksum;
        uint32_t        jhdr_size;
};
typedef struct _HfsJJournalHeader HfsJJournalHeader;

struct __attribute__ ((packed)) _HfsJBlockInfo {
        uint64_t        bnum;          /* sector number */
        uint32_t        bsize;         /* size in bytes */
        uint32_t        next;
};
typedef struct _HfsJBlockInfo HfsJBlockInfo;

struct __attribute__ ((packed)) _HfsJBlockListHeader {
        uint16_t        max_blocks;    /* reserved */
        uint16_t        num_blocks;
        uint32_t        bytes_used;
        uint32_t        checksum;
        uint32_t        pad;
        HfsJBlockInfo   binfo[1];
};
typedef struct _HfsJBlockListHeader HfsJBlockListHeader;



/* ---------------------------------------- */
/* --      INTERNAL DATA STRUCTURES      -- */
/* ---------------------------------------- */

/* Data of an opened HFS file */
struct _HfsPrivateFile {
        PedSector       sect_nb;
        PedFileSystem*  fs;
        uint32_t        CNID;          /* disk order (BE) */
        HfsExtDataRec   first;         /* disk order (BE) */
        HfsExtDataRec   cache;         /* disk order (BE) */
        uint16_t        start_cache;   /* CPU order */
};
typedef struct _HfsPrivateFile HfsPrivateFile;

/* To store bad block list */
struct _HfsPrivateLinkExtent {
        HfsExtDescriptor                 extent;
        struct _HfsPrivateLinkExtent*    next;
};
typedef struct _HfsPrivateLinkExtent HfsPrivateLinkExtent;

/* HFS Filesystem specific data */
struct _HfsPrivateFSData {
        uint8_t                     alloc_map[(1<<16) / 8];
        HfsMasterDirectoryBlock*    mdb;
        HfsPrivateFile*             extent_file;
        HfsPrivateFile*             catalog_file;
        HfsPrivateLinkExtent*       bad_blocks_xtent_list;
        unsigned int                bad_blocks_xtent_nb;
        char                        bad_blocks_loaded;
};
typedef struct _HfsPrivateFSData HfsPrivateFSData;

/* Generic btree key */
struct __attribute__ ((packed)) _HfsPrivateGenericKey {
        uint8_t         key_length;
        uint8_t         key_content[1];                /* we use 1 as a minimum size */
};
typedef struct _HfsPrivateGenericKey HfsPrivateGenericKey;

/* ----- HFS+ ----- */

/* Data of an opened HFS file */
struct _HfsPPrivateFile {
        PedSector       sect_nb;
        PedFileSystem*  fs;
        HfsPNodeID      CNID;          /* disk order (BE) */
        HfsPExtDataRec  first;         /* disk order (BE) */
        HfsPExtDataRec  cache;         /* disk order (BE) */
        uint32_t        start_cache;   /* CPU order */
};
typedef struct _HfsPPrivateFile HfsPPrivateFile;

struct _HfsPPrivateExtent {
        PedSector       start_sector;
        PedSector       sector_count;
};
typedef struct _HfsPPrivateExtent HfsPPrivateExtent;

/* To store bad block list */
struct _HfsPPrivateLinkExtent {
        HfsPExtDescriptor                 extent;
        struct _HfsPPrivateLinkExtent*    next;
};
typedef struct _HfsPPrivateLinkExtent HfsPPrivateLinkExtent;

/* HFS+ file system specific data */
struct _HfsPPrivateFSData {
        PedFileSystem*          wrapper;      /* NULL if hfs+ is not embedded */
        PedGeometry*            plus_geom;    /* Geometry of HFS+ _volume_ */
        uint8_t*                alloc_map;
        uint8_t*                dirty_alloc_map;
        HfsPVolumeHeader*       vh;
        HfsPPrivateFile*        extents_file;
        HfsPPrivateFile*        catalog_file;
        HfsPPrivateFile*        attributes_file;
        HfsPPrivateFile*        allocation_file;
        HfsPPrivateLinkExtent*  bad_blocks_xtent_list;
        uint32_t                jib_start_block;
        uint32_t                jl_start_block;
        unsigned int            bad_blocks_xtent_nb;
        char                    bad_blocks_loaded;
        char                    free_geom;    /* 1 = plus_geom must be freed */
};
typedef struct _HfsPPrivateFSData HfsPPrivateFSData;

/* Generic + btree key */
struct __attribute__ ((packed)) _HfsPPrivateGenericKey {
        uint16_t        key_length;
        uint8_t         key_content[1];       /* we use 1 as a minimum size */
};
typedef struct _HfsPPrivateGenericKey HfsPPrivateGenericKey;

/* ---- common ---- */

/* node and lead record reference for a BTree search */
struct _HfsCPrivateLeafRec {
        unsigned int    node_size;     /* in sectors */
        unsigned int    node_number;
        unsigned int    record_pos;
        unsigned int    record_number;
};
typedef struct _HfsCPrivateLeafRec HfsCPrivateLeafRec;

extern uint8_t*    hfs_block;
extern uint8_t*    hfsp_block;
extern unsigned    hfs_block_count;
extern unsigned    hfsp_block_count;

#endif /* _HFS_H */
