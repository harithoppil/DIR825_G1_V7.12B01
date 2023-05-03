
/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp_skb_parser.c
* 文件描述 : tbs加速器数据包解析文件
*
* 修订记录 :
*          1 创建 : cairong
*            日期 : 2011-12-03
*            描述 :
*
*******************************************************************************/
#include "tbs_nfp_common.h"
#include "tbs_nfp.h"
#include "tbs_nfp_skb_parser.h"


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/
#ifndef TBS_NFP_VLAN
#define TBS_NFP_VLAN
#endif

#ifndef TBS_NFP_PPP
#define TBS_NFP_PPP
#endif

//调试总开关
#if defined(CONFIG_TBS_NFP_DEBUG)

/*调试子模块开关 skb_parser */
//#define TBS_NFP_DEBUG_SKB_PARSER
#ifdef TBS_NFP_DEBUG_SKB_PARSER
//打印接收的skb
#define TBS_NFP_DEBUG_DUMP_PACKET(skb,mac_hdr)         tbs_nfp_dump_packet(skb,mac_hdr)
//跟踪解析器的执行
#define TBS_NFP_DEBUG_PARSER_TRACE(fmt, args...)       TBS_NFP_DEBUG(fmt,##args)
//验证包解析是否正确
#define TBS_NFP_DEBUG_CHECK_SKBPARSER(skb_desc)        tbs_nfp_debug_check_skbparser(skb_desc)
#else
#define TBS_NFP_DEBUG_DUMP_PACKET(skb,mac_hdr)         if(0){tbs_nfp_dump_packet(skb,mac_hdr);}
#define TBS_NFP_DEBUG_PARSER_TRACE(fmt, args...)
#define TBS_NFP_DEBUG_CHECK_SKBPARSER(skb_desc)        if(0){tbs_nfp_debug_check_skbparser(skb_desc);}
#endif /*#ifdef TBS_NFP_DEBUG_SKB_PARSER*/

/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/


static void tbs_nfp_dump_ipv6hdr_info(const struct ipv6hdr *ipv6_header);
static void tbs_nfp_dump_ipv4hdr_info(const struct iphdr *ip_header);
static int ip6t_ext_hdr(u8 nexthdr);
static void tbs_nfp_dump_L4_info(const struct sk_buff *skb,
            const unsigned char *mac_header,
            const __u16 l3_protocol,
            const unsigned int iphoff);
static void tbs_nfp_dump_mac(const unsigned char *mac_header);

/*
包解析器开发阶段，测试解析器是否正确解析了包
数据包解析后填充了skb_desc结构，
此函数根据skb_desc结构打印出数据包各种基本信息
*/
void tbs_nfp_debug_check_skbparser(struct tbs_nfp_rx_desc* skb_desc)
{
    unsigned char *mac_header = NULL;
    struct vlan_hdr *vhdr = NULL;
    struct pppoe_hdr *ppp_header = NULL;
    struct iphdr  *ip_header = NULL;
    struct ipv6hdr *ipv6_header = NULL;
    struct tcphdr *tcpudphdr = NULL;
    unsigned int l3_offset;
    unsigned int status = 0;
    unsigned int uivlan_n = 0;

    status = skb_desc->status;
    mac_header = skb_desc->mac_header;
    l3_offset = skb_desc->l3_offset;

    if(!(status&TBS_NFP_RX_PARSER_MASK))
    {
        printk("can not parse this packet!\n");
        return;
    }

    if(NULL == mac_header)
    {
       TBS_NFP_ERROR("***********ERROR!!!************");
       return;
    }

   	printk("MAC=");
    {
		int i;
		const unsigned char *p = mac_header;
		for (i = 0; i < 14; i++,p++)
			printk("%02x%c", *p,
			       i==14 - 1
			       ? ' ':':');
	}

    printk("status: %0x ",skb_desc->status);
    printk("l3_offset: %u ",skb_desc->l3_offset);
    printk("shift: %u ",skb_desc->shift);

    if(TBS_NFP_RX_IS_VLAN(status))
    {
        uivlan_n = 1;
        vhdr = (struct vlan_hdr *)(mac_header+14);
        printk("vlanId: %u ",htons(vhdr->h_vlan_TCI) & VLAN_VID_MASK);
    }

    if(TBS_NFP_RX_IS_QINQ(status))
    {
        uivlan_n = 2;
        vhdr = (struct vlan_hdr *)(mac_header+14);
        printk("vlanId1: %u ",htons(vhdr->h_vlan_TCI) & VLAN_VID_MASK);

        vhdr = (struct vlan_hdr *)((unsigned char*)vhdr+sizeof(struct vlan_hdr));
        printk("vlanId2: %u ",htons(vhdr->h_vlan_TCI) & VLAN_VID_MASK);
    }

    if(TBS_NFP_RX_IS_MORE_QINQ(status))
    {
        vhdr = (struct vlan_hdr *)(mac_header+14);

        printk("more vlan tags!! ");

        printk("vlanId1: %u ",htons(vhdr->h_vlan_TCI) & VLAN_VID_MASK);

        vhdr = (struct vlan_hdr *)((unsigned char*)vhdr+sizeof(struct vlan_hdr));
        printk("vlanId2: %u ",htons(vhdr->h_vlan_TCI) & VLAN_VID_MASK);
    }

    if(TBS_NFP_RX_IS_PPP(status))
    {
        ppp_header = (struct pppoe_hdr *)((unsigned char*)vhdr+sizeof(struct vlan_hdr));
        printk("PPP_session: %u ",ppp_header->sid);
    }

    if(TBS_NFP_RX_L3_IS_IP4(status))
    {
        ip_header =(struct iphdr*)(mac_header + l3_offset);
        if(ip_header)
        printk("SRC=%pI4 DST=%pI4 ",
               &ip_header->saddr, &ip_header->daddr);

    }

    if(TBS_NFP_RX_L3_IS_IP6(status))
    {
        ipv6_header = (struct ipv6hdr*)(mac_header + l3_offset);
        if(ipv6_header)
            printk("SRC=%pI6 DST=%pI6 ", &ipv6_header->saddr, &ipv6_header->daddr);
    }

    if(TBS_NFP_RX_L3_UNKNOW(status))
    {
        printk(" L3_UNKNOW packet!");
    }

    if(TBS_NFP_RX_L4_IS_TCP(status))
    {
        if(TBS_NFP_RX_L3_IS_IP4(status))
        {
            tcpudphdr = (struct tcphdr *)(mac_header+l3_offset+sizeof(struct iphdr));
        }
        else if(TBS_NFP_RX_L3_IS_IP6(status))
        {
            tcpudphdr = (struct tcphdr *)(mac_header+l3_offset+sizeof(struct ipv6hdr));
        }
        else
        {
            printk("***********ERROR!!!************");
        }

        printk("tcp ");
        if(tcpudphdr)
            printk("sport:%u dport:%u ",ntohs(tcpudphdr->source),ntohs(tcpudphdr->dest));
    }

    if(TBS_NFP_RX_L4_IS_UDP(status))
    {
        if(TBS_NFP_RX_L3_IS_IP4(status))
        {
            tcpudphdr = (struct tcphdr *)(mac_header+l3_offset+sizeof(struct iphdr));
        }
        else if(TBS_NFP_RX_L3_IS_IP6(status))
        {
            tcpudphdr = (struct tcphdr *)(mac_header+l3_offset+sizeof(struct ipv6hdr));
        }
        else
        {
            printk("***********ERROR!!!************");
        }

        printk("udp ");
        if(tcpudphdr)
             printk("sport:%u dport:%u ",ntohs(tcpudphdr->source),ntohs(tcpudphdr->dest));
    }

    if(TBS_NFP_RX_L4_UNKNOW(status))
    {
        printk(" L4_UNKNOW packet!");
    }
    printk("\n");
}
/*
打印 ipv6 地址信息
*/
static void tbs_nfp_dump_ipv6hdr_info(const struct ipv6hdr *ipv6_header)
{
    if(ipv6_header)
    {
    	/* Max length: 88 "SRC=0000.0000.0000.0000.0000.0000.0000.0000 DST=0000.0000.0000.0000.0000.0000.0000.0000 " */
    	printk("SRC=%pI6 DST=%pI6 ", &ipv6_header->saddr, &ipv6_header->daddr);

    	/* Max length: 44 "LEN=65535 TC=255 HOPLIMIT=255 FLOWLBL=FFFFF " */
    	printk("LEN=%Zu TC=%u HOPLIMIT=%u FLOWLBL=%u ",
    	       ntohs(ipv6_header->payload_len) + sizeof(struct ipv6hdr),
    	       (ntohl(*(__be32 *)ipv6_header) & 0x0ff00000) >> 20,
    	       ipv6_header->hop_limit,
    	       (ntohl(*(__be32 *)ipv6_header) & 0x000fffff));
    }
}

static int ip6t_ext_hdr(u8 nexthdr)
{
	return ( (nexthdr == IPPROTO_HOPOPTS)   ||
		 (nexthdr == IPPROTO_ROUTING)   ||
		 (nexthdr == IPPROTO_FRAGMENT)  ||
		 (nexthdr == IPPROTO_ESP)       ||
		 (nexthdr == IPPROTO_AH)        ||
		 (nexthdr == IPPROTO_NONE)      ||
		 (nexthdr == IPPROTO_DSTOPTS) );
}

/*
打印 ipv4 基本信息
*/
static void tbs_nfp_dump_ipv4hdr_info(const struct iphdr *ip_header)
{
    if(ip_header)
    {
        /* Important fields:
         * TOS, len, DF/MF, fragment offset, TTL, src, dst, options. */
        /* Max length: 40 "SRC=255.255.255.255 DST=255.255.255.255 " */
        printk("SRC=%pI4 DST=%pI4 ",
               &ip_header->saddr, &ip_header->daddr);

        /* Max length: 46 "LEN=65535 TOS=0xFF PREC=0xFF TTL=255 ID=65535 " */
        printk("LEN=%u TOS=0x%02X PREC=0x%02X TTL=%u ID=%u ",
               ntohs(ip_header->tot_len), ip_header->tos & IPTOS_TOS_MASK,
               ip_header->tos & IPTOS_PREC_MASK, ip_header->ttl, ntohs(ip_header->id));

        /* Max length: 6 "CE DF MF " */
        if (ntohs(ip_header->frag_off) & IP_CE)
        	printk("CE ");
        if (ntohs(ip_header->frag_off) & IP_DF)
        	printk("DF ");
        if (ntohs(ip_header->frag_off) & IP_MF)
        	printk("MF ");

        /* Max length: 11 "FRAG:65535 " */
        if (ntohs(ip_header->frag_off) & IP_OFFSET)
        	printk("FRAG:%u ", ntohs(ip_header->frag_off) & IP_OFFSET);

        if (ip_header->ihl * 4 > sizeof(struct iphdr)) {
        	const unsigned char *op;
        	unsigned int i, optsize;

        	optsize = ip_header->ihl * 4 - sizeof(struct iphdr);
        	op = (unsigned char*)ip_header + sizeof(struct iphdr);
        	if (op == NULL) {
        		printk("TRUNCATED");
        		return;
        	}

        	/* Max length: 127 "OPT (" 15*4*2chars ") " */
        	printk("OPT (");
        	for (i = 0; i < optsize; i++)
        		printk("%02X", op[i]);
        	printk(") ");
        }
    }
 }



/*
打印 L4传输层信息
*/
static void tbs_nfp_dump_L4_info(const struct sk_buff *skb,
     const unsigned char *mac_header,
     const __u16 l3_protocol,
     const unsigned int iphoff)
{
    struct iphdr  *ih = NULL;
    unsigned int l4offset;

    u_int8_t currenthdr;
	int fragment;
	const struct ipv6hdr *ih6;
	unsigned int hdrlen = 0;

    if(ETH_P_IP == l3_protocol)
    {
          ih = (struct iphdr*)(mac_header+iphoff);
           switch(ih->protocol)
           {
        	case IPPROTO_TCP: {
        		//struct tcphdr _tcph;
        		const struct tcphdr *th;

        		/* Max length: 10 "PROTO=TCP " */
        		printk("PROTO=TCP ");

        		if (ntohs(ih->frag_off) & IP_OFFSET)
        			break;

        		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
        		th = (struct tcphdr *)(mac_header+iphoff +ih->ihl * 4);
                //skb_header_pointer(skb, iphoff + ih->ihl * 4,
        		//			sizeof(_tcph), &_tcph);
        		if (th == NULL) {
        			printk("INCOMPLETE [%u bytes] ",
        			       skb->len - iphoff - ih->ihl*4);
        			break;
        		}

        		/* Max length: 20 "SPT=65535 DPT=65535 " */
        		 printk("SPT=%u DPT=%u ",
        		       ntohs(th->source), ntohs(th->dest));
        		/* Max length: 30 "SEQ=4294967295 ACK=4294967295 " */
        		//if (logflags & IPT_LOG_TCPSEQ)
        		 	printk("SEQ=%u ACK=%u ",
        		 	       ntohl(th->seq), ntohl(th->ack_seq));
        		/* Max length: 13 "WINDOW=65535 " */
        		 printk("WINDOW=%u ", ntohs(th->window));
        		/* Max length: 9 "RES=0x3F " */
        		 printk("RES=0x%02x ", (u8)(ntohl(tcp_flag_word(th) & TCP_RESERVED_BITS) >> 22));
        		/* Max length: 32 "CWR ECE URG ACK PSH RST SYN FIN " */
        		if (th->cwr)
        			printk("CWR ");
        		if (th->ece)
        			printk("ECE ");
        		if (th->urg)
        			printk("URG ");
        		if (th->ack)
        			printk("ACK ");
        		if (th->psh)
        			printk("PSH ");
        		if (th->rst)
        			printk("RST ");
        		if (th->syn)
        			printk("SYN ");
        		if (th->fin)
        			printk("FIN ");
        		/* Max length: 11 "URGP=65535 " */
        		printk("URGP=%u ", ntohs(th->urg_ptr));
        		break;
        	}
        	case IPPROTO_UDP:
        	case IPPROTO_UDPLITE: {
        		const struct udphdr *uh;

        		if (ih->protocol == IPPROTO_UDP)
        			/* Max length: 10 "PROTO=UDP "     */
        			printk("PROTO=UDP " );
        		else	/* Max length: 14 "PROTO=UDPLITE " */
        			printk("PROTO=UDPLITE ");

        		if (ntohs(ih->frag_off) & IP_OFFSET)
        			break;

        		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
        		uh = (struct udphdr *)(mac_header+iphoff+ih->ihl*4);
        		if (uh == NULL) {
        			printk("INCOMPLETE [%u bytes] ",
        			       skb->len - iphoff - ih->ihl*4);
        			break;
        		}

        		/* Max length: 20 "SPT=65535 DPT=65535 " */
        		printk("SPT=%u DPT=%u LEN=%u ",
        		       ntohs(uh->source), ntohs(uh->dest),
        		       ntohs(uh->len));
        		break;
        	}
        	case IPPROTO_ICMP: {
        		const struct icmphdr *ich;
        		static const size_t required_len[NR_ICMP_TYPES+1]
        			= { [ICMP_ECHOREPLY] = 4,
        			    [ICMP_DEST_UNREACH]
        			    = 8 + sizeof(struct iphdr),
        			    [ICMP_SOURCE_QUENCH]
        			    = 8 + sizeof(struct iphdr),
        			    [ICMP_REDIRECT]
        			    = 8 + sizeof(struct iphdr),
        			    [ICMP_ECHO] = 4,
        			    [ICMP_TIME_EXCEEDED]
        			    = 8 + sizeof(struct iphdr),
        			    [ICMP_PARAMETERPROB]
        			    = 8 + sizeof(struct iphdr),
        			    [ICMP_TIMESTAMP] = 20,
        			    [ICMP_TIMESTAMPREPLY] = 20,
        			    [ICMP_ADDRESS] = 12,
        			    [ICMP_ADDRESSREPLY] = 12 };

        		/* Max length: 11 "PROTO=ICMP " */
        		printk("PROTO=ICMP ");

        		if (ntohs(ih->frag_off) & IP_OFFSET)
        			break;

        		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
        		ich = (struct icmphdr *)(mac_header+iphoff + ih->ihl * 4);
        		if (ich == NULL) {
        			printk("INCOMPLETE [%u bytes] ",
        			       skb->len - iphoff - ih->ihl*4);
        			break;
        		}

        		/* Max length: 18 "TYPE=255 CODE=255 " */
        		printk("TYPE=%u CODE=%u ", ich->type, ich->code);

        		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
        		if (ich->type <= NR_ICMP_TYPES
        		    && required_len[ich->type]
        		    && skb->len-iphoff-ih->ihl*4 < required_len[ich->type]) {
        			printk("INCOMPLETE [%u bytes] ",
        			       skb->len - iphoff - ih->ihl*4);
        			break;
        		}

        		switch (ich->type) {
        		case ICMP_ECHOREPLY:
        		case ICMP_ECHO:
        			/* Max length: 19 "ID=65535 SEQ=65535 " */
        			printk("ID=%u SEQ=%u ",
        			       ntohs(ich->un.echo.id),
        			       ntohs(ich->un.echo.sequence));
        			break;

        		case ICMP_PARAMETERPROB:
        			/* Max length: 14 "PARAMETER=255 " */
        			printk("PARAMETER=%u ",
        			       ntohl(ich->un.gateway) >> 24);
        			break;
        		case ICMP_REDIRECT:
        			/* Max length: 24 "GATEWAY=255.255.255.255 " */
        			printk("GATEWAY=%pI4 ", &ich->un.gateway);
        		break;
        		}
        	}
        	/* Max Length */
        	case IPPROTO_AH: {
       // 		struct ip_auth_hdr _ahdr;
        		const struct ip_auth_hdr *ah;

        		if (ntohs(ih->frag_off) & IP_OFFSET)
        			break;

        		/* Max length: 9 "PROTO=AH " */
        		printk("PROTO=AH ");

        		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
        		ah = (struct ip_auth_hdr *)(mac_header+iphoff+ih->ihl*4);
        		if (ah == NULL) {
        			printk("INCOMPLETE [%u bytes] ",
        			       skb->len - iphoff - ih->ihl*4);
        			break;
        		}

        		/* Length: 15 "SPI=0xF1234567 " */
        		printk("SPI=0x%x ", ntohl(ah->spi));
        		break;
        	}
        	case IPPROTO_ESP: {
        //		struct ip_esp_hdr _esph;
        		const struct ip_esp_hdr *eh;

        		/* Max length: 10 "PROTO=ESP " */
        		printk("PROTO=ESP ");

        		if (ntohs(ih->frag_off) & IP_OFFSET)
        			break;

        		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
        		eh = (struct ip_esp_hdr *)(mac_header+iphoff+ih->ihl*4);
        		if (eh == NULL) {
        			printk("INCOMPLETE [%u bytes] ",
        			       skb->len - iphoff - ih->ihl*4);
        			break;
        		}

        		/* Length: 15 "SPI=0xF1234567 " */
        		printk("SPI=0x%x ", ntohl(eh->spi));
        		break;
        	}
        	/* Max length: 10 "PROTO 255 " */
        	default:
        		printk("PROTO=%u ", ih->protocol);
        	}
    }
    else if(ETH_P_IPV6 == l3_protocol)
    {
        fragment = 0;
	    l4offset = iphoff + sizeof(struct ipv6hdr);
        ih6 = (struct ipv6hdr*)(mac_header+iphoff);
	    currenthdr = ih6->nexthdr;

        while (currenthdr != NEXTHDR_NONE && ip6t_ext_hdr(currenthdr)) {
		//struct ipv6_opt_hdr _hdr;
		const struct ipv6_opt_hdr *hp;

		hp = (struct ipv6_opt_hdr *)(mac_header + l4offset);
		if (hp == NULL) {
			printk("TRUNCATED");
			return;
		}

		/* Max length: 48 "OPT (...) " */
		//if (logflags & IP6T_LOG_IPOPT)
			printk("OPT ( ");

		switch (currenthdr) {
		case IPPROTO_FRAGMENT: {
		//	struct frag_hdr _fhdr;
			const struct frag_hdr *fh;

			printk("FRAG:");
			fh = (struct frag_hdr *)(mac_header + l4offset);
			if (fh == NULL) {
				printk("TRUNCATED ");
				return;
			}

			/* Max length: 6 "65535 " */
			printk("%u ", ntohs(fh->frag_off) & 0xFFF8);

			/* Max length: 11 "INCOMPLETE " */
			if (fh->frag_off & htons(0x0001))
				printk("INCOMPLETE ");

			printk("ID:%08x ", ntohl(fh->identification));

			if (ntohs(fh->frag_off) & 0xFFF8)
				fragment = 1;

			hdrlen = 8;

			break;
		}
		case IPPROTO_DSTOPTS:
		case IPPROTO_ROUTING:
		case IPPROTO_HOPOPTS:
			if (fragment) {
				//if (logflags & IP6T_LOG_IPOPT)
					printk(")");
				return;
			}
			hdrlen = ipv6_optlen(hp);
			break;
		/* Max Length */
		case IPPROTO_AH:
			//if (logflags & IP6T_LOG_IPOPT)
			{
		//		struct ip_auth_hdr _ahdr;
				const struct ip_auth_hdr *ah;

				/* Max length: 3 "AH " */
				printk("AH ");

				if (fragment) {
					printk(")");
					return;
				}

				ah = (struct ip_auth_hdr *)(mac_header + l4offset);
				if (ah == NULL) {
					/*
					 * Max length: 26 "INCOMPLETE [65535
					 *  bytes] )"
					 */
					printk("INCOMPLETE [%u bytes] )",
					       skb->len - l4offset);
					return;
				}

				/* Length: 15 "SPI=0xF1234567 */
				printk("SPI=0x%x ", ntohl(ah->spi));

			}

			hdrlen = (hp->hdrlen+2)<<2;
			break;
		case IPPROTO_ESP:
			//if (logflags & IP6T_LOG_IPOPT)
			{
			//	struct ip_esp_hdr _esph;
				const struct ip_esp_hdr *eh;

				/* Max length: 4 "ESP " */
				printk("ESP ");

				if (fragment) {
					printk(")");
					return;
				}

				/*
				 * Max length: 26 "INCOMPLETE [65535 bytes] )"
				 */
				eh = (struct ip_esp_hdr *)(mac_header + l4offset);
				if (eh == NULL) {
					printk("INCOMPLETE [%u bytes] )",
					       skb->len - l4offset);
					return;
				}

				/* Length: 16 "SPI=0xF1234567 )" */
				printk("SPI=0x%x )", ntohl(eh->spi) );

			}
			return;
		default:
			/* Max length: 20 "Unknown Ext Hdr 255" */
			printk("Unknown Ext Hdr %u", currenthdr);
			return;
		}

		currenthdr = hp->nexthdr;
		l4offset += hdrlen;
	}

	switch (currenthdr) {
	case IPPROTO_TCP: {
	  // struct tcphdr _tcph;
		const struct tcphdr *th;

		/* Max length: 10 "PROTO=TCP " */
		printk("PROTO=TCP ");

		if (fragment)
			break;

		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
		th = (struct tcphdr *)(mac_header + l4offset);
		if (th == NULL) {
			printk("INCOMPLETE [%u bytes] ", skb->len - l4offset);
			return;
		}

		/* Max length: 20 "SPT=65535 DPT=65535 " */
		printk("SPT=%u DPT=%u ",
		       ntohs(th->source), ntohs(th->dest));
		/* Max length: 30 "SEQ=4294967295 ACK=4294967295 " */
		//if (logflags & IP6T_LOG_TCPSEQ)
			printk("SEQ=%u ACK=%u ",
			       ntohl(th->seq), ntohl(th->ack_seq));
		/* Max length: 13 "WINDOW=65535 " */
		printk("WINDOW=%u ", ntohs(th->window));
		/* Max length: 9 "RES=0x3C " */
		printk("RES=0x%02x ", (u_int8_t)(ntohl(tcp_flag_word(th) & TCP_RESERVED_BITS) >> 22));
		/* Max length: 32 "CWR ECE URG ACK PSH RST SYN FIN " */
		if (th->cwr)
			printk("CWR ");
		if (th->ece)
			printk("ECE ");
		if (th->urg)
			printk("URG ");
		if (th->ack)
			printk("ACK ");
		if (th->psh)
			printk("PSH ");
		if (th->rst)
			printk("RST ");
		if (th->syn)
			printk("SYN ");
		if (th->fin)
			printk("FIN ");
		/* Max length: 11 "URGP=65535 " */
		printk("URGP=%u ", ntohs(th->urg_ptr));

		if (th->doff * 4 > sizeof(struct tcphdr)) {
			//u_int8_t _opt[60 - sizeof(struct tcphdr)];
			const u_int8_t *op;
			unsigned int i;
			unsigned int optsize = th->doff * 4
					       - sizeof(struct tcphdr);

			op = (u_int8_t *)(mac_header + l4offset + sizeof(struct tcphdr));
			if (op == NULL) {
				printk("OPT (TRUNCATED)");
				return;
			}

			/* Max length: 127 "OPT (" 15*4*2chars ") " */
			printk("OPT (");
			for (i =0; i < optsize; i++)
				printk("%02X", op[i]);
			printk(") ");
		}
		break;
	}
	case IPPROTO_UDP:
	case IPPROTO_UDPLITE: {
		//struct udphdr _udph;
		const struct udphdr *uh;

		if (currenthdr == IPPROTO_UDP)
			/* Max length: 10 "PROTO=UDP "     */
			printk("PROTO=UDP " );
		else	/* Max length: 14 "PROTO=UDPLITE " */
			printk("PROTO=UDPLITE ");

		if (fragment)
			break;

		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
		uh = (struct udphdr *)(mac_header + l4offset);
		if (uh == NULL) {
			printk("INCOMPLETE [%u bytes] ", skb->len - l4offset);
			return;
		}

		/* Max length: 20 "SPT=65535 DPT=65535 " */
		printk("SPT=%u DPT=%u LEN=%u ",
		       ntohs(uh->source), ntohs(uh->dest),
		       ntohs(uh->len));
		break;
	}
	case IPPROTO_ICMPV6: {
		//struct icmp6hdr _icmp6h;
		const struct icmp6hdr *ic;

		/* Max length: 13 "PROTO=ICMPv6 " */
		printk("PROTO=ICMPv6 ");

		if (fragment)
			break;

		/* Max length: 25 "INCOMPLETE [65535 bytes] " */
		ic = (struct icmp6hdr *)(mac_header + l4offset);
		if (ic == NULL) {
			printk("INCOMPLETE [%u bytes] ", skb->len - l4offset);
			return;
		}

		/* Max length: 18 "TYPE=255 CODE=255 " */
		printk("TYPE=%u CODE=%u ", ic->icmp6_type, ic->icmp6_code);

		switch (ic->icmp6_type) {
		case ICMPV6_ECHO_REQUEST:
		case ICMPV6_ECHO_REPLY:
			/* Max length: 19 "ID=65535 SEQ=65535 " */
			printk("ID=%u SEQ=%u ",
				ntohs(ic->icmp6_identifier),
				ntohs(ic->icmp6_sequence));
			break;
		case ICMPV6_MGM_QUERY:
		case ICMPV6_MGM_REPORT:
		case ICMPV6_MGM_REDUCTION:
			break;

		case ICMPV6_PARAMPROB:
			/* Max length: 17 "POINTER=ffffffff " */
			printk("POINTER=%08x ", ntohl(ic->icmp6_pointer));
			/* Fall through */
		case ICMPV6_DEST_UNREACH:
		case ICMPV6_PKT_TOOBIG:
		case ICMPV6_TIME_EXCEED:
			/* Max length: 10 "MTU=65535 " */
			if (ic->icmp6_type == ICMPV6_PKT_TOOBIG)
				printk("MTU=%u ", ntohl(ic->icmp6_mtu));
		}
		break;
	}
	/* Max length: 10 "PROTO=255 " */
	default:
		printk("PROTO=%u ", currenthdr);
	}

		printk("MARK=0x%x ", skb->mark);
    }

}

static void tbs_nfp_dump_mac(const unsigned char *mac_header)
{
   	printk("MAC=");
    {
		int i;
		const unsigned char *p = mac_header;
		for (i = 0; i < 14; i++,p++)
			printk("%02x%c", *p,
			       i==14 - 1
			       ? ' ':':');
	}
}

/*
通用函数，
打印数据包基本信息
*/
void tbs_nfp_dump_packet(const struct sk_buff *skb,
			const unsigned char *mac_header)
{
      /*指向各层协议类型，依次为vlan,pppoe,ip*/
    __be16 new_proto;
    int ivlan_n;//带几层vlan tag
    unsigned char *new_header = NULL;//指向各层协议头，依次为vlan,pppoe,ip,TCP/UDP

    struct vlan_hdr *vhdr = NULL;
    struct pppoe_hdr *ppp_header = NULL;
    __be16 ptop_protocol;

    struct iphdr  *ip_header = NULL;
    struct ipv6hdr *ipv6_header = NULL;
    unsigned int iphoffmac;

    ivlan_n = 0;
    new_header = (unsigned char *)(mac_header + 14);
    new_proto = *(__be16*)(mac_header+12);

    if (skb == NULL ||mac_header!= skb_mac_header(skb))
    {
        TBS_NFP_DEBUG("mac header changed!! mac_header!= skb_mac_header(skb)!\n");
      //  return TBS_NFP_ERR;
    }

	printk("dev=%s ",skb->dev? skb->dev->name : "");

	printk("pkt_type: ");
    switch(skb->pkt_type)
    {
        case PACKET_HOST:
             printk("PACKET_HOST ");
             break;
        case PACKET_BROADCAST:
             printk("PACKET_BROADCAST ");
             break;
        case PACKET_MULTICAST:
             printk("PACKET_MULTICAST ");
             break;
        case PACKET_OTHERHOST:
             printk("PACKET_OTHERHOST ");
             break;
        case PACKET_OUTGOING:
             printk("PACKET_OUTGOING ");
             break;
        case PACKET_LOOPBACK:
             printk("PACKET_LOOPBACK ");
             break;
        case PACKET_FASTROUTE:
             printk("PACKET_FASTROUTE ");
             break;
        default:
             printk("unkown!! ");
            break;
    }

/*打印mac地址*/
    tbs_nfp_dump_mac(mac_header);
    printk("skb LEN=%u ",skb->len);

/*解析并打印二层vlan*/
    if (ETH_P_8021Q == ntohs(new_proto))
    {
        do{
            vhdr = (struct vlan_hdr *)new_header;
            new_header = (unsigned char *)vhdr + sizeof(struct vlan_hdr);//next header
            new_proto = vhdr->h_vlan_encapsulated_proto;//next proto
            ivlan_n++;
            printk("vlanId%u=%u ",ivlan_n,ntohs(vhdr->h_vlan_TCI) & VLAN_VID_MASK);
        }while (ETH_P_8021Q == ntohs(new_proto));

    }

/*解析二层ppp 和ppp上的三层协议*/
/*打印PPP session*/
/*
L2 ppp解析
ETH_P_PPP_DISC为PPPoE discovery messages，
ETH_P_PPP_SES 为PPoE session messages
只处理数据传输的报文，类型为ETH_P_PPP_SES ,
不处理ppp拨号阶段的报文，ETH_P_PPP_DISC
*/
    if (ETH_P_PPP_SES == ntohs(new_proto))
    {
        ppp_header = (struct pppoe_hdr *)new_header ;

        printk("ppp.sessionId:%u\n",(unsigned int)ppp_header->sid);
        /*
        获取ppp包的l3层协议类型
        注意:pppoe 数据包的协议类型不在pppoe session头内，
        而是pppoe session头后的16个bit位
        */
        ptop_protocol = *(__be16 *)((unsigned char *)ppp_header + sizeof(struct pppoe_hdr));

        // ppp 包L3解析
        if(PPP_IP == ntohs(ptop_protocol))
        {
            ip_header = (struct iphdr *)(new_header
                                 + sizeof(struct pppoe_hdr)
                                 + sizeof(ptop_protocol));
        }
        else if(PPP_IPV6 == ntohs(ptop_protocol))
        {
            ipv6_header = (struct ipv6hdr *)(new_header
                                            + sizeof(struct pppoe_hdr)
                                            + sizeof(ptop_protocol));
        }
        else
        {
     //       printk("parser pkt is ppp L3 unknow\n");
        }

        goto L4_parse;
    }

/*解析三层协议，非pppoe类型*/
    if (ETH_P_IP == ntohs(new_proto))
    {
        ip_header = (struct iphdr *)new_header;
    }
    else if(ETH_P_IPV6 == ntohs(new_proto))
    {
        ipv6_header = (struct ipv6hdr *)new_header;
    }
    else
    {
   //     printk("parser pkt is L3 unkow\n");
    }

/*解析并打印四层协议信息*/
L4_parse:
    if(ip_header)
    {
        /*ip头相对于mac头的偏移位置，大于14*/
         iphoffmac = (const unsigned char *)ip_header - mac_header;
         /*打印三层信息*/
         tbs_nfp_dump_ipv4hdr_info(ip_header);
         /*打印四层信息*/
         tbs_nfp_dump_L4_info(skb,(const unsigned char *)mac_header, ETH_P_IP, iphoffmac);

    }
    else if(ipv6_header)
    {
         /*ipv6头相对于mac头的偏移位置，大于14*/
         iphoffmac = (const unsigned char *)ipv6_header - mac_header;
          /*打印三层信息*/
         tbs_nfp_dump_ipv6hdr_info(ipv6_header);
         /*打印四层信息*/
         tbs_nfp_dump_L4_info(skb,(const unsigned char *)mac_header, ETH_P_IPV6, iphoffmac);

    }
    else
    {
         printk("is not a usable ipv4 or ipv6 packet!\n");
         goto parser_end;
    }

	printk("\n");

parser_end:
       return;
}
#else
#define TBS_NFP_DEBUG_DUMP_PACKET(skb,mac_hdr)
#define TBS_NFP_DEBUG_PARSER_TRACE(fmt, args...)
#define TBS_NFP_DEBUG_CHECK_SKBPARSER(skb_desc)
#endif /* defined(CONFIG_TBS_NFP_DEBUG) */

/*=========================================================================
 Function:		int tbs_nfp_skb_parser(struct sk_buff *skb, struct tbs_nfp_rx_desc *skb_desc)

 Description:		解析接收的skb，获取各层协议类型如:vlan.ppp.ip.tcp.udp等信息，用来填充skb_desc结构。

 Data Accessed:
 Data Updated:

 Input:			    struct sk_buff *skb               接收的skb
                    struct tbs_nfp_rx_desc *skb_desc  包描述结构体指针
 Output:			struct tbs_nfp_rx_desc *skb_desc  包描述结构体指针

 Return:			TBS_NFP_ERR:解析错误
                    TBS_NFP_OK: 解析成功
 Others:
=========================================================================*/
int tbs_nfp_skb_parser(const struct  sk_buff *skb, TBS_NFP_RX_DESC *skb_desc)
{
    /*指向各层协议类型，依次为vlan,pppoe,ip*/
    __be16 new_proto;
    int ivlan_n;//带几层vlan tag
    unsigned char *mac_header = NULL;
    unsigned char *new_header = NULL;//指向各层协议头，依次为vlan,pppoe,ip,TCP/UDP

     struct vlan_hdr *vhdr = NULL;
     struct pppoe_hdr *ppp_header = NULL;
     __be16 ptop_protocol;

    struct iphdr  *ip_header = NULL;
    struct ipv6hdr *ipv6_header = NULL;

    ivlan_n = 0;
    mac_header = skb_mac_header(skb);
    new_header = skb_mac_header(skb)+ ETH_HLEN;
    new_proto = skb->protocol;

    //TBS_NFP_INTO_FUNC;

    if (skb_desc == NULL || skb == NULL )
    {
        TBS_NFP_ERROR("error : args error!!!!!!!!!!!!\n");
        return TBS_NFP_ERR;
    }


    //TBS_NFP_DEBUG_DUMP_PACKET(skb,mac_header);

    // L2 802.1Q VLAN解析
#ifdef TBS_NFP_VLAN
    if (ETH_P_8021Q == ntohs(new_proto))
    {
        TBS_NFP_DEBUG_PARSER_TRACE("parser pkt is vlan\n");

        do {
            vhdr = (struct vlan_hdr *)new_header;
            new_header = (unsigned char *)vhdr + sizeof(struct vlan_hdr);//next header
            new_proto = vhdr->h_vlan_encapsulated_proto;//next protol
            ivlan_n++;
        }while (ETH_P_8021Q == ntohs(new_proto));


        TBS_NFP_DEBUG_PARSER_TRACE("vlan tag number %d !\n",ivlan_n);

        switch(ivlan_n)
        {
            case 1:
                  skb_desc->status |= TBS_NFP_RX_VLAN_FRAME_TYPE;
                  break;

            case 2:
                  skb_desc->status |= TBS_NFP_RX_QINQ_FRAME_TYPE;
                  break;

            default:
                  skb_desc->status |= TBS_NFP_RX_MORE_QINQ_FRAME_TYPE;
                  break;
        }
    }

#endif

/*
   L2 ppp解析
   ETH_P_PPP_DISC为PPPoE discovery messages，
   ETH_P_PPP_SES 为PPoE session messages
*/
#ifdef TBS_NFP_PPP
    /* 只处理数据传输的报文，类型为ETH_P_PPP_SES ,
       不处理ppp拨号阶段的报文，ETH_P_PPP_DISC
    */
    if (ETH_P_PPP_SES == ntohs(new_proto))
    {
        TBS_NFP_DEBUG_PARSER_TRACE("parser pkt is ppp\n");

        skb_desc->status |= TBS_NFP_RX_PPPOE_FRAME_MASK;//记录数据包状态
        ppp_header = (struct pppoe_hdr *)new_header ;

        /*
        获取ppp包的l3层协议类型
        注意:pppoe 数据包的协议类型不在pppoe session头内，
        而是pppoe session头后的16个bit位
        */
        ptop_protocol = *(__be16 *)((unsigned char *)ppp_header + sizeof(struct pppoe_hdr));

        // ppp 包L3解析
        if(PPP_IP == ntohs(ptop_protocol))
        {
            TBS_NFP_DEBUG_PARSER_TRACE("parser pkt is ppp ipv4\n");
            skb_desc->status |= TBS_NFP_RX_IP_FRAME_TYPE_MASK;//记录数据包状态

            //iP头赋值
            ip_header = (struct iphdr *)(new_header + sizeof(struct pppoe_hdr) + sizeof(ptop_protocol));
        }
        else if(PPP_IPV6 == ntohs(ptop_protocol))
        {
            TBS_NFP_DEBUG_PARSER_TRACE("parser pkt is ppp ipv6\n");
            skb_desc->status |= TBS_NFP_RX_IP6_FRAME_TYPE_MASK;//记录数据包状态

            //iPv6头赋值
            ipv6_header = (struct ipv6hdr *)(new_header + sizeof(struct pppoe_hdr) + sizeof(ptop_protocol));
        }
        else
        {
            TBS_NFP_DEBUG_PARSER_TRACE("parser pkt is ppp unknow\n");
            skb_desc->status |= TBS_NFP_RX_L3_UNKNOWN_MASK;//记录数据包状态
        }
    }
    else
#endif
    // L3解析
    if (ETH_P_IP == ntohs(new_proto))
    {
        TBS_NFP_DEBUG_PARSER_TRACE("parser pkt is ipv4\n");
        skb_desc->status |= TBS_NFP_RX_IP_FRAME_TYPE_MASK;//记录数据包状态

        //iP头赋值
        ip_header = (struct iphdr *)new_header;
    }
    else if(ETH_P_IPV6 == ntohs(new_proto))
    {
        TBS_NFP_DEBUG_PARSER_TRACE("parser pkt is ipv6\n");
        skb_desc->status |= TBS_NFP_RX_IP6_FRAME_TYPE_MASK;//记录数据包状态

        //iPv6头赋值
        ipv6_header = (struct ipv6hdr *)new_header;
    }
    else
    {
        TBS_NFP_DEBUG_PARSER_TRACE("parser pkt is L3 unkow\n");
        skb_desc->status |= TBS_NFP_RX_L3_UNKNOWN_MASK;//记录数据包状态
    }

    if ((NULL == ip_header) && (NULL == ipv6_header))
    {
        TBS_NFP_DEBUG_PARSER_TRACE("is not a usable ipv4 or ipv6 packet!\n");
        goto parser_end;
    }

    // L4解析
    if(NULL != ip_header)//ipv4 packet
    {
        if(SOL_TCP == ip_header->protocol)
        {
            skb_desc->status |= TBS_NFP_RX_L4_TCP_TYPE;//记录数据包状态
            TBS_NFP_DEBUG_PARSER_TRACE("parser pkt is TCP\n");
        }
        else if(SOL_UDP == ip_header->protocol)
        {
            skb_desc->status |= TBS_NFP_RX_L4_UDP_TYPE;//记录数据包状态
            TBS_NFP_DEBUG_PARSER_TRACE("parser pkt is udp\n");
        }
        else
        {
            skb_desc->status |= TBS_NFP_RX_L4_UNKNOWN_MASK;//记录数据包状态
            TBS_NFP_DEBUG_PARSER_TRACE("parser pkt is L4 unknow\n");
           // return TBS_OK;
        }

    }
    else //ipv6 packet
    {
        if(SOL_TCP == ipv6_header->nexthdr)
        {
            skb_desc->status |= TBS_NFP_RX_L4_TCP_TYPE;//记录数据包状态
            TBS_NFP_DEBUG_PARSER_TRACE("parser pkt is TCP\n");
        }
        else if(SOL_UDP == ipv6_header->nexthdr)
        {
            skb_desc->status |= TBS_NFP_RX_L4_UDP_TYPE;//记录数据包状态
            TBS_NFP_DEBUG_PARSER_TRACE("parser pkt is udp\n");
        }
        else
        {
            skb_desc->status |= TBS_NFP_RX_L4_UNKNOWN_MASK;//记录数据包状态
            TBS_NFP_DEBUG_PARSER_TRACE("parser pkt is L4 unknow\n");
           // return TBS_OK;
        }

    }

parser_end:
//给其他的结构体成员赋值
    skb_desc->mac_header = (unsigned char *)mac_header;

    if(NULL!=ip_header)//ipv4 packet
    {
        skb_desc->l3_offset =(unsigned char*)ip_header - mac_header;
    }
    else if(NULL!=ipv6_header)
    {
        skb_desc->l3_offset =(unsigned char*)ipv6_header - mac_header;
    }

    skb_desc->skb = (struct sk_buff *)skb;
    skb_desc->shift = 0;

    //调试代码，打印目的 mac地址，包状态码，包类型检测接口验证，包ip层偏移量
    TBS_NFP_DEBUG_CHECK_SKBPARSER(skb_desc);

    //TBS_NFP_OUT_FUNC;
    return TBS_NFP_OK;
}

