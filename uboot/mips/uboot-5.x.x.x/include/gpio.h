#ifndef _GPIO_H_
#define _GPIO_H_

extern void gpio_init(void);
extern void gpio_init_mdio(void);
extern void gpio_init_usb(int do_wait);

extern int DETECT_BTN_RESET(void);
extern int DETECT_BTN_WPS(void);

extern void LED_HIDE_ALL(void);
extern void LED_POWER_ON(void);

extern void LED_ALERT_ON(void);
extern void LED_ALERT_OFF(void);
extern void LED_ALERT_BLINK(void);

#endif
