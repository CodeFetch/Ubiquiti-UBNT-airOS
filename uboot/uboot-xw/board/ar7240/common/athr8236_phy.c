/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright © 2007 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * Manage the atheros ethernet PHY.
 *
 * All definitions in this file are operating system independent!
 */

#include <config.h>
#include <linux/types.h>
#include <common.h>
#include <miiphy.h>
#include "phy.h"
#include <asm/addrspace.h>
#include "ar7240_soc.h"
#include "athr8236_phy.h"
#include "../../../cpu/mips/ar7240/ag934x.h"


/* PHY selections and access functions */

typedef enum
{
	PHY_SRCPORT_INFO,
	PHY_PORTINFO_SIZE,
} PHY_CAP_TYPE;

typedef enum
{
	PHY_SRCPORT_NONE,
	PHY_SRCPORT_VLANTAG,
	PHY_SRCPORT_TRAILER,
} PHY_SRCPORT_TYPE;

#define DRV_LOG(DBG_SW, X0, X1, X2, X3, X4, X5, X6)
#define DRV_MSG(x,a,b,c,d,e,f)
#define DRV_PRINT(DBG_SW,X)

#define ATHR_LAN_PORT_VLAN          1
#define ATHR_WAN_PORT_VLAN          2
#define ENET_UNIT_LAN 1
#define ENET_UNIT_WAN 0

#define TRUE    1
#define FALSE   0

#define ATHR_PHY0_ADDR   0x0
#define ATHR_PHY1_ADDR   0x1
#define ATHR_PHY2_ADDR   0x2
#define ATHR_PHY3_ADDR   0x3
#define ATHR_PHY4_ADDR   0x4

#define MODULE_NAME "ATHR8236"

/*
 * Track per-PHY port information.
 */
typedef struct
{
	BOOL   isEnetPort;       /* normal enet port */
	BOOL   isPhyAlive;       /* last known state of link */
	int    ethUnit;          /* MAC associated with this phy port */
	uint32_t phyBase;
	uint32_t phyAddr;          /* PHY registers associated with this phy port */
	uint32_t VLANTableSetting; /* Value to be written to VLAN table */
	unsigned int     id1;
	unsigned int     id2;
} athrPhyInfo_t;

/*
 * Per-PHY information, indexed by PHY unit number.
 */
static athrPhyInfo_t athrPhyInfo[] =
{

	{
		TRUE,   /* port 1 -- LAN port 1 */
		FALSE,
		ENET_UNIT_LAN,
		0,
		ATHR_PHY0_ADDR,
		ATHR_LAN_PORT_VLAN,
		ATHR_PHY_ID1_EXPECTED,
		ATHR_AR8236_PHY_ID2_EXPECTED
	},

	{
		TRUE,   /* port 2 -- LAN port 2 */
		FALSE,
		ENET_UNIT_LAN,
		0,
		ATHR_PHY1_ADDR,
		ATHR_LAN_PORT_VLAN,
		ATHR_PHY_ID1_EXPECTED,
		ATHR_AR8236_PHY_ID2_EXPECTED
	},

	{
		TRUE,   /* port 3 -- LAN port 3 */
		FALSE,
		ENET_UNIT_LAN,
		0,
		ATHR_PHY2_ADDR,
		ATHR_LAN_PORT_VLAN,
		ATHR_PHY_ID1_EXPECTED,
		ATHR_AR8236_PHY_ID2_EXPECTED
	},

	{
		TRUE,   /* port 4 --  LAN port 4 */
		FALSE,
		ENET_UNIT_LAN,
		0,
		ATHR_PHY3_ADDR,
		ATHR_LAN_PORT_VLAN, /* Send to all ports */
		ATHR_PHY_ID1_EXPECTED,
		ATHR_AR8236_PHY_ID2_EXPECTED
	},

	{
		TRUE,  /* port 5 -- WAN Port 5 */
		FALSE,
		ENET_UNIT_WAN,
		0,
		ATHR_PHY4_ADDR,
		ATHR_LAN_PORT_VLAN, /* Send to all ports */
		ATHR_PHY_ID1_EXPECTED,
		ATHR_AR8236_PHY_ID2_EXPECTED
	},

	{
		FALSE,   /* port 0 -- cpu port 0 */
		TRUE,
		ENET_UNIT_LAN,
		0,
		0x00,
		ATHR_LAN_PORT_VLAN,
		ATHR_PHY_ID1_EXPECTED,
		ATHR_AR8236_PHY_ID2_EXPECTED
	},

};

static uint8_t athr8236_init_flag = 0,athr8236_init_flag1 = 0;


#define ATHR_GLOBALREGBASE    0

#define ATHR_PHY_MAX 5
#define ATHR_PORT_MAX 6
#define USING_ONE_CPU_MAC 1

/* Range of valid PHY IDs is [MIN..MAX] */
#define ATHR_ID_MIN 0
#define ATHR_ID_MAX (ATHR_PHY_MAX-1)

/* Convenience macros to access myPhyInfo */
#define ATHR_IS_ENET_PORT(phyUnit) (athrPhyInfo[phyUnit].isEnetPort)
#define ATHR_IS_PHY_ALIVE(phyUnit) (athrPhyInfo[phyUnit].isPhyAlive)
#define ATHR_ETHUNIT(phyUnit) (athrPhyInfo[phyUnit].ethUnit)
#define ATHR_PHYBASE(phyUnit) (athrPhyInfo[phyUnit].phyBase)
#define ATHR_PHYADDR(phyUnit) (athrPhyInfo[phyUnit].phyAddr)
#define ATHR_VLAN_TABLE_SETTING(phyUnit) (athrPhyInfo[phyUnit].VLANTableSetting)

#ifdef USING_ONE_CPU_MAC
#define ATHR_IS_ETHUNIT(phyUnit, ethUnit) \
            (ATHR_IS_ENET_PORT(phyUnit))
#else // #ifdef USING_ONE_CPU_MAC
#define ATHR_IS_ETHUNIT(phyUnit, ethUnit) \
            (ATHR_IS_ENET_PORT(phyUnit) &&        \
            ATHR_ETHUNIT(phyUnit) == (ethUnit))
#endif // #ifdef USING_ONE_CPU_MAC

#define ATHR_IS_WAN_PORT(phyUnit) (!(ATHR_ETHUNIT(phyUnit)==ENET_UNIT_LAN))

/* Forward references */
BOOL athr8236_phy_is_link_alive(int phyUnit);
uint32_t athr8236_reg_read(uint32_t reg_addr);
void athr8236_reg_write(uint32_t reg_addr, uint32_t reg_val);
unsigned int ar8236_rd_phy(unsigned int phy_addr, unsigned int reg_addr);
void ar8236_wr_phy(unsigned int phy_addr, unsigned int reg_addr, unsigned int write_data);


void athr8236_powersave_off(int phy_addr)
{
	ar8236_wr_phy(phy_addr,ATHR_DEBUG_PORT_ADDRESS,0x29);
	ar8236_wr_phy(phy_addr,ATHR_DEBUG_PORT_DATA,0x36c0);

}
void athr8236_sleep_off(int phy_addr)
{
	ar8236_wr_phy(phy_addr,ATHR_DEBUG_PORT_ADDRESS,0xb);
	ar8236_wr_phy(phy_addr,ATHR_DEBUG_PORT_DATA,0x3c00);
}

void athr8236_reg_init(void)
{

#if AR8236_PHY_DEBUG
	uint32_t rd_val;
#endif
	uint32_t ar7240_revid;
	/* if using header for register configuration, we have to     */
	/* configure 8236 register after frame transmission is enabled */

	if (athr8236_init_flag)
		return;

	ar7240_revid = ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK;
	if(ar7240_revid == AR7240_REV_1_0)
	{
#ifdef AR8236_FORCE_100M
		ar8236_wr_phy(ATHR_PHY4_ADDR,ATHR_PHY_FUNC_CONTROL,0x800);
		ar8236_wr_phy(ATHR_PHY4_ADDR,ATHR_PHY_CONTROL,0xa100);
#endif

#ifdef AR8236_FORCE_10M
		athr8236_powersave_off(ATHR_PHY4_ADDR);
		athr8236_sleep_off(ATHR_PHY4_ADDR);
		ar8236_wr_phy(ATHR_PHY4_ADDR,ATHR_PHY_FUNC_CONTROL,0x800);
		ar8236_wr_phy(ATHR_PHY4_ADDR,ATHR_PHY_CONTROL,0x8100);
		ar8236_wr_phy(ATHR_PHY4_ADDR,ATHR_DEBUG_PORT_ADDRESS,0x0);
		ar8236_wr_phy(ATHR_PHY4_ADDR,ATHR_DEBUG_PORT_DATA,0x12ee);
		ar8236_wr_phy(ATHR_PHY4_ADDR,ATHR_DEBUG_PORT_ADDRESS,0x3);
		ar8236_wr_phy(ATHR_PHY4_ADDR,ATHR_DEBUG_PORT_DATA,0x3bf0);
		ar8236_wr_phy(ATHR_PHY4_ADDR,ATHR_PHY_CONTROL,0x8100);
#endif
	}

#if AR8236_PHY_DEBUG
	rd_val = ar8236_rd_phy(ATHR_PHY4_ADDR,ATHR_PHY_FUNC_CONTROL);
	printf("AR8236 PHY FUNC CTRL  (%d) :%x\n",ATHR_PHY4_ADDR, rd_val);
	rd_val = ar8236_rd_phy(ATHR_PHY4_ADDR,ATHR_PHY_CONTROL);
	printf("AR8236 PHY CTRL  (%d) :%x\n",ATHR_PHY4_ADDR, rd_val);
#endif


	/*
	   If using two MACs on the CPU, WAN and LAN init should be called directly based on which MAC is being brought up.
	    For Wasp 9342 where there is one MAC connecting to both WAN and LAN on AR8236 over one MII connection, we will
	    initialize all the ports.
	*/
	athr8236_reg_init_wan();
#ifdef USING_ONE_CPU_MAC
	athr8236_reg_init_lan();
#endif
}

void athr8236_reg_init_wan(void)
{
	if (athr8236_init_flag)
		return;

	athr8236_reg_write(PORT_STATUS_REGISTER5, 0x200);  /* WAN - 5 */
	athr8236_reg_init_lan();
	athr8236_init_flag = 1;
}

void athr8236_reg_init_lan(void)
{
	int i = 60;
#if AR8236_PHY_DEBUG
	uint32_t rd_val;
#endif
	int       phyUnit;
	uint32_t  phyBase = 0;
	BOOL      foundPhy = FALSE;
	uint32_t  phyAddr = 0;
	uint32_t ar7240_revid;

	/* if using header for register configuration, we have to     */
	/* configure 8236 register after frame transmission is enabled */
	if (athr8236_init_flag1)
		return;

	/* reset switch */
#if AR8236_PHY_DEBUG
	printf(MODULE_NAME ": resetting 8236\n");
#endif
	athr8236_reg_write(0x0, athr8236_reg_read(0x0)|0x80000000);

	while(i--)
	{
		if(!is_ar933x())
			sysMsDelay(100);
		if(!(athr8236_reg_read(0x0)&0x80000000))
			break;
	}
#if AR8236_PHY_DEBUG
	printf(MODULE_NAME ": 8236 reset done\n");
#endif

	for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++)
	{

		foundPhy = TRUE;
		phyBase = ATHR_PHYBASE(phyUnit);
		phyAddr = ATHR_PHYADDR(phyUnit);

		ar7240_revid = ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK;
		if(ar7240_revid == AR7240_REV_1_0)
		{
#ifdef AR8236_FORCE_100M
			/*
			 *  Force MDI and MDX to alternate ports
			 *  Phy 0 and 2 -- MDI
			 *  Phy 1 and 3 -- MDX
			 */
			if(phyUnit%2)
				ar8236_wr_phy(phyAddr,ATHR_PHY_FUNC_CONTROL,0x820);
			else
				ar8236_wr_phy(phyAddr,ATHR_PHY_FUNC_CONTROL,0x800);

			ar8236_wr_phy(phyAddr,ATHR_PHY_CONTROL,0xa100);
#endif

#ifdef AR8236_FORCE_10M
			/*
			 *  Force MDI and MDX to alternate ports
			 *  Phy 0 and 2 -- MDI
			 *  Phy 1 and 3 -- MDX
			 */
			if(phyUnit%2)
				ar8236_wr_phy(phyAddr,ATHR_PHY_FUNC_CONTROL,0x820);
			else
				ar8236_wr_phy(phyAddr,ATHR_PHY_FUNC_CONTROL,0x800);

			athr8236_powersave_off(phyAddr);
			athr8236_sleep_off(phyAddr);

			ar8236_wr_phy(phyAddr,ATHR_PHY_CONTROL,0x8100);
			ar8236_wr_phy(phyAddr,ATHR_DEBUG_PORT_ADDRESS,0x0);
			ar8236_wr_phy(phyAddr,ATHR_DEBUG_PORT_DATA,0x12ee);
			ar8236_wr_phy(phyAddr,ATHR_DEBUG_PORT_ADDRESS,0x3);
			ar8236_wr_phy(phyAddr,ATHR_DEBUG_PORT_DATA,0x3bf0);
			ar8236_wr_phy(phyAddr,ATHR_PHY_CONTROL,0x8100);
#endif
		}

#if AR8236_PHY_DEBUG
		rd_val = ar8236_rd_phy(phyAddr,ATHR_PHY_ID1);
		printf("AR8236 PHY ID  (%d) :%x\n",phyAddr,rd_val);
		rd_val = ar8236_rd_phy(phyAddr,ATHR_PHY_CONTROL);
		printf("AR8236 PHY CTRL  (%d) :%x\n",phyAddr,rd_val);
		rd_val = ar8236_rd_phy(phyAddr,ATHR_PHY_STATUS);
		printf("AR8236 ATHR PHY STATUS  (%d) :%x\n",phyAddr,rd_val);
#endif
	}
	/*
	 * CPU port Enable
	 */
	athr8236_reg_write(CPU_PORT_REGISTER,(1 << 8));

	/*
	 * status[1:0]=2'h2;   - (0x10 - 1000 Mbps , 0x0 - 10 Mbps)
	 * status[2]=1'h1;     - Tx Mac En
	 * status[3]=1'h1;     - Rx Mac En
	 * status[4]=1'h1;     - Tx Flow Ctrl En
	 * status[5]=1'h1;     - Rx Flow Ctrl En
	 * status[6]=1'h1;     - Duplex Mode
	 */
	athr8236_reg_write(PORT_STATUS_REGISTER0, 0x7D); // MT: 100FDX, TX+RX EN, TX+RX FLOW EN - no autoneg -- NB: Was 4D, changed to 7D based on FAQ
	athr8236_reg_write(PORT_STATUS_REGISTER1, 0x200);  /* LAN - 1 */
	athr8236_reg_write(PORT_STATUS_REGISTER2, 0x200);  /* LAN - 2 */
	athr8236_reg_write(PORT_STATUS_REGISTER3, 0x200);  /* LAN - 3 */
	athr8236_reg_write(PORT_STATUS_REGISTER4, 0x200);  /* LAN - 4 */
	athr8236_reg_write(PORT_STATUS_REGISTER5, 0x200);  /* WAN - 5 */

	/*enable multicats packets*/
	athr8236_reg_write(FLOOD_MASK_REG, 0x7e3f003f);

	athr8236_reg_write(FLOW_CTRL_REGISTER1, 0xc000050e);

	/*
	 * status[11]=1'h0;    - CPU Disable
	 * status[7] = 1'b1;   - Learn One Lock
	 * status[14] = 1'b0;  - Learn Enable
	 */
#ifdef CONFIG_AR7240_EMU
	athr8236_reg_write(PORT_CONTROL_REGISTER0, 0x04);
	athr8236_reg_write(PORT_CONTROL_REGISTER1, 0x4004);
#else
	/* Atheros Header Disable */
	athr8236_reg_write(PORT_CONTROL_REGISTER0, 0x4004);
#endif

	/* Tag Priority Mapping */
	athr8236_reg_write(0x70, 0xfa50);

	/* Enable ARP packets to CPU port */
	athr8236_reg_write(ARL_TBL_CTRL_REG,(athr8236_reg_read(ARL_TBL_CTRL_REG) | 0x100000));
	/* Enable MII PHY mode to Wasp CPU*/
	athr8236_reg_write(PORT0_PAD_MODE_CTRL, 0x00000400); // PHY MODE + MII ENABLE
	/* Disable all special functions for port5, it's just WAN for us. */
	athr8236_reg_write(PORT5_PAD_MODE_CTRL, 0x00000000); // All off

#if AR8236_PHY_DEBUG
	rd_val = athr8236_reg_read ( CPU_PORT_REGISTER );
	printf("AR8236 CPU_PORT_REGISTER :%x\n",rd_val);
	rd_val = athr8236_reg_read ( PORT_STATUS_REGISTER0 );
	printf("AR8236 PORT_STATUS_REGISTER0  :%x\n",rd_val);
	rd_val = athr8236_reg_read ( PORT_STATUS_REGISTER1 );
	printf("AR8236 PORT_STATUS_REGISTER1  :%x\n",rd_val);
	rd_val = athr8236_reg_read ( PORT_STATUS_REGISTER2 );
	printf("AR8236 PORT_STATUS_REGISTER2  :%x\n",rd_val);
	rd_val = athr8236_reg_read ( PORT_STATUS_REGISTER3 );
	printf("AR8236 PORT_STATUS_REGISTER3  :%x\n",rd_val);
	rd_val = athr8236_reg_read ( PORT_STATUS_REGISTER4 );
	printf("AR8236 PORT_STATUS_REGISTER4  :%x\n",rd_val);

	rd_val = athr8236_reg_read ( PORT_CONTROL_REGISTER0 );
	printf("AR8236 PORT_CONTROL_REGISTER0 :%x\n",rd_val);
	rd_val = athr8236_reg_read ( PORT_CONTROL_REGISTER1 );
	printf("AR8236 PORT_CONTROL_REGISTER1 :%x\n",rd_val);
	rd_val = athr8236_reg_read ( PORT_CONTROL_REGISTER2 );
	printf("AR8236 PORT_CONTROL_REGISTER2 :%x\n",rd_val);
	rd_val = athr8236_reg_read ( PORT_CONTROL_REGISTER3 );
	printf("AR8236 PORT_CONTROL_REGISTER3 :%x\n",rd_val);
	rd_val = athr8236_reg_read ( PORT_CONTROL_REGISTER4 );
	printf("AR8236 PORT_CONTROL_REGISTER4 :%x\n",rd_val);
#endif

	athr8236_init_flag1 = 1;
}
static unsigned int phy_val_saved = 0;
/******************************************************************************
*
* athr8236_phy_is_link_alive - test to see if the specified link is alive
*
* RETURNS:
*    TRUE  --> link is alive
*    FALSE --> link is down
*/
BOOL
athr8236_phy_is_link_alive(int phyUnit)
{
	uint16_t phyHwStatus;
	uint32_t phyBase;
	uint32_t phyAddr;
	phyBase = ATHR_PHYBASE(phyUnit);
	phyAddr = ATHR_PHYADDR(phyUnit);

	phyHwStatus = ar8236_rd_phy( phyAddr, ATHR_PHY_SPEC_STATUS);

	if (phyHwStatus & ATHR_STATUS_LINK_PASS)
		return TRUE;

	return FALSE;
}

/******************************************************************************
*
* athr8236_phy_setup - reset and setup the PHY associated with
* the specified MAC unit number.
*
* Resets the associated PHY port.
*
* RETURNS:
*    TRUE  --> associated PHY is alive
*    FALSE --> no LINKs on this ethernet unit
*/

BOOL
athr8236_phy_setup(int ethUnit)
{
	int       phyUnit;
	uint16_t  phyHwStatus;
	uint16_t  timeout;
	int       liveLinks = 0;
	uint32_t  phyBase = 0;
	BOOL      foundPhy = FALSE;
	uint32_t  phyAddr = 0;
#if AR8236_PHY_DEBUG
	uint32_t  rd_val = 0;
#endif
	uint32_t  ar7240_revid;


	/* See if there's any configuration data for this enet */
	/* start auto negogiation on each phy */
	for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++)
	{
		if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit))
		{
			continue;
		}

		foundPhy = TRUE;
		phyBase = ATHR_PHYBASE(phyUnit);
		phyAddr = ATHR_PHYADDR(phyUnit);

		ar8236_wr_phy(phyAddr, ATHR_AUTONEG_ADVERT,ATHR_ADVERTISE_ALL);
#if AR8236_PHY_DEBUG
		rd_val = ar8236_rd_phy(phyAddr,ATHR_AUTONEG_ADVERT  );
		printf("%s ATHR_AUTONEG_ADVERT %d :%x\n",__func__,phyAddr, rd_val);
#endif

		ar7240_revid = ar7240_reg_rd(AR7240_REV_ID) & AR7240_REV_ID_MASK;
		if(ar7240_revid != AR7240_REV_1_0)
		{
			ar8236_wr_phy( phyAddr, ATHR_PHY_CONTROL,ATHR_CTRL_AUTONEGOTIATION_ENABLE
						   | ATHR_CTRL_SOFTWARE_RESET);
		}

#if AR8236_PHY_DEBUG
		rd_val = ar8236_rd_phy(phyAddr,ATHR_AUTONEG_ADVERT  );
		rd_val = ar8236_rd_phy(phyAddr,ATHR_PHY_CONTROL);
		printf("%s ATHR_PHY_CONTROL %d :%x\n",__func__,phyAddr, rd_val);
#endif
	}

	if (!foundPhy)
	{
		return FALSE; /* No PHY's configured for this ethUnit */
	}

	/*
	 * After the phy is reset, it takes a little while before
	 * it can respond properly.
	 */
	if(!is_ar933x())
	{
		if (ethUnit == ENET_UNIT_LAN)
			sysMsDelay(1000);
		else
			sysMsDelay(3000);
	}

	/*
	 * Wait up to 3 seconds for ALL associated PHYs to finish
	 * autonegotiation.  The only way we get out of here sooner is
	 * if ALL PHYs are connected AND finish autonegotiation.
	 */
	for (phyUnit=0; (phyUnit < ATHR_PHY_MAX) /*&& (timeout > 0) */; phyUnit++)
	{
		if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit))
		{
			continue;
		}

		timeout=20;
		for (;;)
		{
			phyHwStatus =  ar8236_rd_phy(phyAddr, ATHR_PHY_CONTROL);

			if (ATHR_RESET_DONE(phyHwStatus))
			{
				DRV_PRINT(DRV_DEBUG_PHYSETUP,
						  ("Port %d, Neg Success\n", phyUnit));
				break;
			}
			if (timeout == 0)
			{
				DRV_PRINT(DRV_DEBUG_PHYSETUP,
						  ("Port %d, Negogiation timeout\n", phyUnit));
				break;
			}
			if (--timeout == 0)
			{
				DRV_PRINT(DRV_DEBUG_PHYSETUP,
						  ("Port %d, Negogiation timeout\n", phyUnit));
				break;
			}

			if(!is_ar933x())
				sysMsDelay(150);
		}
	}

	/*
	 * All PHYs have had adequate time to autonegotiate.
	 * Now initialize software status.
	 *
	 * It's possible that some ports may take a bit longer
	 * to autonegotiate; but we can't wait forever.  They'll
	 * get noticed by mv_phyCheckStatusChange during regular
	 * polling activities.
	 */
	for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++)
	{
		if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit))
		{
			continue;
		}

		if (athr8236_phy_is_link_alive(phyUnit))
		{
			liveLinks++;
			ATHR_IS_PHY_ALIVE(phyUnit) = TRUE;
		}
		else
		{
			ATHR_IS_PHY_ALIVE(phyUnit) = FALSE;
		}
		DRV_PRINT(DRV_DEBUG_PHYSETUP,
				  ("eth%d: Phy Specific Status=%4.4x\n",
				   ethUnit,
				   ar8236_rd_phy(ATHR_PHYADDR(phyUnit),ATHR_PHY_SPEC_STATUS)));
	}

	return (liveLinks > 0);
}

/******************************************************************************
*
* athr8236_phy_is_fdx - Determines whether the phy ports associated with the
* specified device are FULL or HALF duplex.
*
* RETURNS:
*    1 --> FULL
*    0 --> HALF
*/
int
athr8236_phy_is_fdx(int ethUnit)
{
	int       phyUnit;
	uint32_t  phyBase;
	uint32_t  phyAddr;
	uint16_t  phyHwStatus;
	int       ii = 200;

	if (ethUnit == ENET_UNIT_LAN)
		return TRUE;

	for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++)
	{
		if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit))
		{
			continue;
		}

		if (athr8236_phy_is_link_alive(phyUnit))
		{

			phyBase = ATHR_PHYBASE(phyUnit);
			phyAddr = ATHR_PHYADDR(phyUnit);

			do
			{
				phyHwStatus = ar8236_rd_phy(ATHR_PHYADDR(phyUnit),ATHR_PHY_SPEC_STATUS);
				sysMsDelay(10);
			}
			while((!(phyHwStatus & ATHR_STATUS_RESOVLED)) && --ii);

			if (phyHwStatus & ATHER_STATUS_FULL_DEPLEX)
				return TRUE;
		}
	}

	return FALSE;
}


/******************************************************************************
*
* athr8236_phy_speed - Determines the speed of phy ports associated with the
* specified device.
*
* RETURNS:
*               _10BASET, _100BASET;
*               _1000BASET;
*/

int
athr8236_phy_speed(int ethUnit)
{
	int       phyUnit;
	uint16_t  phyHwStatus;
	uint32_t  phyBase;
	uint32_t  phyAddr;
	int       ii = 200;

	if (ethUnit == ENET_UNIT_LAN)
		return _1000BASET;

	for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++)
	{
		if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit))
		{
			continue;
		}

		if (athr8236_phy_is_link_alive(phyUnit))
		{

			phyBase = ATHR_PHYBASE(phyUnit);
			phyAddr = ATHR_PHYADDR(phyUnit);
			do
			{
				phyHwStatus = ar8236_rd_phy(ATHR_PHYADDR(phyUnit),ATHR_PHY_SPEC_STATUS);
				sysMsDelay(10);
			}
			while((!(phyHwStatus & ATHR_STATUS_RESOVLED)) && --ii);

			phyHwStatus = ((phyHwStatus & ATHER_STATUS_LINK_MASK) >>
						   ATHER_STATUS_LINK_SHIFT);

			switch(phyHwStatus)
			{
			case 0:
				return _10BASET;
			case 1:
#ifdef CONFIG_MACH_HORNET
				/* For IEEE 100M voltage test */
				ar8236_wr_phy(phyAddr, ATHR_DEBUG_PORT_ADDRESS, 0x4);
				ar8236_wr_phy(phyAddr, ATHR_DEBUG_PORT_DATA, 0xebbb);
				ar8236_wr_phy(phyAddr, ATHR_DEBUG_PORT_ADDRESS, 0x5);
				ar8236_wr_phy(phyAddr, ATHR_DEBUG_PORT_DATA, 0x2c47);
#endif /* CONFIG_MACH_HORNET */
				return _100BASET;
			case 2:
				return _1000BASET;
			default:
				printf("Unkown speed read!\n");
			}
		}
	}

	return _10BASET;
}

/*****************************************************************************
*
* athr_phy_is_up -- checks for significant changes in PHY state.
*
* A "significant change" is:
*     dropped link (e.g. ethernet cable unplugged) OR
*     autonegotiation completed + link (e.g. ethernet cable plugged in)
*
* When a PHY is plugged in, phyLinkGained is called.
* When a PHY is unplugged, phyLinkLost is called.
*/

int
athr8236_phy_is_up(int ethUnit)
{
	int           phyUnit;
	uint16_t      phyHwStatus, phyHwControl;
	athrPhyInfo_t *lastStatus;
	int           linkCount   = 0;
	int           lostLinks   = 0;
	int           gainedLinks = 0;
	uint32_t      phyBase;
	uint32_t      phyAddr;

	for (phyUnit=0; phyUnit < ATHR_PHY_MAX; phyUnit++)
	{
		if (!ATHR_IS_ETHUNIT(phyUnit, ethUnit))
		{
			continue;
		}

		phyBase = ATHR_PHYBASE(phyUnit);
		phyAddr = ATHR_PHYADDR(phyUnit);

		lastStatus = &athrPhyInfo[phyUnit];

		if (lastStatus->isPhyAlive)   /* last known link status was ALIVE */
		{

			phyHwStatus = ar8236_rd_phy(ATHR_PHYADDR(phyUnit),ATHR_PHY_SPEC_STATUS);

			/* See if we've lost link */
			if (phyHwStatus & ATHR_STATUS_LINK_PASS)   /* check realtime link */
			{
				linkCount++;
			}
			else
			{
				phyHwStatus = ar8236_rd_phy(ATHR_PHYADDR(phyUnit),ATHR_PHY_STATUS);
				/* If realtime failed check link in latch register before
				 * asserting link down.
				 */
				if (phyHwStatus & ATHR_LATCH_LINK_PASS)
					linkCount++;
				else
				{
					lostLinks++;
				}
				DRV_PRINT(DRV_DEBUG_PHYCHANGE,("\nenet%d port%d down\n",
											   ethUnit, phyUnit));
				lastStatus->isPhyAlive = FALSE;
			}
		}
		else     /* last known link status was DEAD */
		{

			/* Check for reset complete */

			phyHwStatus = ar8236_rd_phy(ATHR_PHYADDR(phyUnit),ATHR_PHY_STATUS);

			if (!ATHR_RESET_DONE(phyHwStatus))
				continue;

			phyHwControl = ar8236_rd_phy(ATHR_PHYADDR(phyUnit),ATHR_PHY_CONTROL);

			/* Check for AutoNegotiation complete */

			if ((!(phyHwControl & ATHR_CTRL_AUTONEGOTIATION_ENABLE))
					|| ATHR_AUTONEG_DONE(phyHwStatus))
			{
				phyHwStatus = ar8236_rd_phy(ATHR_PHYADDR(phyUnit),ATHR_PHY_SPEC_STATUS);

				if (phyHwStatus & ATHR_STATUS_LINK_PASS)
				{
					gainedLinks++;
					linkCount++;
					DRV_PRINT(DRV_DEBUG_PHYCHANGE,("\nenet%d port%d up\n",
												   ethUnit, phyUnit));
					lastStatus->isPhyAlive = TRUE;
				}
			}
		}
	}

	return (linkCount);

#if AR8236_PHY_DEBUG
	if (linkCount == 0)
	{
		if (lostLinks)
		{
			/* We just lost the last link for this MAC */
			phyLinkLost(ethUnit);
		}
	}
	else
	{
		if (gainedLinks == linkCount)
		{
			/* We just gained our first link(s) for this MAC */
			phyLinkGained(ethUnit);
		}
	}
#endif
}

uint32_t
athr8236_reg_read(unsigned int addr)
{
	unsigned int addr_temp;
	unsigned int rd_csr_low, rd_csr_high, rd_csr;
	unsigned int data, unit = 0;
	unsigned int phy_address, reg_address;

	addr_temp = (addr & 0xfffffffc) >>2;
	data = addr_temp >> 7;

	phy_address = 0x1f;
	reg_address = 0x10;

	if (is_ar7240())
	{
		unit = 0;
	}
	else if (is_ar7241() || is_ar7242() || is_ar933x())
	{
		unit = 1;
	}

	phy_reg_write(unit,phy_address, reg_address, data);

	phy_address = (0x17 & ((addr_temp >> 4) | 0x10));
	reg_address = ((addr_temp << 1) & 0x1e);
	rd_csr_low = (uint32_t) phy_reg_read(unit, phy_address, reg_address);

	reg_address = reg_address | 0x1;
	rd_csr_high = (uint32_t) phy_reg_read(unit, phy_address, reg_address);
	rd_csr = (rd_csr_high << 16) | rd_csr_low ;

	return(rd_csr);
}

void
athr8236_reg_write(unsigned int addr, unsigned int write_data)
{
	unsigned int addr_temp;
	unsigned int data, unit = 0;
	unsigned int phy_address, reg_address;


	addr_temp = (addr &  0xfffffffc) >>2;
	data = addr_temp >> 7;

	phy_address = 0x1f;
	reg_address = 0x10;

	if (is_ar7240())
	{
		unit = 0;
	}
	else if (is_ar7241() || is_ar7242()|| is_ar933x())
	{
		unit = 1;
	}

#ifdef CONFIG_MACH_HORNET
	//The write sequence , 0x98: L->H, 0x40 H->L, 0x50 H->L , others should not care.
	if(addr!=0x98)
	{
		//printf("[%s:%d] unit=%d\n",__FUNCTION__,__LINE__,unit);
		phy_reg_write(unit, phy_address, reg_address, data);

		phy_address = 0x17 & ((addr_temp >> 4) | 0x10);
		reg_address = ((addr_temp << 1) & 0x1e) | 0x1;
		data =  write_data >> 16;
		phy_reg_write(unit, phy_address, reg_address, data);

		reg_address = reg_address & 0x1e;
		data = write_data  & 0xffff;
		phy_reg_write(unit, phy_address, reg_address, data);
	}
	else
	{
		phy_reg_write(unit, phy_address, reg_address, data);

		phy_address = (0x17 & ((addr_temp >> 4) | 0x10));
		reg_address = ((addr_temp << 1) & 0x1e);

		data = write_data  & 0xffff;
		phy_reg_write(unit, phy_address, reg_address, data);

		reg_address = (((addr_temp << 1) & 0x1e) | 0x1);
		data = write_data >> 16;
		phy_reg_write(unit, phy_address, reg_address, data);

	}
#else
	phy_reg_write(unit, phy_address, reg_address, data);

	phy_address = (0x17 & ((addr_temp >> 4) | 0x10));
	reg_address = ((addr_temp << 1) & 0x1e);
	data = write_data  & 0xffff;
	phy_reg_write(unit, phy_address, reg_address, data);

	reg_address = (((addr_temp << 1) & 0x1e) | 0x1);
	data = write_data >> 16;
	phy_reg_write(unit, phy_address, reg_address, data);
#endif
}

unsigned int ar8236_rd_phy(unsigned int phy_addr, unsigned int reg_addr)
{
	unsigned int rddata;
	int retries = 200;

	// MDIO_CMD is set for read

	rddata = athr8236_reg_read(MDIO_CONTROL);
	rddata = (rddata & 0x0)
			 | (reg_addr<<MDIO_CONTROL__REGADDR_S)
			 | (phy_addr<<MDIO_CONTROL__PHYADDR_S)
			 | MDIO_CONTROL__MDIO_CMD_WRITE
			 | MDIO_CONTROL__MDIO_MASTER_EN
			 | MDIO_CONTROL__MDIO_BUSY;
	athr8236_reg_write(MDIO_CONTROL, rddata);

	rddata = athr8236_reg_read(MDIO_CONTROL); // READ MDIO_CONTROL
	rddata = rddata & MDIO_CONTROL__MDIO_BUSY;

	// Check MDIO_BUSY status
	while(rddata && (--retries))
	{
		rddata = athr8236_reg_read(MDIO_CONTROL);
		rddata = rddata & MDIO_CONTROL__MDIO_BUSY; //MDIO_BUSY
		udelay(10);
	}
	if (!retries)
	{
		printf("%s: ERROR: MDIO_BUSY status never cleared.\n",__func__);
	}

	// Read the data from phy
	rddata = athr8236_reg_read(MDIO_CONTROL) & 0xffff;
	//    printf("%s(0x%02x,0x%02x) => 0x%04x\n",__func__,phy_addr,reg_addr,rddata);
	athr8236_reg_write(MDIO_CONTROL,0); // Disable MDIO_MASTER_EN
	return(rddata);

}

void ar8236_wr_phy(unsigned int phy_addr, unsigned int reg_addr, unsigned int write_data)
{
	int retries = 200;
	unsigned int rddata;
	//    printf("%s(0x%02x,0x%02x,0x%04x)\n",__func__,phy_addr,reg_addr,write_data);
	// MDIO_CMD is set for read
	rddata = athr8236_reg_read(MDIO_CONTROL);
	rddata = (rddata & 0x0)
			 | (write_data & MDIO_CONTROL__DATA_M)
			 | (reg_addr<<MDIO_CONTROL__REGADDR_S)
			 | (phy_addr<<MDIO_CONTROL__PHYADDR_S)
			 | MDIO_CONTROL__MDIO_CMD_READ
			 | MDIO_CONTROL__MDIO_MASTER_EN
			 | MDIO_CONTROL__MDIO_BUSY ;
	athr8236_reg_write(MDIO_CONTROL, rddata);

	rddata = athr8236_reg_read(MDIO_CONTROL);
	rddata = rddata & MDIO_CONTROL__MDIO_BUSY;

	// Check MDIO_BUSY status
	while(rddata && (--retries))
	{
		rddata = athr8236_reg_read(MDIO_CONTROL);
		rddata = rddata & MDIO_CONTROL__MDIO_BUSY;
		udelay(10);
	}

	if (!retries)
	{
		printf("%s: ERROR: MDIO_BUSY status never cleared.\n",__func__);
	}
	athr8236_reg_write(MDIO_CONTROL,0); // Disable MDIO_MASTER_EN

}

int
athr8236_mdc_check()
{
	int i;

	for (i=0; i<4000; i++)
	{
		if(athr8236_reg_read(0x10c) != 0x18007fff)
			return -1;
	}
	return 0;
}

static athrPhyInfo_t *
athr8236_phy_find(int unit)
{
	unsigned int i;
	athrPhyInfo_t *phy;
	u32 id1, id2;

	for(i = 0; i < sizeof(athrPhyInfo)/sizeof(athrPhyInfo_t); i++)
	{
		phy = &athrPhyInfo[i];
		if (phy->isEnetPort && (phy->ethUnit == unit))
		{
			id1 = phy_reg_read(unit, phy->phyAddr, ATHR_PHY_ID1);
			id2 = phy_reg_read(unit, phy->phyAddr, ATHR_PHY_ID2);
			//printf("ethUnit %d phyAddr Addr %d - ID1=0x%08x ID2=0x%08x\n",phy->ethUnit, phy->phyAddr,id1,id2);
			if ((id1 == phy->id1) && (id2 == phy->id2))
			{
				/* UBNT: this hack is needed for 2% of the problematic boards in production*/
				ag7240_reset_ge();
				return phy;
			}
		}
	}
	return NULL;
}

int athr8236_phy_test(void)
{
	return !!athr8236_phy_find(0);
}

inline void
athr8236_configure(void)
{
	ar7240_reg_wr(AG7240_ETH_CFG, AG7240_ETH_CFG_MII_GE0 | AG7240_ETH_CFG_MII_GE0_SLAVE);
	ar7240_reg_wr(0x19000004, (ar7240_reg_rd(0x1900004) & ~AG7240_MAC_CFG2_IF_1000) | (AG7240_MAC_CFG2_PAD_CRC_EN | AG7240_MAC_CFG2_LEN_CHECK | AG7240_MAC_CFG2_IF_10_100));

}

BOOL
athr8236_phy_probe(int unit)
{
	static int found = 0;
	// If already detected, skip search.  If not yet found, always check again
	if(!found)
	{
		athr8236_configure();
		found = !!athr8236_phy_find(unit);
		if(found)
		{
			printf("AR8236\n");
		}
	}
	return found;
}

