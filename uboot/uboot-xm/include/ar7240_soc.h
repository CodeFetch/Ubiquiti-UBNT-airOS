/*
 * Atheror AR7240 series processor SOC registers
 *
 * (C) Copyright 2008 Atheros Communications, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _AR7240_SOC_H
#define _AR7240_SOC_H

#include <config.h>

#ifdef CONFIG_AR7240_EMU
#define AR7240_EMU 1
#endif

#include <ar934x_soc.h>
/*
 * Address map
 */
#define AR7240_PCI_MEM_BASE             0x10000000  /* 128M */
#define AR7240_APB_BASE                 0x18000000  /* 384M */
#define AR7240_GE0_BASE                 0x19000000  /* 16M */
#define AR7240_GE1_BASE                 0x1a000000  /* 16M */
#define AR7240_USB_EHCI_BASE            0x1b000000
#define AR7240_USB_OHCI_BASE            0x1c000000
#define AR7240_SPI_BASE                 0x1f000000

/*
 * APB block
 */
#define AR7240_DDR_CTL_BASE             AR7240_APB_BASE+0x00000000
#define AR7240_CPU_BASE                 AR7240_APB_BASE+0x00010000
#define AR7240_UART_BASE                AR7240_APB_BASE+0x00020000
#define AR7240_USB_CONFIG_BASE          AR7240_APB_BASE+0x00030000
#define AR7240_GPIO_BASE                AR7240_APB_BASE+0x00040000
#define AR7240_PLL_BASE                 AR7240_APB_BASE+0x00050000
#define AR7240_RESET_BASE               AR7240_APB_BASE+0x00060000
#define AR7240_PCI_LCL_BASE             AR7240_APB_BASE+0x000f0000

/*
 * DDR block
 */

#define AR7240_DDR_CONFIG               AR7240_DDR_CTL_BASE+0
#define AR7240_DDR_CONFIG2              AR7240_DDR_CTL_BASE+4
#define AR7240_DDR_MODE                 AR7240_DDR_CTL_BASE+0x08
#define AR7240_DDR_EXT_MODE             AR7240_DDR_CTL_BASE+0x0c
#define AR7240_DDR_CONTROL              AR7240_DDR_CTL_BASE+0x10
#define AR7240_DDR_REFRESH              AR7240_DDR_CTL_BASE+0x14
#define AR7240_DDR_RD_DATA_THIS_CYCLE   AR7240_DDR_CTL_BASE+0x18
#define AR7240_DDR_TAP_CONTROL0         AR7240_DDR_CTL_BASE+0x1c
#define AR7240_DDR_TAP_CONTROL1         AR7240_DDR_CTL_BASE+0x20
#define AR7240_DDR_TAP_CONTROL2         AR7240_DDR_CTL_BASE+0x24
#define AR7240_DDR_TAP_CONTROL3         AR7240_DDR_CTL_BASE+0x28
#define AR7240_DDR_DDR2_CONFIG          AR7240_DDR_CTL_BASE+0x8c

#define AR7240_DDR_CONFIG_16BIT             (1 << 31)
#define AR7240_DDR_CONFIG_PAGE_OPEN         (1 << 30)
#define AR7240_DDR_CONFIG_CAS_LAT_SHIFT      27
#define AR7240_DDR_CONFIG_TMRD_SHIFT         23
#define AR7240_DDR_CONFIG_TRFC_SHIFT         17
#define AR7240_DDR_CONFIG_TRRD_SHIFT         13
#define AR7240_DDR_CONFIG_TRP_SHIFT          9
#define AR7240_DDR_CONFIG_TRCD_SHIFT         5
#define AR7240_DDR_CONFIG_TRAS_SHIFT         0

#define AR7240_DDR_CONFIG2_BL2          (2 << 0)
#define AR7240_DDR_CONFIG2_BL4          (4 << 0)
#define AR7240_DDR_CONFIG2_BL8          (8 << 0)

#define AR7240_DDR_CONFIG2_BT_IL        (1 << 4)
#define AR7240_DDR_CONFIG2_CNTL_OE_EN   (1 << 5)
#define AR7240_DDR_CONFIG2_PHASE_SEL    (1 << 6)
#define AR7240_DDR_CONFIG2_DRAM_CKE     (1 << 7)
#define AR7240_DDR_CONFIG2_TWR_SHIFT    8
#define AR7240_DDR_CONFIG2_TRTW_SHIFT   12
#define AR7240_DDR_CONFIG2_TRTP_SHIFT   17
#define AR7240_DDR_CONFIG2_TWTR_SHIFT   21
#define AR7240_DDR_CONFIG2_HALF_WIDTH_L (1 << 31)

#define AR7240_DDR_TAP_DEFAULT          0x18

/*
 * PLL
 */
#define AR7240_CPU_PLL_CONFIG           AR7240_PLL_BASE
#define AR7240_USB_PLL_CONFIG           AR7240_PLL_BASE+0x4
#define AR7240_PCIE_PLL_CONFIG          AR7240_PLL_BASE+0x10
#define AR7240_CPU_CLOCK_CONTROL        AR7240_PLL_BASE+8

#define AR7240_USB_PLL_GE0_OFFSET       AR7240_PLL_BASE+0x10
#define AR7240_USB_PLL_GE1_OFFSET       AR7240_PLL_BASE+0x14
#define AR7242_ETH_XMII_CONFIG          AR7240_PLL_BASE+0x2c

#define PLL_CONFIG_PLL_DIV_SHIFT        0
#define PLL_CONFIG_PLL_DIV_MASK         (0x3ff<< PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_PLL_REF_DIV_SHIFT    10
#define PLL_CONFIG_PLL_REF_DIV_MASK     (0xf << PLL_CONFIG_PLL_REF_DIV_SHIFT)
#define PLL_CONFIG_PLL_BYPASS_SHIFT     16
#define PLL_CONFIG_PLL_BYPASS_MASK      (0x1 << PLL_CONFIG_PLL_BYPASS_SHIFT)
#define PLL_CONFIG_PLL_UPDATE_SHIFT     17
#define PLL_CONFIG_PLL_UPDATE_MASK      (0x1 << PLL_CONFIG_PLL_UPDATE_SHIFT)
#define PLL_CONFIG_PLL_NOPWD_SHIFT      18
#define PLL_CONFIG_PLL_NOPWD_MASK       (0x1 << PLL_CONFIG_PLL_NOPWD_SHIFT)
#define PLL_CONFIG_AHB_DIV_SHIFT        19
#define PLL_CONFIG_AHB_DIV_MASK         (0x1 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_DDR_DIV_SHIFT        22
#define PLL_CONFIG_DDR_DIV_MASK         (0x1 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_PLL_RESET_SHIFT      25
#define PLL_CONFIG_PLL_RESET_MASK       (0x1 << PLL_CONFIG_PLL_RESET_SHIFT)

/* Hornet's CPU PLL Configuration Register */
#define HORNET_PLL_CONFIG_NINT_SHIFT            10
#define HORNET_PLL_CONFIG_NINT_MASK             (0x3f << HORNET_PLL_CONFIG_NINT_SHIFT)
#define HORNET_PLL_CONFIG_REFDIV_SHIFT          16
#define HORNET_PLL_CONFIG_REFDIV_MASK           (0x1f << HORNET_PLL_CONFIG_REFDIV_SHIFT)
#define HORNET_PLL_CONFIG_OUTDIV_SHIFT          23
#define HORNET_PLL_CONFIG_OUTDIV_MASK           (0x7 << HORNET_PLL_CONFIG_OUTDIV_SHIFT)
#define HORNET_PLL_CONFIG_PLLPWD_SHIFT          30
#define HORNET_PLL_CONFIG_PLLPWD_MASK           (0x1 << HORNET_PLL_CONFIG_PLLPWD_SHIFT)
#define HORNET_PLL_CONFIG_UPDATING_SHIFT        31
#define HORNET_PLL_CONFIG_UPDATING_MASK         (0x1 << HORNET_PLL_CONFIG_UPDATING_SHIFT)
/* Hornet's CPU PLL Configuration 2 Register */
#define HORNET_PLL_CONFIG2_SETTLE_TIME_SHIFT    0
#define HORNET_PLL_CONFIG2_SETTLE_TIME_MASK     (0xfff << HORNET_PLL_CONFIG2_SETTLE_TIME_SHIFT)
/* Hornet's CPU Clock Control Register */
#define HORNET_CLOCK_CONTROL_BYPASS_SHIFT       2
#define HORNET_CLOCK_CONTROL_BYPASS_MASK        (0x1 << HORNET_CLOCK_CONTROL_BYPASS_SHIFT)
#define HORNET_CLOCK_CONTROL_CPU_POST_DIV_SHIFT 5
#define HORNET_CLOCK_CONTROL_CPU_POST_DIV_MASK  (0x3 << HORNET_CLOCK_CONTROL_CPU_POST_DIV_SHIFT)
#define HORNET_CLOCK_CONTROL_DDR_POST_DIV_SFIFT 10
#define HORNET_CLOCK_CONTROL_DDR_POST_DIV_MASK  (0x3 << HORNET_CLOCK_CONTROL_DDR_POST_DIV_SFIFT)
#define HORNET_CLOCK_CONTROL_AHB_POST_DIV_SFIFT 15
#define HORNET_CLOCK_CONTROL_AHB_POST_DIV_MASK  (0x3 << HORNET_CLOCK_CONTROL_AHB_POST_DIV_SFIFT)

#define CLOCK_CONTROL_CLOCK_SWITCH_SHIFT  0
#define CLOCK_CONTROL_CLOCK_SWITCH_MASK  (1 << CLOCK_CONTROL_CLOCK_SWITCH_SHIFT)
#define CLOCK_CONTROL_RST_SWITCH_SHIFT    1
#define CLOCK_CONTROL_RST_SWITCH_MASK    (1 << CLOCK_CONTROL_RST_SWITCH_SHIFT)

/*
** PLL config for different CPU/DDR/AHB frequencies
*/
#define PLL_CONFIG_PLL_NOPWD_VAL        (1 << PLL_CONFIG_PLL_NOPWD_SHIFT)


/* defines for 400/400/200 */
#define PLL_CONFIG_400_400_200_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_400_400_200_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_400_400_200_PLL_DIV_VAL	(0x28 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_400_400_200_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 400/400/100 */
#define PLL_CONFIG_400_400_100_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_400_400_100_AHB_DIV_VAL	(0x1 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_400_400_100_PLL_DIV_VAL	(0x28 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_400_400_100_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 360/360/90 */
#define PLL_CONFIG_360_360_90_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_360_360_90_AHB_DIV_VAL	(0x1 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_360_360_90_PLL_DIV_VAL	(0x24 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_360_360_90_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 350/350/175 */
#define PLL_CONFIG_350_350_175_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_350_350_175_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_350_350_175_PLL_DIV_VAL	(0x23 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_350_350_175_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 340/340/170 */
#define PLL_CONFIG_340_340_170_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_340_340_170_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_340_340_170_PLL_DIV_VAL	(0x22 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_340_340_170_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 330/330/165 */
#define PLL_CONFIG_330_330_165_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_330_330_165_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_330_330_165_PLL_DIV_VAL	(0x21 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_330_330_165_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 320/320/160 */
#define PLL_CONFIG_320_320_160_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_320_320_160_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_320_320_160_PLL_DIV_VAL	(0x20 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_320_320_160_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 320/320/80 */
#define PLL_CONFIG_320_320_80_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_320_320_80_AHB_DIV_VAL	(0x1 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_320_320_80_PLL_DIV_VAL	(0x20 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_320_320_80_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 310/310/155 */
#define PLL_CONFIG_310_310_155_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_310_310_155_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_310_310_155_PLL_DIV_VAL	(0x1f << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_310_310_155_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 300/300/150 */
#define PLL_CONFIG_300_300_150_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_300_300_150_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_300_300_150_PLL_DIV_VAL	(0x1e << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_300_300_150_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 300/300/75 */
#define PLL_CONFIG_300_300_75_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_300_300_75_AHB_DIV_VAL	(0x1 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_300_300_75_PLL_DIV_VAL	(0x1e << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_300_300_75_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 200/200/100 */
#define PLL_CONFIG_200_200_100_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_200_200_100_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_200_200_100_PLL_DIV_VAL	(0x14 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_200_200_100_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 240/240/120 */
#define PLL_CONFIG_240_240_120_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_240_240_120_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_240_240_120_PLL_DIV_VAL	(0x18 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_240_240_120_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 160/160/80 */
#define PLL_CONFIG_160_160_80_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_160_160_80_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_160_160_80_PLL_DIV_VAL	(0x10 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_160_160_80_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 400/200/200 */
#define PLL_CONFIG_400_200_200_DDR_DIV_VAL	(0x1 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_400_200_200_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_400_200_200_PLL_DIV_VAL	(0x28 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_400_200_200_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* some weird defines - came from AR7241 support */
/* defines for 280/280/130 */
#define PLL_CONFIG_280_280_130_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_280_280_130_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_280_280_130_PLL_DIV_VAL	(0x1c << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_280_280_130_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 260/260/130 */
#define PLL_CONFIG_260_260_130_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_260_260_130_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_260_260_130_PLL_DIV_VAL	(0x1a << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_260_260_130_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)


/* defines for 250/250/125 --- 2GHz Harmonic Safe 2390-2480 MHz */
#define PLL_CONFIG_250_250_125_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_250_250_125_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_250_250_125_PLL_DIV_VAL	(0x19 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_250_250_125_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 280/280/140 --- 2GHz Harmonic Safe 2390-2480 MHz */
#define PLL_CONFIG_280_280_140_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_280_280_140_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_280_280_140_PLL_DIV_VAL	(0x1c << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_280_280_140_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 340/340/170 --- 2GHz Harmonic Safe 2390-2480 MHz */
#define PLL_CONFIG_340_340_170_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_340_340_170_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_340_340_170_PLL_DIV_VAL	(0x22 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_340_340_170_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 360/360/180 --- 2GHz Harmonic Safe 2390-2480 MHz */
#define PLL_CONFIG_360_360_180_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_360_360_180_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_360_360_180_PLL_DIV_VAL	(0x24 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_360_360_180_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 390/390/195 -- 2GHz Harmonc SAfe 2390-2480 MHz */
#define PLL_CONFIG_390_390_195_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_390_390_195_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_390_390_195_PLL_DIV_VAL	(0x27 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_390_390_195_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 420/420/210 -- 2GHz Harmonc SAfe 2390-2480 MHz */
#define PLL_CONFIG_420_420_210_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_420_420_210_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_420_420_210_PLL_DIV_VAL	(0x2a << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_420_420_210_PLL_REF_DIV_VAL	(0x2 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

/* defines for 396/396/198 -- Verizon safe frequency */
#define PLL_CONFIG_396_396_198_DDR_DIV_VAL	(0x0 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_396_396_198_AHB_DIV_VAL	(0x0 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_396_396_198_PLL_DIV_VAL	(0x63 << PLL_CONFIG_PLL_DIV_SHIFT)
#define PLL_CONFIG_396_396_198_PLL_REF_DIV_VAL	(0x5 << PLL_CONFIG_PLL_REF_DIV_SHIFT)

#define PLL_MASK (PLL_CONFIG_DDR_DIV_MASK | PLL_CONFIG_AHB_DIV_MASK | PLL_CONFIG_PLL_NOPWD_MASK | PLL_CONFIG_PLL_REF_DIV_MASK | PLL_CONFIG_PLL_DIV_MASK)

#define PLL_VAL(x,y,z) (PLL_CONFIG_##x##_##y##_##z##_PLL_REF_DIV_VAL|PLL_CONFIG_##x##_##y##_##z##_PLL_DIV_VAL|PLL_CONFIG_##x##_##y##_##z##_AHB_DIV_VAL|PLL_CONFIG_##x##_##y##_##z##_DDR_DIV_VAL|PLL_CONFIG_PLL_NOPWD_VAL)



#define UBOOT_SIZE		(256 * 1024)
#define PLL_FLASH_ADDR		(CFG_FLASH_BASE + UBOOT_SIZE)
#define PLL_CONFIG_VAL_F	(PLL_FLASH_ADDR + CFG_FLASH_SECTOR_SIZE - 0x10)
#define PLL_MAGIC		0xaabbccdd

#ifndef PLL_CONFIG_VAL
#define PLL_CONFIG_VAL	PLL_VAL(400,400,200)
#endif

#ifdef CONFIG_SUPPORT_AR7241

#ifndef PLL_CONFIG_7241_VAL
#define PLL_CONFIG_7241_VAL	PLL_VAL(400,400,200)
#endif

#endif


/*
 * PLL block
 */
#define AR7240_PLL_CONFIG               AR7240_PLL_BASE+0x0

/*
 * CLOCK
 */
#define AR7240_CPU_CLOCK_CONTROL        AR7240_PLL_BASE+8

/*
 * FIFO flushes
 */
#define AR7240_DDR_GE0_FLUSH            AR7240_DDR_CTL_BASE+0x9c
#define AR7240_DDR_GE1_FLUSH            AR7240_DDR_CTL_BASE+0xa0
#define AR7240_DDR_PCI_FLUSH            AR7240_DDR_CTL_BASE+0xa8

/*
 * USB block
 */
#define AR7240_USB_FLADJ_VAL            AR7240_USB_CONFIG_BASE
#define AR7240_USB_CONFIG               AR7240_USB_CONFIG_BASE+0x4
#define AR7240_USB_WINDOW               0x1000000

/*
 * PCI block
 */
#define AR7240_PCI_WINDOW           0x8000000       /* 128MB */
#define AR7240_PCI_WINDOW0_OFFSET   AR7240_DDR_CTL_BASE+0x7c
#define AR7240_PCI_WINDOW1_OFFSET   AR7240_DDR_CTL_BASE+0x80
#define AR7240_PCI_WINDOW2_OFFSET   AR7240_DDR_CTL_BASE+0x84
#define AR7240_PCI_WINDOW3_OFFSET   AR7240_DDR_CTL_BASE+0x88
#define AR7240_PCI_WINDOW4_OFFSET   AR7240_DDR_CTL_BASE+0x8c
#define AR7240_PCI_WINDOW5_OFFSET   AR7240_DDR_CTL_BASE+0x90
#define AR7240_PCI_WINDOW6_OFFSET   AR7240_DDR_CTL_BASE+0x94
#define AR7240_PCI_WINDOW7_OFFSET   AR7240_DDR_CTL_BASE+0x98

#define AR7240_PCI_WINDOW0_VAL      0x10000000
#define AR7240_PCI_WINDOW1_VAL      0x11000000
#define AR7240_PCI_WINDOW2_VAL      0x12000000
#define AR7240_PCI_WINDOW3_VAL      0x13000000
#define AR7240_PCI_WINDOW4_VAL      0x14000000
#define AR7240_PCI_WINDOW5_VAL      0x15000000
#define AR7240_PCI_WINDOW6_VAL      0x16000000
#define AR7240_PCI_WINDOW7_VAL      0x07000000


/*
 * CRP. To access the host controller config and status registers
 */
#define AR7240_PCI_CRP   	   0x180c0000
#define AR7240_PCI_DEV_CFGBASE     0x14000000

#define AR7240_PCI_CRP_AD_CBE               AR7240_PCI_CRP
#define AR7240_PCI_CRP_WRDATA               AR7240_PCI_CRP+0x4
#define AR7240_PCI_CRP_RDDATA               AR7240_PCI_CRP+0x8
#define AR7240_PCI_ERROR            AR7240_PCI_CRP+0x1c
#define AR7240_PCI_ERROR_ADDRESS    AR7240_PCI_CRP+0x20
#define AR7240_PCI_AHB_ERROR            AR7240_PCI_CRP+0x24
#define AR7240_PCI_AHB_ERROR_ADDRESS    AR7240_PCI_CRP+0x28

#define AR7240_CRP_CMD_WRITE             0x00010000
#define AR7240_CRP_CMD_READ              0x00000000

/*
 * PCI CFG. To generate config cycles
 */
#define AR7240_PCI_CFG_AD           AR7240_PCI_CRP+0xc
#define AR7240_PCI_CFG_CBE          AR7240_PCI_CRP+0x10
#define AR7240_PCI_CFG_WRDATA       AR7240_PCI_CRP+0x14
#define AR7240_PCI_CFG_RDDATA       AR7240_PCI_CRP+0x18
#define AR7240_CFG_CMD_READ         0x0000000a
#define AR7240_CFG_CMD_WRITE        0x0000000b

#define AR7240_PCI_IDSEL_ADLINE_START           17

#define AR7240_PCI_LCL_RESET        AR7240_PCI_LCL_BASE+0x18

/*
 * gpio configs
 */
#define AR7240_GPIO_OE                  AR7240_GPIO_BASE+0x0
#define AR7240_GPIO_IN                  AR7240_GPIO_BASE+0x4
#define AR7240_GPIO_OUT                 AR7240_GPIO_BASE+0x8
#define AR7240_GPIO_SET                 AR7240_GPIO_BASE+0xc
#define AR7240_GPIO_CLEAR               AR7240_GPIO_BASE+0x10
#define AR7240_GPIO_INT_ENABLE          AR7240_GPIO_BASE+0x14
#define AR7240_GPIO_INT_TYPE            AR7240_GPIO_BASE+0x18
#define AR7240_GPIO_INT_POLARITY        AR7240_GPIO_BASE+0x1c
#define AR7240_GPIO_INT_PENDING         AR7240_GPIO_BASE+0x20
#define AR7240_GPIO_INT_MASK            AR7240_GPIO_BASE+0x24
#define AR7240_GPIO_FUNC                AR7240_GPIO_BASE+0x28

/*
 * bits for AR7240_GPIO_FUNC register
 */
#define AR7240_GPIO_FUNC_JTAG_DIS	(1 << 0)	/* GPIO 6,7,8 */
#define AR7240_GPIO_FUNC_UART_EN	(1 << 1)	/* GPIO 9,10 */
#define AR7240_GPIO_FUNC_UART_RTSCTS_EN	(1 << 2)	/* GPIO 11,12 */
#define AR7240_GPIO_FUNC_LED0_EN	(1 << 3)	/* GPIO 13 */
#define AR7240_GPIO_FUNC_LED1_EN	(1 << 4)	/* GPIO 14 */
#define AR7240_GPIO_FUNC_LED2_EN	(1 << 5)	/* GPIO 15 */
#define AR7240_GPIO_FUNC_LED3_EN	(1 << 6)	/* GPIO 16 */
#define AR7240_GPIO_FUNC_LED4_EN	(1 << 7)	/* GPIO 17 */
#define AR7240_GPIO_FUNC_CLKOBS1_EN	(1 << 8)	/* GPIO 13 */
#define AR7240_GPIO_FUNC_CLKOBS2_EN	(1 << 9)	/* GPIO 14 */
#define AR7240_GPIO_FUNC_CLKOBS3_EN	(1 << 10)	/* GPIO 15 */
#define AR7240_GPIO_FUNC_CLKOBS4_EN	(1 << 11)	/* GPIO 16 */
#define AR7240_GPIO_FUNC_CLKOBS5_EN	(1 << 12)	/* GPIO 17 */
#define AR7240_GPIO_FUNC_SPI_CS_EN1	(1 << 13)	/* GPIO 0 */
#define AR7240_GPIO_FUNC_SPI_CS_EN2	(1 << 14)	/* GPIO 1 */
#define AR7240_GPIO_FUNC_ES_UART_DIS	(1 << 15)	/* GPIO ??? */
#define AR7240_GPIO_FUNC_PCIEPHYTST_EN	(1 << 16)	/* GPIO 1,11,15,16,17 */
#define AR7240_GPIO_FUNC_DDR_DQOE_EN	(1 << 17)	/* GPIO 0 */
#define AR7240_GPIO_FUNC_SPI_EN		(1 << 18)	/* GPIO 2,3,4,5 */
#define AR7240_GPIO_FUNC_GE0_CLK_EN	(1 << 19)	/* GPIO 1 */
#define AR7240_GPIO_FUNC_SSTL_XOR_EN	(1 << 20)	/* GPIO ??? */
#define AR7240_GPIO_FUNC_EXT_MDIO_SEL	(1 << 21)	/* GPIO 0,1 */
#define AR7240_GPIO_FUNC_PLL_SHIFT_EN	(1 << 22)	/* - */

/*
 * IRQ Map.
 * There are 4 conceptual ICs in the system. We generally give a block of 16
 * irqs to each IC.
 * CPU:                     0    - 0xf
 *      MISC:               0x10 - 0x1f
 *          GPIO:           0x20 - 0x2f
 *      PCI :               0x30 - 0x40
 *
 */
#define AR7240_CPU_IRQ_BASE         0x00
#define AR7240_MISC_IRQ_BASE        0x10
#define AR7240_GPIO_IRQ_BASE        0x20
#define AR7240_PCI_IRQ_BASE         0x30

/*
 * The IPs. Connected to CPU (hardware IP's; the first two are software)
 */
#define AR7240_CPU_IRQ_PCI                  AR7240_CPU_IRQ_BASE+2
#define AR7240_CPU_IRQ_USB                  AR7240_CPU_IRQ_BASE+3
#define AR7240_CPU_IRQ_GE0                  AR7240_CPU_IRQ_BASE+4
#define AR7240_CPU_IRQ_GE1                  AR7240_CPU_IRQ_BASE+5
#define AR7240_CPU_IRQ_MISC                 AR7240_CPU_IRQ_BASE+6
#define AR7240_CPU_IRQ_TIMER                AR7240_CPU_IRQ_BASE+7

/*
 * Interrupts connected to the CPU->Misc line.
 */
#define AR7240_MISC_IRQ_TIMER               AR7240_MISC_IRQ_BASE+0
#define AR7240_MISC_IRQ_ERROR               AR7240_MISC_IRQ_BASE+1
#define AR7240_MISC_IRQ_GPIO                AR7240_MISC_IRQ_BASE+2
#define AR7240_MISC_IRQ_UART                AR7240_MISC_IRQ_BASE+3
#define AR7240_MISC_IRQ_WATCHDOG            AR7240_MISC_IRQ_BASE+4
#define AR7240_MISC_IRQ_COUNT                 5

#define MIMR_TIMER                          0x01
#define MIMR_ERROR                          0x02
#define MIMR_GPIO                           0x04
#define MIMR_UART                           0x08
#define MIMR_WATCHDOG                       0x10

#define MISR_TIMER                          MIMR_TIMER
#define MISR_ERROR                          MIMR_ERROR
#define MISR_GPIO                           MIMR_GPIO
#define MISR_UART                           MIMR_UART
#define MISR_WATCHDOG                       MIMR_WATCHDOG

/*
 * Interrupts connected to the Misc->GPIO line
 */
#define AR7240_GPIO_IRQn(_gpio)             AR7240_GPIO_IRQ_BASE+(_gpio)
#define AR7240_GPIO_IRQ_COUNT                 16

/*
 * Interrupts connected to CPU->PCI
 */
#define AR7240_PCI_IRQ_DEV0                  AR7240_PCI_IRQ_BASE+0
#define AR7240_PCI_IRQ_DEV1                  AR7240_PCI_IRQ_BASE+1
#define AR7240_PCI_IRQ_DEV2                  AR7240_PCI_IRQ_BASE+2
#define AR7240_PCI_IRQ_CORE                  AR7240_PCI_IRQ_BASE+3
#define AR7240_PCI_IRQ_COUNT                 4

/*
 * PCI interrupt mask and status
 */
#define PIMR_DEV0                           0x01
#define PIMR_DEV1                           0x02
#define PIMR_DEV2                           0x04
#define PIMR_CORE                           0x10

#define PISR_DEV0                           PIMR_DEV0
#define PISR_DEV1                           PIMR_DEV1
#define PISR_DEV2                           PIMR_DEV2
#define PISR_CORE                           PIMR_CORE

#define AR7240_GPIO_COUNT                   16

/*
 * Reset block
 */
#define AR7240_GENERAL_TMR            AR7240_RESET_BASE+0
#define AR7240_GENERAL_TMR_RELOAD     AR7240_RESET_BASE+4
#define AR7240_WATCHDOG_TMR_CONTROL   AR7240_RESET_BASE+8
#define AR7240_WATCHDOG_TMR           AR7240_RESET_BASE+0xc
#define AR7240_MISC_INT_STATUS        AR7240_RESET_BASE+0x10
#define AR7240_MISC_INT_MASK          AR7240_RESET_BASE+0x14
#define AR7240_GLOBAL_INT_STATUS      AR7240_RESET_BASE+0x18
#define AR7240_RESET                  AR7240_RESET_BASE+0x1c
#define HORNET_BOOTSTRAP_STATUS       AR7240_RESET_BASE+0xac /* Hornet's bootstrap register */
#define AR7240_REV_ID                 (AR7240_RESET_BASE + 0x90)
#define AR7240_REV_ID_MASK            0xffff
#define HORNET_REV_ID_MASK            0xfff
#define AR9344_REV_ID_MASK            0xfff0	/* Ignore minor id */
#define HORNET_BOOTSTRAP_SEL_25M_40M_MASK   0x00000001 /* Hornet's bootstrap register */
#define HORNET_BOOTSTRAP_MEM_TYPE_MASK      0x00003000 /* Hornet's bootstrap register */
#define HORNET_BOOTSTRAP_MDIO_SLAVE_MASK    0x00020000 /* Hornet's bootstrap register */

#define AR7240_REV_ID_AR7130    0xa0
#define AR7240_REV_ID_AR7141    0xa1
#define AR7240_REV_ID_AR7161    0xa2
#define AR7240_REV_1_0          0xc0
#define AR7240_REV_1_1          0xc1
#define AR7240_REV_1_2          0xc2

#define AR7241_REV_1_0          0x0100
#define AR7241_REV_1_1          0x0101

#define AR7242_REV_1_0          0x1100
#define AR7242_REV_1_1          0x1101
#define AR7242_REV_1_2          0x1102
#define AR7242_REV_1_3          0x1103

#define AR9330_REV_1_0			0x0110                  /* 5-port:0x110, 4-port 0x1110 */
#define AR9331_REV_1_0			0x1110
#define AR9330_REV_1_1			0x0111                  /* 5-port:0x111, 4-port 0x1111 */
#define AR9331_REV_1_1			0x1111
#define AR9330_REV_1_2			0x0112
#define AR9331_REV_1_2			0x1112

#define AR9344_REV_1_x		0x2120	/* Wasp 1.x, ignore minor id */
#define AR9342_REV_1_x		0x1120
#define AR9341_REV_1_x		0x0120




#define _is_ar7240(revid)	(((revid) == AR7240_REV_1_2) || \
                         ((revid) == AR7240_REV_1_1) || \
                         ((revid) == AR7240_REV_1_0))

#define _is_ar7241(revid)	(((revid) == AR7241_REV_1_0) || \
                         ((revid) == AR7241_REV_1_1))

#define _is_ar7242(revid)	(((revid) == AR7242_REV_1_0) || \
                         ((revid) == AR7242_REV_1_1) || \
			 ((revid) == AR7242_REV_1_2) || \
			 ((revid) == AR7242_REV_1_3))

#define is_ar7240()     (_is_ar7240(ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK))

#define is_ar7241()     (_is_ar7241(ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK))

#define is_ar7242()     (_is_ar7242(ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK))

#define is_ar9330() (((ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK) == AR9330_REV_1_0) || \
                        ((ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK) == AR9330_REV_1_1) || \
                        ((ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK) == AR9330_REV_1_2))

#define is_ar9331() (((ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK) == AR9331_REV_1_0) || \
                        ((ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK) == AR9331_REV_1_1) || \
                        ((ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK) == AR9331_REV_1_2))

#define is_ar9344()	((ar7240_reg_rd(AR7240_REV_ID) & AR9344_REV_ID_MASK) == AR9344_REV_1_x)
#define is_ar9342()	((ar7240_reg_rd(AR7240_REV_ID) & AR9344_REV_ID_MASK) == AR9342_REV_1_x)
#define is_ar9341()	((ar7240_reg_rd(AR7240_REV_ID) & AR9344_REV_ID_MASK) == AR9341_REV_1_x)

/*
 * AR7240_RESET bit defines
 */
#define AR7240_RESET_EXTERNAL               (1 << 28)
#define AR7240_RESET_FULL_CHIP              (1 << 24)
#define AR7240_RESET_CPU_NMI                (1 << 21)
#define AR7240_RESET_CPU_COLD_RESET_MASK    (1 << 20)
#define AR7240_RESET_DDR                    (1 << 16)
#define AR7240_RESET_GE1_MAC                (1 << 13)
#define AR7240_RESET_GE1_MDIO               (1 << 23)
#define AR7240_RESET_GE1_PHY                (1 << 12) /* Not valid */
#define AR7240_RESET_PCIE_PHY_SERIAL        (1 << 10)
#define AR7240_RESET_GE0_MAC                (1 << 9)
#define AR7240_RESET_GE0_MDIO               (1 << 22)
#define AR7240_RESET_GE0_PHY                (1 << 8) /* Switch reset */
#define AR7240_RESET_PCIE_PHY               (1 << 7)
#define AR7240_RESET_PCIE                   (1 << 6)
#define AR7240_RESET_USB_HOST               (1 << 5)
#define AR7240_RESET_USB_OHCI_DLL           (1 << 3)

#define AR7240_MII0_CTRL                    0x18070000
#define AR7240_MII1_CTRL                    0x18070004

#define K1BASE KSEG1

#ifndef __ASSEMBLY__
typedef enum {
    AR7240_DDR_16B_LOW,
    AR7240_DDR_16B_HIGH,
    AR7240_DDR_32B,
}ar7240_ddr_width_t;

#define ar7240_reg_rd(_phys)    (*(volatile unsigned int *)KSEG1ADDR(_phys))
#define ar7240_reg_wr_nf(_phys, _val) \
                    ((*(volatile unsigned int *)KSEG1ADDR(_phys)) = (_val))

#define ar7240_reg_wr(_phys, _val) do {     \
                    ar7240_reg_wr_nf(_phys, _val);  \
                    ar7240_reg_rd(_phys);       \
}while(0);

#define ar7240_write_pci_window(_no)             \
  ar7240_reg_wr(AR7240_PCI_WINDOW##_no##_OFFSET, AR7240_PCI_WINDOW##_no##_VAL);

#define BIT(_x) (1 << (_x))

#define ar7240_reg_rmw_set(_reg, _mask)  do {                        \
    ar7240_reg_wr((_reg), (ar7240_reg_rd((_reg)) | (_mask)));      \
    ar7240_reg_rd((_reg));                                           \
}while(0);

#define ar7240_reg_rmw_clear(_reg, _mask)  do {                        \
    ar7240_reg_wr((_reg), (ar7240_reg_rd((_reg)) & ~(_mask)));      \
    ar7240_reg_rd((_reg));                                           \
}while(0);

#define ar7240_get_bit(_reg, _bit)  (ar7240_reg_rd((_reg)) & (1 << (_bit)))

#define ar7240_flush_ge(_unit) do {                             \
    u32     reg = (_unit) ? AR7240_DDR_GE1_FLUSH : AR7240_DDR_GE0_FLUSH;   \
    ar7240_reg_wr(reg, 1);                 \
    while((ar7240_reg_rd(reg) & 0x1));   \
    ar7240_reg_wr(reg, 1);                 \
    while((ar7240_reg_rd(reg) & 0x1));   \
}while(0);

#define ar7240_flush_pci() do {                             \
    ar7240_reg_wr(AR7240_DDR_PCI_FLUSH, 1);                 \
    while((ar7240_reg_rd(AR7240_DDR_PCI_FLUSH) & 0x1));   \
    ar7240_reg_wr(AR7240_DDR_PCI_FLUSH, 1);                 \
    while((ar7240_reg_rd(AR7240_DDR_PCI_FLUSH) & 0x1));   \
}while(0);

#endif  /*__ASSEMBLY*/
#endif
