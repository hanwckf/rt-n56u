#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>

#include <asm/rt2880/rt_mmap.h>

#if defined (CONFIG_RALINK_MT7621)
#define ADDR_SIFSLV_BASE		0xBE1D0000
#define ADDR_SIFSLV_FM_FEG_BASE		(ADDR_SIFSLV_BASE+0x0100)
#define ADDR_U2_PHY_P0_BASE		(ADDR_SIFSLV_BASE+0x0800)
#define ADDR_U2_PHY_P1_BASE		(ADDR_SIFSLV_BASE+0x1000)
#define U2_SR_COEFF			28
#elif defined (CONFIG_RALINK_MT7628)
#define ADDR_SIFSLV_FM_FEG_BASE		(RALINK_USB_DEV_BASE+0xf00)
#define ADDR_U2_PHY_P0_BASE		(RALINK_USB_DEV_BASE+0x800)
#define U2_SR_COEFF			32
#endif

#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)

#define REG_SIFSLV_FM_FEG_FMCR0		(ADDR_SIFSLV_FM_FEG_BASE+0x00)
#define REG_SIFSLV_FM_FEG_FMCR1		(ADDR_SIFSLV_FM_FEG_BASE+0x04)
#define REG_SIFSLV_FM_FEG_FMCR2		(ADDR_SIFSLV_FM_FEG_BASE+0x08)
#define REG_SIFSLV_FM_FEG_FMMONR0	(ADDR_SIFSLV_FM_FEG_BASE+0x0C)
#define REG_SIFSLV_FM_FEG_FMMONR1	(ADDR_SIFSLV_FM_FEG_BASE+0x10)

/* SIFSLV_FM_FEG_FMCR0 */
#define RG_LOCKTH			(0xf<<28)
#define RG_MONCLK_SEL			(0x3<<26)
#define RG_FM_MODE			(0x1<<25)
#define RG_FREQDET_EN			(0x1<<24)
#define RG_CYCLECNT			(0x00ffffff)

/* SIFSLV_FM_FEG_FMMONR1 */
#define RG_MONCLK_SEL_3			(0x1<<9)
#define RG_FRCK_EN			(0x1<<8)
#define USBPLL_LOCK			(0x1<<1)
#define USB_FM_VLD			(0x1<<0)

#define OFS_U2_PHY_AC0			0x00
#define OFS_U2_PHY_AC1			0x04
#define OFS_U2_PHY_AC2			0x08
#define OFS_U2_PHY_ACR0			0x10
#define OFS_U2_PHY_ACR1			0x14
#define OFS_U2_PHY_ACR2			0x18
#define OFS_U2_PHY_ACR3			0x1C
#define OFS_U2_PHY_ACR4			0x20
#define OFS_U2_PHY_AMON0		0x24
#define OFS_U2_PHY_DCR0			0x60
#define OFS_U2_PHY_DCR1			0x64
#define OFS_U2_PHY_DTM0			0x68
#define OFS_U2_PHY_DTM1			0x6C

/* U2_PHY_ACR0 */
#define RG_USB20_ICUSB_EN		(0x1<<24)
#define RG_USB20_HSTX_SRCAL_EN		(0x1<<23)
#define RG_USB20_HSTX_SRCTRL		(0x7<<16)
#define RG_USB20_LS_CR			(0x7<<12)
#define RG_USB20_FS_CR			(0x7<<8)
#define RG_USB20_LS_SR			(0x7<<4)
#define RG_USB20_FS_SR			(0x7<<0)

static atomic_t uphy_init_instance = ATOMIC_INIT(0);

static void
u2_slew_rate_calibration(int port_id, u32 u2_phy_reg_base)
{
	int i;
	u32 reg_val;
	u32 u4FmOut = 0;

	// => RG_USB20_HSTX_SRCAL_EN = 1
	// enable HS TX SR calibration
	reg_val = sysRegRead(u2_phy_reg_base + OFS_U2_PHY_ACR0);
	reg_val |= RG_USB20_HSTX_SRCAL_EN;
	sysRegWrite(u2_phy_reg_base + OFS_U2_PHY_ACR0, reg_val);
	msleep(1);

	// => RG_FRCK_EN = 1
	// Enable free run clock
	reg_val = sysRegRead(REG_SIFSLV_FM_FEG_FMMONR1);
	reg_val |= RG_FRCK_EN;
	sysRegWrite(REG_SIFSLV_FM_FEG_FMMONR1, reg_val);

	// MT6290 HS signal quality patch
	// => RG_CYCLECNT = 400
	// Setting cyclecnt = 400
	reg_val = sysRegRead(REG_SIFSLV_FM_FEG_FMCR0);
	reg_val &= ~RG_CYCLECNT;
	reg_val |= 0x400;
	sysRegWrite(REG_SIFSLV_FM_FEG_FMCR0, reg_val);

	// => RG_FREQDET_EN = 1
	// Enable frequency meter
	reg_val = sysRegRead(REG_SIFSLV_FM_FEG_FMCR0);
	reg_val |= RG_FREQDET_EN;
	sysRegWrite(REG_SIFSLV_FM_FEG_FMCR0, reg_val);

	// wait for FM detection done, set 10ms timeout
	for (i = 0; i < 10; i++) {
		// => u4FmOut = USB_FM_OUT
		// read FM_OUT
		u4FmOut = sysRegRead(REG_SIFSLV_FM_FEG_FMMONR0);
		
		// check if FM detection done
		if (u4FmOut != 0)
			break;
		
		msleep(1);
	}

	// => RG_FREQDET_EN = 0
	// disable frequency meter
	reg_val = sysRegRead(REG_SIFSLV_FM_FEG_FMCR0);
	reg_val &= ~RG_FREQDET_EN;
	sysRegWrite(REG_SIFSLV_FM_FEG_FMCR0, reg_val);

	// => RG_FRCK_EN = 0
	// disable free run clock
	reg_val = sysRegRead(REG_SIFSLV_FM_FEG_FMMONR1);
	reg_val &= ~RG_FRCK_EN;
	sysRegWrite(REG_SIFSLV_FM_FEG_FMMONR1, reg_val);

	// => RG_USB20_HSTX_SRCAL_EN = 0
	// disable HS TX SR calibration
	reg_val = sysRegRead(u2_phy_reg_base + OFS_U2_PHY_ACR0);
	reg_val &= ~RG_USB20_HSTX_SRCAL_EN;
	sysRegWrite(u2_phy_reg_base + OFS_U2_PHY_ACR0, reg_val);
	msleep(1);

	// => RG_USB20_HSTX_SRCTRL
	reg_val = sysRegRead(u2_phy_reg_base + OFS_U2_PHY_ACR0);
	reg_val &= ~RG_USB20_HSTX_SRCTRL;
	if (u4FmOut != 0) {
		// set reg = (1024/FM_OUT) * 25 * 0.028 (round to the nearest digits)
		u32 u4Tmp = (((1024 * 25 * U2_SR_COEFF) / u4FmOut) + 500) / 1000;
		reg_val |= ((u4Tmp & 0x07)<<16);
		printk("U2PHY P%d set SRCTRL %s value: %d\n", port_id, "calibration", u4Tmp);
	} else {
		reg_val |= (0x4<<16);
		printk("U2PHY P%d set SRCTRL %s value: %d\n", port_id, "default", 4);
	}
	sysRegWrite(u2_phy_reg_base + OFS_U2_PHY_ACR0, reg_val);
}
#endif

#if defined (CONFIG_RALINK_MT7621)

void
uphy_init(void)
{
	if (atomic_inc_return(&uphy_init_instance) != 1)
		return;

	u2_slew_rate_calibration(0, ADDR_U2_PHY_P0_BASE);
	u2_slew_rate_calibration(1, ADDR_U2_PHY_P1_BASE);
}

#elif defined (CONFIG_RALINK_MT7628)

static void
u2_phy_init(void)
{
	u32 reg_val;

	reg_val = sysRegRead(ADDR_U2_PHY_P0_BASE + OFS_U2_PHY_AC2);
	reg_val = sysRegRead(ADDR_U2_PHY_P0_BASE + OFS_U2_PHY_ACR0);
	reg_val = sysRegRead(ADDR_U2_PHY_P0_BASE + OFS_U2_PHY_DCR0);

	sysRegWrite(ADDR_U2_PHY_P0_BASE + OFS_U2_PHY_DCR0, 0x00ffff02);
	reg_val = sysRegRead(ADDR_U2_PHY_P0_BASE + OFS_U2_PHY_DCR0);
	sysRegWrite(ADDR_U2_PHY_P0_BASE + OFS_U2_PHY_DCR0, 0x00555502);
	reg_val = sysRegRead(ADDR_U2_PHY_P0_BASE + OFS_U2_PHY_DCR0);
	sysRegWrite(ADDR_U2_PHY_P0_BASE + OFS_U2_PHY_DCR0, 0x00aaaa02);
	reg_val = sysRegRead(ADDR_U2_PHY_P0_BASE + OFS_U2_PHY_DCR0);
	sysRegWrite(ADDR_U2_PHY_P0_BASE + OFS_U2_PHY_DCR0, 0x00000402);
	reg_val = sysRegRead(ADDR_U2_PHY_P0_BASE + OFS_U2_PHY_DCR0);

	sysRegWrite(ADDR_U2_PHY_P0_BASE + OFS_U2_PHY_AC0, 0x0048086a);
	sysRegWrite(ADDR_U2_PHY_P0_BASE + OFS_U2_PHY_AC1, 0x4400001c);
	sysRegWrite(ADDR_U2_PHY_P0_BASE + OFS_U2_PHY_ACR3, 0xc0200000);
	sysRegWrite(ADDR_U2_PHY_P0_BASE + OFS_U2_PHY_DTM0, 0x02000000);
}

void
uphy_init(void)
{
	if (atomic_inc_return(&uphy_init_instance) != 1)
		return;

	u2_phy_init();
	u2_slew_rate_calibration(0, ADDR_U2_PHY_P0_BASE);
}

#endif
