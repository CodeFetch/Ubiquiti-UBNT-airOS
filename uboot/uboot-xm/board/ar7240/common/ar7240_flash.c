#include <common.h>
#include <jffs2/jffs2.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include "ar7240_soc.h"
#include "ar7240_flash.h"

typedef struct spi_flash {
	char* name;
	u32 jedec_id;
	u32 sector_count;
	u32 sector_size;
} spi_flash_t;

#define SECTOR_SIZE_64K                 (64*1024)
#define SECTOR_SIZE_128K                (128*1024)
#define SECTOR_SIZE_256K                (256*1024)

static spi_flash_t SUPPORTED_FLASH_LIST[] = {
	/* JEDEC id's for supported flashes ONLY */
	/* ST Microelectronics */
	{ "m25p32",  0x202016,	 64, SECTOR_SIZE_64K },  /*  4MB */
	{ "m25p64",  0x202017,	128, SECTOR_SIZE_64K },  /*  8MB */
	{ "m25p128", 0x202018,	 64, SECTOR_SIZE_256K }, /* 16MB */

	/* Macronix */
	{ "mx25l32",  0xc22016,  64, SECTOR_SIZE_64K },  /*  4MB */
	{ "mx25l64",  0xc22017, 128, SECTOR_SIZE_64K },  /*  8MB */
	{ "mx25l128", 0xc22018, 256, SECTOR_SIZE_64K },  /* 16MB */

	/* Spansion */
	{ "s25sl032a", 0x010215,  64, SECTOR_SIZE_64K }, /*  4MB */
	{ "s25sl064a", 0x010216, 128, SECTOR_SIZE_64K }, /*  8MB */

	/* Intel */
	{ "s33_32M",    0x898912,  64, SECTOR_SIZE_64K }, /*  4MB */
	{ "s33_64M",    0x898913, 128, SECTOR_SIZE_64K }, /*  8MB */

	/* Winbond */
	{ "w25x32",	0xef3016,  64, SECTOR_SIZE_64K }, /*  4MB */
	{ "w25x64",	0xef3017, 128, SECTOR_SIZE_64K }, /*  8MB */
	{ "w25q64",	0xef4017, 128, SECTOR_SIZE_64K }, /*  8MB */
	{ "w25q128",	0xef4018, 256, SECTOR_SIZE_64K }, /*  16MB */
};

#define N(a) (sizeof((a)) / sizeof((a)[0]))

/*
 * globals
 */
flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

#undef display
#define display(x)  ;

/*
 * statics
 */
static void ar7240_spi_write_enable(void);
static void ar7240_spi_poll(void);
static void ar7240_spi_write_page(uint32_t addr, uint8_t *data, int len);
static void ar7240_spi_sector_erase(uint32_t addr);

static u32
read_id(u8 id_cmd, u8* data, int n)
{
	int i = 0;
	ar7240_reg_wr_nf(AR7240_SPI_FS, 1);
	ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);
	ar7240_spi_bit_banger(id_cmd);
	while (i < n) {
		ar7240_spi_delay_8();
		data[i] = ar7240_reg_rd(AR7240_SPI_RD_STATUS) & 0xff;
		++i;
	}
	ar7240_spi_done();
	return 0;
}

static spi_flash_t*
find_flash_data(u32 jedec)
{
	spi_flash_t* tmp;
	int i;
	for (i = 0, tmp = &SUPPORTED_FLASH_LIST[0];
			i < N(SUPPORTED_FLASH_LIST);
			i++, tmp++) {
		if (tmp->jedec_id == jedec)
			return tmp;
	}
	return NULL;
}

unsigned long
flash_init (void)
{
	u8 idbuf[5];
	u32 jedec;
	spi_flash_t* f;
	flash_info_t* flinfo = &flash_info[0];
	int i;
	u32 sector_size = 0;

	/* XXX: do we really need this crap here? */
	ar7240_reg_wr_nf(AR7240_SPI_CLOCK, 0x43);

	read_id(0x9f, idbuf, 3);
	jedec = (idbuf[0] << 16) | (idbuf[1] << 8) | idbuf[2];

	f = find_flash_data(jedec);
	if (f) {
		flinfo->flash_id = jedec;
		flinfo->size = f->sector_count * f->sector_size;
		flinfo->sector_count = f->sector_count;
	} else {
		/* flash was not detected - take the one, hardcoded
		 * in board info */
		flash_get_geom(flinfo);
	}

	sector_size = flinfo->size / flinfo->sector_count;

	/* fill in sector info */
	for (i = 0; i < flinfo->sector_count; ++i) {
		flinfo->start[i] = CFG_FLASH_BASE + (i * sector_size);
		flinfo->protect[i] = 0;
	}

	return flinfo->size;
}


void flash_print_info(flash_info_t *info)
{
	spi_flash_t* f;
	f = find_flash_data(info->flash_id);
	if (f) {
		printf("%s ", f->name);
	}
	printf("(Id: 0x%06x)\n", info->flash_id);

	printf("\tSize: %ld MB in %d sectors\n",
			info->size >> 20,
			info->sector_count);
}

int
flash_erase(flash_info_t *info, int s_first, int s_last)
{
    int i, sector_size = info->size/info->sector_count;

#ifdef DEBUG
    printf("\nFirst %#x last %#x sector size %#x\n",
           s_first, s_last, sector_size);
#endif

    for (i = s_first; i <= s_last; i++) {
        printf(".");
        ar7240_spi_sector_erase(i * sector_size);
    }
    ar7240_spi_done();
    printf(" done\n");

    return 0;
}

/*
 * Write a buffer from memory to flash:
 * 0. Assumption: Caller has already erased the appropriate sectors.
 * 1. call page programming for every 256 bytes
 */
int 
write_buff(flash_info_t *info, uchar *source, ulong addr, ulong len)
{
    int total = 0, len_this_lp, bytes_this_page;
    ulong dst;
    uchar *src;
#ifdef DEBUG
    printf ("write addr: %x\n", addr);
#endif
    addr = addr - CFG_FLASH_BASE;

    while(total < len) {
        src              = source + total;
        dst              = addr   + total;
        bytes_this_page  = AR7240_SPI_PAGE_SIZE - (addr % AR7240_SPI_PAGE_SIZE);
        len_this_lp      = ((len - total) > bytes_this_page) ? bytes_this_page
                                                             : (len - total);
        ar7240_spi_write_page(dst, src, len_this_lp);
        total += len_this_lp;
    }

    ar7240_spi_done();

    return 0;
}

static void
ar7240_spi_write_enable()  
{
    ar7240_reg_wr_nf(AR7240_SPI_FS, 1);                  
    ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
    ar7240_spi_bit_banger(AR7240_SPI_CMD_WREN);             
    ar7240_spi_go();
}

static void
ar7240_spi_poll()   
{
    int rd;                                                 

    do {
        ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);     
        ar7240_spi_bit_banger(AR7240_SPI_CMD_RD_STATUS);        
        ar7240_spi_delay_8();
        rd = (ar7240_reg_rd(AR7240_SPI_RD_STATUS) & 1);               
    }while(rd);
}

static void
ar7240_spi_write_page(uint32_t addr, uint8_t *data, int len)
{
    int i;
    uint8_t ch;

    display(0x77);
    ar7240_spi_write_enable();
    ar7240_spi_bit_banger(AR7240_SPI_CMD_PAGE_PROG);
    ar7240_spi_send_addr(addr);

    for(i = 0; i < len; i++) {
        ch = *(data + i);
        ar7240_spi_bit_banger(ch);
    }

    ar7240_spi_go();
    display(0x66);
    ar7240_spi_poll();
    display(0x6d);
}

static void
ar7240_spi_sector_erase(uint32_t addr)
{
    ar7240_spi_write_enable();
    ar7240_spi_bit_banger(AR7240_SPI_CMD_SECTOR_ERASE);
    ar7240_spi_send_addr(addr);
    ar7240_spi_go();
    display(0x7d);
    ar7240_spi_poll();
}
