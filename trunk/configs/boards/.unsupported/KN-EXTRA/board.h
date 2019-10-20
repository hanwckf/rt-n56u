/* ZyXEL Keenetic Extra (ku_rc) */

#define BOARD_PID		"KN-EXTRA"
#define BOARD_NAME		"KN-EXTRA"
#define BOARD_DESC		"ZyXEL Keenetic Extra Wireless Router"
#define BOARD_VENDOR_NAME	"ZyXEL Communications Corp."
#define BOARD_VENDOR_URL	"https://zyxel.ru/"
#define BOARD_MODEL_URL		"https://zyxel.ru/keenetic-extra"
#define BOARD_BOOT_TIME		25
#define BOARD_FLASH_TIME	120
#define BOARD_GPIO_BTN_RESET	1
#define BOARD_GPIO_BTN_WPS	2
#define BOARD_GPIO_BTN_FN1	9
#undef  BOARD_GPIO_LED_ALL
#undef  BOARD_GPIO_LED_WIFI
#define BOARD_GPIO_LED_SW2G	11	/* soft-blink led (11: green, 13: orange) */
#define BOARD_GPIO_LED_SW5G	11	/* soft-blink led (11: green, 13: orange) */
#define BOARD_GPIO_LED_POWER	14
#undef  BOARD_GPIO_LED_LAN
#define BOARD_GPIO_LED_WAN	7
#define BOARD_GPIO_LED_USB	8
#undef  BOARD_GPIO_LED_ROUTER
#define BOARD_GPIO_PWR_USB	12
#define BOARD_GPIO_PWR_USB_ON	1	/* 1: 5V Power ON, 0: 5V Power OFF */
#define BOARD_HAS_5G_11AC	0
#define BOARD_NUM_ANT_5G_TX	2
#define BOARD_NUM_ANT_5G_RX	2
#define BOARD_NUM_ANT_2G_TX	2
#define BOARD_NUM_ANT_2G_RX	2
#define BOARD_NUM_ETH_LEDS	1
#define BOARD_ETH_LED_SWAP	1
#define BOARD_HAS_EPHY_L1000	1
#define BOARD_HAS_EPHY_W1000	1
