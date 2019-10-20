/* ZyXEL Keenetic Ultra II (ku_rd) */

#define BOARD_PID		"KN-ULTRA2"
#define BOARD_NAME		"KN-ULTRA2"
#define BOARD_DESC		"ZyXEL Keenetic Ultra II Wireless Router"
#define BOARD_VENDOR_NAME	"ZyXEL Communications Corp."
#define BOARD_VENDOR_URL	"https://zyxel.ru/"
#define BOARD_MODEL_URL		"https://zyxel.ru/keenetic-ultra-2"
#define BOARD_BOOT_TIME		30
#define BOARD_FLASH_TIME	40
#define BOARD_GPIO_BTN_RESET	6
#define BOARD_GPIO_BTN_WPS	18
#define BOARD_GPIO_BTN_FN1	7
#define BOARD_GPIO_BTN_FN2	10
#define BOARD_GPIO_LED_INVERTED		/* LED pins value is inverted (1: LED show, 0: LED hide) */
#undef  BOARD_GPIO_LED_ALL
#undef  BOARD_GPIO_LED_WIFI
#define BOARD_GPIO_LED_SW2G	15	/* soft-blink LED */
#define BOARD_GPIO_LED_SW5G	16	/* soft-blink LED */
#define BOARD_GPIO_LED_POWER	9
#define BOARD_GPIO_LED_LAN	12	/* LED Fn */
#define BOARD_GPIO_LED_WAN	17
#define BOARD_GPIO_LED_USB	13	/* USB #1 LED */
#define BOARD_GPIO_LED_USB2	14	/* USB #2 LED */
#undef  BOARD_GPIO_LED_ROUTER
#define BOARD_GPIO_PWR_USB_ON	1	/* 1: 5V Power ON, 0: 5V Power OFF */
#define BOARD_GPIO_PWR_USB	5	/* USB2.0 5V Power */
#define BOARD_GPIO_PWR_USB2	11	/* USB3.0 5V Power */
#define BOARD_HAS_5G_11AC	1
#define BOARD_NUM_ANT_5G_TX	2
#define BOARD_NUM_ANT_5G_RX	2
#define BOARD_NUM_ANT_2G_TX	2
#define BOARD_NUM_ANT_2G_RX	2
#define BOARD_NUM_ETH_LEDS	1
#define BOARD_NUM_ETH_EPHY	8
#define BOARD_ETH_LED_SWAP	1
#define BOARD_HAS_EPHY_L1000	1
#define BOARD_HAS_EPHY_W1000	1
#define BOARD_NUM_UPHY_USB3	1
#define BOARD_USB_PORT_SWAP	1	/* USB3.0 = #2, USB2.0 = #1 */
