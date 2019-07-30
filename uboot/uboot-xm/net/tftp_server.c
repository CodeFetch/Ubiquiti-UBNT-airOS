/*
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000, 2001 DENX Software Engineering, Wolfgang Denk, wd@denx.de
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include "tftp.h"
#include "bootp.h"
#include <jffs2/load_kernel.h>

#if 0
#undef	ET_DEBUG
#endif

#if (CONFIG_COMMANDS & CFG_CMD_TFTP_SERVER)

#define WELL_KNOWN_PORT	69		/* Well known TFTP port #		*/
#define TIMEOUT		1		/* Seconds to timeout for a lost pkt	*/
#ifndef	CONFIG_NET_RETRY_COUNT
# define TIMEOUT_COUNT	5		/* # of timeouts before giving up  */
#else
# define TIMEOUT_COUNT  (CONFIG_NET_RETRY_COUNT * 8)
#endif
					/* (for checking the image size)	*/
#define HASHES_PER_LINE	65		/* Number of "loading" hashes per line	*/

#define STR_IP "%d.%d.%d.%d"
#define IP_STR(a) \
	((unsigned char*)a)[0], ((unsigned char*)a)[1], \
	((unsigned char*)a)[2], ((unsigned char*)a)[3]

/*
 *	TFTP operations.
 */
#define TFTP_RRQ	1
#define TFTP_WRQ	2
#define TFTP_DATA	3
#define TFTP_ACK	4
#define TFTP_ERROR	5
#define TFTP_OACK	6

struct	tftphdr {
	short	opcode;			/* packet type */
	union {
		unsigned short	block;	/* block # */
		short	code;		/* error code */
		char	stuff[1];	/* request packet stuff */
	} __attribute__ ((__packed__)) th_u;
	char	data[1];		/* data or error string */
} __attribute__ ((__packed__));

static int	TftpClientPort;		/* The UDP port at their end		*/
static IPaddr_t	TftpOurClientIP;	/* Their IP address			*/
static int	TftpOurPort;		/* The UDP port at our end		*/
static int	TftpTimeoutCount;
static ulong	TftpBlock;		/* packet sequence number		*/
static ulong	TftpLastBlock;		/* last packet sequence number received */
static ulong	TftpBlockWrap;		/* count of sequence number wraparounds */
static ulong	TftpBlockWrapOffset;	/* memory offset due to wrapping	*/
static int	TftpServerState;
static ulong	TftpLastTimer;
static ulong	TftpFileAddr;		/* data address to upload when user issues "GET" */
static ulong	TftpFileSize;		/* data amount to upload to user	*/

int	TftpServerOverwriteBootloader = 0; /* 1 - update U-Boot if found in FW */
int	TftpServerUseDefinedIP = 0; /* 1 - use default IP for server */

#define TFTP_SRV_STATE_CONNECT	1
#define TFTP_SRV_STATE_WRQ	2
#define TFTP_SRV_STATE_RRQ	3
#define TFTP_SRV_STATE_DATA	4
#define TFTP_SRV_STATE_TOO_LARGE	5
#define TFTP_SRV_STATE_BAD_MAGIC	6
#define TFTP_SRV_STATE_OACK	7
#define TFTP_SRV_STATE_BADFW	8
#define TFTP_SRV_STATE_FWUPDATE	9
#define TFTP_SRV_STATE_DATA_SEND	10

#define TFTP_BLOCK_SIZE		512		    /* default TFTP block size	*/
#define TFTP_SEQUENCE_SIZE	((ulong)(1<<16))    /* sequence number is 16 bit */

#define DEFAULT_NAME_LEN	32
static char tftp_filename[DEFAULT_NAME_LEN];

#ifdef CFG_DIRECT_FLASH_TFTP
extern flash_info_t flash_info[];
#endif

static __inline__ void
store_block (unsigned block, uchar * src, unsigned len)
{
	ulong offset = block * TFTP_BLOCK_SIZE + TftpBlockWrapOffset;
	ulong newsize = offset + len;
#ifdef CFG_DIRECT_FLASH_TFTP
	int i, rc = 0;

	for (i=0; i<CFG_MAX_FLASH_BANKS; i++) {
		/* start address in flash? */
		if (load_addr + offset >= flash_info[i].start[0]) {
			rc = 1;
			break;
		}
	}

	if (rc) { /* Flash is destination for this packet */
		rc = flash_write ((char *)src, (ulong)(load_addr+offset), len);
		if (rc) {
			flash_perror (rc);
			NetState = NETLOOP_FAIL;
			return;
		}
	}
	else
#endif /* CFG_DIRECT_FLASH_TFTP */
	{
		(void)memcpy((void *)(load_addr + offset), src, len);
	}

	if (NetBootFileXferSize < newsize)
		NetBootFileXferSize = newsize;
}

extern flash_info_t flash_info[];	/* info for FLASH chips */

static void TftpServer (void);
static void TftpTimeout (void);

/**********************************************************************/

static void
TftpServer (void)
{
	volatile uchar *	pkt;
	volatile uchar *	xp;
	int			len = 0;
	volatile ushort *s;

	/*
	 *	We will always be sending some sort of packet, so
	 *	cobble together the packet headers now.
	 */
	pkt = NetTxPacket + NetEthHdrSize() + IP_HDR_SIZE;

	switch (TftpServerState) {

	case TFTP_SRV_STATE_RRQ:
		xp = pkt;
		s = (ushort *)pkt;
		*s++ = htons(TFTP_RRQ);
		pkt = (uchar *)s;
		strcpy ((char *)pkt, tftp_filename);
		pkt += strlen(tftp_filename) + 1;
		strcpy ((char *)pkt, "octet");
		pkt += 5 /*strlen("octet")*/ + 1;
		strcpy ((char *)pkt, "timeout");
		pkt += 7 /*strlen("timeout")*/ + 1;
		sprintf((char *)pkt, "%d", TIMEOUT);
#ifdef ET_DEBUG
		printf("send option \"timeout %s\"\n", (char *)pkt);
#endif
		pkt += strlen((char *)pkt) + 1;
		len = pkt - xp;
		break;

	case TFTP_SRV_STATE_DATA_SEND: /* send data to client */
		if ((TftpBlock - 1) * TFTP_BLOCK_SIZE > TftpFileSize) {
			printf("Client requested data block outside available data! (#%d)\n",
				TftpBlock);
			return;
		}
		xp = pkt;
		s = (ushort *)pkt;
		*s++ = htons(TFTP_DATA);
		*s++ = htons(TftpBlock);
		if ((TftpBlock - 1) * TFTP_BLOCK_SIZE >= TftpFileSize)
			len = TftpFileSize % TFTP_BLOCK_SIZE;
		else
			len = TFTP_BLOCK_SIZE;
		memmove((uchar *)s, TftpFileAddr + (TftpBlock - 1) * TFTP_BLOCK_SIZE, len);
		pkt = (uchar *)s;
		len += pkt - xp;
		break;

	case TFTP_SRV_STATE_DATA: /* send ACK for received data */
	case TFTP_SRV_STATE_OACK:
		xp = pkt;
		s = (ushort *)pkt;
		*s++ = htons(TFTP_ACK);
		*s++ = htons(TftpBlock);
		pkt = (uchar *)s;
		len = pkt - xp;
		break;

	case TFTP_SRV_STATE_TOO_LARGE:
		xp = pkt;
		s = (ushort *)pkt;
		*s++ = htons(TFTP_ERROR);
		*s++ = htons(3);
		pkt = (uchar *)s;
		strcpy ((char *)pkt, "File too large");
		pkt += 14 /*strlen("File too large")*/ + 1;
		len = pkt - xp;
		break;

	case TFTP_SRV_STATE_BAD_MAGIC:
		xp = pkt;
		s = (ushort *)pkt;
		*s++ = htons(TFTP_ERROR);
		*s++ = htons(2);
		pkt = (uchar *)s;
		strcpy ((char *)pkt, "File has bad magic");
		pkt += 18 /*strlen("File has bad magic")*/ + 1;
		len = pkt - xp;
		break;
	case TFTP_SRV_STATE_BADFW:
		xp = pkt;
		s = (ushort *)pkt;
		*s++ = htons(TFTP_ERROR);
		*s++ = htons(2);
		pkt = (uchar *)s;
#define TFTP_SRV_FW_CHECK_FAIL	"Firmware check failed"
		strcpy ((char *)pkt, TFTP_SRV_FW_CHECK_FAIL);
		pkt += strlen(TFTP_SRV_FW_CHECK_FAIL) + 1;
		len = pkt - xp;
		break;
    case TFTP_SRV_STATE_CONNECT:
         // NB: Do not send invalid packets while waiting for connections...
        return;
	}

	NetSendUDPPacket(NetServerEther, TftpOurClientIP, TftpClientPort, TftpOurPort, len);
}

extern int do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]);

static void
TftpServerHandler (uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	ushort proto;
	ushort *s;
	char cmd_str[CFG_CBSIZE];
	int cmd_st;
	//volatile struct tftphdr *hdr = (volatile struct tftphdr *)pkt;
	//int i;
	unsigned char *ip;

	/* for find_mtd_part() */
	struct part_info *prt;
	struct mtd_device *cdev;
	u8 pnum;
	flash_info_t* flinfo = &flash_info[0];

	ip = (unsigned char *)pkt;
	ip -= IP_HDR_SIZE;

#if 0
	printf("-- port from %d to %d, len = %d, TFTP opcode = %d\n", src, dest, len, hdr->opcode);

	for (i=0; i<len+IP_HDR_SIZE; i++) {
		if ((i % 16) == 0) puts("\n");
		printf("%02x ", ip[i]);
	}
	puts("\n");

#endif
	if ((TftpServerState != TFTP_SRV_STATE_WRQ) && (TftpServerState != TFTP_SRV_STATE_RRQ) &&
			(TftpServerState != TFTP_SRV_STATE_CONNECT) && TftpClientPort &&
			(src != TftpClientPort)) {
		return;
	}
	if (dest != TftpOurPort) {
		return;
	}

	if (len < 2) {
		return;
	}

	len -= 2;
	/* warning: don't use increment (++) in ntohs() macros!! */
	s = (ushort *)pkt;
	proto = *s++;
	pkt = (uchar *)s;

	switch (ntohs(proto)) {
	case TFTP_WRQ:
		printf("\nReceiving file from " STR_IP":%d\n",
			ip[12], ip[13], ip[14], ip[15], src);

		TftpServerState = TFTP_SRV_STATE_DATA;
		TftpClientPort = src;
		TftpBlock = 0;
		TftpLastBlock = 0;
		TftpBlockWrap = 0;
		TftpBlockWrapOffset = 0;

		memcpy(&TftpOurClientIP, &ip[12], sizeof(TftpOurClientIP));

		TftpServer (); /* Send ACK for block 0 */
		break;

	case TFTP_RRQ:
#if 0 /* ignore "GET" requests from client */
		printf("\nIgnoring GET request from " STR_IP":%d\n",
			ip[12], ip[13], ip[14], ip[15], src);
		return;
#else
		/* TBD: check filename */
		strncpy(tftp_filename, (uchar *)s, sizeof(tftp_filename));

		/* file for each partition (you can use something like "GET u-boot-env") */
		if (!mtdparts_init() && find_mtd_part(tftp_filename, &cdev, &pnum, &prt)) {
			TftpFileAddr = flinfo->start[0] + prt->offset;
			TftpFileSize = prt->size;
		}
		else if (!strcmp("flash_dump", tftp_filename)) {
			/* send whole flash dump */
			TftpFileAddr = flinfo->start[0];
			TftpFileSize = flinfo->size;
		} else {
			/* just ignore */
			printf("Ignoring GET attempt for unknown file: <%s>\n", tftp_filename);
			return;
		}

		TftpServerState = TFTP_SRV_STATE_DATA_SEND;
		printf("\nSending <%s> (0x%08X@0x%08X) to " STR_IP":%d\n",
			tftp_filename, TftpFileSize, TftpFileAddr,
			ip[12], ip[13], ip[14], ip[15], src);

		memcpy(&TftpOurClientIP, &ip[12], sizeof(TftpOurClientIP));

		TftpClientPort = src;
		TftpBlock = 1;

		TftpServer (); /* Send DATA */
#endif
		break;

	case TFTP_ACK:
		/* ack from client - send next data block */
		if (TftpServerState == TFTP_SRV_STATE_DATA_SEND) {
			/* update TftpBlock */
			TftpBlock = ntohs(*(ushort *)pkt);
			/* move to next block ( if this is not the last one ) */
			if ((TftpBlock * TFTP_BLOCK_SIZE) <= TftpFileSize)
				TftpBlock++;
			else {
				/* this is ACK to the last DATA packet */
				printf("File transfer completed successfuly.\n");
				TftpServerState = TFTP_SRV_STATE_CONNECT;
			}
			NetSetTimeout (TIMEOUT * CFG_HZ, TftpTimeout);
			TftpServer (); /* Send DATA */
		}
		break;

	default:
		break;

	case TFTP_OACK:
#ifdef ET_DEBUG
		printf("Got OACK: %s %s\n", pkt, pkt+strlen(pkt)+1);
#endif
		TftpServerState = TFTP_SRV_STATE_OACK;
		TftpClientPort = src;
		TftpServer (); /* Send ACK */
		break;

	case TFTP_DATA:
		if (len < 2)
			return;
		len -= 2;
		TftpBlock = ntohs(*(ushort *)pkt);

		/*
		 * RFC1350 specifies that the first data packet will
		 * have sequence number 1. If we receive a sequence
		 * number of 0 this means that there was a wrap
		 * around of the (16 bit) counter.
		 */
		if (TftpBlock == 0) {
			TftpBlockWrap++;
			TftpBlockWrapOffset += TFTP_BLOCK_SIZE * TFTP_SEQUENCE_SIZE;
			printf ("\n\t %lu MB reveived\n\t ", TftpBlockWrapOffset>>20);
		} else {
#if 1
			/* every half-second or on last block */
			if (((get_timer(TftpLastTimer)) > (CFG_HZ/2)) || (len < TFTP_BLOCK_SIZE)) {
				printf("Received %d bytes\r",
						(TftpBlock -1 ) * TFTP_BLOCK_SIZE + len);
				TftpLastTimer = get_timer(0);
			}
#else
			if (((TftpBlock - 1) % 10) == 0) {
				putc ('#');
			} else if ((TftpBlock % (10 * HASHES_PER_LINE)) == 0) {
				puts ("\n\t ");
			}
#endif
		}

#ifdef ET_DEBUG
		if (TftpServerState == TFTP_SRV_STATE_RRQ) {
			puts ("Server did not acknowledge timeout option!\n");
		}
#endif
		if (TftpServerState == TFTP_SRV_STATE_WRQ ||
				TftpServerState == TFTP_SRV_STATE_RRQ ||
				TftpServerState == TFTP_SRV_STATE_OACK) {
			/* first block received */
			TftpServerState = TFTP_SRV_STATE_DATA;
			TftpClientPort = src;
			TftpLastBlock = 0;
			TftpBlockWrap = 0;
			TftpBlockWrapOffset = 0;

			if (TftpBlock != 1) {	/* Assertion */
				printf ("\nTFTP error: "
					"First block is not block 1 (%ld)\n"
					"Starting again\n\n",
					TftpBlock);
				NetStartAgain ();
				break;
			}
		}

		if (TftpBlock == TftpLastBlock) {
			/*
			 *	Same block again; ignore it.
			 */
			break;
		}

		TftpLastBlock = TftpBlock;
		NetSetTimeout (TIMEOUT * CFG_HZ, TftpTimeout);

		store_block (TftpBlock - 1, pkt + 2, len);

		if (len < TFTP_BLOCK_SIZE) {
			uchar *fw = (uchar *)load_addr;
			unsigned total_len = (TftpLastBlock - 1) * TFTP_BLOCK_SIZE +
				TftpBlockWrapOffset + len;
			int ret;

			/*
			 *	We received the whole thing.
			 *	NOTE: last ACK still pending, don't forget to send it!!!
			 */
			putc('\n');

			/* Firmware file is uploaded to load_addr, size <len>.
			 * Check headers and CRC.
			 * If everything is ok - write FW to flash memory and reboot.
			 * Oh, don't forget to send last ACK!
			 * Else - return (TFTP server should restart).
			 */

			sprintf(cmd_str, "go ${ubntaddr} ucheck_fw %x %x", fw, total_len);
			cmd_st = run_command(cmd_str,0);
			if ((ret = (cmd_st==-1)?1:0)) {
				printf("Firmware check failed! (%d)\n", ret);
				/* send last ACK with error message */
				TftpServerState = TFTP_SRV_STATE_BADFW;
			}

			/* Last ACK */
			TftpServer();

			if (!ret) {
				TftpServerState = TFTP_SRV_STATE_FWUPDATE;
				sprintf(cmd_str, "go ${ubntaddr} uupdate_fw %x %x %d", fw, total_len,!TftpServerOverwriteBootloader);
				cmd_st = run_command(cmd_str,0);
				if (cmd_st!=-1) {
					do_reset(NULL, 0, 0, NULL);
				}
			}

			/*
			 * If we are here - then something went wrong with urescue,
			 * wait for firmware (again)...
			 */

			NetStartAgain ();
			/* Use this to return to U-Boot cmd prompt
			 * NetState = NETLOOP_SUCCESS; */
		} else {
			/*
			 *	Acknowledge the block just received, which will prompt
			 *	the server for the next one.
			 */
			TftpServer ();
		}
		break;

	case TFTP_ERROR:
		printf ("\nTFTP error: '%s' (%d)\n",
					pkt + 2, ntohs(*(ushort *)pkt));
		puts ("Starting again\n\n");
		NetStartAgain ();
		break;
	}
}

static uchar ticker_cnt = 0;
static uchar ticker[] = {'-', '\\', '|', '/'};

static void
TftpTimeout (void)
{
	/* In case this gets called while fwupdate is in progress */
	if (TftpServerState == TFTP_SRV_STATE_FWUPDATE) {
		return;
	}

	if (TftpServerState == TFTP_SRV_STATE_CONNECT) {
#ifdef CONFIG_SHOW_BOOT_PROGRESS
		call_boot_progress_app((ticker_cnt % 2) ? -5 : -6);
#endif
		/* we are waiting for new connection */
		putc(ticker[ticker_cnt]);
		putc('\b');
		if (++ticker_cnt >= (sizeof(ticker)/sizeof(ticker[0])))
			ticker_cnt = 0;

		NetSetTimeout (TIMEOUT * CFG_HZ, TftpTimeout);
		TftpServer ();
		return;
	}

	if (++TftpTimeoutCount > TIMEOUT_COUNT) {
		puts ("\nRetry count exceeded; starting again\n");
		NetStartAgain ();
	} else {
		/* retry */
		puts ("T ");
		NetSetTimeout (TIMEOUT * CFG_HZ, TftpTimeout);
		TftpServer ();
	}
}

#ifdef	UBNT_USE_WATCHDOG
extern void ubnt_wd_disable();
#endif

void
TftpServerStart (void)
{
#ifdef	UBNT_USE_WATCHDOG
	ubnt_wd_disable();
#endif

	puts("Starting TFTP server...\n");

#if defined(CONFIG_NET_MULTI)
	printf ("Using %s ", eth_get_name());
#endif
	puts ("(");	print_IPaddr (NetOurIP); puts ("), address: ");
	printf ("0x%lx\n", load_addr);

	TftpServerState = TFTP_SRV_STATE_CONNECT;
	TftpOurPort = WELL_KNOWN_PORT;
	TftpBlock = 0;
	TftpTimeoutCount = 0;
	TftpClientPort = 0;
	TftpLastTimer = get_timer(0);

	run_command("go ${ubntaddr} uhandlereset", 0);

	puts ("Waiting for connection: *\b");

	NetSetTimeout (TIMEOUT * CFG_HZ, TftpTimeout);
	NetSetHandler (TftpServerHandler);

	/* zero out server ether in case the server ip has changed */
	memset(NetServerEther, 0, 6);
}

#endif /* CFG_CMD_NET */
