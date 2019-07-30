/*
 * linux/drivers/mtd/nand/ath_nand.c
 * vim: tabstop=8 : noexpandtab
 * Derived from alauda.c
 */
#include <common.h>
#include <command.h>
#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/types.h>
#include <ar7240_soc.h>
#include <malloc.h>

#include <linux/types.h>
#include <linux/string.h>
#include <linux/bitops.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>

#define ENOMEM		12
#define EINVAL		22

#define writesize		oobblock
#define ath_reg_rd		ar7240_reg_rd
#define ath_reg_wr		ar7240_reg_wr
#define ath_reg_rmw_set		ar7240_reg_rmw_set
#define ath_reg_rmw_clear	ar7240_reg_rmw_clear

#define DRV_NAME	"ath-nand"
#define DRV_VERSION	"0.1"
#define DRV_AUTHOR	"Atheros"
#define DRV_DESC	"Atheros on-chip NAND FLash Controller Driver"

#define ATH_NF_COMMAND		(ATH_NAND_FLASH_BASE + 0x200u)
#define ATH_NF_CTRL		(ATH_NAND_FLASH_BASE + 0x204u)
#define ATH_NF_STATUS		(ATH_NAND_FLASH_BASE + 0x208u)
#define ATH_NF_INT_MASK		(ATH_NAND_FLASH_BASE + 0x20cu)
#define ATH_NF_INT_STATUS	(ATH_NAND_FLASH_BASE + 0x210u)
#define ATH_NF_ECC_CTRL		(ATH_NAND_FLASH_BASE + 0x214u)
#define ATH_NF_ECC_OFFSET	(ATH_NAND_FLASH_BASE + 0x218u)
#define ATH_NF_ADDR0_0		(ATH_NAND_FLASH_BASE + 0x21cu)
#define ATH_NF_ADDR1_0		(ATH_NAND_FLASH_BASE + 0x220u)
#define ATH_NF_ADDR0_1		(ATH_NAND_FLASH_BASE + 0x224u)
#define ATH_NF_ADDR1_1		(ATH_NAND_FLASH_BASE + 0x228u)
#define ATH_NF_SPARE_SIZE	(ATH_NAND_FLASH_BASE + 0x230u)
#define ATH_NF_PROTECT		(ATH_NAND_FLASH_BASE + 0x238u)
#define ATH_NF_LOOKUP_EN	(ATH_NAND_FLASH_BASE + 0x240u)
#define ATH_NF_LOOKUP0		(ATH_NAND_FLASH_BASE + 0x244u)
#define ATH_NF_LOOKUP1		(ATH_NAND_FLASH_BASE + 0x248u)
#define ATH_NF_LOOKUP2		(ATH_NAND_FLASH_BASE + 0x24cu)
#define ATH_NF_LOOKUP3		(ATH_NAND_FLASH_BASE + 0x250u)
#define ATH_NF_LOOKUP4		(ATH_NAND_FLASH_BASE + 0x254u)
#define ATH_NF_LOOKUP5		(ATH_NAND_FLASH_BASE + 0x258u)
#define ATH_NF_LOOKUP6		(ATH_NAND_FLASH_BASE + 0x25cu)
#define ATH_NF_LOOKUP7		(ATH_NAND_FLASH_BASE + 0x260u)
#define ATH_NF_DMA_ADDR		(ATH_NAND_FLASH_BASE + 0x264u)
#define ATH_NF_DMA_COUNT	(ATH_NAND_FLASH_BASE + 0x268u)
#define ATH_NF_DMA_CTRL		(ATH_NAND_FLASH_BASE + 0x26cu)
#define ATH_NF_MEM_CTRL		(ATH_NAND_FLASH_BASE + 0x280u)
#define ATH_NF_PG_SIZE		(ATH_NAND_FLASH_BASE + 0x284u)
#define ATH_NF_RD_STATUS	(ATH_NAND_FLASH_BASE + 0x288u)
#define ATH_NF_TIME_SEQ		(ATH_NAND_FLASH_BASE + 0x28cu)
#define ATH_NF_TIMINGS_ASYN	(ATH_NAND_FLASH_BASE + 0x290u)
#define ATH_NF_TIMINGS_SYN	(ATH_NAND_FLASH_BASE + 0x294u)
#define ATH_NF_FIFO_DATA	(ATH_NAND_FLASH_BASE + 0x298u)
#define ATH_NF_TIME_MODE	(ATH_NAND_FLASH_BASE + 0x29cu)
#define ATH_NF_DMA_ADDR_OFFSET	(ATH_NAND_FLASH_BASE + 0x2a0u)
#define ATH_NF_FIFO_INIT	(ATH_NAND_FLASH_BASE + 0x2b0u)
#define ATH_NF_GENERIC_SEQ_CTRL	(ATH_NAND_FLASH_BASE + 0x2b4u)

#define ATH_NF_TIMING_ASYN	0x11
#define ATH_NF_STATUS_OK	0xc0
#define ATH_NF_RD_STATUS_MASK	0xc7

#define ATH_NF_CTRL_SMALL_BLOCK_EN	(1 << 21)

#define ATH_NF_CTRL_ADDR_CYCLE1_0	(0 << 18)
#define ATH_NF_CTRL_ADDR_CYCLE1_1	(1 << 18)
#define ATH_NF_CTRL_ADDR_CYCLE1_2	(2 << 18)
#define ATH_NF_CTRL_ADDR_CYCLE1_3	(3 << 18)
#define ATH_NF_CTRL_ADDR_CYCLE1_4	(4 << 18)
#define ATH_NF_CTRL_ADDR_CYCLE1_5	(5 << 18)

#define ATH_NF_CTRL_ADDR1_AUTO_INC_EN	(1 << 17)
#define ATH_NF_CTRL_ADDR0_AUTO_INC_EN	(1 << 16)
#define ATH_NF_CTRL_WORK_MODE_SYNC	(1 << 15)
#define ATH_NF_CTRL_PROT_EN		(1 << 14)
#define ATH_NF_CTRL_LOOKUP_EN		(1 << 13)
#define ATH_NF_CTRL_IO_WIDTH_16BIT	(1 << 12)
#define ATH_NF_CTRL_CUSTOM_SIZE_EN	(1 << 11)

#define ATH_NF_CTRL_PAGE_SIZE_256	(0 <<  8)	/* bytes */
#define ATH_NF_CTRL_PAGE_SIZE_512	(1 <<  8)
#define ATH_NF_CTRL_PAGE_SIZE_1024	(2 <<  8)
#define ATH_NF_CTRL_PAGE_SIZE_2048	(3 <<  8)
#define ATH_NF_CTRL_PAGE_SIZE_4096	(4 <<  8)
#define ATH_NF_CTRL_PAGE_SIZE_8192	(5 <<  8)
#define ATH_NF_CTRL_PAGE_SIZE_16384	(6 <<  8)
#define ATH_NF_CTRL_PAGE_SIZE_0		(7 <<  8)

#define ATH_NF_CTRL_BLOCK_SIZE_32	(0 <<  6)	/* pages */
#define ATH_NF_CTRL_BLOCK_SIZE_64	(1 <<  6)
#define ATH_NF_CTRL_BLOCK_SIZE_128	(2 <<  6)
#define ATH_NF_CTRL_BLOCK_SIZE_256	(3 <<  6)

#define ATH_NF_CTRL_ECC_EN		(1 <<  5)
#define ATH_NF_CTRL_INT_EN		(1 <<  4)
#define ATH_NF_CTRL_SPARE_EN		(1 <<  3)

#define ATH_NF_CTRL_ADDR_CYCLE0_0	(0 <<  0)
#define ATH_NF_CTRL_ADDR_CYCLE0_1	(1 <<  0)
#define ATH_NF_CTRL_ADDR_CYCLE0_2	(2 <<  0)
#define ATH_NF_CTRL_ADDR_CYCLE0_3	(3 <<  0)
#define ATH_NF_CTRL_ADDR_CYCLE0_4	(4 <<  0)
#define ATH_NF_CTRL_ADDR_CYCLE0_5	(5 <<  0)


#define ATH_NF_DMA_CTRL_DMA_START	(1 << 7)
#define ATH_NF_DMA_CTRL_DMA_DIR_WRITE	(0 << 6)
#define ATH_NF_DMA_CTRL_DMA_DIR_READ	(1 << 6)
#define ATH_NF_DMA_CTRL_DMA_MODE_SG	(1 << 5)
/*
 * 000 ­ incrementing precise burst of precisely four transfers
 * 001 ­ stream burst (address const)
 * 010 ­ single transfer (address increment)
 * 011 ­ burst of unspecified length (address increment)
 * 100 ­ incrementing precise burst of precisely eight transfers
 * 101 ­ incrementing precise burst of precisely sixteen transfers
 */
#define ATH_NF_DMA_CTRL_DMA_BURST_0	(0 << 2)
#define ATH_NF_DMA_CTRL_DMA_BURST_1	(1 << 2)
#define ATH_NF_DMA_CTRL_DMA_BURST_2	(2 << 2)
#define ATH_NF_DMA_CTRL_DMA_BURST_3	(3 << 2)
#define ATH_NF_DMA_CTRL_DMA_BURST_4	(4 << 2)
#define ATH_NF_DMA_CTRL_DMA_BURST_5	(5 << 2)
#define ATH_NF_DMA_CTRL_ERR_FLAG	(1 << 1)
#define ATH_NF_DMA_CTRL_DMA_READY	(1 << 0)

#define ATH_NF_ECC_CTRL_ERR_THRESH(x)	((x << 8) & (0x1fu << 8))
#define ATH_NF_ECC_CTRL_ECC_CAP(x)	((x << 5) & (0x07u << 5))
#define ATH_NF_ECC_CTRL_ERR_OVER	(1 << 2)
#define ATH_NF_ECC_CTRL_ERR_UNCORR	(1 << 1)
#define ATH_NF_ECC_CTRL_ERR_CORR	(1 << 0)

#define ATH_NF_CMD_END_INT		(1 << 1)

#define ATH_NF_HW_ECC		1
#define ATH_NF_STATUS_RETRY	1000

#define ath_nand_get_cmd_end_status(void)	\
	(ath_reg_rd(ATH_NF_INT_STATUS) & ATH_NF_CMD_END_INT)

#define ath_nand_clear_int_status()	ath_reg_wr(ATH_NF_INT_STATUS, 0)

static int ath_nand_hw_init(void *);

#define ATH_NAND_IO_DBG		0
#define ATH_NAND_OOB_DBG	0
#define ATH_NAND_IN_DBG		0

#if ATH_NAND_IO_DBG
#	define iodbg	printk
#else
#	define iodbg(...)
#endif

#if ATH_NAND_OOB_DBG
#	define oobdbg	printk
#else
#	define oobdbg(...)
#endif

#if ATH_NAND_IN_DBG
#	define indbg(a, ...)					\
	do {							\
		printk("--- %s(%d):" a "\n",			\
			__func__, __LINE__, ## __VA_ARGS__);	\
	} while (0)
#else
#	define indbg(...)
#	define indbg1(a, ...)					\
	do {							\
		printk("--- %s(%d):" a "\n",			\
			__func__, __LINE__, ## __VA_ARGS__);	\
	} while (0)
#endif

/*
 * Data structures for ath nand flash controller driver
 */

typedef union {
	uint8_t			byte_id[8];

	struct {
		uint8_t		sa1	: 1,	// Serial access time (bit 1)
				org	: 1,	// Organisation
				bs	: 2,	// Block size
				sa0	: 1,	// Serial access time (bit 0)
				ss	: 1,	// Spare size per 512 bytes
				ps	: 2,	// Page Size

				wc	: 1,	// Write Cache
				ilp	: 1, 	// Interleaved Programming
				nsp	: 2, 	// No. of simult prog pages
				ct	: 2,	// Cell type
				dp	: 2,	// Die/Package

				did,		// Device id
				vid,		// Vendor id

				res1	: 2,	// Reserved
				pls	: 2,	// Plane size
				pn	: 2,	// Plane number
				res2	: 2;	// Reserved
	} __details;
} ath_nand_id_t;

uint64_t ath_plane_size[] = {
	64 << 20,
	 1 << 30,
	 2 << 30,
	 4 << 30,
	 8 << 30
};


/* ath nand info */
typedef struct {
	/* mtd info */
	struct mtd_info		*mtd;

	/* platform info */
	unsigned short		page_size,
				data_width;

	/* NAND MTD partition information */
	int			nr_partitions;
	struct mtd_partition	*partitions;

	unsigned		*bbt;

	unsigned		ba0,
				ba1,
				cmd;	// Current command
	ath_nand_id_t		__id;	// for readid
#if ATH_NF_HW_ECC
	uint32_t		ecc_offset;
#endif
} ath_nand_sc_t;

ath_nand_sc_t ath_nand_sc;
struct mtd_info nand_info[CFG_MAX_NAND_DEVICE];
int nand_curr_device = 0;

int ath_nand_block_isbad(struct mtd_info *mtd, loff_t ofs);

#define	nid	__id.__details
#define	bid	__id.byte_id

static unsigned
ath_nand_status(void)
{
	unsigned	rddata, i, j, dmastatus;

	rddata = ath_reg_rd(ATH_NF_STATUS);
	for (i = 0; i < ATH_NF_STATUS_RETRY && rddata != 0xff; i++) {
		udelay(25);
		rddata = ath_reg_rd(ATH_NF_STATUS);
	}

	dmastatus = ath_reg_rd(ATH_NF_DMA_CTRL);
	for (j = 0; j < ATH_NF_STATUS_RETRY && !(dmastatus & 1); j++) {
		udelay(25);
		dmastatus = ath_reg_rd(ATH_NF_DMA_CTRL);
	}

	if ((i == ATH_NF_STATUS_RETRY) || (j == ATH_NF_STATUS_RETRY)) {
		//printk("ath_nand_status: i = %u j = %u\n", i, j);
		ath_nand_hw_init(NULL);
		return -1;
	}
	ath_nand_clear_int_status();
	ath_reg_wr(ATH_NF_COMMAND, 0x07024);	// READ STATUS
	while (ath_nand_get_cmd_end_status() == 0);
	rddata = ath_reg_rd(ATH_NF_RD_STATUS);

	return rddata;
}

static unsigned
ath_nand_rw_page(ath_nand_sc_t *sc, int rd, unsigned addr0, unsigned addr1, unsigned count, unsigned char *buf)
{
	unsigned	i = 0, tmp, rddata;
	char		*err[] = { "Write", "Read" };
#define ATH_MAX_RETRY	25
retry:
	ath_nand_clear_int_status();
	ath_reg_wr(ATH_NF_ADDR0_0, addr0);
	ath_reg_wr(ATH_NF_ADDR0_1, addr1);
	ath_reg_wr(ATH_NF_DMA_ADDR, (unsigned)buf);
	ath_reg_wr(ATH_NF_DMA_COUNT, count);

	{
		unsigned x0, x1;
		x0 = ath_reg_rd(ATH_NF_ADDR0_0);
		x1 = ath_reg_rd(ATH_NF_ADDR0_1);

		if (x0 != addr0 || x1 != addr1) {
			printf("=== reg write failed buf = %p ===\n", buf);
			printf("%s: 0x%x 0x%x, 0x%x 0x%x\n", __func__,
				x0, x1, addr0, addr1);
			printf("========================\n");
			panic("Register writes to NAND failed\n");
		}
	}
#if ATH_NF_HW_ECC
	if (sc->ecc_offset && (count & sc->mtd->writesize_mask) == 0) {
		/*
		 * ECC can operate only on the device's pages.
		 * Cannot be used for non-page-sized read/write
		 */
		ath_reg_wr(ATH_NF_ECC_OFFSET, sc->ecc_offset);
		ath_reg_wr(ATH_NF_ECC_CTRL, 0x20e0);
		ath_reg_wr(ATH_NF_CTRL,	ATH_NF_CTRL_ADDR_CYCLE0_5 |
				ATH_NF_CTRL_BLOCK_SIZE_64 |
				ATH_NF_CTRL_PAGE_SIZE_2048 |
				ATH_NF_CTRL_ECC_EN);
	} else
#endif
	{
		//ath_reg_wr(ATH_NF_ECC_OFFSET, 0);
		//ath_reg_wr(ATH_NF_ECC_CTRL, 0);
		ath_reg_wr(ATH_NF_CTRL,	ATH_NF_CTRL_ADDR_CYCLE0_5 |
				ATH_NF_CTRL_BLOCK_SIZE_64 |
				ATH_NF_CTRL_PAGE_SIZE_2048 |
				ATH_NF_CTRL_CUSTOM_SIZE_EN);
		ath_reg_wr(ATH_NF_PG_SIZE, count);
	}

	if (rd) {	// Read Page
		ath_reg_wr(ATH_NF_DMA_CTRL,
					ATH_NF_DMA_CTRL_DMA_START |
					ATH_NF_DMA_CTRL_DMA_DIR_READ |
					ATH_NF_DMA_CTRL_DMA_BURST_3);
		ath_reg_wr(ATH_NF_COMMAND, 0x30006a);
	} else {	// Write Page
		ath_reg_wr(ATH_NF_DMA_CTRL,
					ATH_NF_DMA_CTRL_DMA_START |
					ATH_NF_DMA_CTRL_DMA_DIR_WRITE |
					ATH_NF_DMA_CTRL_DMA_BURST_3);
		ath_reg_wr(ATH_NF_COMMAND, 0x10804c);
	}

	while (ath_nand_get_cmd_end_status() == 0);

	//printk(KERN_DEBUG "%s(%c): 0x%x 0x%x 0x%x 0x%p\n", __func__,
	//	rd ? 'r' : 'w', addr0, addr1, count, buf);
	udelay(1000);
	rddata = (tmp = ath_nand_status()) & ATH_NF_RD_STATUS_MASK;
	if ((rddata != ATH_NF_STATUS_OK) && (i < ATH_MAX_RETRY)) {
		i++;
		goto retry;
	}

	if (rddata != ATH_NF_STATUS_OK) {
		printk("%s: %s Failed. tmp = 0x%x, status = 0x%x 0x%x retries = %d\n", __func__,
			err[rd], tmp, rddata, ath_reg_rd(ATH_NF_DMA_CTRL), i);
	}
#if ATH_NF_HW_ECC
	else {
		uint32_t	r = ath_reg_rd(ATH_NF_ECC_CTRL);
#define DDR_WB_FLUSH_USB_ADDRESS		0x180000a4

		ath_reg_wr(DDR_WB_FLUSH_USB_ADDRESS, 1);
		while (ath_reg_rd(DDR_WB_FLUSH_USB_ADDRESS) & 1);
		udelay(50);

		if (r & ATH_NF_ECC_CTRL_ERR_UNCORR) {
			printk("%s: %s uncorrectable errors. ecc = 0x%x\n",
				__func__, err[rd], r);
			return -1;
		}
	}
#endif
	return rddata;
}

void
ath_nand_dump_buf(loff_t addr, void *v, unsigned count)
{
	unsigned	*buf = v,
			*end = buf + (count / sizeof(*buf));

	iodbg("____ Dumping %d bytes at 0x%p 0x%llx_____\n", count, buf, addr);

	for (; buf && buf < end; buf += 4, addr += 16) {
		printk("%08lx: %08x %08x %08x %08x\n",
			(unsigned)addr, buf[0], buf[1], buf[2], buf[3]);
	}
	iodbg("___________________________________\n");
	//while(1);
}


/* max page size + oob buf size */
uint8_t	ath_nand_io_buf[4096 + 256] __attribute__((aligned(4096)));

static int
ath_nand_rw_buff(struct mtd_info *mtd, int rd, uint8_t *buf,
		loff_t addr, size_t len, size_t *iodone)
{
	unsigned	iolen, ret = ATH_NF_STATUS_OK;
	unsigned char	*pa;
	ath_nand_sc_t	*sc = mtd->priv;

	*iodone = 0;

	while (len) {
		unsigned b, p, c, ba0, ba1;

		if (ath_nand_block_isbad(mtd, addr)) {
			printk("Skipping bad block[0x%x]\n", (unsigned)addr);
			addr += mtd->erasesize;
			continue;
		}

		b = (addr >> mtd->erasesize_shift);
		p = (addr & mtd->erasesize_mask) >> mtd->writesize_shift;
		c = (addr & mtd->writesize_mask);

		/*
		 * addr format:
		 * a0 - a11 - xxxx - a19 - a27 == 32 bits, will be in ba0
		 * a28 - a31 - xxxxxxxxxxxxxxxx == 4 bits, will be in ba1 in lsb
		 */

		ba0 = (b << 22) | (p << 16);
		ba1 = (b >>  9) & 0xf;
		if (c) {
			iolen = mtd->writesize - c;
		} else {
			iolen = mtd->writesize;
		}

		if (len < iolen) {
			iolen = len;
		}

		if (!rd) {
			/* FIXME for writes FIXME */
			memcpy(ath_nand_io_buf, buf, iolen);
		}

		pa = (void *)virt_to_phys(ath_nand_io_buf);
		if (!rd) {
			flush_cache((unsigned)ath_nand_io_buf,
				mtd->writesize);
		}

		//printk("%s(%c): 0x%x 0x%x 0x%x 0x%p\n", __func__,
		//	rd ? 'r' : 'w', ba0, ba1, iolen, pa);

		ret = ath_nand_rw_page(sc, rd, ba0, ba1, mtd->writesize, pa);

		if (rd) memcpy(ath_nand_io_buf, pa, mtd->writesize);

		if (rd) {
			memcpy(buf, ath_nand_io_buf + c, iolen);
		}

		//	ath_nand_dump_buf(addr, buf, iolen);

		if (ret != ATH_NF_STATUS_OK) {
			return 1;
		}

		len -= iolen;
		buf += iolen;
		addr += iolen;
		*iodone += iolen;
	}

	return 0;
}

#define ath_nand_write_verify	0

#if ath_nand_write_verify
uint8_t	ath_nand_rd_buf[4096 + 256] __attribute__((aligned(4096)));
#endif

static int
ath_nand_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	int	ret;
#if ath_nand_write_verify
	int	r, rl;
#endif

	if (!len || !retlen) return (0);

	indbg("0x%llx	%u", to, len);

	ret = ath_nand_rw_buff(mtd, 0 /* write */, (u_char *)buf, to, len, retlen);
#if ath_nand_write_verify
	//printk("Verifying 0x%llx 0x%x\n", to, len);
	r = ath_nand_rw_buff(mtd, 1 /* read */, ath_nand_rd_buf, to, len, &rl);
	if (r || memcmp(ath_nand_rd_buf, buf, len)) {
		printk("write failed at 0x%llx 0x%x\n", to, len);
		while (1);
	}
#endif
	return ret;
}

static int
ath_nand_read(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, u_char *buf)
{
	int	ret;

	if (!len || !retlen) return (0);

	ret = ath_nand_rw_buff(mtd, 1 /* read */, buf, from, len, retlen);

	return ret;
}

static inline int
ath_nand_block_erase(unsigned addr0, unsigned addr1)
{
	unsigned	rddata;

	indbg("0x%x 0x%x", addr1, addr0);

	ath_nand_clear_int_status();
	ath_reg_wr(ATH_NF_ADDR0_0, addr0);
	ath_reg_wr(ATH_NF_ADDR0_1, addr1);
	ath_reg_wr(ATH_NF_COMMAND, 0xd0600e);	// BLOCK ERASE

	while (ath_nand_get_cmd_end_status() == 0);

	rddata = ath_nand_status() & ATH_NF_RD_STATUS_MASK;
	if (rddata != ATH_NF_STATUS_OK) {
		printk("Erase Failed. status = 0x%x", rddata);
		return 1;
	}
	return 0;
}


static int
ath_nand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	loff_t s_first, i;
	unsigned n, j;
	int ret, bad = 0;

	if (instr->addr + instr->len > mtd->size) {
		return (-EINVAL);
	}

	s_first = instr->addr;
	n = instr->len >> mtd->erasesize_shift;

	if (instr->len & mtd->erasesize_mask) n ++;

	indbg("0x%llx 0x%x 0x%x", instr->addr, n, mtd->erasesize);

	for (j = 0, i = s_first; j < n; j++, i += mtd->erasesize) {
		ulong b, ba0, ba1;

		if (ath_nand_block_isbad(mtd, i)) {
			bad ++;
			continue;
		}

		b = (i >> mtd->erasesize_shift);

		ba0 = (b << 22);
		ba1 = (b >>  9) & 0xf;

		printk("\b\b\b\b%4d", j);

		if ((ret = ath_nand_block_erase(ba0, ba1)) != 0) {
			iodbg("%s: erase failed 0x%llx 0x%x 0x%x %llu "
				"%lx %lx\n", __func__, instr->addr, n,
				mtd->erasesize, i, ba1, ba0);
			break;
		}

	}

	if (instr->callback) {
		if (j < n) {
			instr->state = MTD_ERASE_FAILED;
		} else {
			instr->state = MTD_ERASE_DONE;
		}
		mtd_erase_callback(instr);
	}

	printk("Skipped %d bad blocks\n", bad);

	return ret;
}

/* lifted from linux */
typedef enum {
	MTD_OOB_PLACE,
	MTD_OOB_AUTO,
	MTD_OOB_RAW,
} mtd_oob_mode_t;

struct mtd_oob_ops {
	mtd_oob_mode_t  mode;
	size_t          len;
	size_t          retlen;
	size_t          ooblen;
	size_t          oobretlen;
	uint32_t        ooboffs;
	uint8_t         *datbuf;
	uint8_t         *oobbuf;
};

static int
ath_nand_rw_oob(struct mtd_info *mtd, int rd, loff_t addr,
		struct mtd_oob_ops *ops)
{
	unsigned	ret = ATH_NF_STATUS_OK;
	unsigned char	*pa;
	unsigned	b, p, c, ba0, ba1;
	uint8_t		*oob = ath_nand_io_buf + mtd->writesize;
	ath_nand_sc_t	*sc = mtd->priv;

	b = (addr >> mtd->erasesize_shift);
	p = (addr & mtd->erasesize_mask) >> mtd->writesize_shift;
	c = (addr & mtd->writesize_mask);

	ba0 = (b << 22) | (p << 16);
	ba1 = (b >>  9) & 0xf;

	if (!rd) {
		if (ops->datbuf) {
			/*
			 * XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
			 * We assume that the caller gives us a full
			 * page to write. We don't read the page and
			 * update the changed portions alone.
			 *
			 * Hence, not checking for len < or > pgsz etc...
			 * XXX XXX XXX XXX XXX XXX XXX XXX XXX XXX
			 */
			memcpy(ath_nand_io_buf, ops->datbuf, ops->len);
		}
		if (ops->mode == MTD_OOB_PLACE) {
			oob += ops->ooboffs;
		} else if (ops->mode == MTD_OOB_AUTO) {
			// clean markers
			oob[0] = oob[1] = 0xff;
			oob += 2;
		}
		memcpy(oob, ops->oobbuf, ops->ooblen);
	}

	pa = (void *)virt_to_phys(ath_nand_io_buf);
	if (!rd) flush_cache(ath_nand_io_buf, mtd->writesize + mtd->oobsize);	// for writes...

	//printk("%s(%c): 0x%x 0x%x 0x%x 0x%p\n", __func__,
	//	rd ? 'r' : 'w', ba0, ba1, mtd->writesize + mtd->oobsize, pa);

	ret = ath_nand_rw_page(sc, rd, ba0, ba1, mtd->writesize + mtd->oobsize, pa);

	if (ret != ATH_NF_STATUS_OK) {
		return 1;
	}

	if (rd) memcpy(ath_nand_io_buf, pa, mtd->writesize + mtd->oobsize);	// for reads...
	//ath_nand_dump_buf(addr, buf, iolen);

	if (rd) {
		if (ops->datbuf) {
			memcpy(ops->datbuf, ath_nand_io_buf, ops->len);
		}
		if (ops->mode == MTD_OOB_PLACE) {
			oob += ops->ooboffs;
		} else if (ops->mode == MTD_OOB_AUTO) {
			// copy after clean marker
			oob += 2;
		}
		memcpy(ops->oobbuf, oob, ops->ooblen);
	}

	//if (rd) {
	//	ath_nand_dump_buf(addr, ops->datbuf, ops->len);
	//	ath_nand_dump_buf(addr, ops->oobbuf, ops->ooblen);
	//}


	if (ops->datbuf) {
		ops->retlen = ops->len;
	}
	ops->oobretlen = ops->ooblen;

	return 0;
}

//static int
//ath_nand_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
int nand_read_raw (struct mtd_info *mtd, uint8_t *buf, loff_t from, size_t len, size_t ooblen)
{
	struct mtd_oob_ops ops = { MTD_OOB_RAW, len, 0, ooblen, 0,
					0, buf, buf + mtd->writesize };

	oobdbg(	"%s: from: 0x%llx mode: 0x%x len: 0x%x retlen: 0x%x\n"
		"ooblen: 0x%x oobretlen: 0x%x ooboffs: 0x%x datbuf: %p "
		"oobbuf: %p\n", __func__, from,
		ops.mode, ops.len, ops.retlen, ops.ooblen,
		ops.oobretlen, ops.ooboffs, ops.datbuf,
		ops.oobbuf);

	oobdbg("0x%llx %p %p %u", from, ops.oobbuf, ops.datbuf, ops.len);

	if (len == 0) {
		ops.datbuf = 0;
		ops.oobbuf = buf;
	}
	if (ooblen == 0) {
		ops.oobbuf = NULL;
	}

	return ath_nand_rw_oob(mtd, 1 /* read */, from, &ops);
}

#if 0
static int
ath_nand_write_oob(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
	int ret;
	unsigned char oob[128];
	struct mtd_oob_ops	rops = {
		.mode	= MTD_OOB_RAW,
		.ooblen	= mtd->oobsize,
		.oobbuf	= oob,
	};

	if (ops->mode == MTD_OOB_AUTO) {
		/* read existing oob */
		if (ath_nand_read_oob(mtd, to, &rops) ||
			rops.oobretlen != rops.ooblen) {
			printk("%s: oob read failed at 0x%llx\n", __func__, to);
			return 1;
		}
		memcpy(oob + 2, ops->oobbuf, ops->ooblen);
		rops = *ops;
		ops->oobbuf = oob;
		ops->ooblen = mtd->oobsize;
		ops->mode = MTD_OOB_RAW;
	}

	oobdbg(	"%s: from: 0x%llx mode: 0x%x len: 0x%x retlen: 0x%x\n"
		"ooblen: 0x%x oobretlen: 0x%x ooboffs: 0x%x datbuf: %p "
		"oobbuf: %p\n", __func__, to,
		ops->mode, ops->len, ops->retlen, ops->ooblen,
		ops->oobretlen, ops->ooboffs, ops->datbuf,
		ops->oobbuf);

	indbg("0x%llx", to);

	ret = ath_nand_rw_oob(mtd, 0 /* write */, to, ops);

	if (rops.mode == MTD_OOB_AUTO) {
		if (ret == 0) { // rw oob success
			rops.oobretlen = rops.ooblen;
			rops.retlen = rops.len;
		}
		*ops = rops;
	}

	return ret;
}
#endif

#define	bbt_index	(sizeof(*sc->bbt) * 8 / 2)

#define ATH_NAND_BLK_DONT_KNOW	0x0
#define ATH_NAND_BLK_GOOD	0x1
#define ATH_NAND_BLK_BAD	0x2

inline unsigned
ath_nand_get_blk_state(struct mtd_info *mtd, loff_t b)
{
	unsigned		x, y;
	ath_nand_sc_t		*sc = mtd->priv;

	if (!sc->bbt)	return ATH_NAND_BLK_DONT_KNOW;

	b = b >> mtd->erasesize_shift;

	x = b / bbt_index;
	y = b % bbt_index;

	return (sc->bbt[x] >> (y * 2)) & 0x3;
}

inline void
ath_nand_set_blk_state(struct mtd_info *mtd, loff_t b, unsigned state)
{
	unsigned		x, y;
	ath_nand_sc_t		*sc = mtd->priv;

	if (!sc->bbt)	return;

	b = b >> mtd->erasesize_shift;

	x = b / bbt_index;
	y = b % bbt_index;

	sc->bbt[x] = (sc->bbt[x] & ~(3 << (y * 2))) | (state << (y * 2));
}

int
ath_nand_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	unsigned char		oob[128];
	unsigned		bs, i;

	bs = ath_nand_get_blk_state(mtd, ofs);

	if (bs != ATH_NAND_BLK_DONT_KNOW) {
		return (bs - ATH_NAND_BLK_GOOD);
	}

	/*
	 * H27U1G8F2B Series [1 Gbit (128 M x 8 bit) NAND Flash]
	 *
	 * The Bad Block Information is written prior to shipping. Any
	 * block where the 1st Byte in the spare area of the 1st or
	 * 2nd th page (if the 1st page is Bad) does not contain FFh
	 * is a Bad Block. The Bad Block Information must be read
	 * before any erase is attempted as the Bad Block Information
	 * may be erased. For the system to be able to recognize the
	 * Bad Blocks based on the original information it is
	 * recommended to create a Bad Block table following the
	 * flowchart shown in Figure 24. The 1st block, which is
	 * placed on 00h block address is guaranteed to be a valid
	 * block.
	 */

	for (i = 0; i < 2; i++, ofs += mtd->writesize) {
		if (nand_read_raw(mtd, oob, ofs, 0, mtd->oobsize)) {
			printk("%s: oob read failed at 0x%lx\n", __func__, (unsigned)ofs);
			ath_nand_set_blk_state(mtd, ofs, ATH_NAND_BLK_DONT_KNOW);
			return 1;
		}

		/* First two bytes of oob data are clean markers */
		if (oob[0] != 0xff || oob[1] != 0xff) {
			oobdbg("%s: block is bad at 0x%lx\n", __func__, (unsigned)ofs);
			oobdbg(	"%02x %02x %02x %02x %02x %02x %02x %02x "
				"%02x %02x %02x %02x %02x %02x %02x %02x "
				"%02x %02x %02x %02x %02x %02x %02x %02x "
				"%02x %02x %02x %02x %02x %02x %02x %02x "
				"%02x %02x %02x %02x %02x %02x %02x %02x "
				"%02x %02x %02x %02x %02x %02x %02x %02x "
				"%02x %02x %02x %02x %02x %02x %02x %02x "
				"%02x %02x %02x %02x %02x %02x %02x %02x\n",
				0xff & oob[ 0], 0xff & oob[ 1], 0xff & oob[ 2],
				0xff & oob[ 3], 0xff & oob[ 4], 0xff & oob[ 5],
				0xff & oob[ 6], 0xff & oob[ 7], 0xff & oob[ 8],
				0xff & oob[ 9], 0xff & oob[10], 0xff & oob[11],
				0xff & oob[12], 0xff & oob[13], 0xff & oob[14],
				0xff & oob[15], 0xff & oob[16], 0xff & oob[17],
				0xff & oob[18], 0xff & oob[19], 0xff & oob[20],
				0xff & oob[21], 0xff & oob[22], 0xff & oob[23],
				0xff & oob[24], 0xff & oob[25], 0xff & oob[26],
				0xff & oob[27], 0xff & oob[28], 0xff & oob[29],
				0xff & oob[30], 0xff & oob[31], 0xff & oob[32],
				0xff & oob[33], 0xff & oob[34], 0xff & oob[35],
				0xff & oob[36], 0xff & oob[37], 0xff & oob[38],
				0xff & oob[39], 0xff & oob[40], 0xff & oob[41],
				0xff & oob[42], 0xff & oob[43], 0xff & oob[44],
				0xff & oob[45], 0xff & oob[46], 0xff & oob[47],
				0xff & oob[48], 0xff & oob[49], 0xff & oob[50],
				0xff & oob[51], 0xff & oob[52], 0xff & oob[53],
				0xff & oob[54], 0xff & oob[55], 0xff & oob[56],
				0xff & oob[57], 0xff & oob[58], 0xff & oob[59],
				0xff & oob[60], 0xff & oob[61], 0xff & oob[62],
				0xff & oob[63]);
			ath_nand_set_blk_state(mtd, ofs, ATH_NAND_BLK_BAD);
			return 1;
		}
	}

	ath_nand_set_blk_state(mtd, ofs, ATH_NAND_BLK_GOOD);

	return 0;
}

static int
ath_nand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	indbg("unimplemented 0x%llx", ofs);
	return 0;
}

static unsigned long
ath_parse_read_id(ath_nand_sc_t *sc)
{
	int	i;

	extern struct nand_manufacturers nand_manuf_ids[];
	extern struct nand_flash_dev nand_flash_ids[];

	iodbg(	"____ %s _____\n"
		"  vid did wc  ilp nsp ct  dp  sa1 org bs  sa0 ss  "
		"ps  res1 pls pn  res2\n"
		"0x%3x %3x %3x %3x %3x %3x %3x %3x %3x %3x %3x %3x "
		"%3x %3x  %3x %3x %3x\n-------------\n", __func__,
			sc->nid.vid, sc->nid.did, sc->nid.wc, sc->nid.ilp,
			sc->nid.nsp, sc->nid.ct, sc->nid.dp, sc->nid.sa1,
			sc->nid.org, sc->nid.bs, sc->nid.sa0, sc->nid.ss,
			sc->nid.ps, sc->nid.res1, sc->nid.pls, sc->nid.pn,
			sc->nid.res2);

	for (i = 0; i < nand_manuf_ids[i].id; i++) {
		if (nand_manuf_ids[i].id == sc->nid.vid) {
			printk(nand_manuf_ids[i].name);
			break;
		}
	}

	for (i = 0; i < nand_flash_ids[i].id; i++) {
		if (nand_flash_ids[i].id == sc->nid.did) {
			printk(" %s [%uMB]\n", nand_flash_ids[i].name,
				nand_flash_ids[i].chipsize);
			return nand_flash_ids[i].chipsize;
		}
	}

	return 0;
}

/*
 * System initialization functions
 */
static int
ath_nand_hw_init(void *p)
{
	uint8_t		id[8];
	unsigned char	*pa;
	unsigned	rddata, i;

	ath_reg_rmw_set(RST_RESET_ADDRESS, RST_RESET_NANDF_RESET_MASK);
	udelay(250);

	ath_reg_rmw_clear(RST_RESET_ADDRESS, RST_RESET_NANDF_RESET_MASK);
	udelay(100);

	ath_reg_wr(ATH_NF_INT_MASK, ATH_NF_CMD_END_INT);
	ath_nand_clear_int_status();

	// TIMINGS_ASYN Reg Settings
	ath_reg_wr(ATH_NF_TIMINGS_ASYN, ATH_NF_TIMING_ASYN);

	// NAND Mem Control Reg
	ath_reg_wr(ATH_NF_MEM_CTRL, 0xff00);

	// Reset Command
	ath_reg_wr(ATH_NF_COMMAND, 0xff00);

	while (ath_nand_get_cmd_end_status() == 0);

	udelay(1000);

	rddata = ath_reg_rd(ATH_NF_STATUS);
	for (i = 0; i < ATH_NF_STATUS_RETRY && rddata != 0xff; i++) {
		udelay(25);
		rddata = ath_reg_rd(ATH_NF_STATUS);
	}

	if (i == ATH_NF_STATUS_RETRY) {
		printf("device reset failed\n");
		while(1);
	}

	if (p) {
		ath_nand_clear_int_status();
		pa = (void *)virt_to_phys(p ? p : id);
		ath_reg_wr(ATH_NF_DMA_ADDR, (unsigned)pa);
		ath_reg_wr(ATH_NF_ADDR0_0, 0x0);
		ath_reg_wr(ATH_NF_ADDR0_1, 0x0);
		ath_reg_wr(ATH_NF_DMA_COUNT, 0x8);
		ath_reg_wr(ATH_NF_PG_SIZE, 0x8);
		ath_reg_wr(ATH_NF_DMA_CTRL, 0xcc);
		ath_reg_wr(ATH_NF_COMMAND, 0x9061);	// READ ID
		while (ath_nand_get_cmd_end_status() == 0);

		rddata = ath_nand_status();
		if ((rddata & ATH_NF_RD_STATUS_MASK) != ATH_NF_STATUS_OK) {
			printf("%s: ath nand status = 0x%x\n", __func__, rddata);
		}

		pa = p;
		printk("Ath Nand ID[%p]: %02x:%02x:%02x:%02x:%02x\n",
				pa, pa[0], pa[1], pa[2], pa[3], pa[4]);

		iodbg("******* %s done ******\n", __func__);
	}

	return 0;
}

/*
 * Copied from drivers/mtd/nand/nand_base.c
 * http://ptgmedia.pearsoncmg.com/images/chap17_9780132396554/elementLinks/17fig04.gif
 *
 * +---...---+--+----------+---------+
 * |  2048   |  |          |         |
 * | File    |cm| FS spare | ecc data|
 * | data    |  |          |         |
 * +---...---+--+----------+---------+
 * cm -> clean marker (2 bytes)
 * FS Spare -> 38 bytes available for jffs2
 */

static void
ath_nand_ecc_init(struct mtd_info *mtd)
{
	ath_nand_sc_t *sc = mtd->priv;

	sc->ecc_offset = 0;

	if (mtd->oobsize == 128) {
#if ATH_NF_HW_ECC
		sc->ecc_offset = mtd->writesize + 80;
#endif
	} else if (mtd->oobsize == 64) {
#if ATH_NF_HW_ECC
		sc->ecc_offset = mtd->writesize + 40;
#endif
	}

}

/*
 * ath_nand_probe
 *
 * called by device layer when it finds a device matching
 * one our driver can handled. This code checks to see if
 * it can allocate all necessary resources then calls the
 * nand layer to look for devices
 */
static ulong ath_nand_probe(void)
{
	ath_nand_sc_t	*sc = NULL;
	struct mtd_info	*mtd = NULL;
	int		err = 0, bbt_size;

	sc = &ath_nand_sc;
	sc->mtd = &nand_info[nand_curr_device];

	/* initialise the hardware */
	err = ath_nand_hw_init(&sc->nid);
	if (err) {
		goto out_err_hw_init;
	}

	/* initialise mtd sc data struct */
	mtd = sc->mtd;
	mtd->size = ath_parse_read_id(sc) << 20;

	mtd->name		= DRV_NAME;
	if (mtd->size == 0) {
		mtd->size	= ath_plane_size[sc->nid.pls] << sc->nid.pn;
	}

	mtd->writesize_shift	= 10 + sc->nid.ps;
	mtd->writesize		= (1 << mtd->writesize_shift);
	mtd->writesize_mask	= (mtd->writesize - 1);

	mtd->erasesize_shift	= 16 + sc->nid.bs;
	mtd->erasesize		= (1 << mtd->erasesize_shift);
	mtd->erasesize_mask	= (mtd->erasesize - 1);

	mtd->oobsize		= (mtd->writesize / 512) * (8 << sc->nid.ss);
	mtd->oobavail		= mtd->oobsize;

	mtd->type		= MTD_NANDFLASH;
	mtd->flags		= MTD_CAP_NANDFLASH;

	mtd->read		= ath_nand_read;
	mtd->write		= ath_nand_write;
	mtd->erase		= ath_nand_erase;

	//mtd->read_oob		= ath_nand_read_oob;
	//mtd->write_oob		= ath_nand_write_oob;

	mtd->block_isbad	= ath_nand_block_isbad;
	mtd->block_markbad	= ath_nand_block_markbad;

	mtd->priv		= sc;

	ath_nand_ecc_init(mtd);

	// bbt has 2 bits per block
	bbt_size = ((mtd->size >> mtd->erasesize_shift) * 2) / 8;
	sc->bbt = malloc(bbt_size);

	if (sc->bbt) {
		memset(sc->bbt, 0, bbt_size);
	}

	printk("%s: sc->bbt = 0x%x size = 0x%x\n", __func__, sc->bbt, bbt_size);

	return mtd->size;

out_err_hw_init:
	return 0;
}

#if 0
static struct platform_driver ath_nand_driver = {
	//.probe		= ath_nand_probe,
	.remove		= __exit_p(ath_nand_remove),
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};
#endif

ulong ath_nand_init(void)
{
	printk(DRV_DESC ", Version " DRV_VERSION
		" (c) 2010 Atheros Communications, Ltd.\n");

	//return platform_driver_register(&ath_nand_driver);
	//return platform_driver_probe(&ath_nand_driver, ath_nand_probe);
	return ath_nand_probe();
}
