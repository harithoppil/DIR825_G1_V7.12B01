#include <linux/module.h>
#include <asm/tc3162/tc3162.h>
#include <linux/netdevice.h>
#include <linux/mii.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include "femac.h"
#include "fe_verify.h"


uint32 macLoopback = 0;
uint32 macRxLoopback;
extern macAdapter_t *mac_p;
extern struct net_device *tc3262_gmac_dev;

#ifdef TCSUPPORT_MT7510_FE

static char test_udp[68] = 
	{0x00,0x22,0x22,0x00,0x22,0x00,0x00,0x11,0x11,0x00,0x11,0x00,0x81,0x00,0x00,0x01,0x08,0x00,0x45,0x00,
	 0x00,0x32,0x00,0x00,0x00,0x00,0x80,0x11,0x9a,0x20,0xc0,0xa8,0x0f,0x6e,0xc0,0xa8,
	 0x0f,0xdc,0x30,0x3a,0x30,0x39,0x00,0x1e,0xe0,0x7a,0xa1,0xa2,0xa3,0xa4,0x65,0x66,
	 0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76};

static char test_tcp[68] = 
	{0x00,0x22,0x22,0x00,0x22,0x00,0x00,0x11,0x11,0x00,0x11,0x00,0x81,0x00,0x00,0x01,0x08,0x00,0x45,0x00,
	 0x00,0x32,0x00,0x00,0x00,0x00,0x80,0x06,0x9a,0x2b,0xc0,0xa8,0x0f,0x6e,0xc0,0xa8,
	 0x0f,0xdc,0x30,0x3a,0x30,0x39,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x50,0x18,
	 0x01,0x00,0x32,0x33,0x00,0x00,0xa1,0xa2,0xa3,0xa4,0x65,0x66,0x67,0x68,0x69,0x6a};

struct _reg_value reg_value[] = {
		//FE REG
		{0xc,0xffffffff,0x0},{0x10,0xffff,0x0},{0x14,0xffffffff,0x0},
		{0x280,0xffff01,0x0},{0x284,0xffff,0x0},
		{0x400,0xffffffff,0x0},{0x404,0xffffffff,0x0},
		{0x500,0xffffffff,0x0},{0x504,0xffffffff,0x0},{0x508,0xffffffff,0x0},{0x50c,0xffff,0x0},{0x510,0xffffffff,0x0},{0x514,0xffffffff,0x0},
		{0x800,0xffffffff,0x0},{0x804,0xfff,0x0},{0x808,0xfff,0x0},
		{0x900,0xffffffff,0x0},{0x904,0xfff,0x0},
		{0xa0c,0xffffffff,0x0},{0xa10,0xf,0x0},{0xa28,0xffffffff,0x0},
		{0x1500,0xffffffff,0x0},{0x1504,0xffffffff,0x0},{0x1508,0xffffffff,0x0},{0x150c,0xffff,0x0},{0x1510,0xffffffff,0x0},{0x1514,0xfff,0x0},
		{0x1524,0xffffffff,0x0},{0x1528,0xf3,0x0},{0x152c,0xffffffff,0x0},{0x1530,0xffffffff,0x0},
		//PPE_REG
		{0xe00,0x3ff,0x0},{0xe04,0x1fffff,0x0},{0xe08,0xffffffff,0x0},{0xe0c,0xffffffff,0x0},
		{0xe10,0xffffffff,0x0},{0xe14,0xffffffff,0x0},{0xe18,0xffffffff,0x0},{0xe1c,0xffff,0x0},
		{0xe20,0xffffffff,0x0},{0xe24,0x0,0x0},{0xe28,0xffffffff,0x0},{0xe2c,0x3fffffff,0x0},
		{0xe30,0xffffff,0x0},{0xe34,0xffffffff,0x0},{0xe38,0xffffffff,0x0},{0xe3c,0xffffffff,0x0},
		{0xe40,0xffffffff,0x0},{0xe44,0xffffffff,0x0},{0xe48,0xffffffff,0x0},{0xe84,0xf,0x0},
		{0xf08,0xff,0x0},{0xf0c,0x3fffffff,0x0},{0xf10,0x3fffffff,0x0},{0xf14,0x3fff,0x0},{0xf18,0xffff,0x0},{0xf24,0xffff,0x0},
		{0xffffffff,0x0,0x0}
};

#endif




void macPhyLoopback(macAdapter_t *mac_p, int enable)
{
	uint32 data;

	data = miiStationRead(mac_p, MII_BMCR);
	if (enable)
		data |= BMCR_LOOPBACK;
	else
		data &= ~BMCR_LOOPBACK;
	miiStationWrite(mac_p, MII_BMCR, data);
}

void dump_skb(struct sk_buff *skb)
{
	char tmp[80];
	char *p = skb->data;
	char *t = tmp;
	int i, n = 0;

	printk("ERR skb=%08lx data=%08lx len=%d\n", (uint32) skb, (uint32) skb->data, skb->len);
	for (i = 0; i < skb->len; i++) {
		t += sprintf(t, "%02x ", *p++ & 0xff);
		if ((i & 0x0f) == 0x0f) {
			printk("%04x: %s\n", n, tmp);
			n += 16;
			t = tmp;
		}
	}
	if (i & 0x0f)
		printk("%04x: %s\n", n, tmp);
}

void dump_data(char *p, int len)
{
	char tmp[80];
	char *t = tmp;
	int i, n = 0;

	printk("ERR data=%08lx len=%d\n", (uint32) p, len);
	for (i = 0; i < len; i++) {
		t += sprintf(t, "%02x ", *p++ & 0xff);
		if ((i & 0x0f) == 0x0f) {
			printk("%04x: %s\n", n, tmp);
			n += 16;
			t = tmp;
		}
	}
	if (i & 0x0f)
		printk("%04x: %s\n", n, tmp);
}

unsigned short in_csum(unsigned short *ptr, int nbytes)
{
	register int			sum;		/* assumes long == 32 bits */
	unsigned short			oddbyte;
	register unsigned short	answer; 	/* assumes u_short == 16 bits */

	/*
	 * Our algorithm is simple, using a 32-bit accumulator (sum),
	 * we add sequential 16-bit words to it, and at the end, fold back
	 * all the carry bits from the top 16 bits into the lower 16 bits.
	 */

	sum = 0;

	while (nbytes > 1)	{
		sum += *ptr++;
		nbytes -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nbytes == 1) {
		oddbyte = 0;		/* make sure top half is zero */
		*((unsigned char *) &oddbyte) = *(unsigned char *)ptr;   /* one byte only */
		sum += oddbyte;
	}

	/*
	 * Add back carry outs from top 16 bits to low 16 bits.
	 */

	sum  = (sum >> 16) + (sum & 0xffff);	/* add high-16 to low-16 */
	sum += (sum >> 16); 		/* add carry */
	answer = ~sum;		/* ones-complement, then truncate to 16 bits */
	return(answer);
}


int tc3262_gmac_loopback_gen(struct net_device *dev, int npackets, int txlen)
{
	int i, k;
	struct sk_buff *skb;
	int tx_len;
	uint8 *tx_data;
	uint8 tx_seed;
	int offset = 0;
	int tx_priority;
	unsigned short chksum;
	int ip_len = 5;
	unsigned long flags;	

	printk("Loopback test packets=%d txlen=%d\n", npackets, txlen);

	macRxLoopback = 0;
	for (i = 0; i < npackets; i++) {
		if ((i & 0x1fff) == 0x1fff)
			printk("Gen %d packets.\n", i);
		skb = dev_alloc_skb(RX_BUF_LEN);
		if (skb == NULL)
			continue;

		offset =  random32();
		if (offset & 0x1)
			offset = 2;
		else
			offset = 0;

		skb_reserve(skb, offset);
		
		if (txlen == 0) {
			tx_len = random32();
			tx_len = tx_len % 1515;
			if (tx_len > 1514)
				tx_len = 1514;
			if (tx_len < 60)
				tx_len = 60;
		} else {
			tx_len = txlen;
		}
#ifdef TCSUPPORT_MT7510_FE
		if((macLoopback & LOOPBACK_TX_UDPCS) || (macLoopback & LOOPBACK_TX_TCPCS) || (macLoopback & LOOPBACK_TX_VLAN)){
			skb->ip_summed = CHECKSUM_PARTIAL;
			tx_data = skb_put(skb, 68);
			if(macLoopback & LOOPBACK_TX_UDPCS){
				memcpy(tx_data, test_udp, 68);
				tx_data[44] = 0;//error tcp cs
				tx_data[28] = 0;//error ip cs
			}else if(macLoopback & LOOPBACK_TX_TCPCS){
				memcpy(tx_data, test_tcp, 68);
				tx_data[54] = 0;//error tcp cs
				tx_data[28] = 0;//error ip cs
			}else if(macLoopback & LOOPBACK_TX_VLAN){
				memcpy(tx_data, test_udp, 68);
			}	
		}else
#endif
		{
			tx_data = skb_put(skb, tx_len);
			for (k = 0; k < 6; k++)
				tx_data[k] = 0x11;
			for (k = 6; k < 12; k++)
				tx_data[k] = 0x22;

			tx_data[12] = 0x08;
			tx_data[13] = 0x00;

			tx_seed = (uint8) random32();
			if (macLoopback & LOOPBACK_TX_RANDOM)
				tx_priority = random32() & GMAC_PRIORITY_MASK;
			else
				tx_priority = 0;

			skb->priority = tx_priority;
			spin_lock_irqsave(&mac_p->lock, flags);
        		if (mac_p->txUnReleasedBufCnt[tx_priority] == mac_p->txRingSize - 1) 
				mdelay(2);
			spin_unlock_irqrestore(&mac_p->lock, flags);	
			if (tx_len < (ip_len * 4 + 14))
				ip_len = (tx_len - 14) >> 2;

			tx_data[14] = 0x40 | ip_len;
			tx_data[15] = tx_seed;

			for (k = 16; k < tx_len; k++)
				tx_data[k] = (uint8) (tx_seed++ & 0xff);

			tx_data[24] = 0;
			tx_data[25] = 0;

			chksum = in_csum((unsigned short *) (skb->data + 14), ip_len << 2);
			tx_data[24] = (chksum >> 8) & 0xff;
			tx_data[25] = chksum & 0xff;

			ip_len++;
			if (ip_len > 15)
				ip_len = 5;
		}
		tc3262_gmac_tx(skb, dev);
	}
	printk("Gen %d packets done.\n", npackets);

	mdelay(3000);

	printk("Chk %ld packets done.\n", macRxLoopback);

	if (macRxLoopback != npackets)
		printk("ERR TX/RX packet number mismatch.\n");

	return 0;
}

int tc3262_gmac_loopback_chk(struct sk_buff *skb, struct net_device *dev)
{
	int k;
	uint8 *tx_data;
	uint8 tx_seed;
	int ip_len;

	macRxLoopback++;

	tx_data = skb->data;

#ifdef TCSUPPORT_MT7510_FE
	if((macLoopback & LOOPBACK_TX_UDPCS) || (macLoopback & LOOPBACK_TX_TCPCS) || (macLoopback & LOOPBACK_TX_VLAN)){
		if(macLoopback & LOOPBACK_TX_UDPCS){
			if(memcmp(tx_data + 16, test_udp + 16, 50) == 0){
				printk("Udp/Ip offload test OK\n");
			}else{
				printk("Udp/Ip offload test Fail\n");
				goto err;
			}
		}else if(macLoopback & LOOPBACK_TX_TCPCS){
			if(memcmp(tx_data + 16, test_tcp + 16, 50) == 0){
				printk("Tcp/IP offload test OK\n");
			}else{
				printk("Tcp/IP offload test Fail\n");
				goto err;
			}
		}else if(macLoopback & LOOPBACK_TX_VLAN){
			if((tx_data[12] == 0x81) && (tx_data[13] == 0x00) && (tx_data[14] == 0x50) && (tx_data[15] == 0x99)
				&& (memcmp(tx_data + 20, test_udp + 16, 50) == 0)){
				printk("VLAN offload test OK\n");
			}else{
				printk("VLAN offload test Fail\n");
				goto err;
			}
		}	
	}else
#endif
	{
		for (k = 0; k < 6; k++) {
			if (tx_data[k] != 0x11) {
				printk("loopback fail: dst mac unmatch\n");
				goto err;
			}
		}
		for (k = 6; k < 12; k++) {
			if (tx_data[k] != 0x22) {
				printk("loopback fail: src mac unmatch\n");
				goto err;
			}
		}

		if ((tx_data[12] != 0x08) || (tx_data[13] != 0x00)) {
			printk("loopback fail: ether type unmatch\n");
			goto err;
		}

		if ((tx_data[14] & 0xf0) != 0x40) {
			printk("loopback fail: ip header unmatch\n");
			goto err;
		}
		ip_len = (tx_data[14] & 0x0f) << 2;
		tx_seed = tx_data[15];

		for (k = 16; k < skb->len; k++) {
			if ((k != 24) && (k != 25)) {
				if (tx_data[k] != (uint8) (tx_seed++ & 0xff)) {
					printk("loopback fail: payload unmatch pos=%04x\n", k);
					goto err;
				}
			} else {
				tx_seed++;
			}
		}

		if (in_csum((unsigned short *) (skb->data + 14), ip_len) != 0) {
			printk("loopback fail: ip checksum unmatch\n");
			goto err;
		}
	}
	dev_kfree_skb(skb);
	return 0;

err:
	dump_skb(skb);
	dev_kfree_skb(skb);
	return 1;
}

int tc3262_gmac_qos_gen(struct net_device *dev, int tx_len)
{
	int i, k;
	struct sk_buff *skb;
	uint8 *tx_data;
	uint8 tx_seed;
	uint8 tx_queue;
	int txq;

	macDrvStop();

	for (txq = TX_QUEUE_NUM - 1; txq >= 0; txq--) {
		for (i = 0; i < MAC_TXDESCP_NO; i++) {
			skb = dev_alloc_skb(RX_BUF_LEN);
			if (skb == NULL) {
				printk("skb == NULL\n");
				continue;
			}

			skb->priority = txq;

			//tx_len = random32();
			if (tx_len == 0)
				tx_len = 60;
			tx_len = tx_len % 1514;
			if (tx_len > 1514)
				tx_len = 1514;
			if (tx_len < 60)
				tx_len = 60;

			tx_data = skb_put(skb, tx_len);
			for (k = 0; k < 6; k++) {
				if (k == 0)
					tx_data[k] = 0x10;
				else
					tx_data[k] = 0x10 + txq;
			}
			for (k = 6; k < 12; k++)
				tx_data[k] = 0x22;

			tx_data[12] = 0x08;
			tx_data[13] = 0x01;

			tx_seed = (uint8) random32();
			tx_queue = (uint8) random32();

			tx_data[14] = tx_seed;
			tx_data[15] = tx_queue;

			for (k = 16; k < tx_len; k++)
				tx_data[k] = (uint8) (tx_seed++ & 0xff);

			tc3262_gmac_tx(skb, dev);
		}
	}

	macDrvStart();

	return 0;
}

#ifdef TCSUPPORT_MT7510_FE

int tc3262_gmac_isrtest_gen(struct net_device *dev)
{
	int k;
	struct sk_buff *skb;
	uint8 *tx_data;
	int txq;
	int tx_len = 60;

	for (txq = TX_QUEUE_NUM - 1; txq >= 0; txq--) {
			printk("send packet to TX Queue %d\n",txq);
			skb = dev_alloc_skb(RX_BUF_LEN);
			if (skb == NULL) {
				printk("skb == NULL\n");
				continue;
			}

			skb->priority = txq;
			
			tx_data = skb_put(skb, tx_len);
			for (k = 0; k < 6; k++) {
				if (k == 0)
					tx_data[k] = 0x10;
				else
					tx_data[k] = 0x10 + txq;
			}
			for (k = 6; k < 12; k++)
				tx_data[k] = 0x22;

			tx_data[12] = 0x08;
			tx_data[13] = 0x01;

			tc3262_gmac_tx(skb, dev);
	}

	return 0;

}


int tc3262_gmac_reg_test(void)
{
	int i;
	u32 reg, addr;
	char result;

	//set dma stop
	macDrvStop();
	result = 0;
	
	for(i=0; reg_value[i].reg_offset != 0xffffffff; i++){
		//test write all 1
		addr = FE_BASE + reg_value[i].reg_offset;
		write_reg_word(addr, 0xffffffff);
		reg = read_reg_word(addr);
		if(reg != reg_value[i].all1_value){
			printk("test 1 : addr %x Value %x\n",addr,reg);
			result = -1;
		}
		//test wite all 0
		write_reg_word(addr, 0x0);
		reg = read_reg_word(addr);
		if(reg != reg_value[i].all0_value){
			printk("test 0 : addr %x Value %x\n",addr,reg);
			result = -1;
		}
	}

	//Some similar pattern
	for(i = 0; i < 15 ; i ++){
		//test write all 1
		addr = FE_BASE + 0x200 + i*8;
		write_reg_word(addr, 0xffffffff);
		reg = read_reg_word(addr);
		if(reg != 0xfff73fff){
			printk("test 1 : addr %x Value %x\n",addr,reg);
			result = -1;
		}
		//test wite all 0
		write_reg_word(addr, 0x0);
		reg = read_reg_word(addr);
		if(reg != 0){
			printk("test 0 : addr %x Value %x\n",addr,reg);
			result = -1;
		}	
		//test write all 1
		addr = FE_BASE + 0x204 + i*8 ;
		write_reg_word(addr, 0xffffffff);
		reg = read_reg_word(addr);
		if(reg != 0xfff700c0){
			printk("test 1 : addr %x Value %x\n",addr,reg);
			result = -1;
		}
		//test wite all 0
		write_reg_word(addr, 0x0);
		reg = read_reg_word(addr);
		if(reg != 0){
			printk("test 0 : addr %x Value %x\n",addr,reg);
			result = -1;
		}
	}

	//Some similar pattern
	for(i = 0; i < 7 ; i ++){
		//test write all 1
		addr = FE_BASE + 0x290 + i*4;
		write_reg_word(addr, 0xffffffff);
		reg = read_reg_word(addr);
		if(reg != 0xffffffff){
			printk("test 1 : addr %x Value %x\n",addr,reg);
			result = -1;
		}
		//test wite all 0
		write_reg_word(addr, 0x0);
		reg = read_reg_word(addr);
		if(reg != 0){
			printk("test 0 : addr %x Value %x\n",addr,reg);
			result = -1;
		}	
	}
	
	for(i = 0; i < 64 ; i ++){
		//test write all 1
		addr = FE_BASE + 0x200c + i*0x10;
		write_reg_word(addr, 0xffffffff);
		reg = read_reg_word(addr);
		if(reg != 0xffffffff){
			printk("test 1 : addr %x Value %x\n",addr,reg);
			result = -1;
		}
		//test wite all 0
		write_reg_word(addr, 0x0);
		reg = read_reg_word(addr);
		if(reg != 0){
			printk("test 0 : addr %x Value %x\n",addr,reg);
			result = -1;
		}	
	}	
	
	printk("Reg test %s, Please reboot System\n",(result)? "Fail" : "Success");

	return 0;

}
#endif

int eth_loopback_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%08lx\n", macLoopback);
	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}

int eth_loopback_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];
	int tx_len;
	unsigned int reg;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	macLoopback = simple_strtoul(val_string, NULL, 16);
	tx_len = (macLoopback & 0xffff0000) >> 16;
	macLoopback &= 0xffff;
	if (LOOPBACK_MODE(macLoopback) == LOOPBACK_RX_CHK) {
		/* check if external loopback */
		if (macLoopback & LOOPBACK_EXT) {
			macPhyLoopback(mac_p, 1);
			mdelay(2000);
		}
	} else if (LOOPBACK_MODE(macLoopback) == LOOPBACK_TX_QOS) {
		tc3262_gmac_qos_gen(tc3262_gmac_dev, tx_len);
	}
#ifdef TCSUPPORT_MT7510_FE	
	else if (macLoopback & LOOPBACK_ISR_TEST) {
		reg = read_reg_word(INT_MASK);
		reg |= (TX_DONE_INT0 | TX_DONE_INT1 | TX_DONE_INT2 | TX_DONE_INT3);
		write_reg_word(INT_MASK, reg);
		tc3262_gmac_isrtest_gen(tc3262_gmac_dev);
	} else if (macLoopback & LOOPBACK_REG_TEST) {
		tc3262_gmac_reg_test();
	}
#endif	
	return count;
}

int eth_loopback_test_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "\n");
	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}

int eth_loopback_test_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];
	int npackets, txlen;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	sscanf(val_string, "%d %d", &npackets, &txlen);
	tc3262_gmac_loopback_gen(tc3262_gmac_dev, npackets, txlen);

	return count;
}


