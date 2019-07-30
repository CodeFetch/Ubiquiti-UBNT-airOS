#include <common.h>
#include <asm/global_data.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm-mips/regdef.h>
#include <config.h>
#include <version.h>
#include "ar7240_soc.h"

extern void ar7240_ddr_initial_config(uint32_t refresh);
extern int ar7240_ddr_find_size(void);

DECLARE_GLOBAL_DATA_PTR;

/* global variable for storing RESET button state */
int ResetButtonOnStartup = 0;

static void
ar7240_usb_initial_config(void) {
	ar7240_reg_wr_nf(AR7240_USB_PLL_CONFIG, 0x0a04081e);
	ar7240_reg_wr_nf(AR7240_USB_PLL_CONFIG, 0x0804081e);
}

static void
ar7240_gpio_config(void) {
	/* Disable clock obs */
	ar7240_reg_wr(AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) & 0xffe7e0ff));
	/* Enable eth Switch LEDs */
#ifdef CONFIG_K31
	ar7240_reg_wr(AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) | 0xd8));
#else
	ar7240_reg_wr(AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) | 0xfa));
#endif

	/*
	 * According AR7242 datasheet bits 7:4 are reserved; set to 0
	 * Gpio 13 bit should be set to 0 as well. Reason - Ethernet LED function does not work on AR7242.
	 */
	if (is_ar7242()) {
		ar7240_reg_wr(AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) & ~0xf8));
	}

}

long int
initdram(int board_type) {
	UNUSED(board_type);
	unsigned int cpurevid = 0;

	ar7240_ddr_initial_config(CFG_DDR_REFRESH_VAL);
	cpurevid = ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK;

	/* Default tap values for starting the tap_init*/
	if (!(_is_ar7241(cpurevid) || _is_ar7242(cpurevid)))  {
		ar7240_reg_wr(AR7240_DDR_TAP_CONTROL0, 0x8);
		ar7240_reg_wr(AR7240_DDR_TAP_CONTROL1, 0x9);
		ar7240_ddr_tap_init();
	} else {
		ar7240_reg_wr(AR7240_DDR_TAP_CONTROL0, 0x2);
		ar7240_reg_wr(AR7240_DDR_TAP_CONTROL1, 0x2);
		ar7240_reg_wr(AR7240_DDR_TAP_CONTROL2, 0x0);
		ar7240_reg_wr(AR7240_DDR_TAP_CONTROL3, 0x0);
	}

#ifdef DEBUG
	{
		unsigned int tap_val1, tap_val2;
		tap_val1 = ar7240_reg_rd(0xb800001c);
		tap_val2 = ar7240_reg_rd(0xb8000020);

		printf("#### TAP VALUE 1 = 0x%x, 2 = 0x%x [0x%x: 0x%x]\n",
				tap_val1, tap_val2, *(unsigned *)0x80500000,
				*(unsigned *)0x80500004);
	}
#endif

	/* XXX why secPLL/GPIO configs are here? */
	ar7240_usb_initial_config();
	ar7240_gpio_config();

	return (ar7240_ddr_find_size());
}

void ar7240_hw_reset() {
	for (;;) {
		ar7240_reg_wr(AR7240_RESET,
				(AR7240_RESET_FULL_CHIP | AR7240_RESET_DDR));
		}
}


void show_boot_progress(int arg) {
}


#ifdef	UBNT_USE_WATCHDOG

void ubnt_wd_enable(u32 counter) {
	ar7240_reg_wr_nf (AR7240_WATCHDOG_TMR_CONTROL, 0x00000003);
	ar7240_reg_wr_nf (AR7240_WATCHDOG_TMR, counter);
}

void ubnt_wd_reset(u32 counter) {
	ar7240_reg_wr_nf (AR7240_WATCHDOG_TMR, counter);
}

void ubnt_wd_disable() {
	ar7240_reg_wr_nf (AR7240_WATCHDOG_TMR_CONTROL, 0x00000000);
}

#endif

#ifdef UBNT_APP
int call_boot_progress_app(int status) {
	char app_cmd[CFG_CBSIZE];
	sprintf(app_cmd, "go ${ubntaddr} ushowbootprogress %d",status);
	run_command(app_cmd, 0);
	return 0;
}

#endif /* #ifdef UBNT_APP */


extern flash_info_t flash_info[];	/* info for FLASH chips */

#ifdef CAL_ON_LAST_SECTOR
#define WLANCAL_OFFSET		0x1000
#define BOARDCAL_OFFSET		0x0
u32 ar724x_get_bdcfg_addr(void);
u32 ar724x_get_wlancfg_addr(void);
#endif

#define SWAP16(_x) ( (unsigned short)( (((const unsigned char *)(&_x))[1] ) | ( ( (const unsigned char *)( &_x ) )[0]<< 8) ) )

#define MERLIN_EEPROM_OFFSET 0x200
#define KITE_EEPROM_OFFSET 0x80
#define KIWI_EEPROM_OFFSET 0x100

#define WLAN_MAC_OFFSET 0xC


static int radio_identify();

u32
ar724x_get_pcicfg_offset(void) {
	u32 off = 12;
	int radio_type  = radio_identify(NULL);
	switch (radio_type) {
		case RADIO_TYPE_KITE:
		case RADIO_TYPE_OSPREY:
			off = 3;
			break;
		default:
			off = 12;
			break;
	}
	return off;
}

#ifdef CAL_ON_LAST_SECTOR
static inline u32
ar724x_get_cfgsector_addr(void) {
	/* check whether flash_info is already populated */
	if ((u32)KSEG1ADDR(AR7240_SPI_BASE) !=
			(u32)KSEG1ADDR(flash_info[0].start[0])) {
		/* TODO: this is true only for 64k sector size single SPI flash */
		return 0x9fff0000;
	}
	return (flash_info[0].start[0] + flash_info[0].size -
			   (flash_info[0].size / flash_info[0].sector_count));
}

u32
ar724x_get_bdcfg_addr(void) {
	return ar724x_get_cfgsector_addr() + BOARDCAL_OFFSET;
}

u32
ar724x_get_wlancfg_addr(void) {
	return ar724x_get_cfgsector_addr() + WLANCAL_OFFSET;
}

#endif

static int
check_eeprom_crc(void *base, void* end) {
	u_int16_t sum = 0, data, *pHalf;
	u_int16_t lengthStruct, i;

	lengthStruct = SWAP16(*(u_int16_t*)base);

	if ((((unsigned int)base+lengthStruct) > ((unsigned int)end))
			|| (lengthStruct == 0) ) {
		return 0;
	}

	/* calc */
	pHalf = (u_int16_t *)base;

	for (i = 0; i < lengthStruct/2; i++) {
		data = *pHalf++;
		sum ^= SWAP16(data);
	}

	if (sum != 0xFFFF) {
		return 0;
	}

	return 1;
}

static int
radio_ar928x_identify(unsigned long wlancal_addr, unsigned long wlancal_end) {
	int radio_type = RADIO_TYPE_UNKNOWN;
	uint16_t eeoffset = MERLIN_EEPROM_OFFSET;

	/* safety check - ar928x calibration data ALWAYS starts with a55a */
	if (*(unsigned short *)wlancal_addr != 0xa55a) {
		return RADIO_TYPE_UNKNOWN;
	}

	if (check_eeprom_crc((void*)(wlancal_addr + KIWI_EEPROM_OFFSET), wlancal_end)) {
		radio_type = RADIO_TYPE_KIWI;
		eeoffset = KIWI_EEPROM_OFFSET;
	} else if (check_eeprom_crc((void*)(wlancal_addr + KITE_EEPROM_OFFSET), wlancal_end)) {
		radio_type = RADIO_TYPE_KITE;
		eeoffset = KITE_EEPROM_OFFSET;
	} else if (check_eeprom_crc((void*)(wlancal_addr + MERLIN_EEPROM_OFFSET), wlancal_end)) {
		radio_type = RADIO_TYPE_MERLIN;
		eeoffset = MERLIN_EEPROM_OFFSET;
	}

	return radio_type;
}

static int
radio_identify(void) {
	int radio_type = RADIO_TYPE_UNKNOWN;

	unsigned long wlancal_addr = WLANCAL;
	unsigned long boardcal_addr = BOARDCAL;
	unsigned int sector_size = CFG_FLASH_SECTOR_SIZE;
	void* boardcal_end = (void*)(boardcal_addr + sector_size);
	unsigned short signature = *(unsigned short *)wlancal_addr;

	switch (signature) {
	case 0xa55a:
		radio_type = radio_ar928x_identify( wlancal_addr, boardcal_end);
		break;
	}

	return radio_type;
}


void ubnt_cer(void) {
#ifdef DEBUG
	printf("%s deasserting external reset\n", __func__, status);
#endif
	ar7240_reg_rmw_clear(AR7240_RESET, AR7240_RESET_EXTERNAL);
	udelay(100);

}
