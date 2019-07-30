#ifndef __BOARD_H
#define __BOARD_H

#define BOARD_CPU_TYPE_AR9341 1
#define BOARD_CPU_TYPE_AR9342 2
#define BOARD_CPU_TYPE_AR9344 4

#define ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#define UNUSED(x) ((void)(x))


#ifndef RADIO_TYPE_UNKNOWN
#define	RADIO_TYPE_UNKNOWN	0xff
#define RADIO_TYPE_MERLIN	0
#define	RADIO_TYPE_KITE		1
#define	RADIO_TYPE_KIWI		2
#endif /* RADIO_TYPE_UNKNOWN */

#define subsysid subdeviceid

typedef struct radio_info {
	u8 type;
} radio_info_t;


typedef struct ubnt_bdinfo {
	radio_info_t radio;
} ubnt_bdinfo_t;

const ubnt_bdinfo_t* board_identify(ubnt_bdinfo_t* b, int scan_pci);

u32 ar724x_get_pcicfg_offset(void);


#ifdef CAL_ON_LAST_SECTOR
#define WLANCAL_OFFSET		0x1000
#define BOARDCAL_OFFSET		0x0
u32 ar724x_get_bdcfg_addr(void);

u32 ar724x_get_wlancfg_addr(void);

u32 ar724x_get_bdcfg_sector(void);
#endif

#define SWAP16(_x) ( (unsigned short)( (((const unsigned char *)(&_x))[1] ) | ( ( (const unsigned char *)( &_x ) )[0]<< 8) ) )

#define MERLIN_EEPROM_OFFSET 0x200
#define KITE_EEPROM_OFFSET 0x80
#define KIWI_EEPROM_OFFSET 0x100

#define WLAN_MAC_OFFSET 0xC
#define OSPREY_MAC_OFFSET 0x2

#endif /* __BOARD_H */
