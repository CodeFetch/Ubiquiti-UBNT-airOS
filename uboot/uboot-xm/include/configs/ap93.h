/*
 * This file contains the configuration parameters for the dbau1x00 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/ar7240.h>
/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS     1	    /* max number of memory banks */
#define CFG_MAX_FLASH_SECT      128    /* max number of sectors on one chip */
#define CFG_FLASH_SECTOR_SIZE   (64*1024)
#define CFG_FLASH_SIZE          0x00800000 /* Total flash size */

#if (CFG_MAX_FLASH_SECT * CFG_FLASH_SECTOR_SIZE) != CFG_FLASH_SIZE
#	error "Invalid flash configuration"
#endif

#define CFG_FLASH_WORD_SIZE     unsigned short 

/* 
 * We boot from this flash
 */
#define CFG_FLASH_BASE		    0x9f000000

/* 
 * The following #defines are needed to get flash environment right 
 */
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(192 << 10)

#define CFG_CONSOLE_INFO_QUIET 1

#undef CONFIG_BOOTARGS
/* XXX - putting rootfs in last partition results in jffs errors */
#define	CONFIG_BOOTARGS     "console=ttyS0,115200 root=31:03 rootfstype=squashfs init=/init"

/* default mtd partition table */
#undef MTDPARTS_DEFAULT
#define MTDPARTS_DEFAULT    "mtdparts=ar7240-nor0:256k(u-boot),64k(u-boot-env),1024k(kernel),6528k(rootfs),256k(cfg),64k(EEPROM)"

#undef CFG_PLL_FREQ
#define CFG_PLL_FREQ	CFG_PLL_400_400_200

#undef CFG_HZ
/*
 * MIPS32 24K Processor Core Family Software User's Manual
 *
 * 6.2.9 Count Register (CP0 Register 9, Select 0)
 * The Count register acts as a timer, incrementing at a constant
 * rate, whether or not an instruction is executed, retired, or
 * any forward progress is made through the pipeline.  The counter
 * increments every other clock, if the DC bit in the Cause register
 * is 0.
 */
/* Since the count is incremented every other tick, divide by 2 */
/* XXX derive this from CFG_PLL_FREQ */
#if (CFG_PLL_FREQ == CFG_PLL_200_200_100)
#	define CFG_HZ          (200000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_300_300_150)
#	define CFG_HZ          (300000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_350_350_175)
#	define CFG_HZ          (350000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_333_333_166)
#	define CFG_HZ          (333000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_266_266_133)
#	define CFG_HZ          (266000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_266_266_66)
#	define CFG_HZ          (266000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_400_400_200) || (CFG_PLL_FREQ == CFG_PLL_400_400_100)
#	define CFG_HZ          (400000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_320_320_80) || (CFG_PLL_FREQ == CFG_PLL_320_320_160)
#	define CFG_HZ          (400000000/2)
#endif


/* 
 * timeout values are in ticks 
 */
#define CFG_FLASH_ERASE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Write */

/*
 * Cache lock for stack
 */
#define CFG_INIT_SP_OFFSET	0x1000

#define	CFG_ENV_IS_IN_FLASH    1
#undef CFG_ENV_IS_NOWHERE  

/* Address and size of Primary Environment Sector	*/
#define CFG_ENV_ADDR		0x9f040000
#define CFG_ENV_SIZE		0x10000

#define CONFIG_BOOTCOMMAND "bootm 0x9f050000"
//#define CONFIG_FLASH_16BIT

/* DDR init values */

#define CONFIG_NR_DRAM_BANKS	2
#define CFG_DDR_REFRESH_VAL     0x4f10
#define CFG_DDR_CONFIG_VAL      0xc7bc8cd0
#define CFG_DDR_MODE_VAL_INIT   0x133
#ifdef LOW_DRIVE_STRENGTH
#	define CFG_DDR_EXT_MODE_VAL    0x2
#else
#	define CFG_DDR_EXT_MODE_VAL    0x0
#endif
#define CFG_DDR_MODE_VAL        0x33

#define CFG_DDR_TRTW_VAL        0x1f
#define CFG_DDR_TWTR_VAL        0x1e

#define CFG_DDR_CONFIG2_VAL	 0x9dd0e6a8
#define CFG_DDR_RD_DATA_THIS_CYCLE_VAL  0x00ff

/* DDR2 Init values */
#define CFG_DDR2_EXT_MODE_VAL    0x402

#define CONFIG_NET_MULTI

#define CONFIG_MEMSIZE_IN_BYTES
#define CONFIG_PCI

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_COMMANDS	(( CONFIG_CMD_DFL | CFG_CMD_ELF |	\
	CFG_CMD_PING | CFG_CMD_NET | CFG_CMD_ENV | CFG_CMD_JFFS2 |	\
	CFG_CMD_FLASH | CFG_CMD_RUN | CFG_CMD_MII | CFG_CMD_TFTP_SERVER ) & \
	~(CFG_CMD_ITEST | CFG_CMD_NFS | CFG_CMD_LOADS | CFG_CMD_LOADB))

/* Ubiquiti fw rescue extensions */
#define CONFIG_URESCUE

#define CONFIG_IPADDR   192.168.1.20
#define CONFIG_SERVERIP 192.168.1.254
#define CONFIG_ETHADDR 00:15:6d:0d:00:00
#define CFG_FAULT_ECHO_LINK_DOWN    1


#define CFG_PHY_ADDR 0 
#define CFG_AG7240_NMACS 2
#define CFG_GMII     0
#define CFG_MII0_RMII             1
#define CFG_AG7100_GE0_RMII             1

#define CFG_BOOTM_LEN	(16 << 20) /* 16 MB */
#if 0
#define DEBUG
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "hush>"
#endif

/*
** Parameters defining the location of the calibration/initialization
** information for the two Merlin devices.
** NOTE: **This will change with different flash configurations**
*/
// something found from AP96
//#define MERLIN24CAL                    0xbfff1000
//#define MERLIN50CAL                    0xbfff5000
//#define BOARDCAL                       0xbfff0000

#define WLANCAL                        0xbfff1000
#define BOARDCAL                       0xbfff0000
#define ATHEROS_PRODUCT_ID             137
#define CAL_SECTOR                     (CFG_MAX_FLASH_SECT - 1)

/* For Merlin, both PCI, PCI-E interfaces are valid */
#define AR7240_ART_PCICFG_OFFSET        12

/* Reset button is on GPIO12 */
#define AR7240_RESET_BTN	12

/* Airwire jump-start button is on GPIO6 */
#define AR7240_AIRWIRE_BTN	6

/* Call misc_init_r() on startup (used for RESET button */
#define CONFIG_MISC_INIT_R

/* LED stuff */
#define CONFIG_SHOW_BOOT_PROGRESS

/* Enable POE Switch support
 * Valid for all python boards, autodetection is based on PCIE WLAN module presence.
 * If there is no WLAN PCIE - do not enable UART.
 */
/* #define CONFIG_UBIQUITI_SWITCH */

#include <cmd_confdefs.h>

#endif	/* __CONFIG_H */
