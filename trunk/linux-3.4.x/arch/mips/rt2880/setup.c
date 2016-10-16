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
#include <linux/string.h>
#include <linux/ioport.h>
#include <asm/cpu.h>
#include <asm/cpu-info.h>
#include <asm/bootinfo.h>
#if defined (CONFIG_DMA_MAYBE_COHERENT)
#include <asm/gcmpregs.h>
#include <asm/dma-coherence.h>
#endif

#include <asm/rt2880/prom.h>

extern void mips_reboot_setup(void);

const char *get_system_type(void)
{
#if defined (CONFIG_RALINK_RT3883)
	return "Ralink RT3883/RT3662 SoC";
#elif defined (CONFIG_RALINK_RT3052)
#if defined (CONFIG_RALINK_RT3350)
	return "Ralink RT3350 SoC";
#else
	return "Ralink RT3052 SoC";
#endif
#elif defined (CONFIG_RALINK_RT3352)
	return "Ralink RT3352 SoC";
#elif defined (CONFIG_RALINK_RT5350)
	return "Ralink RT5350 SoC";
#elif defined (CONFIG_RALINK_MT7620)
	return "MediaTek MT7620 SoC";
#elif defined (CONFIG_RALINK_MT7621)
	return "MediaTek MT7621 SoC";
#elif defined (CONFIG_RALINK_MT7628)
	return "MediaTek MT7628 SoC";
#else
	return "Ralink SoC";
#endif
}

#if defined (CONFIG_DMA_MAYBE_COHERENT)
static inline int plat_enable_iocoherency(void)
{
	int supported = 0;

	if (gcmp_niocu() != 0) {
		pr_info("GCMP IOCU detected\n");
#if defined (CONFIG_RALINK_MT7621)
		/* MT7621 GCMP reported about IOCU=1, but IOCU switch disabled by default */
//		supported = 1;
#endif
	}

	return supported;
}

static void __init plat_setup_iocoherency(void)
{
	/*
	 * Kernel has been configured with software coherency
	 * but we might choose to turn it off and use hardware
	 * coherency instead.
	 */
	if (plat_enable_iocoherency()) {
		coherentio = 1;
		hw_coherentio = 1;
		pr_info("Hardware DMA cache coherency enabled\n");
	}
}
#endif

void __init plat_mem_setup(void)
{
	char *argptr;

	iomem_resource.start = 0;
	iomem_resource.end = ~0;
	ioport_resource.start = 0;
	ioport_resource.end = 0x1fffffff;

	argptr = prom_getcmdline();

	if ((argptr = strstr(argptr, "nofpu")) != NULL)
		cpu_data[0].options &= ~MIPS_CPU_FPU;

#if defined (CONFIG_DMA_MAYBE_COHERENT)
	plat_setup_iocoherency();
#endif
	mips_reboot_setup();
}

