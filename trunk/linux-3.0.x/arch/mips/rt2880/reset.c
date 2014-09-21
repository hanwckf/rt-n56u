/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     reboot/reset setting for Ralink RT2880 solution
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

#include <asm/reboot.h>
#include <asm/rt2880/generic.h>
#include <linux/pm.h>
#include <linux/delay.h>

static void mips_machine_restart(char *command);
static void mips_machine_halt(void);
static void mips_machine_power_off(void);

static void mips_machine_restart(char *command)
{
#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7628)
	*(volatile u32*)(SOFTRES_REG) = RALINK_PCIE0_RST;
	mdelay(10);
#endif
	*(volatile u32*)(SOFTRES_REG) = GORESET;
	*(volatile u32*)(SOFTRES_REG) = 0;
}

static void mips_machine_halt(void)
{
#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7628)
	*(volatile u32*)(SOFTRES_REG) = RALINK_PCIE0_RST;
	mdelay(10);
#endif
	*(volatile u32*)(SOFTRES_REG) = GORESET;
	*(volatile u32*)(SOFTRES_REG) = 0;
}

static void mips_machine_power_off(void)
{
	*(volatile u32*)(POWER_DIR_REG) = POWER_DIR_OUTPUT;
	*(volatile u32*)(POWER_POL_REG) = 0;
	*(volatile u32*)(POWEROFF_REG) = POWEROFF;
}

void mips_reboot_setup(void)
{
	_machine_restart = mips_machine_restart;
	_machine_halt = mips_machine_halt;
	//_machine_power_off = mips_machine_power_off;
	pm_power_off = mips_machine_power_off;
}
