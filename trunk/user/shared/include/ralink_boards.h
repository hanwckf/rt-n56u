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

////////////////////////////////////////////////////////////////////////////////
// BOARD DEPENDENCIES
////////////////////////////////////////////////////////////////////////////////

#if defined(BOARD_N56U)
 #define BOARD_PID		"RT-N56U"
 #define BOARD_NAME		"RT-N56U"
 #define BOARD_DESC		"ASUS RT-N56U Wireless Router"
 #define BOARD_VENDOR_NAME	"ASUSTek Computer Inc."
 #define BOARD_VENDOR_URL	"http://www.asus.com/"
 #define BOARD_MODEL_URL	"http://www.asus.com/Networking/RTN56U/"
 #define BOARD_FLASH_TIME	180
 #define BOARD_GPIO_BTN_RESET	13
 #define BOARD_GPIO_BTN_WPS	26
 #undef  BOARD_GPIO_BTN_WLTOG
 #undef  BOARD_GPIO_LED_ALL
 #undef  BOARD_GPIO_LED_WIFI
 #define BOARD_GPIO_LED_POWER	0
 #define BOARD_GPIO_LED_LAN	19
 #define BOARD_GPIO_LED_WAN	27
 #define BOARD_GPIO_LED_USB	24
 #define BOARD_HAS_5G_11AC	0
 #define BOARD_5G_IN_SOC	1
 #define BOARD_2G_IN_SOC	0
 #define BOARD_NUM_ANT_5G_TX	2
 #define BOARD_NUM_ANT_5G_RX	3
 #define BOARD_NUM_ANT_2G_TX	2
 #define BOARD_NUM_ANT_2G_RX	2
 #define BOARD_NUM_ETH_LEDS	2
 #define BOARD_HAS_EPHY_1000	1
#elif defined(BOARD_N65U)
 #define BOARD_PID		"RT-N65U"
 #define BOARD_NAME		"RT-N65U"
 #define BOARD_DESC		"ASUS RT-N65U Wireless Router"
 #define BOARD_VENDOR_NAME	"ASUSTek Computer Inc."
 #define BOARD_VENDOR_URL	"http://www.asus.com/"
 #define BOARD_MODEL_URL	"http://www.asus.com/Networking/RTN65U/"
 #define BOARD_FLASH_TIME	150
 #define BOARD_GPIO_BTN_RESET	13
 #define BOARD_GPIO_BTN_WPS	26
 #undef  BOARD_GPIO_BTN_WLTOG
 #define BOARD_GPIO_LED_ALL	10
 #undef  BOARD_GPIO_LED_WIFI
 #define BOARD_GPIO_LED_POWER	0
 #define BOARD_GPIO_LED_LAN	19
 #define BOARD_GPIO_LED_WAN	27
 #define BOARD_GPIO_LED_USB	24
 #define BOARD_HAS_5G_11AC	0
 #define BOARD_5G_IN_SOC	1
 #define BOARD_2G_IN_SOC	0
 #define BOARD_NUM_ANT_5G_TX	3
 #define BOARD_NUM_ANT_5G_RX	3
 #define BOARD_NUM_ANT_2G_TX	2
 #define BOARD_NUM_ANT_2G_RX	2
 #define BOARD_NUM_ETH_LEDS	2
 #define BOARD_HAS_EPHY_1000	1
#elif defined(BOARD_N11P)
 #define BOARD_PID		"RT-N11P"
 #define BOARD_NAME		"RT-N11P"
 #define BOARD_DESC		"ASUS RT-N11P Wireless Router"
 #define BOARD_VENDOR_NAME	"ASUSTek Computer Inc."
 #define BOARD_VENDOR_URL	"http://www.asus.com/"
 #define BOARD_MODEL_URL	"http://www.asus.com/Networking/RTN11P/"
 #define BOARD_FLASH_TIME	90
 #undef  BOARD_GPIO_BTN_RESET
 #define BOARD_GPIO_BTN_WPS	17
 #undef  BOARD_GPIO_BTN_WLTOG
 #undef  BOARD_GPIO_LED_ALL
 #define BOARD_GPIO_LED_WIFI	72
 #undef  BOARD_GPIO_LED_POWER
 #define BOARD_GPIO_LED_LAN	39
 #define BOARD_GPIO_LED_WAN	44
 #undef  BOARD_GPIO_LED_USB
 #define BOARD_HAS_5G_11AC	0
 #define BOARD_5G_IN_SOC	0
 #define BOARD_2G_IN_SOC	1
 #define BOARD_NUM_ANT_5G_TX	2
 #define BOARD_NUM_ANT_5G_RX	2
 #define BOARD_NUM_ANT_2G_TX	2
 #define BOARD_NUM_ANT_2G_RX	2
 #define BOARD_NUM_ETH_LEDS	0
 #define BOARD_HAS_EPHY_1000	0
#elif defined(BOARD_N14U)
 #define BOARD_PID		"RT-N14U"
 #define BOARD_NAME		"RT-N14U"
 #define BOARD_DESC		"ASUS RT-N14U Wireless Router"
 #define BOARD_VENDOR_NAME	"ASUSTek Computer Inc."
 #define BOARD_VENDOR_URL	"http://www.asus.com/"
 #define BOARD_MODEL_URL	"http://www.asus.com/Networking/RTN14U/"
 #define BOARD_FLASH_TIME	120
 #define BOARD_GPIO_BTN_RESET	1
 #define BOARD_GPIO_BTN_WPS	2
 #undef  BOARD_GPIO_BTN_WLTOG
 #undef  BOARD_GPIO_LED_ALL
 #define BOARD_GPIO_LED_WIFI	72
 #define BOARD_GPIO_LED_POWER	43
 #define BOARD_GPIO_LED_LAN	41
 #define BOARD_GPIO_LED_WAN	40
 #define BOARD_GPIO_LED_USB	42
 #define BOARD_HAS_5G_11AC	0
 #define BOARD_5G_IN_SOC	0
 #define BOARD_2G_IN_SOC	1
 #define BOARD_NUM_ANT_5G_TX	2
 #define BOARD_NUM_ANT_5G_RX	2
 #define BOARD_NUM_ANT_2G_TX	2
 #define BOARD_NUM_ANT_2G_RX	2
 #define BOARD_NUM_ETH_LEDS	0
 #define BOARD_HAS_EPHY_1000	0
#elif defined(BOARD_AC51U)
 #define BOARD_PID		"RT-AC51U"
 #define BOARD_NAME		"RT-AC51U"
 #define BOARD_DESC		"ASUS RT-AC51U Wireless Router"
 #define BOARD_VENDOR_NAME	"ASUSTek Computer Inc."
 #define BOARD_VENDOR_URL	"http://www.asus.com/"
 #define BOARD_MODEL_URL	"http://www.asus.com/Networking/RTAC51U/"
 #define BOARD_FLASH_TIME	120
 #define BOARD_GPIO_BTN_RESET	1
 #define BOARD_GPIO_BTN_WPS	2
 #undef  BOARD_GPIO_BTN_WLTOG
 #define BOARD_GPIO_LED_ALL	10
 #undef  BOARD_GPIO_LED_WIFI
 #define BOARD_GPIO_LED_POWER	9
 #undef  BOARD_GPIO_LED_LAN
 #undef  BOARD_GPIO_LED_WAN
 #define BOARD_GPIO_LED_USB	14
 #define BOARD_HAS_5G_11AC	1
 #define BOARD_5G_IN_SOC	0
 #define BOARD_2G_IN_SOC	1
 #define BOARD_NUM_ANT_5G_TX	1
 #define BOARD_NUM_ANT_5G_RX	1
 #define BOARD_NUM_ANT_2G_TX	2
 #define BOARD_NUM_ANT_2G_RX	2
 #define BOARD_NUM_ETH_LEDS	1
 #define BOARD_HAS_EPHY_1000	0
#elif defined(BOARD_AC52U)
 #define BOARD_PID		"RT-AC52U"
 #define BOARD_NAME		"RT-AC52U"
 #define BOARD_DESC		"ASUS RT-AC52U Wireless Router"
 #define BOARD_VENDOR_NAME	"ASUSTek Computer Inc."
 #define BOARD_VENDOR_URL	"http://www.asus.com/"
 #define BOARD_MODEL_URL	"http://www.asus.com/Networking/RTAC52U/"
 #define BOARD_FLASH_TIME	90
 #define BOARD_GPIO_BTN_RESET	1
 #define BOARD_GPIO_BTN_WPS	2
 #define BOARD_GPIO_BTN_WLTOG	13
 #define BOARD_GPIO_LED_ALL	10
 #undef  BOARD_GPIO_LED_WIFI
 #define BOARD_GPIO_LED_POWER	9
 #define BOARD_GPIO_LED_LAN	12
 #define BOARD_GPIO_LED_WAN	8
 #define BOARD_GPIO_LED_USB	14
 #define BOARD_HAS_5G_11AC	1
 #define BOARD_5G_IN_SOC	0
 #define BOARD_2G_IN_SOC	1
 #define BOARD_NUM_ANT_5G_TX	1
 #define BOARD_NUM_ANT_5G_RX	1
 #define BOARD_NUM_ANT_2G_TX	2
 #define BOARD_NUM_ANT_2G_RX	2
 #define BOARD_NUM_ETH_LEDS	0
 #define BOARD_HAS_EPHY_1000	0
#elif defined(BOARD_SWR1100)
 #define BOARD_PID		"SWR1100"
 #define BOARD_NAME		"SWR-1100"
 #define BOARD_DESC		"Samsung CY-SWR-1100 Wireless Router"
 #define BOARD_VENDOR_NAME	"Samsung Electronics Co."
 #define BOARD_VENDOR_URL	"http://www.samsung.com/us/"
 #define BOARD_MODEL_URL	"http://www.samsung.com/us/video/tvs-accessories/CY-SWR1100/ZA"
 #define BOARD_FLASH_TIME	180
 #define BOARD_GPIO_BTN_RESET	6
 #define BOARD_GPIO_BTN_WPS	3
 #undef  BOARD_GPIO_BTN_WLTOG
 #undef  BOARD_GPIO_LED_ALL
 #undef  BOARD_GPIO_LED_WIFI
 #define BOARD_GPIO_LED_POWER	0
 #undef  BOARD_GPIO_LED_LAN
 #undef  BOARD_GPIO_LED_WAN
 #define BOARD_GPIO_LED_USB	25
 #define BOARD_HAS_5G_11AC	0
 #define BOARD_5G_IN_SOC	1
 #define BOARD_2G_IN_SOC	0
 #define BOARD_NUM_ANT_5G_TX	2
 #define BOARD_NUM_ANT_5G_RX	2
 #define BOARD_NUM_ANT_2G_TX	2
 #define BOARD_NUM_ANT_2G_RX	2
 #define BOARD_NUM_ETH_LEDS	1
 #define BOARD_HAS_EPHY_1000	1
#elif defined(BOARD_BN750DB)
 #define BOARD_PID		"F9K1103"
 #define BOARD_NAME		"BL-N750DB"
 #define BOARD_DESC		"Belkin N750 DB Wireless Router"
 #define BOARD_VENDOR_NAME	"Belkin International Inc."
 #define BOARD_VENDOR_URL	"http://www.belkin.com/us/"
 #define BOARD_MODEL_URL	"http://www.belkin.com/us/p/P-F9K1103/"
 #define BOARD_FLASH_TIME	150
 #define BOARD_GPIO_BTN_RESET	25
 #define BOARD_GPIO_BTN_WPS	26
 #undef  BOARD_GPIO_BTN_WLTOG
 #undef  BOARD_GPIO_LED_ALL
 #undef  BOARD_GPIO_LED_WIFI
 #define BOARD_GPIO_LED_POWER	0
 #define BOARD_GPIO_LED_LAN	13
 #define BOARD_GPIO_LED_WAN	12
 #define BOARD_GPIO_LED_USB	9
 #define BOARD_HAS_5G_11AC	0
 #define BOARD_5G_IN_SOC	1
 #define BOARD_2G_IN_SOC	0
 #define BOARD_NUM_ANT_5G_TX	3
 #define BOARD_NUM_ANT_5G_RX	3
 #define BOARD_NUM_ANT_2G_TX	2
 #define BOARD_NUM_ANT_2G_RX	2
 #define BOARD_NUM_ETH_LEDS	0
 #define BOARD_HAS_EPHY_1000	1
#endif

#define BTN_PRESSED		0
#define LED_ON			0
#define LED_OFF			1

#define FW_MTD_NAME		"Firmware_Stub"
#define FW_IMG_NAME		"/tmp/linux.trx"

#ifndef BOARD_FLASH_TIME
 #define BOARD_FLASH_TIME	150
#endif

#ifndef BOARD_NUM_ETH_EPHY
 #define BOARD_NUM_ETH_EPHY	5
#endif

#endif
