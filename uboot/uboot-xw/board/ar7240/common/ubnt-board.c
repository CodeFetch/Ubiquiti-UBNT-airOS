#include <common.h>
#include <command.h>
#include <asm/addrspace.h>
#include "ar7240_soc.h"
#include "ubnt-board.h"
#include "../../../cpu/mips/ar7240/ag934x.h"

extern flash_info_t flash_info[];	/* info for FLASH chips */

static ubnt_bdinfo_t ubnt_bdinfo;

typedef unsigned char enet_addr_t[6];

static int wifi_identify(wifi_info_t* wifi, int idx);

/* FIXME */
u32
ar724x_get_pcicfg_offset(void) {
	u32 off = 12;
	const ubnt_bdinfo_t* b = board_identify(NULL);
	if (b) {
		switch (b->wifi0.type) {
		case WIFI_TYPE_KITE:
		case WIFI_TYPE_OSPREY:
			off = 3;
			break;
		default:
			off = 12;
			break;
		}
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
ar724x_get_wlancfg_addr(int idx) {
	return ar724x_get_cfgsector_addr() + WLANCAL_OFFSET + (WLANCAL_SIZE * idx);
}

u32
ar724x_get_bdcfg_sector(void) {
	return flash_info[0].sector_count - 1;
}

#endif

#define ATH7K_OFFSET_PCI_VENDOR_ID 0x04
#define ATH7K_OFFSET_PCI_DEVICE_ID 0x05

#define ATH7K_OFFSET_PCI_SUBSYSTEM_VENDOR_ID 0x0A
#define ATH7K_OFFSET_PCI_SUBSYSTEM_DEVICE_ID 0x0B

#define ATH7K_OFFSET_PCIE_VENDOR_ID 0x0D
#define ATH7K_OFFSET_PCIE_DEVICE_ID 0x0E
#define ATH7K_OFFSET_PCIE_SUBSYSTEM_VENDOR_ID 0x13
#define ATH7K_OFFSET_PCIE_SUBSYSTEM_DEVICE_ID 0x14

#define ATH7K_OFFSET_MAC1 0x106
#define ATH7K_OFFSET_MAC2 0x107
#define ATH7K_OFFSET_MAC3 0x108


static inline uint16_t
eeprom_get(void* ee, int off) {
	return *(uint16_t*)((char*)ee + off*2);

}

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
wifi_ar928x_identify(wifi_info_t* wifi, unsigned long wlancal_addr, unsigned long wlancal_end) {
	int wifi_type = WIFI_TYPE_UNKNOWN;
	uint16_t eeoffset = MERLIN_EEPROM_OFFSET;
	uint16_t subdev_offset = ATH7K_OFFSET_PCIE_SUBSYSTEM_DEVICE_ID;
	uint16_t subvend_offset = ATH7K_OFFSET_PCIE_SUBSYSTEM_VENDOR_ID;

	/* safety check - ar928x calibration data ALWAYS starts with a55a */
	if (*(unsigned short *)wlancal_addr != 0xa55a) {
		if (wifi) wifi->type = WIFI_TYPE_UNKNOWN;
		return WIFI_TYPE_UNKNOWN;
	}

	if (check_eeprom_crc((void*)(wlancal_addr + KIWI_EEPROM_OFFSET), (void *)wlancal_end)) {
		wifi_type = WIFI_TYPE_KIWI;
		eeoffset = KIWI_EEPROM_OFFSET;
	} else if (check_eeprom_crc((void*)(wlancal_addr + KITE_EEPROM_OFFSET), (void *)wlancal_end)) {
		wifi_type = WIFI_TYPE_KITE;
		eeoffset = KITE_EEPROM_OFFSET;
		subdev_offset = ATH7K_OFFSET_PCI_SUBSYSTEM_DEVICE_ID;
		subvend_offset = ATH7K_OFFSET_PCI_SUBSYSTEM_VENDOR_ID;
	} else if (check_eeprom_crc((void*)(wlancal_addr + MERLIN_EEPROM_OFFSET), (void *)wlancal_end)) {
		wifi_type = WIFI_TYPE_MERLIN;
		eeoffset = MERLIN_EEPROM_OFFSET;
	}

	if (wifi) {
		wifi->type = wifi_type;
	}
	return wifi_type;
}


static int
wifi_ar93xx_identify(wifi_info_t* wifi,
		unsigned long wlancal_addr, unsigned long wlancal_end) {
	/* osprey based designs, including hornet/wasp have abandoned a55a signature
	 * when storing calibration data on flash. Even worse, PCI/PCIE initialization
	 * data, which contains vendor/device/subvendor/subsystem IDs are no longer
	 * stored on flash - it has been moved to OTP inside the chip.
	 * Calibration data checksum has also been omitted - doh!
	 * For the detection to be more reliable, we'd need to query PCI, but it may be
	 * not initialized yet! OTP could be reliable source, however that one is also
	 * available only after PCI initialization...
	 * since all of it is so complicated, let's just do some rudimentary guesses..
	 */
	unsigned char* start = (unsigned char*)wlancal_addr;

	/* let's assume osprey will always use eepromVersion >= 02 and
	 * templateVersion >= 02 */
	if ((*start < 0x02) || (*(start + 1) < 0x02))
		return WIFI_TYPE_UNKNOWN;


	wifi->type = WIFI_TYPE_OSPREY;
	/* hardcode subvendor/subsystem for now
	 * TODO: investigate OTP access without PCI initialization..
	 */
	return wifi->type;
}

static int
wifi_identify(wifi_info_t* wifi, int idx) {
	int wifi_type = WIFI_TYPE_UNKNOWN;

	unsigned long wlancal_addr = WLANCAL(idx);
	unsigned short signature = *(unsigned short *)wlancal_addr;

	switch (signature) {
	case 0xa55a:
		wifi_type = wifi_ar928x_identify(wifi, wlancal_addr, wlancal_addr + WLANCAL_SIZE);
		break;
	case 0xffff:
		wifi_type = WIFI_TYPE_UNKNOWN;
		if (wifi) {
			wifi->type = wifi_type;
		}
		break;
	default:
		wifi_type = wifi_ar93xx_identify(wifi, wlancal_addr, wlancal_addr + WLANCAL_SIZE);
		break;
	}

#ifdef DEBUG 
	if (wifi_type == WIFI_TYPE_UNKNOWN)
		printf("Wifi %d is not calibrated\n", idx);
#endif

	return wifi_type;
}

const ubnt_bdinfo_t*
board_identify(ubnt_bdinfo_t* bd) {
	ubnt_bdinfo_t* b = (bd == NULL) ? &ubnt_bdinfo : bd;

	memset(b, 0, sizeof(*b));

	/* wifi type detection */
	wifi_identify(&b->wifi0, 0);
	wifi_identify(&b->wifi1, 1);

	return b;
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

