
#include <linux/errno.h>
#include <linux/kernel.h>

#include "ralink_smi.h"
#include "ralink_gpp.h"

/////////////////////////////////////////////////////////////////////////////////

int smi_read(u32 addr, u32 *data)
{
	return gpio_smi_read(addr, data);
}

int smi_write(u32 addr, u32 data)
{
	return gpio_smi_write(addr, data);
}

