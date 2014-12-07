#include "configs/rt2880.h"
#include <common.h>
#include "rt_mmap.h"

// register supported device
void nor_init(void);
void nand_init(void);
void spi_init(void);

int nor_read(uint32_t dst, uint32_t src_off, uint32_t size);
int nand_read(uint32_t dst, uint32_t src_off, uint32_t size);
int spi_read(uint32_t dst, uint32_t src_off, uint32_t size);


void udelay(unsigned long usec)
{
#define MIPS_CPU_FREQ_US (580)
	volatile register unsigned long count = (usec * MIPS_CPU_FREQ_US);

	asm volatile (".set noreorder \n"
		      "1: bnez %0, 1b \n\t"
		      "addiu %0, -1 \n\t"
		      :
		      :"r"(count)
		);
}

typedef enum nvram_device {
	NOR_FLASH	= 1<<1,
	NAND_FLASH	= 1<<2,
	SPI_FLASH       = 1<<3,
} nvram_t;


inline nvram_t boot_from(void)
{
	//fixme, read bootstrap configuration 
#if defined (CFG_ENV_IS_IN_FLASH)
	return NOR_FLASH;
#elif defined (CFG_ENV_IS_IN_SPI)
	return SPI_FLASH;
#elif defined (CFG_ENV_IS_IN_NAND)
	return NAND_FLASH;
#else
#error "FIXME, fail to simulate bootstrap setting of booting device"
#endif
}


nvram_t ram = NOR_FLASH;

typedef int (*NVRAM_READ_FUNC)(uint32_t , uint32_t , uint32_t);

inline void nvram_init(NVRAM_READ_FUNC *nvram_read)
{
	ram = boot_from();

	if (ram == NOR_FLASH) {
		//nor_init();	
		*nvram_read = nor_read;
	}
	else if (ram == NAND_FLASH) {
		nand_init();
		*nvram_read = nand_read;
	}
	else if (ram == SPI_FLASH) {
		spi_init();
		*nvram_read = spi_read;
	}

	return;
}

int load_stage2(void)
{
	image_header_t header[1];
	NVRAM_READ_FUNC nvram_read;

	int (*stage2)(int, char **);

	// init flash controller by bootstrap setting
	nvram_init(&nvram_read);

	// read header of stage2
	nvram_read((uint32_t)header, CONFIG_STAGE2_OFFSET, sizeof(header));
	
	// load stage2 image
	nvram_read(ntohl((uint32_t)header[0].ih_load), CONFIG_STAGE2_OFFSET + sizeof(header), ntohl(header[0].ih_size));

	// ready_to_go
	stage2 = (int (*)(int, char **))ntohl(header[0].ih_ep);
	stage2(0, 0);

	return 0;
}


