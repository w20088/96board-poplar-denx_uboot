/******************************************************************************
 *    Copyright (c) 2009-2012 by Hisi.
 *    All rights reserved.
 * ***
 *    Create by Czy. 2012-03-19
 *
 ******************************************************************************/

#include <config.h>

#define dsb() __asm__ __volatile__ ("dsb" : : : "memory")

#define readl(_a)        ({ unsigned int __v = (*(volatile unsigned int *)(_a)); dsb(); __v; })
#define writel(_v, _a)   ({  dsb(); (*(volatile unsigned int *)(_a) = (_v)); })

/******************************************************************************/

static inline void delay(unsigned int cnt)
{
	while (cnt--)
		__asm__ __volatile__("nop");
}

/******************************************************************************/

void reg_read_write(volatile unsigned int reg_addr, unsigned int val, unsigned int mask)
{
	unsigned int v = readl(reg_addr);
	v &= ~mask;
	v |= val;
	writel(v, reg_addr);
}

/******************************************************************************/
//TODO: XXX
//#if defined(CONFIG_AVS_SUPPORT) && !defined(CONFIG_PMU_DEVICE)
void set_core_voltage(void)
{
	unsigned int regval, hpm_code, hpm_code_average, volt_reg_val;
	unsigned int max_threshold =readl(REG_BASE_SCTL+REG_SC_GEN6) >>16;
	unsigned int min_threshold = readl(REG_BASE_SCTL+REG_SC_GEN6) & 0xffff;
	unsigned int max_volt = readl(REG_BASE_SCTL+REG_SC_GEN17) >> 16;
	unsigned int min_volt = readl(REG_BASE_SCTL+REG_SC_GEN17) & 0xffff;
	unsigned int middle_volt = readl(REG_BASE_SCTL+REG_SC_GEN18) & 0xffff;

	/* hpm 0             */
	/* clock is 200M, set time division to (200/50 - 1) */
	regval  = readl(0xf8a23058);
	regval &= 0xffffffc0;
	regval |= 3;
	writel(regval, 0xf8a23058);

	/* set monitor period to 2ms */
	regval = readl(0xf8a23064);
	regval &= 0x00ffffff;
	regval |= (1 << 24);
	writel(regval, 0xf8a23064);

	/* hpm enable */
	regval = readl(0xf8a23058);
	regval |= (1 << 24);
	writel(regval, 0xf8a23058);

	/* hpm monitor enable */
	regval = readl(0xf8a23058);
	regval |= (1 << 26);
	writel(regval, 0xf8a23058);

	delay(6000000);

	regval = readl(0xf8a2305c);
	hpm_code = (regval & 0x3ff);
	hpm_code_average = hpm_code;
	hpm_code = ((regval >> 12) & 0x3ff);
	hpm_code_average += hpm_code;

	regval = readl(0xf8a23060);
	hpm_code = (regval & 0x3ff);
	hpm_code_average += hpm_code;
	hpm_code = ((regval >> 12) & 0x3ff);
	hpm_code_average += hpm_code;

	hpm_code_average = (hpm_code_average>>2);

	volt_reg_val  = readl(0xf8a2301c);
	volt_reg_val &= 0xffff;
	if (hpm_code_average >= max_threshold) {
		volt_reg_val |= max_volt << 16; /* ff chip */
	} else if (hpm_code_average < min_threshold) {
		volt_reg_val |= min_volt << 16; /* ss chip */
	} else {
		volt_reg_val |= middle_volt << 16; /* tt chip */
	}

	writel(volt_reg_val, 0xf8a2301c);
	delay(8000000);

	return;
}
//#endif
/******************************************************************************/

long long get_chipid_reg(void)
{
	long long chipid = 0;
	long long val = 0;

	chipid = (long long)readl(REG_BASE_SCTL + REG_SC_SYSID0);
	val = (long long)readl(REG_BASE_PERI_CTRL + REG_PERI_SOC_FUSE);
	chipid = ((val & (0x1F << 16)) << 16) | (chipid & 0xFFFFFFFF);

	return chipid;
}

/******************************************************************************/

unsigned int get_ca_vendor_reg(void)
{
	unsigned long ca_vendor = 0;

	// TODO: XXXX

	return ca_vendor;
}

/******************************************************************************/

void reset_cpu(unsigned long addr)
{
	unsigned int val;
	/* enable the wdg0 crg clock */
	val = readl(REG_BASE_CRG + REG_PERI_CRG94);
	val &= ~(1<<4);
	val |= 1;
	writel(val, REG_BASE_CRG + REG_PERI_CRG94);
	delay(1000);

	/* unclock wdg */
	writel(0x1ACCE551, REG_BASE_WDG0 + 0x0C00);
	/* wdg load value */
	writel(0x00000100, REG_BASE_WDG0 + 0x0000);
	/* bit0: int enable bit1: reboot enable */
	writel(0x00000003, REG_BASE_WDG0 + 0x0008);

	while (1)
		;
}

/******************************************************************************/

int get_boot_mode(void)
{
	int boot_media;
	int boot_sel;

	/*read bootsel_ctrl from OTP controller*/
	boot_sel = readl(REG_OTP_CTRL + REG_OTP_BOOT_SEL);
	boot_sel = (boot_sel & OTP_BOOT_SEL_MASK)  >> OTP_BOOT_SEL_OFFSET;

	if (0 == boot_sel) {
		/* read from pin */
		boot_media = readl(REG_BASE_PERI_CTRL + REG_START_MODE);
		boot_media = ((boot_media >> NORMAL_BOOTMODE_OFFSET)
					& NORMAL_BOOTMODE_MASK);
	} else {
		/* read from OTP controller */
		boot_media = readl(REG_OTP_CTRL + REG_OTP_BOOT_MODE);
		boot_media = (boot_media & NORMAL_BOOTMODE_MASK);
	}

	return boot_media;
}

/******************************************************************************/
void check_bootfromsd(void)
{
	if (BOOT_FROM_SD == get_boot_mode()) {
		reg_read_write(REG_IOSHARE_EBI_DQ7, 0x0, 0x3);
		reg_read_write(REG_IOSHARE_EBI_DQ6, 0x0, 0x1);
		reg_read_write(REG_IOSHARE_EBI_DQ5, 0x0, 0x1);
		reg_read_write(REG_IOSHARE_EBI_DQ4, 0x0, 0x1);
	}
}

/******************************************************************************/
void start_ddr_hw_training(void)
{
	unsigned int initctrl_phy0, initctrl_phy1;

	/* Read PHY0 HW DDR training value */
	initctrl_phy0 = readl(REG_BASE_SCTL + REG_SC_GEN32);
	if (initctrl_phy0) {
		unsigned int timing;
		unsigned int timing_backup;
		/* Save the Auto-self refresh timing */
		timing = readl(REG_DDR_DMC0_CFG_TIMING2);
		timing_backup = timing & 0x7FF;
		/* Set Auto-self refresh timing to 0 */
		writel((timing & 0xFFFFF800), REG_DDR_DMC0_CFG_TIMING2);
		/* Start HW DDR training */
		writel(initctrl_phy0, REG_DDR_PHY0_INITCTRL);
		do {
			initctrl_phy0 = readl(REG_DDR_PHY0_INITCTRL) & 0x7FFFF;
		} while(initctrl_phy0);
		/* Restore the Auto-self refresh timing */
		timing = readl(REG_DDR_DMC0_CFG_TIMING2);
		timing = (timing & 0xFFFFF800) | timing_backup;
		writel(timing, REG_DDR_DMC0_CFG_TIMING2);
	}

	/* Clear fifo */
	initctrl_phy0 = readl(REG_DDR_PHY0_INITCTRL);
	initctrl_phy0 = initctrl_phy0 | (1<<15);
	writel(initctrl_phy0, REG_DDR_PHY0_INITCTRL);
	initctrl_phy0 = readl(REG_DDR_PHY0_INITCTRL);
	initctrl_phy0 = initctrl_phy0 & ~(1<<15);
	writel(initctrl_phy0, REG_DDR_PHY0_INITCTRL);

	/* Read PHY1 HW DDR training value */
	initctrl_phy1 = readl(REG_BASE_SCTL + REG_SC_GEN33);
	if (initctrl_phy1) {
		unsigned int timing;
		unsigned int timing_backup;
		/* Save the Auto-self refresh timing */
		timing = readl(REG_DDR_DMC1_CFG_TIMING2);
		timing_backup = timing & 0x7FF;
		/* Set Auto-self refresh timing to 0 */
		writel((timing & 0xFFFFF800), REG_DDR_DMC1_CFG_TIMING2);
		/* Start HW DDR training */
		writel(initctrl_phy1, REG_DDR_PHY1_INITCTRL);
		do {
			initctrl_phy1 = readl(REG_DDR_PHY1_INITCTRL) & 0x7FFFF;
		} while(initctrl_phy1);
		/* Restore the Auto-self refresh timing */
		timing = readl(REG_DDR_DMC1_CFG_TIMING2);
		timing = (timing & 0xFFFFF800) | timing_backup;
		writel(timing, REG_DDR_DMC1_CFG_TIMING2);
	}
	/* Clear fifo */
	initctrl_phy1 = readl(REG_DDR_PHY1_INITCTRL);
	initctrl_phy1 = initctrl_phy1 | (1<<15);
	writel(initctrl_phy1, REG_DDR_PHY1_INITCTRL);
	initctrl_phy1 = readl(REG_DDR_PHY1_INITCTRL);
	initctrl_phy1 = initctrl_phy1 & ~(1<<15);
	writel(initctrl_phy1, REG_DDR_PHY1_INITCTRL);
}

/******************************************************************************/
#ifdef CONFIG_HIFONE_FPGA
unsigned int get_cpu_midr(void)
{
	unsigned int midr = 0;
	/* Read MIDR to midr */
	asm("mrc	p15, 0, r0, c0, c0, 0" :"=r"(midr) : :"memory");
	return midr;
}
#endif
/******************************************************************************/
int is_hi3798cv200_b(void)
{
	if ((get_chipid_reg() & _HIFONE_B02_MASK)
		== (_HI3798CV200_B & _HIFONE_B02_MASK))
		return 1;
	else
		return 0;
}
