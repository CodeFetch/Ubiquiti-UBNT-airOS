#ifndef __BOARD_H
#define __BOARD_H


#define	WIFI_TYPE_UNKNOWN	0xffff
#define	WIFI_TYPE_NORADIO	0x0000
#define	WIFI_TYPE_MERLIN	0x002a
#define	WIFI_TYPE_KITE		0x002b
#define	WIFI_TYPE_KIWI		0x002e
#define	WIFI_TYPE_OSPREY	0x0030


typedef struct wifi_info {
	uint16_t type;
} wifi_info_t;

#define BDINFO_MAGIC 0xDB1FACED

typedef struct ubnt_bdinfo {
    wifi_info_t wifi0;
    wifi_info_t wifi1;
} ubnt_bdinfo_t;

const ubnt_bdinfo_t* board_identify(ubnt_bdinfo_t* b);

u32 ar724x_get_pcicfg_offset(void);
const char* cpu_type_to_name(int);
void board_dump_ids(char*, ubnt_bdinfo_t*);

#ifdef CAL_ON_LAST_SECTOR
#define WLANCAL_OFFSET		0x1000
#define BOARDCAL_OFFSET		0x0
u32 ar724x_get_bdcfg_addr(void);

u32 ar724x_get_wlancfg_addr(int);

u32 ar724x_get_bdcfg_sector(void);
#endif

#define SWAP16(_x) ( (unsigned short)( (((const unsigned char *)(&_x))[1] ) | ( ( (const unsigned char *)( &_x ) )[0]<< 8) ) )

#define MERLIN_EEPROM_OFFSET 0x200
#define KITE_EEPROM_OFFSET 0x80
#define KIWI_EEPROM_OFFSET 0x100

#define UNUSED(x) ((void)(x))

#endif /* __BOARD_H */
