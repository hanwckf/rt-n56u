/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>

#include "ralink_mdio.h"
#include "ralink_gpp.h"

#if defined(CONFIG_RTL8367_API_8370)
#include "api_8370/rtk_error.h"
#else
#include "api_8367b/rtk_error.h"
#endif

#define MDC_MDIO_CTRL0_REG		31
#define MDC_MDIO_START_REG		29
#define MDC_MDIO_CTRL1_REG		21
#define MDC_MDIO_ADDRESS_REG		23
#define MDC_MDIO_DATA_WRITE_REG		24
#define MDC_MDIO_DATA_READ_REG		25
#define MDC_MDIO_PREAMBLE_LEN		32

#define MDC_MDIO_START_OP		0xFFFF
#define MDC_MDIO_ADDR_OP		0x000E
#define MDC_MDIO_READ_OP		0x0001
#define MDC_MDIO_WRITE_OP		0x0003

static spinlock_t g_mdio_lock;
static u32 g_phy_id = 0;

extern u32 mii_mgr_init(void);
extern u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data);
extern u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data);

#define MDC_MDIO_READ(a, b, c, d)		mii_mgr_read(b, c, d);
#define MDC_MDIO_WRITE(a, b, c, d)		mii_mgr_write(b, c, d);

/////////////////////////////////////////////////////////////////////////////////

void mdio_init(u32 phy_id)
{
	g_phy_id = phy_id;
	
	spin_lock_init(&g_mdio_lock);
	
	mii_mgr_init();
}

int smi_read(u32 addr, u32 *data)
{
	unsigned long flags;
	spin_lock_irqsave(&g_mdio_lock, flags);

	/* We enable mdio gpio purpose register, and disable it when exit. */
	gpio_set_mdio_unlocked(1);

	/* Write Start command to register 29 */
	MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Write address control code to register 31 */
	MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_CTRL0_REG, MDC_MDIO_ADDR_OP);

	/* Write Start command to register 29 */
	MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Write address to register 23 */
	MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_ADDRESS_REG, addr);

	/* Write Start command to register 29 */
	MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Write read control code to register 21 */
	MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_CTRL1_REG, MDC_MDIO_READ_OP);

	/* Write Start command to register 29 */
	MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Read data from register 25 */
	MDC_MDIO_READ(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_DATA_READ_REG, data);

	gpio_set_mdio_unlocked(0);

	spin_unlock_irqrestore(&g_mdio_lock, flags);

	return RT_ERR_OK;
}

int smi_write(u32 addr, u32 data)
{
	unsigned long flags;
	spin_lock_irqsave(&g_mdio_lock, flags);

	/* We enable mdio gpio purpose register, and disable it when exit. */
	gpio_set_mdio_unlocked(1);

	/* Write Start command to register 29 */
	MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Write address control code to register 31 */
	MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_CTRL0_REG, MDC_MDIO_ADDR_OP);

	/* Write Start command to register 29 */
	MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Write address to register 23 */
	MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_ADDRESS_REG, addr);

	/* Write Start command to register 29 */
	MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Write data to register 24 */
	MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_DATA_WRITE_REG, data);

	/* Write Start command to register 29 */
	MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Write data control code to register 21 */
	MDC_MDIO_WRITE(MDC_MDIO_PREAMBLE_LEN, g_phy_id, MDC_MDIO_CTRL1_REG, MDC_MDIO_WRITE_OP);

	gpio_set_mdio_unlocked(0);

	spin_unlock_irqrestore(&g_mdio_lock, flags);

	return RT_ERR_OK;
}
