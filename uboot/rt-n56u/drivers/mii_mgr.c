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
#define enable_mdio(x)
#endif

#if defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD) || \
    defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) || \
    defined (RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD)
void enable_mdio(int enable)
{
#if !defined (P5_MAC_TO_PHY_MODE)
	u32 data = inw(GPIO_PRUPOSE);
	if(enable)
		data &= ~GPIO_MDIO_BIT;
	else
		data |= GPIO_MDIO_BIT;
	outw(GPIO_PRUPOSE, data);
#endif
}

#endif

u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data)
{
	u32 volatile  			status	= 0;
	u32 volatile  			data 	= 0;
	u32			  			rc		= 0;
	unsigned long volatile  t_start = get_timer(0);

	/* We enable mdio gpio purpose register, and disable it when exit.	 */
	enable_mdio(1);

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
			enable_mdio(0);
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

			enable_mdio(0);
			return 1;
		}
#else
		if(!( inw(MDIO_PHY_CONTROL_0) & (0x1 << 31)))
		{
			status = inw(MDIO_PHY_CONTROL_0);
			*read_data = (u32)(status & 0x0000FFFF);

			enable_mdio(0);
			return 1;
		}
#endif
		else if(get_timer(t_start) > (5 * CFG_HZ))
		{
			enable_mdio(0);
			printf("\n MDIO Read operation is ongoing and Time Out!!\n");
			return 0;
		}
	}
}


u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
	unsigned long volatile  t_start=get_timer(0);
	u32 volatile  data;

	enable_mdio(1);

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
			enable_mdio(0);
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
			enable_mdio(0);
			return 1;
		}
		else if(get_timer(t_start) > (5 * CFG_HZ))
		{
			enable_mdio(0);
			printf("\n MDIO Write operation is ongoing and Time Out!!\n");
			return 0;
		}
	}
}

#ifdef RALINK_MDIO_ACCESS_FUN
int rt2880_mdio_access(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u32 addr;
	u32 phy_addr;
	u32 value = 0,bit_offset,temp;

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
		phy_addr = simple_strtoul(argv[1], NULL, 10);
		addr = simple_strtoul(argv[2], NULL, 10);
		phy_addr &=0x1f;

		if(mii_mgr_read(phy_addr, addr, &value))
			printf("\n mdio.r addr[0x%08X]=0x%04X\n",addr,value);
		else {
			printf("\n Read addr[0x%08X] is Fail!!\n",addr);
		}
	}
	else if(!memcmp(argv[0],"mdio.w",sizeof("mdio.w")))
	{
		if (argc != 4) {
			printf ("Usage:\n%s\n", cmdtp->usage);
			return 1;
	    	}
		phy_addr = simple_strtoul(argv[1], NULL, 10);
		addr = simple_strtoul(argv[2], NULL, 10);
		value = simple_strtoul(argv[3], NULL, 16);
		phy_addr &=0x1f;

		if(mii_mgr_write(phy_addr, addr,value)) {
			printf("\n mdio.w addr[0x%08X]  value[0x%08X]\n",addr,value);
		}
		else {
			printf("\n Write[0x%08X] is Fail!!\n",addr);
		}
	}
	else if(!memcmp(argv[0],"mdio.wb",sizeof("mdio.wb")))
	{
		if (argc != 4) {
			printf ("Usage:\n%s\n", cmdtp->usage);
			return 1;
		}
		addr = simple_strtoul(argv[1], NULL, 10);
		bit_offset = simple_strtoul(argv[2], NULL, 10);
		value = simple_strtoul(argv[3], NULL, 10);

		if(mii_mgr_read(31, addr,&temp)) {
		    
		}
		else {
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

		if(mii_mgr_write(31, addr,temp)) {
			printf("\n mdio.wb addr[0x%08X]  value[0x%08X]\n",addr,temp);
		}
		else {
			printf("\n Write[0x%08X] is Fail!!\n",addr);
		}
	}
	return 0;
}

U_BOOT_CMD(
 	mdio,	4,	1,	rt2880_mdio_access,
 	"mdio   - Ralink PHY register R/W command !!\n",
 	"mdio.r [phy_addr(dec)] [reg_addr(dec)] \n"
 	"mdio.w [phy_addr(dec)] [reg_addr(dec)] [data(HEX)] \n"
 	"mdio.anoff GMAC1 Force link status enable !!  \n"
 	"mdio.anon GMAC1 Force link status disable !!  \n"
 	"mdio.wb [phy register(dec)] [bit offset(Dec)] [Value(0/1)]  \n"
);
#endif // RALINK_MDIO_ACCESS_FUN //
