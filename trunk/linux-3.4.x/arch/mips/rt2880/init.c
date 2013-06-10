/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     init setup for Ralink RT2880 solution
 *
 *  Copyright 2007 Ralink Inc. (bruce_chang@ralinktech.com.tw)
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2007 Bruce Chang
 *
 * Initial Release
 *
 *
 *
 **************************************************************************
 */

#include <linux/init.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <linux/console.h>
#include <asm/bootinfo.h>
#include <asm/io.h>
#include <asm/serial.h>
#include <asm/rt2880/prom.h>
#include <asm/rt2880/generic.h>
#include <asm/rt2880/surfboard.h>
#include <asm/rt2880/surfboardint.h>
#include <asm/rt2880/rt_mmap.h>

#ifndef STD_COM_FLAGS
#define STD_COM_FLAGS (UPF_BOOT_AUTOCONF | UPF_SKIP_TEST)
#endif

#define UART_BAUDRATE	CONFIG_RALINK_UART_BRATE

extern unsigned long surfboard_sysclk;
u32 mips_cpu_feq;

#ifdef CONFIG_UBOOT_CMDLINE
/* Environment variable */
typedef struct {
	char *name;
	char *val;
} t_env_var;

int prom_argc;
int *_prom_argv, *_prom_envp;

/*
 * YAMON (32-bit PROM) pass arguments and environment as 32-bit pointer.
 * This macro take care of sign extension, if running in 64-bit mode.
 */
#define prom_envp(index) ((char *)(((int *)(int)_prom_envp)[(index)]))
#endif

char *prom_getenv(char *envname)
{
#ifdef CONFIG_UBOOT_CMDLINE
	/*
	 * Return a pointer to the given environment variable.
	 * In 64-bit mode: we're using 64-bit pointers, but all pointers
	 * in the PROM structures are only 32-bit, so we need some
	 * workarounds, if we are running in 64-bit mode.
	 */
	int i, index=0;
	char *p, *q;

	i = strlen(envname);
	while (prom_envp(index)) {
		p = (char*) KSEG0ADDR(prom_envp(index));
		if(!strncmp(envname, p, i)) {
			q = strchr(p, '=');
			if (q)
				q++;
			return q;
		}
		index++;
	}
#endif
	return NULL;
}

static inline unsigned char str2hexnum(unsigned char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return 0; /* foo */
}

static inline void str2eaddr(unsigned char *ea, unsigned char *str)
{
	int i;

	for (i = 0; i < 6; i++) {
		unsigned char num;

		if((*str == '.') || (*str == ':'))
			str++;
		num = str2hexnum(*str++) << 4;
		num |= (str2hexnum(*str++));
		ea[i] = num;
	}
}

static void prom_usbinit(void)
{
#ifndef CONFIG_RALINK_RT3052
	u32 reg=0;
#endif
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350)
	reg = *(volatile u32 *)KSEG1ADDR((RALINK_SYSCTL_BASE + 0x34));
	reg = reg | RALINK_UDEV_RST | RALINK_UHST_RST;
	*(volatile u32 *)KSEG1ADDR((RALINK_SYSCTL_BASE + 0x34))= reg;

	reg = *(volatile u32 *)KSEG1ADDR((RALINK_SYSCTL_BASE + 0x30));
#if defined (CONFIG_RALINK_RT5350)
	reg = reg & ~(RALINK_UPHY0_CLK_EN);
#else
	reg = reg & ~(RALINK_UPHY0_CLK_EN | RALINK_UPHY1_CLK_EN);
#endif
	*(volatile u32 *)KSEG1ADDR((RALINK_SYSCTL_BASE + 0x30))= reg;

#elif defined (CONFIG_RALINK_RT3052)
	*(volatile u32 *)KSEG1ADDR((RALINK_USB_OTG_BASE + 0xE00)) = 0xF;	// power saving
#endif
}

int get_ethernet_addr(char *ethernet_addr)
{
        char *ethaddr_str;

        ethaddr_str = prom_getenv("ethaddr");
	if (!ethaddr_str) {
	        printk("ethaddr not set in boot prom\n");
		return -1;
	}
	str2eaddr(ethernet_addr, ethaddr_str);

#ifdef DEBUG
	int i;
	printk("get_ethernet_addr: ");
	for (i=0; i<5; i++)
		printk("%02x:", (unsigned char)*(ethernet_addr+i));
	printk("%02x\n", *(ethernet_addr+i));
#endif
	return 0;
}

void prom_init_sysclk(void)
{

#if defined(CONFIG_RT2880_FPGA)
        mips_cpu_feq = 25000000; 
#elif defined (CONFIG_RT3052_FPGA) || defined (CONFIG_RT3352_FPGA) || defined (CONFIG_RT2883_FPGA) || defined (CONFIG_RT3883_FPGA) || defined (CONFIG_RT5350_FPGA)
        mips_cpu_feq = 40000000; 
#else
	u32 	reg;
        u8      clk_sel;
#if defined(CONFIG_RT5350_ASIC)
        u8      clk_sel2;
#endif

        reg = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x10)));

#if defined (CONFIG_RT2880_ASIC)
        clk_sel = (reg>>20) & 0x03;
#elif defined (CONFIG_RT2883_ASIC) 
        clk_sel = (reg>>18) & 0x03;
#elif defined (CONFIG_RT3052_ASIC) 
        clk_sel = (reg>>18) & 0x01;
#elif defined (CONFIG_RT3352_ASIC) 
        clk_sel = (reg>>8) & 0x01;
#elif defined (CONFIG_RT5350_ASIC) 
        clk_sel = (reg>>8) & 0x01;
        clk_sel2 = (reg>>10) & 0x01;
        clk_sel |= (clk_sel2 << 1);
#elif defined (CONFIG_RT3883_ASIC) 
        clk_sel = (reg>>8) & 0x03;
#else
#error Please Choice System Type
#endif
        switch(clk_sel) {
#if defined (CONFIG_RALINK_RT2880_SHUTTLE)
	case 0:
		mips_cpu_feq = (233333333);
		break;
	case 1:
		mips_cpu_feq = (250000000);
		break;
	case 2:
		mips_cpu_feq = (266666666);
		break;
	case 3:
		mips_cpu_feq = (280000000);
		break;
#elif defined (CONFIG_RALINK_RT2880_MP)
	case 0:
		mips_cpu_feq = (250000000);
		break;
	case 1:
		mips_cpu_feq = (266666666);
		break;
	case 2:
		mips_cpu_feq = (280000000);
		break;
	case 3:
		mips_cpu_feq = (300000000);
		break;
#elif defined (CONFIG_RALINK_RT2883) 
	case 0:
		mips_cpu_feq = (380*1000*1000);
		break;
	case 1:
		mips_cpu_feq = (390*1000*1000);
		break;
	case 2:
		mips_cpu_feq = (400*1000*1000);
		break;
	case 3:
		mips_cpu_feq = (420*1000*1000);
		break;
#elif defined (CONFIG_RALINK_RT3052) 
#if defined (CONFIG_RALINK_RT3350)
		// MA10 is floating
	case 0:
	case 1:
		mips_cpu_feq = (320*1000*1000);
		break;
#else
	case 0:
		mips_cpu_feq = (320*1000*1000);
		break;
	case 1:
		mips_cpu_feq = (384*1000*1000); 
		break;
#endif
#elif defined (CONFIG_RALINK_RT3352) 
	case 0:
		mips_cpu_feq = (384*1000*1000);
		break;
	case 1:
		mips_cpu_feq = (400*1000*1000); 
		break;
#elif defined (CONFIG_RALINK_RT3883) 
	case 0:
		mips_cpu_feq = (250*1000*1000);
		break;
	case 1:
		mips_cpu_feq = (384*1000*1000); 
		break;
	case 2:
		mips_cpu_feq = (480*1000*1000); 
		break;
	case 3:
		mips_cpu_feq = (500*1000*1000); 
		break;
#elif defined(CONFIG_RALINK_RT5350)
	case 0:
		mips_cpu_feq = (360*1000*1000);
		break;
	case 1:
		//reserved
		break;
	case 2:
		mips_cpu_feq = (320*1000*1000); 
		break;
	case 3:
		mips_cpu_feq = (300*1000*1000); 
		break;

#else
#error Please Choice Chip Type
#endif
	}

#endif
	
#if defined (CONFIG_RT3883_ASIC) 
	if ((reg>>17) & 0x1) { //DDR2
		switch (clk_sel) {
		case 0:
			surfboard_sysclk = (125*1000*1000);
			break;
		case 1:
			surfboard_sysclk = (128*1000*1000);
			break;
		case 2:
			surfboard_sysclk = (160*1000*1000);
			break;
		case 3:
			surfboard_sysclk = (166*1000*1000);
			break;
		}
	}
	else { //SDR
		switch (clk_sel) {
		case 0:
			surfboard_sysclk = (83*1000*1000);
			break;
		case 1:
			surfboard_sysclk = (96*1000*1000);
			break;
		case 2:
			surfboard_sysclk = (120*1000*1000);
			break;
		case 3:
			surfboard_sysclk = (125*1000*1000);
			break;
		}
	}

#elif defined(CONFIG_RT5350_ASIC)
	switch (clk_sel) {
	case 0:
		surfboard_sysclk = (120*1000*1000);
		break;
	case 1:
		//reserved
		break;
	case 2:
		surfboard_sysclk = (80*1000*1000);
		break;
	case 3:
		surfboard_sysclk = (100*1000*1000);
		break;
	}

#elif defined (CONFIG_RALINK_RT2880)
	surfboard_sysclk = mips_cpu_feq/2;
#else
	surfboard_sysclk = mips_cpu_feq/3;
#endif
	printk("\n The CPU frequency set to %d MHz\n", mips_cpu_feq / 1000 / 1000);
}

/*
** This function sets up the local prom_rs_table used only for the fake console
** console (mainly prom_printf for debug display and no input processing)
** and also sets up the global rs_table used for the actual serial console.
** To get the correct baud_base value, prom_init_sysclk() must be called before
** this function is called.
*/
static struct uart_port serial_req[2];

int __init prom_init_serial_port(void)
{
	/*
	 * baud rate = system clock freq / (CLKDIV * 16)
	 * CLKDIV=system clock freq/16/baud rate
	 */
	memset(serial_req, 0, 2 * sizeof(struct uart_port));

	serial_req[0].type = PORT_16550A;
	serial_req[0].line = 0;
	serial_req[0].irq = SURFBOARDINT_UART;
	serial_req[0].flags = STD_COM_FLAGS;
	serial_req[0].uartclk = UART_BAUDRATE * 16;
	serial_req[0].iotype = UPIO_PORT;
	serial_req[0].iobase = KSEG1ADDR(RALINK_UART_BASE);
	serial_req[0].regshift = 2;
	serial_req[0].mapbase = KSEG1ADDR(RALINK_UART_BASE);
	serial_req[0].fifosize = 16;
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350)
	serial_req[0].custom_divisor = (40000000 / SURFBOARD_BAUD_DIV / UART_BAUDRATE);
#else
	serial_req[0].custom_divisor = (surfboard_sysclk / SURFBOARD_BAUD_DIV / UART_BAUDRATE);
#endif

	serial_req[1].type = PORT_16550A;
	serial_req[1].line = 1;
	serial_req[1].irq = SURFBOARDINT_UART1;
	serial_req[1].flags = STD_COM_FLAGS;
	serial_req[1].uartclk = UART_BAUDRATE * 16;
	serial_req[1].iotype = UPIO_PORT;
	serial_req[1].iobase = KSEG1ADDR(RALINK_UART_LITE_BASE);
	serial_req[1].regshift = 2;
	serial_req[1].mapbase = KSEG1ADDR(RALINK_UART_LITE_BASE);
	serial_req[1].fifosize = 16;
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350)
	serial_req[1].custom_divisor = (40000000 / SURFBOARD_BAUD_DIV / UART_BAUDRATE);
#else
	serial_req[1].custom_divisor = (surfboard_sysclk / SURFBOARD_BAUD_DIV / UART_BAUDRATE);
#endif

	early_serial_setup(&serial_req[0]);
	early_serial_setup(&serial_req[1]);

	return (0);
}

//early_initcall(prom_init_serial_port);

int __init prom_get_ttysnum(void)
{
	char *argptr;
	int ttys_num = 0;       /* default */

	/* get ttys_num to use with the fake console/prom_printf */
	argptr = prom_getcmdline();

	if ((argptr = strstr(argptr, "console=ttyS")) != NULL)
	{
                argptr += strlen("console=ttyS");

                if (argptr[0] == '0')           /* ttyS0 */
                        ttys_num = 0;           /* happens to be rs_table[0] */
                else if (argptr[0] == '1')      /* ttyS1 */
                        ttys_num = 1;           /* happens to be rs_table[1] */
	}

	return (ttys_num);
}

static void serial_setbrg(unsigned long wBaud)
{
        unsigned int clock_divisor = 0;
        clock_divisor = (surfboard_sysclk / SURFBOARD_BAUD_DIV);
	
#if 1
 	*(volatile u32 *)(RALINK_SYSCTL_BASE + 0xC08)= 0;
        *(volatile u32 *)(RALINK_SYSCTL_BASE + 0xC10)= 0;
        *(volatile u32 *)(RALINK_SYSCTL_BASE + 0xC14)= 0x3;
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) ||  defined (CONFIG_RALINK_RT5350)
        *(volatile u32 *)(RALINK_SYSCTL_BASE + 0xC28)= (40000000 / SURFBOARD_BAUD_DIV / wBaud);
#else
        *(volatile u32 *)(RALINK_SYSCTL_BASE + 0xC28)= (surfboard_sysclk / SURFBOARD_BAUD_DIV / wBaud);
#endif
 	*(volatile u32 *)(RALINK_SYSCTL_BASE + 0x508)= 0;
        *(volatile u32 *)(RALINK_SYSCTL_BASE + 0x510)= 0;
        *(volatile u32 *)(RALINK_SYSCTL_BASE + 0x514)= 0x3;
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350)
        *(volatile u32 *)(RALINK_SYSCTL_BASE + 0x528)= (40000000 / SURFBOARD_BAUD_DIV / wBaud);
#else
        *(volatile u32 *)(RALINK_SYSCTL_BASE + 0x528)= (surfboard_sysclk / SURFBOARD_BAUD_DIV / wBaud);
#endif
#else
        IER(CFG_RT2880_CONSOLE) = 0;                                    /* Disable for now */
        FCR(CFG_RT2880_CONSOLE) = 0;                                    /* No fifos enabled */

        /* set baud rate */
        LCR(CFG_RT2880_CONSOLE) = LCR_WLS0 | LCR_WLS1 | LCR_DLAB;
        DLL(CFG_RT2880_CONSOLE) = clock_divisor &0xffff;
        LCR(CFG_RT2880_CONSOLE) = LCR_WLS0 | LCR_WLS1;
#endif
}


int serial_init(unsigned long wBaud)
{
        serial_setbrg(wBaud);

        return (0);
}

void __init prom_init(void)
{
	mips_machtype = MACH_RALINK_ROUTER;

#ifdef CONFIG_UBOOT_CMDLINE
	prom_argc = fw_arg0;
	_prom_argv = (int*) fw_arg1;
	_prom_envp = (int*) fw_arg2;
#endif

	prom_init_cmdline();
	prom_init_sysclk();

	set_io_port_base(KSEG1);
	write_c0_wired(0);
	serial_init(UART_BAUDRATE);

	prom_init_serial_port();	/* Needed for Serial Console */
	prom_meminit();
	prom_usbinit();			/* USB power saving */

	prom_setup_printf(prom_get_ttysnum());
	prom_printf("\nLINUX started...\n");
#if defined(CONFIG_RT2880_FPGA) || defined(CONFIG_RT3052_FPGA) || defined(CONFIG_RT3352_FPGA) || defined(CONFIG_RT2883_FPGA) || defined(CONFIG_RT3883_FPGA) || defined(CONFIG_RT5350_FPGA)
	prom_printf("\n THIS IS FPGA\n");
#elif defined(CONFIG_RT2880_ASIC) || defined(CONFIG_RT3052_ASIC) || defined(CONFIG_RT3352_ASIC) || defined (CONFIG_RT2883_ASIC) || defined (CONFIG_RT3883_ASIC) || defined (CONFIG_RT5350_ASIC)
	prom_printf("\n THIS IS ASIC\n");
#endif
}

