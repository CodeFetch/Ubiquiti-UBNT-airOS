#include <common.h>
#include <config.h>
#include <asm/types.h>
#include <flash.h>

#ifndef CFG_FLASH_SIZE
#define CFG_FLASH_SIZE   0x00800000
#endif

/*
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long
flash_get_geom(flash_info_t* flash_info) {
	/* this is hardcoded and used only when flash identification fails */

	flash_info->flash_id  = FLASH_M25P64;
	flash_info->size = CFG_FLASH_SIZE; /* bytes */
	flash_info->sector_count = flash_info->size/CFG_FLASH_SECTOR_SIZE;

	return (flash_info->size);
}
