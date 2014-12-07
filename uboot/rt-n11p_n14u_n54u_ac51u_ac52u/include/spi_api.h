#ifndef _SPI_API_H_
#define _SPI_API_H_

#if defined(CFG_ENV_IS_IN_SPI)
extern unsigned long raspi_init(void);
extern int spi_env_init(void);

extern int raspi_write(char *buf, unsigned int to, int len);
extern int raspi_read(char *buf, unsigned int from, int len);
extern int raspi_erase(unsigned int offs, int len);
extern int raspi_erase_write(char *buf, unsigned int offs, int count);
#endif

#endif
