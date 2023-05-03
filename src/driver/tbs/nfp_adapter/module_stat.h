/*
 *
 *  Copyright (C) 2007 Mindspeed Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __MODULE_STAT_H__
#define __MODULE_STAT_H__

/*Function codes*/
/* 0x00xx : Stat module */
#define CMD_STAT_ENABLE        			0x0E01 
#define CMD_STAT_QUEUE         			0x0E02  
#define CMD_STAT_INTERFACE_PKT              	0x0E03
#define CMD_STAT_CONNECTION           		0x0E04
#define CMD_STAT_PPPOE_STATUS			0x0E05
#define CMD_STAT_PPPOE_ENTRY			0x0E06
#define CMD_STAT_BRIDGE_STATUS			0x0E07
#define CMD_STAT_BRIDGE_ENTRY			0x0E08
#define CMD_STAT_IPSEC_STATUS                  0x0E09
#define CMD_STAT_IPSEC_ENTRY                   0x0E0A
#define CMD_STAT_BRIDGE_INSTANCE         0x0E0B

#define CMM_STAT_RESET  0x0001
#define CMM_STAT_QUERY 0x0002
#define CMM_STAT_QUERY_RESET 0x0003

#define CMM_STAT_ENABLE		0x0001
#define CMM_STAT_DISABLE	0x0000

/* Definitions of Bit Masks for the features */
#define STAT_QUEUE_BITMASK 		0x00000001
#define STAT_INTERFACE_BITMASK 		0x00000002
#define STAT_PPPOE_BITMASK 		0x00000008
#define STAT_BRIDGE_BITMASK 		0x00000010
#define STAT_IPSEC_BITMASK 		0x00000020

#define STAT_UNKNOWN_CMD	                0
#define STAT_ENABLE_CMD                      1 
#define	STAT_QUEUE_CMD	                2
#define STAT_INTERFACE_PKT_CMD		3
#define STAT_CONNECTION_CMD			4
#define STAT_PPPOE_CMD                       5
#define STAT_BRIDGE_CMD                      6
#define STAT_IPSEC_CMD                       7


typedef struct _tStatEnableCmd {
	unsigned short action; /* 1 - Enable, 0 - Disable */
	unsigned int bitmask; /* Specifies the feature to be enabled or disabled */ 
}StatEnableCmd, *PStatEnableCmd;

typedef struct _tStatQueueCmd {
	unsigned short action; /* Reset, Query, Query & Reset */
	unsigned short interface;
	unsigned short queue;
}StatQueueCmd;

typedef struct _tStatInterfaceCmd {
	unsigned short action; /* Reset, Query, Query & Reset */
	unsigned short interface;
}StatInterfaceCmd;

typedef struct _tStatConnectionCmd {
	unsigned short action; /* Reset, Query, Query & Reset */
}StatConnectionCmd, *PStatConnectionCmd;

typedef struct _tStatPPPoEStatusCmd {
	unsigned short action; /* Reset, Query, Query & Reset */
}StatPPPoEStatusCmd, *PStatPPPoEStatusCmd;

typedef struct _tStatBridgeStatusCmd {
	unsigned short action; /* Reset, Query, Query & Reset */
}StatBridgeStatusCmd, *PStatBridgeStatusCmd;

typedef struct _tStatIpsecStatusCmd {
       unsigned short action; /* Reset, Query, Query & Reset */
}StatIpsecStatusCmd, *PStatIpsecStatusCmd;



typedef struct _tStatQueueResponse {
	unsigned short ackstatus; 
	unsigned short rsvd1;
	unsigned int peak_queue_occ; 
	unsigned int emitted_pkts; 
	unsigned int dropped_pkts; 
}StatQueueResponse, *PStatQueueResponse;

typedef struct _tStatInterfacePktResponse {
	unsigned short ackstatus;
	unsigned short rsvd1;
	unsigned int total_pkts_transmitted;
	unsigned int total_pkts_received;
	unsigned int total_bytes_transmitted[2]; /* 64 bit counter stored as 2*32 bit counters */ 
	unsigned int total_bytes_received[2]; /* 64 bit counter stored as 2*32 bit counters */
}StatInterfacePktResponse, *PStatInterfacePktResponse;

typedef struct _tStatConnResponse {
	unsigned short ackstatus;
	unsigned short rsvd1;
	unsigned int max_active_connections;
	unsigned int num_active_connections;
}StatConnResponse, *PStatConnResponse;

typedef struct _tStatPPPoEStatusResponse {
	unsigned short ackstatus;
}StatPPPoEStatusResponse, *PStatPPPoEStatusResponse;

typedef struct _tStatPPPoEEntryResponse {
	unsigned short ackstatus;
	unsigned short eof;
	unsigned short sessionID;
	unsigned short interface_no; /* 0 for eth0 & 1 for eth2 */
	unsigned int total_packets_received;  
	unsigned int total_packets_transmitted; 
}StatPPPoEEntryResponse, *PStatPPPoEEntryResponse;


typedef struct _tStatBridgeStatusResponse {
	unsigned short ackstatus;
}StatBridgeStatusResponse, *PStatBridgeStatusResponse;


typedef struct _tStatBridgeEntryResponse {
	unsigned short ackstatus;
	unsigned short eof;
	unsigned short input_interface;
	unsigned short input_svlan; 
	unsigned short input_cvlan; 
	unsigned char dst_mac[6];
	unsigned char src_mac[6];
	unsigned short etherType;
	unsigned short output_interface;
	unsigned short output_svlan; 
	unsigned short output_cvlan; 
	unsigned short rsvd1;
	unsigned int total_packets_transmitted; 
}StatBridgeEntryResponse, *PStatBridgeEntryResponse;

typedef struct _tStatIpsecEntryResponse {
	unsigned short ackstatus;
	unsigned short eof;
	unsigned short family;
	unsigned short proto;
	unsigned int spi;
	unsigned int dstIP[4];
        unsigned int total_pkts_processed;
        unsigned int total_bytes_processed[2];
}StatIpsecEntryResponse, *PStatIpsecEntryResponse;

extern int StatActionFun(unsigned short action , unsigned int bitmask);
extern int BridgeStatusFun( unsigned short action );

#endif

