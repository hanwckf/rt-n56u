#ifndef GPIO_H
#define GPIO_H

extern int ralink_initGpioPin2(unsigned int idx, int dir);
extern int ralink_gpio_write_bit2(int idx, int value);
extern int ralink_gpio_read_bit2(int idx);
extern void asus_gpio_init(void);

void LEDON(void);
void LEDOFF(void);
unsigned long DETECT(void);
unsigned long DETECT_WPS(void);

#endif
