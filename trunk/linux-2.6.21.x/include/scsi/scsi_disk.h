#ifndef _SCSI_SCSI_DISK_H
#define _SCSI_SCSI_DISK_H

struct scsi_disk {
        struct scsi_driver *driver;     /* always &sd_template */
        struct scsi_device *device;
        struct class_device cdev;
        struct gendisk  *disk;
        unsigned int    openers;        /* protected by BKL for now, yuck */
        sector_t        capacity;       /* size in 512-byte sectors */
        u32             index;
        u8              media_present;
        u8              write_prot;
        unsigned        WCE : 1;        /* state of disk WCE bit */
        unsigned        RCD : 1;        /* state of disk RCD bit, unused */
        unsigned        DPOFUA : 1;     /* state of disk DPOFUA bit */
};

#endif /* _SCSI_SCSI_DISK_H */
