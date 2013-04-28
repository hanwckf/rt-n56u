#include <common.h>
#include <command.h>
#include <rt_mmap.h>
#include <configs/rt2880.h>


#define outw(address, value)    *((volatile uint32_t *)(address)) = cpu_to_le32(value)
#define inw(address)            le32_to_cpu(*(volatile u32 *)(address))

#if defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD) || \
    defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) || \
    defined (RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD)
#define PHY_CONTROL_0 		0xC0   
#define PHY_CONTROL_1 		0xC4   
#define MDIO_PHY_CONTROL_0	(RALINK_ETH_SW_BASE + PHY_CONTROL_0)
#define MDIO_PHY_CONTROL_1 	(RALINK_ETH_SW_BASE + PHY_CONTROL_1)

#define GPIO_MDIO_BIT		(1<<7)
#define GPIO_PURPOSE_SELECT	0x60
#define GPIO_PRUPOSE		(RALINK_SYSCTL_BASE + GPIO_PURPOSE_SELECT)

#else 
#define PHY_CONTROL_0       	0x00
#define PHY_CONTROL_1       	0x04
#define MDIO_PHY_CONTROL_0	(RALINK_FRAME_ENGINE_BASE + PHY_CONTROL_0)
#define MDIO_PHY_CONTROL_1	(RALINK_FRAME_ENGINE_BASE + PHY_CONTROL_1)

#define GPIO_MDIO_BIT		(1<<7)
#define GPIO_PURPOSE_SELECT	0x60
#define GPIO_PRUPOSE		(RALINK_SYSCTL_BASE + GPIO_PURPOSE_SELECT)
#endif

void ctrl_mdc_clock(int enable)
{
	u32 data = inw(GPIO_PRUPOSE);
	if(enable)
		data &= ~GPIO_MDIO_BIT;
	else
		data |= GPIO_MDIO_BIT;
	outw(GPIO_PRUPOSE, data);
}

/* Configure MDC frequency
 * @return:
 * 	0:	success
 * 	-1:	invalid parameter.
 */
static int setup_mdc_freq(u32 freq)
{
	u32 data, mask = 0;

	if (freq == 512)
		mask = 3;
	else if (freq == 1)
		mask = 2;
	else if (freq == 2)
		mask = 1;
	else if (freq == 4)
		mask = 0;
	else
		return -1;

	data = inw(MDIO_PHY_CONTROL_1);

	data &= ~(3 << 6);
	data |= (mask << 6);

	/* Disable auto-polling and clear PHY device #1 address */
	data &= ~(0x1F << 24);

	outw(MDIO_PHY_CONTROL_1, data);

	return 0;
}

static void print_mdc_freq(void)
{
	u32 data;
	char *mdc_freq[4] = {"4MHz", "2MHz", "1MHz", "512kHz"};

	data = inw(MDIO_PHY_CONTROL_1);
	printf(" MDC: %s\n", mdc_freq[(inw(MDIO_PHY_CONTROL_1) >> 6) & 3]);
}

void mii_mgr_init(void)
{
	int r;
	char *f;
	unsigned int freq = 4;

	f = getenv("mdio.clk");
	if (f)
		freq = simple_strtoul(f, NULL, 10);

	ctrl_mdc_clock(0);
	do {
		r = setup_mdc_freq(freq);
		if (!r || (r && freq == 512))
			break;
		freq = 512;
	} while (r);
	print_mdc_freq();
}

/* Read data from PHY register.
 * @return:
 * 	1:	success
 * 	0:	fail
 */
u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data)
{
	u32 volatile  			status	= 0;
	u32 volatile  			data 	= 0;
	u32			  			rc		= 0;
	unsigned long volatile  t_start = get_timer(0);

	// make sure previous read operation is complete
	while(1)
	{
#if defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD) || \
    defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) || \
    defined (RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD)
		// rd_rdy: read operation is complete
		if(!( inw(MDIO_PHY_CONTROL_1) & (0x1 << 1))) 
#else
		// 0 : Read/write operation complet
		if(!( inw(MDIO_PHY_CONTROL_0) & (0x1 << 31))) 
#endif
		{
			break;
		}else if(get_timer(t_start) > (5 * CFG_HZ)){
			printf("\n MDIO Read operation is ongoing !!\n");
			return rc;
		}
	}
	
#if defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD) || \
    defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) || \
    defined (RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD)
	outw(MDIO_PHY_CONTROL_0 , (1<<14) | (phy_register << 8) | (phy_addr));
#else
	data  = (phy_addr << 24) | (phy_register << 16);
	outw(MDIO_PHY_CONTROL_0, data);
	data |= (1<<31);
	outw(MDIO_PHY_CONTROL_0, data);
#endif
	//printf("\n Set Command [0x%08X] to PHY !!\n",MDIO_PHY_CONTROL_0);

	
	// make sure read operation is complete
	t_start = get_timer(0);
	while(1)
	{
#if defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD) || \
    defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) || \
    defined (RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD)
		if( inw(MDIO_PHY_CONTROL_1) & (0x1 << 1))
		{
			status = inw(MDIO_PHY_CONTROL_1);
			*read_data = (u32)(status >>16);

			return 1;
		}
#else
		if(!( inw(MDIO_PHY_CONTROL_0) & (0x1 << 31)))
		{
			status = inw(MDIO_PHY_CONTROL_0);
			*read_data = (u32)(status & 0x0000FFFF);

			return 1;
		}
#endif
		else if(get_timer(t_start) > (5 * CFG_HZ))
		{
			printf("\n MDIO Read operation is ongoing and Time Out!!\n");
			return 0;
		}
	}
}


/* Write data to PHY register.
 * @return:
 * 	1:	success
 * 	0:	fail
 */
u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
	unsigned long volatile  t_start=get_timer(0);
	u32 volatile  data;

	// make sure previous write operation is complete
	while(1)
	{
#if defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD) || \
    defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) || \
    defined (RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD)
		if(!( inw(MDIO_PHY_CONTROL_1) & (0x1 << 0)))
#else
		if (!( inw(MDIO_PHY_CONTROL_0) & (0x1 << 31))) 
#endif
		{
			break;
		}
		else if(get_timer(t_start) > (5 * CFG_HZ))
		{
			printf("\n MDIO Write operation is ongoing !!\n");
			return 0;
		}
	}

#if defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD) || \
    defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) || \
    defined (RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD)
	data = ((write_data & 0xFFFF)<<16);
	data |=  (phy_register << 8) | (phy_addr);
	data |=  (1<<13);
	outw(MDIO_PHY_CONTROL_0, data);
#else
	data = (1<<30) | (phy_addr << 24) | (phy_register << 16) | write_data;
	outw(MDIO_PHY_CONTROL_0, data);
	data |= (1<<31);
	outw(MDIO_PHY_CONTROL_0, data); //start operation
#endif
	//printf("\n Set Command [0x%08X] to PHY !!\n",MDIO_PHY_CONTROL_0);

	t_start = get_timer(0);

	// make sure write operation is complete
	while(1)
	{
#if defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD) || \
    defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) || \
    defined (RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD)
		if( inw(MDIO_PHY_CONTROL_1) & (0x1 << 0)) //wt_done ?= 1
#else
		if(!( inw(MDIO_PHY_CONTROL_0) & (0x1 << 31))) // 0 : Read/write operation complete
#endif
		{
			return 1;
		}
		else if(get_timer(t_start) > (5 * CFG_HZ))
		{
			printf("\n MDIO Write operation is ongoing and Time Out!!\n");
			return 0;
		}
	}
}

#if defined(MDC_MDIO_OPERATION)
int rt2880_mdio_clk(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int r;
	u32 freq;

	printf("argv[0] (%s)\n", argv[0]);
	if (strcmp(argv[0], "mdio.clk"))
		return 0;

	if (argc == 2) {
		freq = simple_strtoul(argv[1], NULL, 10);
		ctrl_mdc_clock(0);
		r = setup_mdc_freq(freq);
		if (r == -1)
			printf("Invalid frequency!\n");
	}

	print_mdc_freq();

	return 1;
}

U_BOOT_CMD(
	mdio,	2,	1,	rt2880_mdio_clk,
	"mdio.clk   - Modify Ralink SoC MDC clock.\n",
	"mdio.clk [512|1|2|4]\n"
);
#elif defined(RALINK_MDIO_ACCESS_FUN)
int rt2880_mdio_access(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u32 addr;
	u32 phy_addr=0;
	u32 value = 0,bit_offset,temp;
	u32 freq;

	if(!memcmp(argv[0],"mdio.anoff",sizeof("mdio.anoff")))
	{
		value = inw(MDIO_PHY_CONTROL_1);
		value |= (1<<15);
		outw(MDIO_PHY_CONTROL_1,value);
		puts("\n GMAC1 Force link status enable !! \n");
	}
	else if(!memcmp(argv[0],"mdio.anon",sizeof("mdio.anon")))
	{
		value = inw(MDIO_PHY_CONTROL_1);
		value &= ~(1 << 15);
		outw(MDIO_PHY_CONTROL_1,value);
		puts("\n GMAC1 Force link status disable !! \n");
	}
	else if(!memcmp(argv[0],"mdio.r",sizeof("mdio.r")))
	{
		if (argc != 3) {
			printf ("Usage:\n%s\n", cmdtp->usage);
			return 1;
	    	}
		phy_addr = simple_strtoul(argv[1], NULL, 0);
		addr = simple_strtoul(argv[2], NULL, 0);
		phy_addr &=0x1f;

		if(mii_mgr_read(phy_addr, addr, &value))
			printf("\n mdio.r phy_addr[0x%02X] reg[0x%08X]=0x%04X\n", phy_addr, addr, value);
		else
			printf("\n Read phy_addr[0x%02X] reg[0x%08X] is Fail!!\n", phy_addr, addr);
	}
	else if(!memcmp(argv[0],"mdio.w",sizeof("mdio.w")))
	{
		if (argc != 4) {
			printf ("Usage:\n%s\n", cmdtp->usage);
			return 1;
	    	}
		phy_addr = simple_strtoul(argv[1], NULL, 0);
		addr = simple_strtoul(argv[2], NULL, 0);
		value = simple_strtoul(argv[3], NULL, 0);
		phy_addr &=0x1f;

		if(mii_mgr_write(phy_addr, addr,value))
			printf("\n mdio.w phy_addr[0x%02X] reg[0x%08X] value[0x%08X]\n", phy_addr, addr, value);
		else
			printf("\n Write phy_addr[0x%02X] reg[0x%08X] is Fail!!\n", phy_addr, addr);
	}
	else if(!memcmp(argv[0],"mdio.wb",sizeof("mdio.wb")))
	{
		if (argc != 4) {
			printf ("Usage:\n%s\n", cmdtp->usage);
			return 1;
		}
		addr = simple_strtoul(argv[1], NULL, 0);
		bit_offset = simple_strtoul(argv[2], NULL, 0);
		value = simple_strtoul(argv[3], NULL, 0);

		if(!mii_mgr_read(31, addr,&temp)) {
			printf("\n Rasd PHY fail while mdio.wb was called\n");
			return 1;
		}

		if(value) {
			printf("\n Set bit[%d] to '1' \n",bit_offset);
			temp |= (1<<bit_offset);
		}
		else {
			printf("\n Set bit[%d] to '0' \n",bit_offset);
			temp &= ~(1<<bit_offset);
		}

		if(mii_mgr_write(31, addr,temp))
			printf("\n mdio.wb phy_addr[0x%02X] reg[0x%08X]  value[0x%08X]\n", 31, addr, temp);
		else
			printf("\n Write phy_addr[0x%02X] reg[0x%08X] is Fail!!\n", phy_addr, addr);
	}
	else if (!memcmp(argv[0],"mdio.clk",sizeof("mdio.clk"))) {
		if (argc == 2) {
			freq = simple_strtoul(argv[1], NULL, 10);
			ctrl_mdc_clock(0);
			setup_mdc_freq(freq);
		}
		print_mdc_freq();
	}
	return 0;
}

U_BOOT_CMD(
 	mdio,	4,	1,	rt2880_mdio_access,
 	"mdio   - Ralink PHY register R/W command !!\n",
	"mdio.r phy_addr reg_addr\n"
	"mdio.w phy_addr reg_addr data\n"
	"mdio.anoff - GMAC1 Force link status enable !!\n"
	"mdio.anon - GMAC1 Force link status disable !!\n"
	"mdio.wb phy_register bit_offset Value(0/1)\n"
	"mdio.clk [512|1|2|4]\n"
);
#endif	/* MDC_MDIO_OPERATION */
