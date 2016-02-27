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

#include "../raeth/mii_mgr.h"

#include "ralink_mdio.h"

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

#define MDC_MDIO_START_OP		0xFFFF
#define MDC_MDIO_ADDR_OP		0x000E
#define MDC_MDIO_READ_OP		0x0001
#define MDC_MDIO_WRITE_OP		0x0003

static DEFINE_SPINLOCK(g_mdio_lock);
static u32 g_phy_id = 0;

/////////////////////////////////////////////////////////////////////////////////

void mdio_init(u32 phy_id)
{
	g_phy_id = phy_id;

	mii_mgr_init();
}

int smi_read(u32 addr, u32 *data)
{
	spin_lock(&g_mdio_lock);

	/* Write Start command to register 29 */
	mii_mgr_write(g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Write address control code to register 31 */
	mii_mgr_write(g_phy_id, MDC_MDIO_CTRL0_REG, MDC_MDIO_ADDR_OP);

	/* Write Start command to register 29 */
	mii_mgr_write(g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Write address to register 23 */
	mii_mgr_write(g_phy_id, MDC_MDIO_ADDRESS_REG, addr);

	/* Write Start command to register 29 */
	mii_mgr_write(g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Write read control code to register 21 */
	mii_mgr_write(g_phy_id, MDC_MDIO_CTRL1_REG, MDC_MDIO_READ_OP);

	/* Write Start command to register 29 */
	mii_mgr_write(g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Read data from register 25 */
	mii_mgr_read(g_phy_id, MDC_MDIO_DATA_READ_REG, data);

	spin_unlock(&g_mdio_lock);

	return RT_ERR_OK;
}

int smi_write(u32 addr, u32 data)
{
	spin_lock(&g_mdio_lock);

	/* Write Start command to register 29 */
	mii_mgr_write(g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Write address control code to register 31 */
	mii_mgr_write(g_phy_id, MDC_MDIO_CTRL0_REG, MDC_MDIO_ADDR_OP);

	/* Write Start command to register 29 */
	mii_mgr_write(g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Write address to register 23 */
	mii_mgr_write(g_phy_id, MDC_MDIO_ADDRESS_REG, addr);

	/* Write Start command to register 29 */
	mii_mgr_write(g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Write data to register 24 */
	mii_mgr_write(g_phy_id, MDC_MDIO_DATA_WRITE_REG, data);

	/* Write Start command to register 29 */
	mii_mgr_write(g_phy_id, MDC_MDIO_START_REG, MDC_MDIO_START_OP);

	/* Write data control code to register 21 */
	mii_mgr_write(g_phy_id, MDC_MDIO_CTRL1_REG, MDC_MDIO_WRITE_OP);

	spin_unlock(&g_mdio_lock);

	return RT_ERR_OK;
}
