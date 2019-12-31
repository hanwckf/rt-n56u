/* mod for Lenovo newifi d1 based on ASUS RT-N56U B1*/

#define BOARD_PID		"B70"
#define BOARD_NAME		"B70"
#define BOARD_DESC		"B70 Wireless Router"
#define BOARD_VENDOR_NAME	"ASUSTek Computer Inc."
#define BOARD_VENDOR_URL	"http://80x86.io/"
#define BOARD_MODEL_URL		"http://80x86.io/"
#define BOARD_BOOT_TIME		80
#define BOARD_FLASH_TIME	80
#define BOARD_GPIO_BTN_RESET 18
#undef BOARD_GPIO_BTN_WPS
#undef  BOARD_GPIO_LED_ALL
#define BOARD_GPIO_LED_WIFI 7 /* red */
#undef BOARD_GPIO_LED_SW2G	/* soft led */
#undef BOARD_GPIO_LED_SW5G	/* soft led */
#undef BOARD_GPIO_LED_POWER
#undef  BOARD_GPIO_LED_LAN
#define BOARD_GPIO_LED_WAN	6	/* white */
#undef BOARD_GPIO_LED_USB
#undef  BOARD_GPIO_LED_ROUTER
#define BOARD_GPIO_PWR_USB_ON	0	/* 0: 5V Power ON, 1: 5V Power OFF */
#define  BOARD_GPIO_PWR_USB 12  /* gpio #12 control usb 2.0 power on */
#define BOARD_HAS_5G_11AC	1
#define BOARD_NUM_ANT_5G_TX	2
#define BOARD_NUM_ANT_5G_RX	2
#define BOARD_NUM_ANT_2G_TX	2
#define BOARD_NUM_ANT_2G_RX	2
#define BOARD_NUM_ETH_LEDS	0
#define BOARD_NUM_ETH_EPHY 4
#define BOARD_HAS_EPHY_L1000	1
#define BOARD_HAS_EPHY_W1000	1
#define BOARD_NUM_UPHY_USB3	1
#define BOARD_USB_PORT_SWAP	1