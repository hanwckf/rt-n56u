/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     memory setup for Ralink RT2880 solution
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
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/ioport.h>
#include <asm/bootinfo.h>
#include <asm/page.h>

#include <asm/rt2880/prom.h>

//#define DEBUG

#define RAM_BASE		0x00000000
#define RAM_SIZE		(CONFIG_RALINK_RAM_SIZE*1024*1024)
#define RAM_SIZE_MIN		(8*1024*1024)

#if defined (CONFIG_RALINK_RT3052) || \
    defined (CONFIG_RALINK_RT5350)
#define RAM_SIZE_MAX		(64*1024*1024)
#elif defined (CONFIG_RALINK_MT7621)
#define RAM_SIZE_MAX		(512*1024*1024)
#else
#define RAM_SIZE_MAX		(256*1024*1024)
#endif

#define PFN_ALIGN(x)		(((unsigned long)(x) + (PAGE_SIZE - 1)) & PAGE_MASK)

/* References to section boundaries */
extern char _end;

static unsigned int __init prom_get_ramsize(void)
{
	char *env_str;
	unsigned int ramsize = 0;

	env_str = prom_getenv("memsize");
	if (env_str == NULL)
		env_str = prom_getenv("ramsize");
	
	if (env_str != NULL)
	{
		printk("prom memory:%sMB\n", env_str);
		ramsize = simple_strtol(env_str, NULL, 0);
		ramsize = ramsize * 1024 * 1024;
	}
        
	return ramsize;
}


void __init prom_meminit(void)
{
	phys_t ramsize = 0;


	ramsize = (phys_t)prom_get_ramsize();

	if (ramsize < RAM_SIZE_MIN || ramsize > RAM_SIZE_MAX)
		ramsize = RAM_SIZE;

#if defined(CONFIG_RALINK_MT7621)
	if (ramsize > 0x1c000000) {
		/* 1. Normal region 0..448MB */
		add_memory_region(RAM_BASE, 0x1c000000, BOOT_MEM_RAM);
		
#ifdef CONFIG_HIGHMEM
		/* 2. Highmem region */
		add_memory_region(0x20000000, (ramsize - 0x1c000000), BOOT_MEM_RAM);
#endif
	} else
#endif
	add_memory_region(RAM_BASE, ramsize, BOOT_MEM_RAM);
}

void __init prom_free_prom_memory(void)
{
#ifdef DEBUG
	/* Nothing to do! Need only for DEBUG. */
	/* This is may be corrupt working memory. */

	unsigned long addr;
	int i;

	for (i = 0; i < boot_mem_map.nr_map; i++) {
		if (boot_mem_map.map[i].type != BOOT_MEM_ROM_DATA)
			continue;

		addr = boot_mem_map.map[i].addr;
		free_init_pages("prom memory", addr, addr + boot_mem_map.map[i].size);
	}
#endif
}

