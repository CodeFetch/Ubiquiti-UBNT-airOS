#include <common.h>
#include <command.h>
#include <asm/addrspace.h>
#include "ar7240_soc.h"
#include "board.h"
#include "pci.h"

extern flash_info_t flash_info[];	/* info for FLASH chips */

static ubnt_bdinfo_t ubnt_bdinfo;

typedef unsigned char enet_addr_t[6];

static int radio_identify(radio_info_t* radio, int scan_pci);

u32
ar724x_get_pcicfg_offset(void) {
	u32 off = 12;
	const ubnt_bdinfo_t* b = board_identify(NULL,1);
	if (b) {
		switch (b->radio.type) {
		case RADIO_TYPE_KITE:
		case RADIO_TYPE_OSPREY:
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
ar724x_get_wlancfg_addr(void) {
	return ar724x_get_cfgsector_addr() + WLANCAL_OFFSET;
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
radio_ar928x_identify(radio_info_t* radio, unsigned long wlancal_addr, unsigned long wlancal_end) {
	int radio_type = RADIO_TYPE_UNKNOWN;
	uint16_t eeoffset = MERLIN_EEPROM_OFFSET;
	uint16_t subdev_offset = ATH7K_OFFSET_PCIE_SUBSYSTEM_DEVICE_ID;
	uint16_t subvend_offset = ATH7K_OFFSET_PCIE_SUBSYSTEM_VENDOR_ID;

	/* safety check - ar928x calibration data ALWAYS starts with a55a */
	if (*(unsigned short *)wlancal_addr != 0xa55a) {
		if (radio) radio->type = RADIO_TYPE_UNKNOWN;
		return RADIO_TYPE_UNKNOWN;
	}

	if (check_eeprom_crc((void*)(wlancal_addr + KIWI_EEPROM_OFFSET), wlancal_end)) {
		radio_type = RADIO_TYPE_KIWI;
		eeoffset = KIWI_EEPROM_OFFSET;
	} else if (check_eeprom_crc((void*)(wlancal_addr + KITE_EEPROM_OFFSET), wlancal_end)) {
		radio_type = RADIO_TYPE_KITE;
		eeoffset = KITE_EEPROM_OFFSET;
		subdev_offset = ATH7K_OFFSET_PCI_SUBSYSTEM_DEVICE_ID;
		subvend_offset = ATH7K_OFFSET_PCI_SUBSYSTEM_VENDOR_ID;
	} else if (check_eeprom_crc((void*)(wlancal_addr + MERLIN_EEPROM_OFFSET), wlancal_end)) {
		radio_type = RADIO_TYPE_MERLIN;
		eeoffset = MERLIN_EEPROM_OFFSET;
	}

	if (radio) {
		radio->type = radio_type;
	}
	return radio_type;
}


static int
radio_ar93xx_identify(radio_info_t* radio,
		unsigned long wlancal_addr, unsigned long wlancal_end, int scan_pci) {
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
	u16 ssid;
	u16 svend;
	int device;
	int function;
	unsigned char header_t;
	unsigned short vendor_id,device_id;
	int bus_num;
	pci_dev_t dev;

	if (scan_pci) {
		for (device = 0; device < PCI_MAX_PCI_DEVICES; device++) {
			header_t = 0;
			vendor_id = 0;
			for (function = 0; function < PCI_MAX_PCI_FUNCTIONS; function++) {
				/*
				 * If this is not a multi-function device, we skip the rest.
				 */
				if (function && !(header_t & 0x80))
					break;

				dev = PCI_BDF(bus_num, device, function);

				pci_read_config_word(dev, PCI_VENDOR_ID, &vendor_id);
				pci_read_config_word(dev, PCI_DEVICE_ID, &device_id);

				if ((vendor_id == 0xFFFF) || (vendor_id == 0x0000))
					continue;

				if (!function) pci_read_config_byte(dev, PCI_HEADER_TYPE, &header_t);

				/*check for peacoc*/
				if ((vendor_id == 0x168c) && (device_id == 0x33)){

					pci_read_config_word(dev, PCI_SUBSYSTEM_VENDOR_ID, &svend);
					pci_read_config_word(dev, PCI_SUBSYSTEM_ID, &ssid);

					radio->type = RADIO_TYPE_PEACOCK;
					return radio->type;
				}
			}
		}
	}


	/*
	 *leave this basic check for situation when PCI access is unavailable or failed
	 */

	/* let's assume osprey/peacock will always use eepromVersion >= 02 and
	 * templateVersion >= 02 */
	if ((*start < 0x02) || (*(start + 1) < 0x02))
		return RADIO_TYPE_UNKNOWN;


	radio->type = RADIO_TYPE_PEACOCK;

	return radio->type;
}



static int
radio_identify(radio_info_t* radio, int scan_pci) {
	int radio_type = RADIO_TYPE_UNKNOWN;

	unsigned long wlancal_addr = WLANCAL;
	unsigned long boardcal_addr = BOARDCAL;
	unsigned int sector_size = CFG_FLASH_SECTOR_SIZE;
	void* boardcal_end = (void*)(boardcal_addr + sector_size);
	unsigned short signature = *(unsigned short *)wlancal_addr;

	switch (signature) {
	case 0xa55a:
		radio_type = radio_ar928x_identify(radio, wlancal_addr, boardcal_end);
		break;
	case 0xffff:
		radio_type = RADIO_TYPE_UNKNOWN;
		if (radio) {
			radio->type = radio_type;
		}
		break;
	default:
		radio_type = radio_ar93xx_identify(radio, wlancal_addr, boardcal_end, scan_pci);
		break;
	}

	if (radio_type == RADIO_TYPE_UNKNOWN)
		printf("Board not calibrated, cannot determine board type.\n");

	return radio_type;
}

const ubnt_bdinfo_t*
board_identify(ubnt_bdinfo_t* bd, int scan_pci) {
	ubnt_bdinfo_t* b = (bd == NULL) ? &ubnt_bdinfo : bd;
	unsigned long boardcal_addr = BOARDCAL;

	memset(b, 0, sizeof(*b));
	radio_identify(&b->radio, scan_pci);

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



