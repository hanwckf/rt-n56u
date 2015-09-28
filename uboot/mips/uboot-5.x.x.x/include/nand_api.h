#ifndef _NAND_API_H_
#define _NAND_SPI_H_


unsigned long ranand_init(void);
int nand_env_init(void);

int ranand_write(char *buf, unsigned int to, int len);
int ranand_read(char *buf, unsigned int from, int len);
int ranand_erase(unsigned int offs, int len);
int ranand_erase_write(char *buf, unsigned int offs, int count);


#endif
