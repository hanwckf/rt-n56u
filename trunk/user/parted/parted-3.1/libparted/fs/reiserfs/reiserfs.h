/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2000, 2007, 2009-2012 Free Software Foundation, Inc.

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

#ifndef REISERFS_H
#define REISERFS_H

#define REISERFS_API_VERSION 0

#define REISERFS_SIGNATURE "ReIsErFs"
#define REISER2FS_SIGNATURE "ReIsEr2Fs"
#define REISER3FS_SIGNATURE "ReIsEr3Fs"

#define DEFAULT_BLOCK_SIZE 4096

struct reiserfs_super_block {
    uint32_t s_block_count;
    uint32_t s_free_blocks;
    uint32_t s_root_block;
    uint32_t s_journal_block;
    uint32_t s_journal_dev;
    uint32_t s_orig_journal_size;
    uint32_t s_journal_trans_max;
    uint32_t s_journal_block_count;
    uint32_t s_journal_max_batch;
    uint32_t s_journal_max_commit_age;
    uint32_t s_journal_max_trans_age;
    uint16_t s_blocksize;
    uint16_t s_oid_maxsize;
    uint16_t s_oid_cursize;
    uint16_t s_state;
    char s_magic[10];
    uint16_t s_fsck_state;
    uint32_t s_hash_function_code;
    uint16_t s_tree_height;
    uint16_t s_bmap_nr;
    uint16_t s_version;
    char padding[438];
};

typedef struct reiserfs_super_block reiserfs_super_block_t;

enum reiserfs_exception_type {
    EXCEPTION_INFORMATION 	= 1,
    EXCEPTION_WARNING 		= 2,
    EXCEPTION_ERROR 		= 3,
    EXCEPTION_FATAL 		= 4,
    EXCEPTION_BUG 		= 5,
    EXCEPTION_NO_FEATURE 	= 6
};

typedef enum reiserfs_exception_type reiserfs_exception_type_t;

enum reiserfs_exception_option {
    EXCEPTION_UNHANDLED 	= 1 << 0,
    EXCEPTION_FIX 		= 1 << 1,
    EXCEPTION_YES 		= 1 << 2,
    EXCEPTION_NO 		= 1 << 3,
    EXCEPTION_OK 		= 1 << 4,
    EXCEPTION_RETRY 		= 1 << 5,
    EXCEPTION_IGNORE 		= 1 << 6,
    EXCEPTION_CANCEL 		= 1 << 7
};

typedef enum reiserfs_exception_option reiserfs_exception_option_t;

typedef void (reiserfs_gauge_handler_t)(const char *, unsigned int, void *, int, int, int);

typedef void * reiserfs_exception_t;
typedef void * reiserfs_gauge_t;
typedef void * reiserfs_fs_t;

#define FS_FORMAT_3_5 			0
#define FS_FORMAT_3_6 			2

#define SUPER_OFFSET_IN_BYTES 		64*1024

#define DEFAULT_JOURNAL_SIZE 8192

#define JOURNAL_MIN_SIZE 		512
#define JOURNAL_MIN_TRANS 		256
#define JOURNAL_MAX_TRANS 		1024

#define JOURNAL_DEF_RATIO 		8
#define JOURNAL_MIN_RATIO 		2
#define JOURNAL_MAX_BATCH   		900
#define JOURNAL_MAX_COMMIT_AGE 		30
#define JOURNAL_MAX_TRANS_AGE 		30

#define TEA_HASH  			1
#define YURA_HASH			2
#define R5_HASH				3

#endif
