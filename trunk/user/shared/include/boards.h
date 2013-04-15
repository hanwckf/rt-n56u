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
#ifndef _BOARDS_H_
#define _BOARDS_H_

////////////////////////////////////////////////////////////////////////////////
// BOARD DEPENDENCIES
////////////////////////////////////////////////////////////////////////////////

#if defined(BOARD_N56U)
 #define BOARD_PID	"RT-N56U"
 #define BOARD_NAME	"RT-N56U"
 #define BTN_RESET	13
 #define BTN_WPS	26
 #undef  LED_ALL
 #define LED_POWER	0
 #define LED_LAN	19
 #define LED_WAN	27
 #define LED_USB	24
 #define RT3883_RF_TX	2
 #define RT3883_RF_RX	3
 #define INIC_RF_TX	2
 #define INIC_RF_RX	2
 #define ETH_PHY_LEDS	2
 #define NUM_USB_PORTS	2
#elif defined(BOARD_N65U)
 #define BOARD_PID	"RT-N65U"
 #define BOARD_NAME	"RT-N65U"
 #define BTN_RESET	13
 #define BTN_WPS	26
 #define LED_ALL	10
 #define LED_POWER	0
 #define LED_LAN	19
 #define LED_WAN	27
 #define LED_USB	24
 #define RT3883_RF_TX	3
 #define RT3883_RF_RX	3
 #define INIC_RF_TX	2
 #define INIC_RF_RX	2
 #define ETH_PHY_LEDS	2
 #define NUM_USB_PORTS	2
#elif defined(BOARD_SWR1100)
 #define BOARD_PID	"SWR1100"
 #define BOARD_NAME	"SWR-1100"
 #define BTN_RESET	6
 #define BTN_WPS	3
 #undef  LED_ALL
 #define LED_POWER	0
 #undef  LED_LAN
 #undef  LED_WAN
 #define LED_USB	25
 #define RT3883_RF_TX	2
 #define RT3883_RF_RX	2
 #define INIC_RF_TX	2
 #define INIC_RF_RX	2
 #define ETH_PHY_LEDS	1
 #define NUM_USB_PORTS	1
#elif defined(BOARD_BN750DB)
 #define BOARD_PID	"BN750DB"
 #define BOARD_NAME	"BL-N750DB"
 #define BTN_RESET	25
 #define BTN_WPS	26
 #undef  LED_ALL
 #define LED_POWER	0
 #define LED_LAN	13
 #define LED_WAN	12
 #define LED_USB	9
 #define RT3883_RF_TX	3
 #define RT3883_RF_RX	3
 #define INIC_RF_TX	2
 #define INIC_RF_RX	2
 #define ETH_PHY_LEDS	0
 #define NUM_USB_PORTS	2
#endif

#define BTN_PRESSED	0
#define LED_ON		0
#define LED_OFF		1


#endif

