#ifndef GPIO_H
#define GPIO_H

void LEDON(void);
void LEDOFF(void);
void asus_gpio_init(void);

unsigned long DETECT(void);
unsigned long DETECT_WPS(void);

#endif
