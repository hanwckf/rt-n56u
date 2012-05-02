#include <linux/hw_tcpip.h>
#include <linux/netdevice.h>

struct hw_tcpip_helpers *hw_tcpip = NULL;

int register_hw_tcpip(struct hw_tcpip_helpers *tcpip)
{
	if (!tcpip || hw_tcpip) return -1;
	hw_tcpip = tcpip;
	return 0;
}

void unregister_hw_tcpip(struct hw_tcpip_helpers *tcpip)
{
	if (tcpip == NULL) hw_tcpip = NULL;
	if (tcpip == hw_tcpip) hw_tcpip = NULL;
	return;
}

EXPORT_SYMBOL(hw_tcpip);
EXPORT_SYMBOL(unregister_hw_tcpip);
EXPORT_SYMBOL(register_hw_tcpip);
