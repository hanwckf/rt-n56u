#include <linux/kernel.h>       /* printk() */
#include <linux/slab.h>
#include <linux/delay.h>

#include "xhci-mtk.h"
#include "xhci-mtk-power.h"
#include "../xhci.h"

static int g_num_u3_port;
static int g_num_u2_port;

void enableXhciAllPortPower(struct xhci_hcd *xhci)
{
	int i;
	u32 port_id, temp;
	//u32 __iomem *addr;
	__le32 __iomem addr;

	g_num_u3_port = SSUSB_U3_PORT_NUM(readl((const volatile void __iomem *)SSUSB_IP_CAP));
	g_num_u2_port = SSUSB_U2_PORT_NUM(readl((const volatile void __iomem *)SSUSB_IP_CAP));
	
	for (i = 1; i <= g_num_u3_port; i++) {
		port_id = i;
		addr =  (__le32 __iomem)(&xhci->op_regs->port_status_base);
		addr += NUM_PORT_REGS * ((port_id - 1) & 0xff) * sizeof(__le32);
		temp = xhci_readl(xhci, (__le32 __iomem *)addr);
		temp = xhci_port_state_to_neutral(temp);
		temp |= PORT_POWER;
		xhci_writel(xhci, temp, (__le32 __iomem *)addr);
	}
	for (i = 1; i <= g_num_u2_port; i++) {
		port_id=i+g_num_u3_port;
		addr =  (__le32 __iomem)(&xhci->op_regs->port_status_base);
		addr += NUM_PORT_REGS * ((port_id - 1) & 0xff) * sizeof(__le32);
		temp = xhci_readl(xhci, (__le32 __iomem *)addr);
		temp = xhci_port_state_to_neutral(temp);
		temp |= PORT_POWER;
		xhci_writel(xhci, temp, (__le32 __iomem *)addr);
	}
}

void enableAllClockPower(void)
{
	int i;
	u32 temp;

	g_num_u3_port = SSUSB_U3_PORT_NUM(readl((const volatile void __iomem *)SSUSB_IP_CAP));
	g_num_u2_port = SSUSB_U2_PORT_NUM(readl((const volatile void __iomem *)SSUSB_IP_CAP));

	//2.	Enable xHC
	writel(readl((const volatile void __iomem *)SSUSB_IP_PW_CTRL) | (SSUSB_IP_SW_RST), (volatile void __iomem *)SSUSB_IP_PW_CTRL);
	writel(readl((const volatile void __iomem *)SSUSB_IP_PW_CTRL) & (~SSUSB_IP_SW_RST), (volatile void __iomem *)SSUSB_IP_PW_CTRL);
	writel(readl((const volatile void __iomem *)SSUSB_IP_PW_CTRL_1) & (~SSUSB_IP_PDN), (volatile void __iomem *)SSUSB_IP_PW_CTRL_1);

	//1.	Enable target ports 
	for(i=0; i<g_num_u3_port; i++){
		temp = readl((const volatile void __iomem *)SSUSB_U3_CTRL(i));
		temp = temp & (~SSUSB_U3_PORT_PDN) & (~SSUSB_U3_PORT_DIS);
		writel(temp, (volatile void __iomem *)SSUSB_U3_CTRL(i));
	}
	for(i=0; i<g_num_u2_port; i++){
		temp = readl((const volatile void __iomem *)SSUSB_U2_CTRL(i));
		temp = temp & (~SSUSB_U2_PORT_PDN) & (~SSUSB_U2_PORT_DIS);
		writel(temp, (volatile void __iomem *)SSUSB_U2_CTRL(i));
	}
	msleep(100);
}


//(X)disable clock/power of a port 
//(X)if all ports are disabled, disable IP ctrl power
//disable all ports and IP clock/power, this is just mention HW that the power/clock of port 
//and IP could be disable if suspended.
//If doesn't not disable all ports at first, the IP clock/power will never be disabled
//(some U2 and U3 ports are binded to the same connection, that is, they will never enter suspend at the same time
//port_index: port number
//port_rev: 0x2 - USB2.0, 0x3 - USB3.0 (SuperSpeed)
void disablePortClockPower(void)
{
	int i;
	u32 temp;

	g_num_u3_port = SSUSB_U3_PORT_NUM(readl((const volatile void __iomem *)SSUSB_IP_CAP));
	g_num_u2_port = SSUSB_U2_PORT_NUM(readl((const volatile void __iomem *)SSUSB_IP_CAP));

	for(i=0; i<g_num_u3_port; i++){
		temp = readl((const volatile void __iomem *)SSUSB_U3_CTRL(i));
		temp = temp | (SSUSB_U3_PORT_PDN);
		writel(temp, (volatile void __iomem *)SSUSB_U3_CTRL(i));
	}
	for(i=0; i<g_num_u2_port; i++){
		temp = readl((const volatile void __iomem *)SSUSB_U2_CTRL(i));
		temp = temp | (SSUSB_U2_PORT_PDN);
		writel(temp, (volatile void __iomem *)SSUSB_U2_CTRL(i));
	}
	writel(readl((const volatile void __iomem *)SSUSB_IP_PW_CTRL_1) | (SSUSB_IP_PDN), (volatile void __iomem *)SSUSB_IP_PW_CTRL_1);
}

//if IP ctrl power is disabled, enable it
//enable clock/power of a port
//port_index: port number
//port_rev: 0x2 - USB2.0, 0x3 - USB3.0 (SuperSpeed)
void enablePortClockPower(int port_index, int port_rev)
{
	u32 temp;

	writel(readl((const volatile void __iomem *)SSUSB_IP_PW_CTRL_1) & (~SSUSB_IP_PDN), (volatile void __iomem *)SSUSB_IP_PW_CTRL_1);

	if(port_rev == 0x3){
		temp = readl((const volatile void __iomem *)SSUSB_U3_CTRL(port_index));
		temp = temp & (~SSUSB_U3_PORT_PDN);
		writel(temp, (volatile void __iomem *)SSUSB_U3_CTRL(port_index));
	}
	else if(port_rev == 0x2){
		temp = readl((const volatile void __iomem *)SSUSB_U2_CTRL(port_index));
		temp = temp & (~SSUSB_U2_PORT_PDN);
		writel(temp, (volatile void __iomem *)SSUSB_U2_CTRL(port_index));
	}
}

