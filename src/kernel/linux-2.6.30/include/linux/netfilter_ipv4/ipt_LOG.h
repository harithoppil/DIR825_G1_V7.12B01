#ifndef _IPT_LOG_H
#define _IPT_LOG_H

/* make sure not to change this without changing netfilter.h:NF_LOG_* (!) */
#define IPT_LOG_TCPSEQ		0x01	/* Log TCP sequence numbers */
#define IPT_LOG_TCPOPT		0x02	/* Log TCP options */
#define IPT_LOG_IPOPT		0x04	/* Log IP options */
#define IPT_LOG_UID		0x08	/* Log UID owning local socket */
#define IPT_LOG_NFLOG		0x10	/* Unsupported, don't reuse */
#define IPT_LOG_MACDECODE	0x20	/* Decode MAC header */
#if 0
#define IPT_LOG_MASK		0x2f
#else
#define IPT_LOG_SYSEVENT    0x30
#define IPT_LOG_MASK		0x3f
#endif

struct ipt_log_info {
	unsigned char level;
	unsigned char logflags;
	char prefix[30];
    char logsysevent[128];
};

#endif /*_IPT_LOG_H*/
