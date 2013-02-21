/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 1999-2002, 2007-2012 Free Software Foundation, Inc.

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

/**
 * \addtogroup PedDisk
 * @{
 */

/** \file disk.h */

#ifndef PED_DISK_H_INCLUDED
#define PED_DISK_H_INCLUDED

/**
 * Disk flags
 */
enum _PedDiskFlag {
        /* This flag (which defaults to true) controls if disk types for
           which cylinder alignment is optional do cylinder alignment when a
           new partition gets added.
           This flag is available for msdos and sun disklabels (for sun labels
           it only controls the aligning of the end of the partition) */
        PED_DISK_CYLINDER_ALIGNMENT=1,
        /* This flag controls whether the boot flag of a GPT PMBR is set */
        PED_DISK_GPT_PMBR_BOOT=2,
};
#define PED_DISK_FIRST_FLAG             PED_DISK_CYLINDER_ALIGNMENT
#define PED_DISK_LAST_FLAG              PED_DISK_GPT_PMBR_BOOT

/**
 * Partition types
 */
enum _PedPartitionType {
        PED_PARTITION_NORMAL            = 0x00,
        PED_PARTITION_LOGICAL           = 0x01,
        PED_PARTITION_EXTENDED          = 0x02,
        PED_PARTITION_FREESPACE         = 0x04,
        PED_PARTITION_METADATA          = 0x08,
        PED_PARTITION_PROTECTED         = 0x10
};

/**
 * Partition flags.
 */
enum _PedPartitionFlag {
        PED_PARTITION_BOOT=1,
        PED_PARTITION_ROOT=2,
        PED_PARTITION_SWAP=3,
        PED_PARTITION_HIDDEN=4,
        PED_PARTITION_RAID=5,
        PED_PARTITION_LVM=6,
        PED_PARTITION_LBA=7,
        PED_PARTITION_HPSERVICE=8,
        PED_PARTITION_PALO=9,
        PED_PARTITION_PREP=10,
        PED_PARTITION_MSFT_RESERVED=11,
        PED_PARTITION_BIOS_GRUB=12,
        PED_PARTITION_APPLE_TV_RECOVERY=13,
        PED_PARTITION_DIAG=14,
        PED_PARTITION_LEGACY_BOOT=15
};
#define PED_PARTITION_FIRST_FLAG        PED_PARTITION_BOOT
#define PED_PARTITION_LAST_FLAG         PED_PARTITION_LEGACY_BOOT

enum _PedDiskTypeFeature {
        PED_DISK_TYPE_EXTENDED=1,       /**< supports extended partitions */
        PED_DISK_TYPE_PARTITION_NAME=2  /**< supports partition names */
};
#define PED_DISK_TYPE_FIRST_FEATURE    PED_DISK_TYPE_EXTENDED
#define PED_DISK_TYPE_LAST_FEATURE     PED_DISK_TYPE_PARTITION_NAME

struct _PedDisk;
struct _PedPartition;
struct _PedDiskOps;
struct _PedDiskType;
struct _PedDiskArchOps;

typedef enum _PedDiskFlag               PedDiskFlag;
typedef enum _PedPartitionType          PedPartitionType;
typedef enum _PedPartitionFlag          PedPartitionFlag;
typedef enum _PedDiskTypeFeature        PedDiskTypeFeature;
typedef struct _PedDisk                 PedDisk;
typedef struct _PedPartition            PedPartition;
typedef const struct _PedDiskOps        PedDiskOps;
typedef struct _PedDiskType             PedDiskType;
typedef const struct _PedDiskArchOps    PedDiskArchOps;

#include <parted/device.h>
#include <parted/filesys.h>
#include <parted/natmath.h>
#include <parted/geom.h>
#include <stdbool.h>

/** @} */

/**
 * \addtogroup PedPartition
 *
 * @{
 */

/** \file disk.h */

/**
 * PedPartition structure represents a partition.
 */
struct _PedPartition {
        PedPartition*           prev;
        PedPartition*           next;

        /**< the partition table of the partition */
        PedDisk*                disk;
        PedGeometry             geom;	/**< geometry of the partition */

        /**< the partition number:  In Linux, this is the
             same as the minor number. No assumption
             should be made about "num" and "type"
             - different disk labels have different rules. */

        int                     num;
        PedPartitionType        type;	/**< the type of partition: a bit field of
					  	PED_PARTITION_LOGICAL, PED_PARTITION_EXTENDED,
						PED_PARTITION_METADATA
						and PED_PARTITION_FREESPACE.
						Both the first two, and the last two are
						mutually exclusive.
							An extended partition is a primary
						partition that may contain logical partitions.
						There is at most one extended partition on
						a disk.
							A logical partition is like a primary
						partition, except it's inside an extended
						partition. Internally, pseudo partitions are
						allocated to represent free space, or disk
						label meta-data.  These have the
						PED_PARTITION_FREESPACE or
						PED_PARTITION_METADATA bit set. */

        /**< The type of file system on the partition. NULL if unknown. */
        const PedFileSystemType* fs_type;

        /**< Only used for an extended partition.  The list of logical
             partitions (and free space and metadata within the extended
             partition). */
        PedPartition*           part_list;

        void*                   disk_specific;
};

/** @} */

/**
 * \addtogroup PedDisk
 * @{
 */

/**
 * Represents a disk label (partition table).
 */
struct _PedDisk {
        PedDevice*          dev;         /**< the device where the
                                              partition table lies */
        const PedDiskType*  type;        /**< type of disk label */
        const int*          block_sizes; /**< block sizes supported
                                              by this label */
        PedPartition*       part_list;   /**< list of partitions. Access with
                                              ped_disk_next_partition() */

        void*               disk_specific;

/* office use only ;-) */
        int                 needs_clobber;      /**< clobber before write? */
        int                 update_mode;        /**< mode without free/metadata
                                                   partitions, for easier
                                                   update */
};

struct _PedDiskOps {
        /* disk label operations */
        int (*probe) (const PedDevice *dev);
        int (*clobber) (PedDevice* dev);
        PedDisk* (*alloc) (const PedDevice* dev);
        PedDisk* (*duplicate) (const PedDisk* disk);
        void (*free) (PedDisk* disk);
        int (*read) (PedDisk* disk);
        int (*write) (const PedDisk* disk);
        int (*disk_set_flag) (
                PedDisk *disk,
                PedDiskFlag flag,
                int state);
        int (*disk_get_flag) (
                const PedDisk *disk,
                PedDiskFlag flag);
        int (*disk_is_flag_available) (
                const PedDisk *disk,
                PedDiskFlag flag);
        /** \todo add label guessing op here */

        /* partition operations */
        PedPartition* (*partition_new) (
                const PedDisk* disk,
                PedPartitionType part_type,
                const PedFileSystemType* fs_type,
                PedSector start,
                PedSector end);
        PedPartition* (*partition_duplicate) (const PedPartition* part);
        void (*partition_destroy) (PedPartition* part);
        int (*partition_set_system) (PedPartition* part,
                                     const PedFileSystemType* fs_type);
        int (*partition_set_flag) (
                PedPartition* part,
                PedPartitionFlag flag,
                int state);
        int (*partition_get_flag) (
                const PedPartition* part,
                PedPartitionFlag flag);
        int (*partition_is_flag_available) (
                const PedPartition* part,
                PedPartitionFlag flag);
        void (*partition_set_name) (PedPartition* part, const char* name);
        const char* (*partition_get_name) (const PedPartition* part);
        int (*partition_align) (PedPartition* part,
                                const PedConstraint* constraint);
        int (*partition_enumerate) (PedPartition* part);
        bool (*partition_check) (const PedPartition* part);

        /* other */
        int (*alloc_metadata) (PedDisk* disk);
        int (*get_max_primary_partition_count) (const PedDisk* disk);
        bool (*get_max_supported_partition_count) (const PedDisk* disk,
                                                   int* supported);
        PedAlignment *(*get_partition_alignment)(const PedDisk *disk);
        PedSector (*max_length) (void);
        PedSector (*max_start_sector) (void);
};

struct _PedDiskType {
        PedDiskType*            next;
        const char*             name; /**< the name of the partition table type.
                                           \todo not very intuitive name */
        PedDiskOps* const       ops;

        PedDiskTypeFeature      features;   /**< bitmap of supported features */
};

/**
 * Architecture-specific operations.  i.e. communication with kernel (or
 * whatever) about changes, etc.
 */
struct _PedDiskArchOps {
        char* (*partition_get_path) (const PedPartition* part);
        int (*partition_is_busy) (const PedPartition* part);
        int (*disk_commit) (PedDisk* disk);
};

extern void ped_disk_type_register (PedDiskType* type);
extern void ped_disk_type_unregister (PedDiskType* type);

extern PedDiskType* ped_disk_type_get_next (PedDiskType const *type)
 
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__pure__))
#endif
;
extern PedDiskType* ped_disk_type_get (const char* name)
 
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__pure__))
#endif
;
extern int ped_disk_type_check_feature (const PedDiskType* disk_type,
                                        PedDiskTypeFeature feature)
 
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__pure__))
#endif
;

extern PedDiskType* ped_disk_probe (PedDevice* dev);
extern int ped_disk_clobber (PedDevice* dev);
extern PedDisk* ped_disk_new (PedDevice* dev);
extern PedDisk* ped_disk_new_fresh (PedDevice* dev,
                                    const PedDiskType* disk_type);
extern PedDisk* ped_disk_duplicate (const PedDisk* old_disk);
extern void ped_disk_destroy (PedDisk* disk);
extern int ped_disk_commit (PedDisk* disk);
extern int ped_disk_commit_to_dev (PedDisk* disk);
extern int ped_disk_commit_to_os (PedDisk* disk);
extern int ped_disk_check (const PedDisk* disk);
extern void ped_disk_print (const PedDisk* disk);

extern int ped_disk_get_primary_partition_count (const PedDisk* disk)
 
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__pure__))
#endif
;
extern int ped_disk_get_last_partition_num (const PedDisk* disk)
 
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__pure__))
#endif
;
extern int ped_disk_get_max_primary_partition_count (const PedDisk* disk);
extern bool ped_disk_get_max_supported_partition_count(const PedDisk* disk,
                                                       int* supported);
extern PedAlignment *ped_disk_get_partition_alignment(const PedDisk *disk);

extern int ped_disk_set_flag(PedDisk *disk, PedDiskFlag flag, int state);
extern int ped_disk_get_flag(const PedDisk *disk, PedDiskFlag flag);
extern int ped_disk_is_flag_available(const PedDisk *disk, PedDiskFlag flag);

extern const char *ped_disk_flag_get_name(PedDiskFlag flag);
extern PedDiskFlag ped_disk_flag_get_by_name(const char *name);
extern PedDiskFlag ped_disk_flag_next(PedDiskFlag flag)
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__const__))
#endif
;

/** @} */

/**
 * \addtogroup PedPartition
 *
 * @{
 */

extern PedPartition* ped_partition_new (const PedDisk* disk,
                                        PedPartitionType type,
                                        const PedFileSystemType* fs_type,
                                        PedSector start,
                                        PedSector end);
extern void ped_partition_destroy (PedPartition* part);
extern int ped_partition_is_active (const PedPartition* part)
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__pure__))
#endif
;
extern int ped_partition_set_flag (PedPartition* part, PedPartitionFlag flag,
                                   int state);
extern int ped_partition_get_flag (const PedPartition* part,
                                   PedPartitionFlag flag);
extern int ped_partition_is_flag_available (const PedPartition* part,
                                            PedPartitionFlag flag);
extern int ped_partition_set_system (PedPartition* part,
                                     const PedFileSystemType* fs_type);
extern int ped_partition_set_name (PedPartition* part, const char* name);
extern const char* ped_partition_get_name (const PedPartition* part);
extern int ped_partition_is_busy (const PedPartition* part);
extern char* ped_partition_get_path (const PedPartition* part);

extern const char* ped_partition_type_get_name (PedPartitionType part_type)
 
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__const__))
#endif
;
extern const char* ped_partition_flag_get_name (PedPartitionFlag flag);
extern PedPartitionFlag ped_partition_flag_get_by_name (const char* name);
extern PedPartitionFlag ped_partition_flag_next (PedPartitionFlag flag)
 
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__const__))
#endif
;

/** @} */

/**
 * \addtogroup PedDisk
 * @{
 */

extern int ped_disk_add_partition (PedDisk* disk, PedPartition* part,
                                   const PedConstraint* constraint);
extern int ped_disk_remove_partition (PedDisk* disk, PedPartition* part);
extern int ped_disk_delete_partition (PedDisk* disk, PedPartition* part);
extern int ped_disk_delete_all (PedDisk* disk);
extern int ped_disk_set_partition_geom (PedDisk* disk, PedPartition* part,
                                        const PedConstraint* constraint,
                                        PedSector start, PedSector end);
extern int ped_disk_maximize_partition (PedDisk* disk, PedPartition* part,
                                        const PedConstraint* constraint);
extern PedGeometry* ped_disk_get_max_partition_geometry (PedDisk* disk,
                PedPartition* part, const PedConstraint* constraint);
extern int ped_disk_minimize_extended_partition (PedDisk* disk);

extern PedPartition* ped_disk_next_partition (const PedDisk* disk,
                                              const PedPartition* part)
 
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__pure__))
#endif
;
extern PedPartition* ped_disk_get_partition (const PedDisk* disk, int num)
 
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__pure__))
#endif
;
extern PedPartition* ped_disk_get_partition_by_sector (const PedDisk* disk,
                                                       PedSector sect)
 
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__pure__))
#endif
;
extern PedPartition* ped_disk_extended_partition (const PedDisk* disk)
 
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__pure__))
#endif
;

extern PedSector ped_disk_max_partition_length (const PedDisk *disk);
extern PedSector ped_disk_max_partition_start_sector (const PedDisk *disk);

/* internal functions */
extern PedDisk* _ped_disk_alloc (const PedDevice* dev, const PedDiskType* type);
extern void _ped_disk_free (PedDisk* disk);


/** @} */

/**
 * \addtogroup PedPartition
 *
 * @{
 */

extern PedPartition* _ped_partition_alloc (const PedDisk* disk,
                                           PedPartitionType type,
                                           const PedFileSystemType* fs_type,
                                           PedSector start,
                                           PedSector end);
extern void _ped_partition_free (PedPartition* part);

extern int _ped_partition_attempt_align (
                PedPartition* part, const PedConstraint* external,
                PedConstraint* internal);

#endif /* PED_DISK_H_INCLUDED */

/** @} */
