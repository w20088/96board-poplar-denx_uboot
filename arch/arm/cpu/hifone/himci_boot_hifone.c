
#include <asm/arch/platform.h>

static inline void emmc_sys_init(void)
{

	unsigned int tmp_reg;

	/* eMMC clock phase */
	tmp_reg = himci_readl(REG_BASE_CRG + REG_EMMC_PERI_CRG);
	tmp_reg &= ~SDIO_CLK_MASK;
	tmp_reg |= SDIO_CLK_50M; /* phasic move to xls table. */
	himci_writel(tmp_reg, REG_BASE_CRG + REG_EMMC_PERI_CRG);

	/* eMMC soft reset */
	tmp_reg |= SDIO_SRST_REQ;
	himci_writel(tmp_reg, REG_BASE_CRG + REG_EMMC_PERI_CRG);
	early_udelay(1000);
	tmp_reg &= ~SDIO_SRST_REQ;
	tmp_reg |= SDIO_CKEN | SDIO_BUS_CKEN;
	himci_writel(tmp_reg, REG_BASE_CRG + REG_EMMC_PERI_CRG);

}

