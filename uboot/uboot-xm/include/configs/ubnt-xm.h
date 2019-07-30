/*
 * This file contains the configuration parameters for the pb93 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/ar7240.h>

#ifndef FLASH_SIZE
#define FLASH_SIZE		8
#endif

/*
 * The following #defines are needed to get flash environment right
 */
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(192 << 10)


/* Flash configuration */
#define CFG_FLASH_BASE		0x9f000000
#define CFG_FLASH_SIZE		0x00800000  /* DEFAULT SIZE if flash can't be detected */
#define CFG_FLASH_SECTOR_SIZE	0x10000	/* DEFAULT SIZE if flash can't be detected */
#define CFG_FLASH_WORD_SIZE	unsigned short
#define CFG_MAX_FLASH_SECT     	256		/* MAX # of SECTORS SUPPORTED */ 
#define CFG_MAX_FLASH_BANKS     1

#undef ENABLE_DYNAMIC_CONF
#define DDR_LOW_DRIVE_STRENGTH 1

#define CONFIG_SUPPORT_AR7241 1
#define DDR_AR7241_LOW_DRIVE_STRENGTH 1
#define CONFIG_AR7242_8021_PHY
#define CONFIG_AR7242_8035_PHY



/* default mtd partition table */
#undef MTDPARTS_DEFAULT
#undef CONFIG_BOOTARGS

#define KEY_MTDPARTS "mtdparts="
#define MTDPARTS_DEV "ar7240-nor0:"
#define MTDPARTS_LIST "256k(u-boot),64k(u-boot-env),1024k(kernel),6528k(rootfs),256k(cfg),64k(EEPROM)"
#define MTDPARTS_DEFAULT KEY_MTDPARTS MTDPARTS_DEV MTDPARTS_LIST

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
#define CALC_HZ(x)	( (x) * 1000000 / 2 )

#undef CFG_HZ
#define CFG_HZ		CALC_HZ(390)

#ifdef CONFIG_SUPPORT_AR7241
#define PLL_CONFIG_7241_VAL	PLL_VAL(400,400,200)
#define PLL_CONFIG_7242_VAL	PLL_VAL(400,400,200) // NB: Rocket M2 Titanium still needs this!
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
#define CFG_ENV_SIZE		CFG_FLASH_SECTOR_SIZE

#define CONFIG_BOOTCOMMAND "bootm 0x9f050000"

/* DDR init values */

#define CONFIG_NR_DRAM_BANKS	2

/* DDR values to support AR7241 */

#ifdef CONFIG_SUPPORT_AR7241
#define CFG_7241_DDR1_CONFIG_VAL      0xc7bc8cd0
//#define CFG_7241_DDR1_CONFIG_VAL      0x6fbc8cd0
#define CFG_7241_DDR1_MODE_VAL_INIT   0x133
#ifdef DDR_AR7241_LOW_DRIVE_STRENGTH
#	define CFG_7241_DDR1_EXT_MODE_VAL    0x2
#else
#	define CFG_7241_DDR1_EXT_MODE_VAL    0x0
#endif
#define CFG_7241_DDR1_MODE_VAL        0x33
//#define CFG_7241_DDR1_MODE_VAL        0x23
#define CFG_7241_DDR1_CONFIG2_VAL	0x9dd0e6a8


#define CFG_7241_DDR2_CONFIG_VAL	0xc7bc8cd0
#define CFG_7241_DDR2_MODE_VAL_INIT	0x133
#define CFG_7241_DDR2_EXT_MODE_VAL	0x402
#define CFG_7241_DDR2_MODE_VAL		0x33
#define CFG_7241_DDR2_CONFIG2_VAL	0x9dd0e6a8
#endif /* _SUPPORT_AR7241 */

/* DDR settings for AR7240 */

#define CFG_DDR_REFRESH_VAL     0x4f10
#define CFG_DDR_CONFIG_VAL      0xc7bc8cd0
#define CFG_DDR_MODE_VAL_INIT   0x133
#ifdef DDR_LOW_DRIVE_STRENGTH
#       define CFG_DDR_EXT_MODE_VAL    0x2
#else
#       define CFG_DDR_EXT_MODE_VAL    0x0
#endif
#define CFG_DDR_MODE_VAL        0x33

#define CFG_DDR_TRTW_VAL        0x1f
#define CFG_DDR_TWTR_VAL        0x1e

#define CFG_DDR_CONFIG2_VAL      0x9dd0e6a8
#define CFG_DDR_RD_DATA_THIS_CYCLE_VAL  0x00ff

/* DDR2 Init values */
#define CFG_DDR2_EXT_MODE_VAL    0x402


#define CONFIG_NET_MULTI
#define CONFIG_NET_RETRY_COUNT 1000

#define CONFIG_MEMSIZE_IN_BYTES
#define CONFIG_PCI

/*-----------------------------------------------------------------------
 * Configuration
 */
#define CONFIG_COMMANDS	(( CONFIG_CMD_DFL | CFG_CMD_ELF |	\
	CFG_CMD_PING | CFG_CMD_NET | CFG_CMD_ENV | CFG_CMD_JFFS2 |	\
	CFG_CMD_FLASH | CFG_CMD_RUN | CFG_CMD_MII | CFG_CMD_TFTP_SERVER ) & \
	~(CFG_CMD_ITEST | CFG_CMD_NFS | CFG_CMD_LOADS | CFG_CMD_LOADB | \
	  CFG_CMD_CONSOLE | CFG_CMD_SNTP))




#define CFG_PHY_ADDR 0
#define CFG_AG7240_NMACS 2
#define CFG_GMII     0
#define CFG_MII0_RMII             1
#define CFG_AG7100_GE0_RMII             1
#define CONFIG_PHY_GIGE 1

#define CFG_BOOTM_LEN	(16 << 20) /* 16 MB */
#if 0
#define DEBUG
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "hush>"
#endif

#define CONFIG_IPADDR   192.168.1.20
#define CONFIG_SERVERIP 192.168.1.254
#define CONFIG_ETHADDR 00:15:6d:0d:00:00
#define CFG_FAULT_ECHO_LINK_DOWN    1


#define	RADIO_TYPE_UNKNOWN	0xff
#define	RADIO_TYPE_MERLIN	0x00
#define	RADIO_TYPE_KITE		0x01
#define	RADIO_TYPE_KIWI		0x02
#define	RADIO_TYPE_OSPREY	0x03
#define	RADIO_TYPE_NORADIO	0x04

#define	HW_ADDR_COUNT 0x02

#define CAL_ON_LAST_SECTOR

#ifdef CAL_ON_LAST_SECTOR
#define WLANCAL			(ar724x_get_wlancfg_addr())
#define BOARDCAL		(ar724x_get_bdcfg_addr())
#define CAL_SECTOR		(ar724x_get_bdcfg_sector())
#define WLANCAL_OFFSET					0x1000
#define BOARDCAL_OFFSET					0x0
#else
#define WLANCAL                         0x9fff1000
#define BOARDCAL                        0x9fff0000
#define CAL_SECTOR                      127
#endif


/* For Merlin, both PCI, PCI-E interfaces are valid,
 * for Kite, however, only PCI-E is valid */

#define AR7240_ART_PCICFG_OFFSET        (ar724x_get_pcicfg_offset())


/* Ubiquiti */
#define UBNT_FLASH_DETECT
// Fallthrough for AR7240
#define PLL_CONFIG_VAL	PLL_VAL(400,400,200)


/* Ubiquiti fw rescue extensions */
#define CONFIG_URESCUE

#undef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY    1
#define CFG_CONSOLE_INFO_QUIET 1

/* Unused code removal */
#define CONFIG_ELF_REDUCED 1
#define CONFIG_NET_REDUCED 1
#define CONFIG_JFFS2_REDUCED 1

#undef CONFIG_BOOTARGS
#define	CONFIG_BOOTARGS        "console=tty0 root=31:03 rootfstype=squashfs init=/init"
#define	CONFIG_BOOTARGS_TTYS0  "console=ttyS0,115200 root=31:03 rootfstype=squashfs init=/init"

/* Reset button is on GPIO12 */
#define AR7240_RESET_BTN	12

/* LED stuff */
#define CONFIG_SHOW_BOOT_PROGRESS

/* Application code locations
*/

#define UBNT_APP_MAGIC_OFFSET           0x80200000
#define UBNT_APP_START_OFFSET           0x80200020
#define UBNT_APP_SIZE                   (128 * 1024)

#define UBNT_FLASH_DETECT
#define FLASH_SIZE 8
#define WLANCAL_SIZE                    (16 * 1024)
#define UBNT_CFG_PART_SIZE                  0x40000 /* 256k(cfg) */


/* WD configuration */
#define	UBNT_USE_WATCHDOG

/* FLASH write protection*/
#define UBNT_FLASH_PROTECT_FEATURE
#ifdef UBNT_FLASH_PROTECT_FEATURE
#define CONFIG_FLASH_WP 1
#define ENABLE_FLASH_WP 1
#define FLASH_WP_GPIO 0
#define ENABLE_UBNT_PROTECT_CMD 1
#endif

#define UNUSED(x) ((void)(x))

/*Check firmware version compatibility*/
#define FW_VER_CHECK

#include <cmd_confdefs.h>

#endif	/* __CONFIG_H */
