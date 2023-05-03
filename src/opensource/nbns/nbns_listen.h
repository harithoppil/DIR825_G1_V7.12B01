#ifndef NBNS_LISTEN_H_
#define NBNS_LISTEN_H_

#define NAME_SIZE 255
#define uint8							unsigned char
#define uint16							unsigned short
#define uint32							unsigned int
#define int8							char
#define int16							short
#define int32							int


#define ETHER_ADDR_LEN				6
struct	ether_header {
	uint8	ether_dhost[ETHER_ADDR_LEN];
	uint8	ether_shost[ETHER_ADDR_LEN];
	uint16	ether_type;
};

struct dns_rr{//
	char name[NAME_SIZE];
	uint16 type;
	uint16 class;
	uint32 ttl;
	uint16 rdatalen;
	char data[NAME_SIZE];
};

union header_flags {
  uint16 flags;
  struct {
    unsigned short int question:1;
    unsigned short int opcode:4;
    unsigned short int authorative:1;
    unsigned short int truncated:1;
    unsigned short int want_recursion:1;
    unsigned short int recursion_avail:1;
    unsigned short int unused:3;
    unsigned short int rcode:4;
  } f;
};
/*****************************************************************************/
struct dns_header_s{
  uint16 id;
  union header_flags flags;
  uint16 qdcount;
  uint16 ancount;
  uint16 nscount;
  uint16 arcount;
};
/*****************************************************************************/
struct dns_message{
  struct dns_header_s header;
  struct dns_rr question[5];
  struct dns_rr answer[5];
};
#if 1
/*****************************************************************************/
struct nbns_query
{
	char	blank;
	char	name[33];
	unsigned short type;
	unsigned short ClassType;
}__attribute__ ( (__packed__) );
/***********************************************************************/
struct nbns_answer
{
	char	blank;
	char	name[33];
	unsigned short type;
	unsigned short ClassType;
	unsigned int ttl;
	unsigned short datalen;
	unsigned short flags;
	unsigned	char ip[4];
}__attribute__ ( (__packed__) );
/*****************************************************************************/
struct nbns_req{
	unsigned short id;
	unsigned short flags;
	unsigned short question;
	unsigned short answer;
	unsigned short auth;
	unsigned short add;
	struct nbns_query queries;
}__attribute__ ( (__packed__) );
/***************************************************************************/
struct nbns_ans{
	unsigned short id;
	unsigned short flags;
	unsigned short question;
	unsigned short answer;
	unsigned short auth;
	unsigned short add;
	struct nbns_answer answers;
}__attribute__ ( (__packed__) );


struct llmnr_answer
{
	uint8  len;
	char*	name;
	unsigned short type;
	unsigned short ClassType;
	unsigned int ttl;
	unsigned short datalen;
	unsigned short flags;
	unsigned	char ip[4];
}__attribute__ ( (__packed__) );

struct llmnr_ans{
	unsigned short id;
	unsigned short flags;
	unsigned short question;
	unsigned short answer;
	unsigned short auth;
	unsigned short add;
	struct llmnr_answer answers;
}__attribute__ ( (__packed__) );

struct llmnr_query
{
	uint8  len;
	char   *name;
	unsigned short type;
	unsigned short ClassType;
}__attribute__ ( (__packed__) );

struct llmnr_req{
	unsigned short id;
	unsigned short flags;
	unsigned short question;
	unsigned short answer;
	unsigned short auth;
	unsigned short add;
	struct llmnr_query queries;
}__attribute__ ( (__packed__) );
#endif
/* TYPE values */
enum{ A = 1,      /* a host address */
	NS,       /* an authoritative name server */
	MD,       /* a mail destination (Obsolete - use MX) */
	MF,       /* */
	CNAME,    /* the canonical name for an alias */
	SOA,      /* marks the start of a zone of authority  */
	MB,       /* a mailbox domain name (EXPERIMENTAL) */
	MG,       /* */
	MR,       /* */
	NUL,      /* */
	WKS,      /* a well known service description */
	PTR,      /* a domain name pointer */
	HINFO,    /* host information */
	MINFO,    /* mailbox or mail list information */
	MX,       /* mail exchange */
	TXT,      /* text strings */

	AAA = 0x1c /* IPv6 A */
	};

/* CLASS values */
enum{
  IN = 1,         /* the Internet */
    CS,
    CH,
    HS
};

/* OPCODE values */
enum{
  QUERY,
    IQUERY,
    STATUS
};

/* Response codes */
enum {
	DNS_NO_ERROR,
	DNS_FMT_ERROR,
	DNS_SRVR_FAIL,
	DNS_NAME_ERR,
	DNS_NOT_IMPLEMENTED,
	DNS_REFUSED
};

#endif
