/*
    Mac-Telnet - Connect to RouterOS routers via MAC address
    Copyright (C) 2010, Håkon Nessjøen <haakon.nessjoen@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef _MACTELNET_H
#define _MACTELNET_H 1

#define MT_HEADER_LEN 22
#define MT_CPHEADER_LEN 9

#define MT_PACKET_LEN 1500

#define MT_MACTELNET_PORT 20561

#define MT_MNDP_PORT 5678
#define MT_MNDP_MAX_STRING_LENGTH 128
#define MT_MNDP_BROADCAST_INTERVAL 30

#define MT_MNDP_TIMEOUT 5
#define MT_MNDP_LONGTIMEOUT 120

/* Packet type */
enum mt_ptype {
	MT_PTYPE_SESSIONSTART,
	MT_PTYPE_DATA,
	MT_PTYPE_ACK,
	MT_PTYPE_PING = 4,
	MT_PTYPE_PONG,
	MT_PTYPE_END = 255
};

/* Control packet type */
enum mt_cptype {
	MT_CPTYPE_BEGINAUTH,
	MT_CPTYPE_ENCRYPTIONKEY,
	MT_CPTYPE_PASSWORD,
	MT_CPTYPE_USERNAME,
	MT_CPTYPE_TERM_TYPE,
	MT_CPTYPE_TERM_WIDTH,
	MT_CPTYPE_TERM_HEIGHT,
	MT_CPTYPE_PACKET_ERROR,
	MT_CPTYPE_END_AUTH = 9,
	/* Internal CPTYPE, not part of protocol */
	MT_CPTYPE_PLAINDATA = -1
};

/* MNDP attribute type */
enum mt_mndp_attrtype {
	MT_MNDPTYPE_ADDRESS   = 0x0001,
	MT_MNDPTYPE_IDENTITY  = 0x0005,
	MT_MNDPTYPE_VERSION   = 0x0007,
	MT_MNDPTYPE_PLATFORM  = 0x0008,
	MT_MNDPTYPE_TIMESTAMP = 0x000a,
	MT_MNDPTYPE_SOFTID    = 0x000b,
	MT_MNDPTYPE_HARDWARE  = 0x000c
};

/* MNDP packet header */
struct mt_mndp_hdr {
  unsigned char version;
  unsigned char ttl;
  unsigned short cksum;
};

struct mt_mactelnet_hdr {
	unsigned char ver;
	enum mt_ptype ptype;
	unsigned char clienttype[2];
	unsigned char srcaddr[6];
	unsigned char dstaddr[6];
	unsigned short seskey;
	unsigned int counter;
	unsigned char *data;
};

struct mt_mactelnet_control_hdr {
	enum mt_cptype cptype;
	unsigned int length;
	unsigned char *data;
};

/* TODO: Add all the other information obtainable from mndp */
struct mt_mndp_info {
	struct mt_mndp_hdr header;
	unsigned char address[ETH_ALEN];
	char identity[MT_MNDP_MAX_STRING_LENGTH];
	char version[MT_MNDP_MAX_STRING_LENGTH];
	char platform[MT_MNDP_MAX_STRING_LENGTH];
	char hardware[MT_MNDP_MAX_STRING_LENGTH];
	char softid[MT_MNDP_MAX_STRING_LENGTH];
	unsigned int uptime;
};

struct mt_packet {
	int size;
	unsigned char data[MT_PACKET_LEN];
};

/* MacTelnet/Winbox packets */
extern int init_packet(struct mt_packet *packet, enum mt_ptype ptype, unsigned char *srcmac, unsigned char *dstmac, unsigned short sessionkey, unsigned int counter);
extern int add_control_packet(struct mt_packet *packet, enum mt_cptype cptype, void *cpdata, int data_len);
extern void parse_packet(unsigned char *data, struct mt_mactelnet_hdr *pkthdr);
extern int parse_control_packet(unsigned char *data, int data_len, struct mt_mactelnet_control_hdr *cpkthdr);

/* MAC-Ping packets */
int init_pingpacket(struct mt_packet *packet, unsigned char *srcmac, unsigned char *dstmac);
int init_pongpacket(struct mt_packet *packet, unsigned char *srcmac, unsigned char *dstmac);
int add_packetdata(struct mt_packet *packet, unsigned char *data, unsigned short length);

/* MNDP packets */
extern int mndp_init_packet(struct mt_packet *packet, unsigned char version, unsigned char ttl);
extern int mndp_add_attribute(struct mt_packet *packet, enum mt_mndp_attrtype attrtype, void *attrdata, unsigned short data_len);

extern struct mt_mndp_info *parse_mndp(const unsigned char *data, const int packet_len);
int query_mndp(const char *identity, unsigned char *mac);

/* Number of milliseconds between each retransmission */
#define MAX_RETRANSMIT_INTERVALS 9
static const int retransmit_intervals[MAX_RETRANSMIT_INTERVALS] = { 15, 20, 30, 50, 90, 170, 330, 660, 1000 };

/* Control packet magic header */
static const unsigned char mt_mactelnet_cpmagic[4] = { 0x56, 0x34, 0x12, 0xff };
static const unsigned char mt_mactelnet_clienttype[2] = { 0x00, 0x15 };

/* Must be initialized by application */
extern unsigned char mt_direction_fromserver;


#endif
