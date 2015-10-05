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
 */

#ifndef _RALINK_BOARDS_H_
#define _RALINK_BOARDS_H_

#include <ralink_board.h>

#define FW_MTD_NAME		"Firmware_Stub"
#define FW_IMG_NAME		"/tmp/linux.trx"

#define BTN_PRESSED		0

#if defined (BOARD_GPIO_LED_INVERTED)
#define LED_ON			1
#define LED_OFF			0
#define LED_BLINK_STAY_SHOW	0
#define LED_BLINK_STAY_HIDE	1
#else
#define LED_ON			0
#define LED_OFF			1
#define LED_BLINK_STAY_SHOW	1
#define LED_BLINK_STAY_HIDE	0
#endif

#ifndef BOARD_BOOT_TIME
#define BOARD_BOOT_TIME		35
#endif

#ifndef BOARD_FLASH_TIME
#define BOARD_FLASH_TIME	150
#endif

#ifndef BOARD_NUM_UPHY_USB3
#define BOARD_NUM_UPHY_USB3	0
#endif

#ifndef BOARD_NUM_ETH_EPHY
#define BOARD_NUM_ETH_EPHY	5
#endif

#ifndef BOARD_HAS_EPHY_L1000
#define BOARD_HAS_EPHY_L1000	BOARD_HAS_EPHY_1000
#endif

#ifndef BOARD_HAS_EPHY_W1000
#define BOARD_HAS_EPHY_W1000	BOARD_HAS_EPHY_1000
#endif

#ifndef BOARD_ETH_LED_SWAP
#define BOARD_ETH_LED_SWAP	0
#endif

#ifndef BOARD_USB_PORT_SWAP
#define BOARD_USB_PORT_SWAP	0
#endif

#endif

