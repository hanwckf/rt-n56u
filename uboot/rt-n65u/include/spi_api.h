#ifndef _SPI_API_H_
#define _SPI_SPI_H_


unsigned long raspi_init(void);
int spi_env_init(void);

int raspi_write(char *buf, unsigned int to, int len);
int raspi_read(char *buf, unsigned int from, int len);
int raspi_erase(unsigned int offs, int len);
int raspi_erase_write(char *buf, unsigned int offs, int count);


#endif
