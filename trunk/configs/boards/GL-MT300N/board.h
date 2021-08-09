/* GLI MT300N */

#define BOARD_PID		"GL-MT300N"
#define BOARD_NAME		"GL-MT300N"
#define BOARD_DESC		"GL Technologies MT300N Router"
#define BOARD_VENDOR_NAME	"GL Technologies"
#define BOARD_VENDOR_URL	"https://www.gl-inet.com"
#define BOARD_MODEL_URL		"https://www.gl-inet.com/mt300n/"
#define BOARD_BOOT_TIME		20
#define BOARD_FLASH_TIME	90
#define BOARD_GPIO_BTN_RESET	1
#undef  BOARD_GPIO_BTN_WPS	2	/* free pins */
#undef  BOARD_GPIO_BTN_BT1	42	/* switch to the left */
#undef  BOARD_GPIO_BTN_BT2	43	/* switch to the right */
#undef  BOARD_GPIO_LED_ALL
#define BOARD_GPIO_LED_WIFI	72	/* front numbering 3 */
#define BOARD_GPIO_LED_POWER	41	/* front numbering 1 */
#define BOARD_GPIO_LED_LAN	40	/* front numbering 2 */
#undef  BOARD_GPIO_LED_WAN
#undef  BOARD_GPIO_LED_USB
#undef  BOARD_GPIO_LED_ROUTER
#define BOARD_GPIO_PWR_USB	26
#define BOARD_GPIO_PWR_USB_ON	1	/* 1: 5V Power ON, 0: 5V Power OFF */
#define BOARD_HAS_5G_11AC	0
#define BOARD_NUM_ANT_5G_TX	0
#define BOARD_NUM_ANT_5G_RX	0
#define BOARD_NUM_ANT_2G_TX	2
#define BOARD_NUM_ANT_2G_RX	2
#define BOARD_NUM_ETH_LEDS	0
#define BOARD_NUM_ETH_EPHY	2
#define BOARD_HAS_EPHY_L1000	0
#define BOARD_HAS_EPHY_W1000	0
