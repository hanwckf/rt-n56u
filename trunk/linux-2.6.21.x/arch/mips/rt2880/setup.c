/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     board setup for Ralink RT2880 solution
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
#include <linux/sched.h>
#include <linux/mc146818rtc.h>
#include <linux/ioport.h>

#include <asm/cpu.h>
#include <asm/bootinfo.h>
#include <asm/irq.h>
#include <asm/rt2880/generic.h>
#include <asm/rt2880/prom.h>
#include <asm/rt2880/surfboardint.h>
#include <asm/time.h>
#include <asm/traps.h>

#if defined(CONFIG_SERIAL_CONSOLE) || defined(CONFIG_PROM_CONSOLE)
extern void console_setup(char *, int *);
char serial_console[20];
#endif

#ifdef CONFIG_KGDB
extern void rs_kgdb_hook(int);
extern void saa9730_kgdb_hook(void);
extern void breakpoint(void);
int remote_debug = 0;
#endif

extern void mips_reboot_setup(void);

const char *get_system_type(void)
{
    uint8_t Id[10];

    memset(Id, 0, sizeof(Id));
    strncpy(Id, (char *)RALINK_CHIPID, 8);

    if(strcmp(Id,"RT2880  ")==0) {
	return "RT2880 Ralink SoC";
    } else if(strcmp(Id,"RT3052  ")==0) {
	return "RT3052 Ralink SoC";
    } else if(strcmp(Id,"RT3883  ")==0) {
	return "RT3883 Ralink SoC";
    } else if(strcmp(Id,"RT3350  ")==0) {
	return "RT3350 Ralink SoC";
    } else if(strcmp(Id,"RT3352  ")==0) {
	return "RT3352 Ralink SoC";
    } else if(strcmp(Id,"RT6855  ")==0) {
	return "RT6855 Ralink SoC";
    }

    return "Unknown Ralink SoC";
}

extern void mips_time_init(void);
extern void mips_timer_setup(struct irqaction *irq);

void __init rt2880_setup(void)
{
#ifdef CONFIG_KGDB
	int rs_putDebugChar(char);
	char rs_getDebugChar(void);
	int saa9730_putDebugChar(char);
	char saa9730_getDebugChar(void);
	extern int (*generic_putDebugChar)(char);
	extern char (*generic_getDebugChar)(void);
#endif
	char *argptr;

	iomem_resource.start = 0;
	iomem_resource.end= ~0;
	ioport_resource.start= 0;
	ioport_resource.end = ~0;
#ifdef CONFIG_SERIAL_CONSOLE
	argptr = prom_getcmdline();
	if ((argptr = strstr(argptr, "console=ttyS")) == NULL) {
		int i = 0;
		char *s =(char *) prom_getenv("modetty0");
		while(s[i] >= '0' && s[i] <= '9')
			i++;
		strcpy(serial_console, "ttyS1,");
		strncpy(serial_console + 6, s, i);
		printk("Config serial console: %s\n", serial_console);
		console_setup(serial_console, NULL);
	}
#endif

#ifdef CONFIG_KGDB
	argptr = prom_getcmdline();
	if ((argptr = strstr(argptr, "kgdb=ttyS")) != NULL) {
		int line;
		argptr += strlen("kgdb=ttyS");
		if (*argptr != '0' && *argptr != '1')
			printk("KGDB: Uknown serial line /dev/ttyS%c, "
			       "falling back to /dev/ttyS1\n", *argptr);
		line = *argptr == '0' ? 0 : 1;
		printk("KGDB: Using serial line /dev/ttyS%d for session\n",
		       line ? 1 : 0);

		if(line == 0) {
			rs_kgdb_hook(line);
			generic_putDebugChar = rs_putDebugChar;
			generic_getDebugChar = rs_getDebugChar;
		} else {
			saa9730_kgdb_hook();
			generic_putDebugChar = saa9730_putDebugChar;
			generic_getDebugChar = saa9730_getDebugChar;
		}

		printk("KGDB: Using serial line /dev/ttyS%d for session, "
			    "please connect your debugger\n", line ? 1 : 0);

		remote_debug = 1;
		/* Breakpoints and stuff are in surfboard_irq_setup() */
	}
#endif
	argptr = prom_getcmdline();

	if ((argptr = strstr(argptr, "nofpu")) != NULL)
		cpu_data[0].options &= ~MIPS_CPU_FPU;

	board_time_init = mips_time_init;
	mips_reboot_setup();
}

#ifdef CONFIG_RAM_SIZE_AUTO
extern unsigned long detect_ram_sequence[3];
#endif
void __init plat_mem_setup(void)
{
#ifdef CONFIG_RAM_SIZE_AUTO
    printk("DetectRAMsequence\nMAX memory:[%ld]\nRAM size detected:[%ld]\nFullviewRAM:[%ld]\n",
           detect_ram_sequence[0],detect_ram_sequence[1],detect_ram_sequence[2]);
#else
    printk("Fixed Ramsize = %d MBytes\n", CONFIG_RALINK_RAM_SIZE );
#endif
    rt2880_setup();
}
