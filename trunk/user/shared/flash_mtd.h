#ifndef __FLASH_MTD_H__
#define __FLASH_MTD_H__

extern int mtd_dev_idx(const char *mtd_part);
extern int flash_mtd_read(const char *mtd_part, int offset, unsigned char *buf, size_t count);
extern int flash_mtd_write(const char *mtd_part, int offset, unsigned char *buf, size_t count);

#endif
