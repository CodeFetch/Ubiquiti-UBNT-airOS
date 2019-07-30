/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright © 2003 Atheros Communications, Inc.,  All Rights Reserved.
 */

#include <config.h>
#include <linux/types.h>
#include <common.h>
#include <miiphy.h>
#include "phy.h"
#include <asm/addrspace.h>
#include "ar7240_soc.h"
#include "athr8021_phy.h"

#define MODULE_NAME "ATHR8021"

#define sysMsDelay(_x) udelay((_x) * 1000)
#define DRV_PRINT(fmt...) do { if (eth_debug > 0) { printf(fmt); } } while (0)

static int eth_debug = -1;

typedef struct {
  int              is_enet_port;
  int              mac_unit;
  unsigned int     phy_addr;
  unsigned int     id1;
  unsigned int     id2;
} athr8021_phy_t;

static athr8021_phy_t phy_info[] = {
    {is_enet_port: 1,
     mac_unit    : 0,
     phy_addr    : 0x01,
     id1         : ATHR_PHY_ID1_EXPECTED,
     id2         : ATHR_AR8021_PHY_ID2_EXPECTED }
};

static athr8021_phy_t *
athr8021_phy_find(int unit)
{
    unsigned int i;
    athr8021_phy_t *phy;
    u32 id1, id2;
    
    for(i = 0; i < sizeof(phy_info)/sizeof(athr8021_phy_t); i++) {
	phy = &phy_info[i];
	if (phy->is_enet_port && (phy->mac_unit == unit)) {
		id1 = phy_reg_read(unit, phy->phy_addr, ATHR_PHY_ID1);
		id2 = phy_reg_read(unit, phy->phy_addr, ATHR_PHY_ID2);
		if ((id1 == phy->id1) && (id2 == phy->id2))
			return phy;
		}
	}

    return NULL;
}

static int getenv_int(const char* var) {
	const char* val = getenv(var);
	if (!val)
		return 0;
	return simple_strtol(val, NULL, 10);
}

BOOL
athr8021_phy_probe(int unit)
{
	static int found = 0;
	// If already detected, skip search.  If not yet found, always check again
	if(!found) {
		found = !!athr8021_phy_find(unit);
		if(found)
			DRV_PRINT( "AR8021 Detected\n");
	}
	return found;
}

BOOL
athr8021_phy_setup(int unit)
{
    athr8021_phy_t *phy = athr8021_phy_find(unit);
    uint16_t  phyHwStatus;
    uint16_t  timeout;

    eth_debug = getenv_int("ethdebug");

    DRV_PRINT("%s: enter athr8021_phy_setup.  MAC unit %d!\n", MODULE_NAME, unit);

    if (!phy) {
        DRV_PRINT("%s: \nNo phy found for unit %d\n", MODULE_NAME, unit);
        return 0;
    }


    /*
     * After the phy is reset, it takes a little while before
     * it can respond properly.
     */
    phy_reg_write(unit, phy->phy_addr, ATHR_AUTONEG_ADVERT, ATHR_ADVERTISE_10M);

    phy_reg_write(unit, phy->phy_addr, ATHR_1000BASET_CONTROL, 0);

    /* necessary PHY FIXUP: delay rx_clk */
    phy_reg_write(unit, phy->phy_addr, 0x1D, 0x0);
    phy_reg_write(unit, phy->phy_addr, 0x1E, 0x34E);
    /* necessary PHY FIXUP: delay tx_clk */
    phy_reg_write(unit, phy->phy_addr, 0x1D, 0x5);
    phy_reg_write(unit, phy->phy_addr, 0x1E, 0x3D47);

    /* Reset PHYs*/
    phy_reg_write(unit, phy->phy_addr, ATHR_PHY_CONTROL,
                  ATHR_CTRL_AUTONEGOTIATION_ENABLE
                  | ATHR_CTRL_SOFTWARE_RESET);

    sysMsDelay(100);

    /*
     * Wait up to 3 seconds for ALL associated PHYs to finish
     * autonegotiation.  The only way we get out of here sooner is
     * if ALL PHYs are connected AND finish autonegotiation.
     */
    for (timeout = 50; timeout; sysMsDelay(150), timeout--) {
        phyHwStatus = phy_reg_read(unit, phy->phy_addr, ATHR_PHY_CONTROL);

	if (!ATHR_RESET_DONE(phyHwStatus))
		continue;
#ifndef PHY_WAIT_AUTONEG
	else {
		sysMsDelay(100);
		break;
	}
#else
        phyHwStatus = phy_reg_read(unit, phy->phy_addr, ATHR_PHY_STATUS);
        if (ATHR_AUTONEG_DONE(phyHwStatus)) {
	    DRV_PRINT("%s: Port %d, Negotiation Success\n", MODULE_NAME, unit);
            break;
        }
#endif
    }
    if (timeout == 0) {
	    DRV_PRINT("%s: Port %d, Negotiation timeout\n", MODULE_NAME, unit);
    }
    DRV_PRINT("%s: Port %d, out of loop (%d)\n", MODULE_NAME, unit, timeout);

    athr8021_phy_speed(unit);

    return 1;
}

int
athr8021_phy_is_up(int unit)
{
    int status;
    athr8021_phy_t *phy = athr8021_phy_find(unit);

    DRV_PRINT("%s: enter athr8021_phy_is_up!\n", MODULE_NAME);
    if (!phy) {
        DRV_PRINT("%s: phy unit %d not found !\n", MODULE_NAME, unit);
       	return 0;
    }

    status = phy_reg_read(unit, phy->phy_addr, ATHR_PHY_SPEC_STATUS);

    if (status & ATHR_STATUS_LINK_PASS) {
	DRV_PRINT("%s: athr8021 Link up! \n", MODULE_NAME);
        return 1;
    }

    DRV_PRINT("%s: athr8021 Link down! \n", MODULE_NAME);
    return 0;
}

int
athr8021_phy_is_fdx(int unit)
{
    int status;
    athr8021_phy_t *phy = athr8021_phy_find(unit);
    int ii = 200;

    DRV_PRINT("%s: enter athr8021_phy_is_fdx!\n", MODULE_NAME);

    if (!phy) {
        DRV_PRINT("%s: phy unit %d not found !\n", MODULE_NAME, unit);
       	return 0;
    }

    do {
    status = phy_reg_read(unit, phy->phy_addr, ATHR_PHY_SPEC_STATUS);
	    sysMsDelay(10);
    } while((!(status & ATHR_STATUS_RESOLVED)) && --ii);
    status = !(!(status & ATHER_STATUS_FULL_DEPLEX));

    return (status);
}

void athr8021_phy_reg_dump(int mac_unit, int phy_unit);

void ar7240_sys_frequency(u32 *cpu_freq, u32 *ddr_freq, u32 *ahb_freq);

u32 athr8021_phy_get_xmii_config(int macUnit, u32 speed) {
	switch (speed)
	{
	case _1000BASET:
		return 0x12000000;
	case _100BASET:
		return 0x0101;
	default:
	case _10BASET:
		{
			u32 ahb = 0;
			ar7240_sys_frequency(0, 0, &ahb);
			if (ahb == 195000000) {
				return 0x1313; // NB: Because we have clock at 195
			} else if (ahb == 200000000) {
				return 0x1616;
			} else {
				printf( "WARNING: Unsupported AHB frequency, %d.  10baseT clock may be wrong.\n", ahb);
				return 0x1616;
			}
		}
	}
}

int
athr8021_phy_speed(int unit)
{
    int status;
    athr8021_phy_t *phy = athr8021_phy_find(unit);
    int ii = 200;

    DRV_PRINT("%s: enter athr8021_phy_speed!\n", MODULE_NAME);

    if (!phy) {
        DRV_PRINT("%s: phy unit %d not found !\n", MODULE_NAME, unit);
       	return 0;
    }

    if (eth_debug > 0) {
        athr8021_phy_reg_dump(phy->mac_unit, phy->phy_addr);
    }

    do {
        status = phy_reg_read(unit, phy->phy_addr, ATHR_PHY_SPEC_STATUS);
	    sysMsDelay(10);
    }while((!(status & ATHR_STATUS_RESOLVED)) && --ii);

    DRV_PRINT("%s status = 0x%08x\n", __func__, status);
    status = ((status & ATHER_STATUS_LINK_MASK) >> ATHER_STATUS_LINK_SHIFT);
    DRV_PRINT("%s speed is %d\n", __func__, status);

    switch(status) {
    case 0:
        return _10BASET;
    case 1:
        return _100BASET;
    case 2:
        return _1000BASET;
    default:
        DRV_PRINT("%s: Unknown speed read!\n", MODULE_NAME);
    }
    return _10BASET;
}

void athr8021_phy_reg_dump(int mac_unit, int phy_unit)
{
  int i, status;

  for ( i = 0; i<0x1d ; i++) {
    status = phy_reg_read(mac_unit, phy_unit, i);
    DRV_PRINT("%04x  ", status);
    if ( (i%8) == 7) DRV_PRINT("\n");
  }
  DRV_PRINT("\n");
  i=0x0;
  phy_reg_write(mac_unit, phy_unit, 0x1D, i);
  status = phy_reg_read(mac_unit, phy_unit, 0x1E);
  DRV_PRINT("DEBUG %02x: %04x\n", i, status);
  i=0x05;
  phy_reg_write(mac_unit, phy_unit, 0x1D, i);
  status = phy_reg_read(mac_unit, phy_unit, 0x1E);
  DRV_PRINT("DEBUG %02x: %04x\n", i, status);
  i=0x10;
  phy_reg_write(mac_unit, phy_unit, 0x1D, i);
  status = phy_reg_read(mac_unit, phy_unit, 0x1E);
  DRV_PRINT("DEBUG %02x: %04x\n", i, status);
  i=0x11;
  phy_reg_write(mac_unit, phy_unit, 0x1D, i);
  status = phy_reg_read(mac_unit, phy_unit, 0x1E);
  DRV_PRINT("DEBUG %02x: %04x\n", i, status);
  i=0x12;
  phy_reg_write(mac_unit, phy_unit, 0x1D, i);
  status = phy_reg_read(mac_unit, phy_unit, 0x1E);
  DRV_PRINT("DEBUG %02x: %04x\n", i, status);
}
