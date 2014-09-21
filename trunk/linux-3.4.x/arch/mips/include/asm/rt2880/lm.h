
#ifndef __ASM_MACH_MIPS_RT2880_LM_H
#define __ASM_MACH_MIPS_RT2880_LM_H

#include <linux/version.h>

struct lm_device {
	struct device		dev;
	struct resource		resource;
	unsigned int		irq;
	unsigned int		id;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20)
	void			*lm_drvdata;
#endif
};

struct lm_driver {
	struct device_driver	drv;
	int			(*probe)(struct lm_device *);
	void			(*remove)(struct lm_device *);
	int			(*suspend)(struct lm_device *, u32);
	int			(*resume)(struct lm_device *);
};

int lm_driver_register(struct lm_driver *drv);
void lm_driver_unregister(struct lm_driver *drv);

int lm_device_register(struct lm_device *dev);

# define lm_get_drvdata(lm)	((lm)->lm_drvdata)
# define lm_set_drvdata(lm,d)	do { (lm)->lm_drvdata = (d); } while (0)

#endif
