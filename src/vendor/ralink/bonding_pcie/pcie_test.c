#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/tc3162/tc3162.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

/************************************************************************
 *				MACRO   D E C L A R A T I O N S
 *************************************************************************
 */
//#define GDMA_INT_MODE  //test slave GDMA interrupt mode
//#define PCIE_TEST_DBG

#ifdef GDMA_INT_MODE
#define GDMA_INT 38
#endif

/************************************************************************
 *				F U N C T I O N   D E C L A R A T I O N S
 *************************************************************************
 */
static void __exit gdma_cleanup_module(void);
static int __init gdma_init_module(void);
static int gdmaTest(void);
static int pcieRegTest(void);
static void dumpPCIeDbgReg(void);


/************************************************************************
 *				GLOBE VARIABLES
 *************************************************************************
 */
int gPattern = 1;
int gBurstSize = 0;
int gTestCase = 0;
int gLongTest = 0;
unsigned long gSlaveVirBaseAddr = 0;
unsigned long gSlaveBytes;
unsigned long *gUnWdata;
unsigned long *gUnRdata;
static int gPcieRegTest = 0;

#ifdef GDMA_INT_MODE
struct tasklet_struct gSlaveTasklet;
static int gSlaveCmpDone = 0;
#endif




/************************************************************************
 *				FUNCTION DECLARATION
 *************************************************************************
 */

/* GDMA test */
static int gdmaTest_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data){
	printk("TEST CMD: echo (gPattern) (gBurstSize) (gTestCase) > /proc/gdma_test \n");
	printk("gPattern : [0~2], 0 is 0x0/ 1 is 0xff/ 2 is incr/ 3 is random \n");
	printk("gBurstSize : [0~4], 5 is random burst size \n");
	printk("gTestCase :  [0~1] \n");
	printk("0 is to test slave gdma (dram to dram) & master gdma (slave reg to slave reg) \n");
	printk("1 is to test slave gdma (drma to slave reg)\n");
	printk("current configuration: gPattern(%d) gBurstSize(%d) gTestCase(%d) \n", gPattern, gBurstSize, gTestCase);

	return 0;
}

static int gdmaTest_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data){
	char val_string[20];

	if (count > sizeof(val_string) - 1)
    		return -EINVAL;
	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';
	sscanf(val_string, "%d %d %d\n", &gPattern, &gBurstSize, &gTestCase);
	if (gBurstSize > 4) {
		gBurstSize = 5;
	}
	printk("-->configure gdma test pattern:%d burst_size:%d testCase:%d \n", gPattern, gBurstSize, gTestCase);


	gdmaTest();

	return count;
}

static int gdmaEnable_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data){
	char val_string[5];

	if (count > sizeof(val_string) - 1)
    		return -EINVAL;
	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';
	sscanf(val_string, "%d\n", &gLongTest);
	printk("-->gLongTest:%d \n", gLongTest);

	return count;
}

/* PCIE reset test*/
int gPcieReset = 0;
static int pcieReset_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data){
	char val_string[40];
	int devIdx;
	int tmpValue;

	if (count > sizeof(val_string) - 1)
    		return -EINVAL;
	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;
	val_string[count] = '\0';

	sscanf(val_string,"%d\n",&gPcieReset);
	return count;
}

static int pcieReset_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;
	len = sprintf(page, "%d", gPcieReset);
	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	chkAhbErr(0);
	return len;
}

/* dump PCIe debug register */
static int pcieDbgReg_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	dumpPCIeDbgReg();
	return 0;
}

/* read pcie configuration register test */
static int pcieRegTest_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data){
	char val_string[5];
	int devIdx;
	int tmpValue;

	if (count > sizeof(val_string) - 1)
    		return -EINVAL;
	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;
	val_string[count] = '\0';

	sscanf(val_string,"%d\n",&gPcieRegTest);

	pcieRegTest();
	return count;
}




static int slaveGdmaCmp(void)
{
		unsigned long *wdataAddr;
		unsigned long *rdataAddr;
		int i,j;
		unsigned char *wchar;
		unsigned char *rchar;

#ifdef PCIE_TEST_DBG
	printk("slave GDMA Word compare---> \n");
#endif
	for(i=0; i<gSlaveBytes/4; i++){
			wdataAddr = gUnWdata + i;
			rdataAddr = gUnRdata + i;
#ifdef PCIE_TEST_DBG
		printk("i=%d wdataAddr=0x%x, *wdataAddr=0x%x \n",i,wdataAddr, *wdataAddr);
		printk("i=%d rdataAddr=0x%x, *rdataAddr=0x%x \n\n",i,rdataAddr, *rdataAddr);
#endif
		if((*wdataAddr) != (*rdataAddr)){
#ifdef PCIE_TEST_DBG
			printk("compare ERROR!! i=%d wdataAddr=0x%x, *wdataAddr=0x%x \n",i,wdataAddr, *wdataAddr);
			printk("compare ERROR!! i=%d rdataAddr=0x%x, *rdataAddr=0x%x \n\n",i,rdataAddr, *rdataAddr);
#endif
			return -1;
		}

		if(((gSlaveBytes%4)!=0) && (i==(gSlaveBytes/4-1))){
			wdataAddr += 1;
			rdataAddr += 1;
#ifdef PCIE_TEST_DBG
			printk("slave GDMA Byte compare---> \n");
#endif
			for(j=0; j<gSlaveBytes%4; j++){
				wchar = (unsigned char*)wdataAddr + j;
				rchar = (unsigned char*)rdataAddr + j;
#ifdef PCIE_TEST_DBG
				printk("j=%d wchar=0x%x, *wchar=0x%x \n",j,wchar, *wchar);
				printk("j=%d rchar=0x%x, *rchar=0x%x \n\n",j,rchar, *rchar);
#endif
				if(*wchar != *rchar){
#ifdef PCIE_TEST_DBG
					printk("compare ERROR!! j=%d wchar=0x%x, *wchar=0x%x \n",j,wchar, *wchar);
					printk("compare ERROR!! j=%d rchar=0x%x, *rchar=0x%x \n\n",j,rchar, *rchar);
#endif
					return -1;
				}
			}
		}
	}

#ifdef GDMA_INT_MODE
	gSlaveCmpDone = 1;
#endif

	return 0;
}

static int masterCmp(unsigned long *sourceReg, unsigned long *destiReg, unsigned long masterBytes)
{
	unsigned long sourceAddr;
	unsigned long destiAddr;
	int i;

#ifdef PCIE_TEST_DBG
	printk("master GDMA data compare---> \n");
#endif
	for(i=0; i<masterBytes/4; i++){
		sourceAddr = sourceReg + i;
		destiAddr = destiReg + i;

#ifdef PCIE_TEST_DBG
		printk("i=%d sourceAddr=0x%x, VPint(sourceReg)=0x%x \n",i,sourceAddr, VPint(sourceAddr));
		printk("i=%d destiAddr=0x%x, VPint(destiAddr)=0x%x \n\n",i,destiAddr, VPint(destiAddr));
#endif
		if(VPint(sourceAddr) != VPint(destiAddr)){
#ifdef PCIE_TEST_DBG
			printk("compare ERROR!! i=%d sourceAddr=0x%x, VPint(sourceAddr)=0x%x \n",i,sourceAddr, VPint(sourceAddr));
			printk("compare ERROR!! i=%d destiAddr=0x%x, VPint(destiAddr)=0x%x \n\n",i,destiAddr, VPint(destiAddr));
#endif
			return -1;
		}
	}

	return 0;
}

static int slaveDramRegCmp(unsigned long *regAddr, unsigned long byte)
{
	unsigned long *wdataAddr;
	unsigned long regVal;
	int i;

#ifdef PCIE_TEST_DBG
	printk("slave GDMA dram reg data compare---> \n");
#endif

	for(i=0; i<byte/4; i++)
	{
		wdataAddr = gUnWdata + i;
		regVal = VPint(regAddr + i*4);
		if(*wdataAddr != regVal)
		{
			printk("i=%d ERROR! wdataAddr=0x%x, *wdataAddr=0x%x \n",i,wdataAddr, *wdataAddr);
			printk("i=%d ERROR! reg=0x%x ,regVal=0x%x \n", i,regAddr + i*4, regVal);
		}
		return -1;
	}

	return 0;
}


static int slaveDramRegTest(void)
{
#define SLAVE_REG_SOURCE_OFFSET 0Xb00864 //slave scu register
	unsigned long *wdata;
	unsigned long *wdataAddr;
	int i,j;
	unsigned long byte;
	unsigned char pat;
	unsigned char burst = 0;
	static unsigned long waitCounter = 0;


	wdata = (unsigned long *)kmalloc(sizeof(unsigned long) * 0xffff, GFP_KERNEL);
	memset(wdata, 2, sizeof(unsigned long) * 0xffff);

	do
	{
		if (gBurstSize > 4) { /*random burst size*/
			burst = rand() & 0x7;
			if (burst > 4) {
				burst = 4;
			}
		} else {
			burst = gBurstSize;
		}

		byte = 64;
#ifdef PCIE_TEST_DBG
		printk("slave GDMA transfer bytes=%d (%d words & %d byte)\n", byte, byte/4, byte%4);
#endif

		printk("slave GDMA test DRAM to slvae reg \n");
		gUnWdata = (unsigned long *)((unsigned long)wdata | 0x20000000);
		memset(gUnWdata, 2, sizeof(unsigned long) * 0xffff);
		for(i=0; i<=byte/4; i++) {
			switch(gPattern) {
				case 0: //data 0x0
					pat = 0x0;
					break;
				case 1: //data 0xff
					pat = 0xffffffff;
					break;
				case 2: //data 0x12345678
					pat = 0x12345678 + i;
					break;
				case 3:
				default:
					pat = rand();
					break;
			}
			wdataAddr = (unsigned long*)(gUnWdata) + i;
			*wdataAddr = pat;
		}

		/* configure slave GDMA register */
		VPint(gSlaveVirBaseAddr+0xb00040) = 0x0; /*release slave chip mc_rst*/
		VPint(gSlaveVirBaseAddr+0xb30000) = ((unsigned long)wdata & 0x1fffffff); /*physical address*/
		VPint(gSlaveVirBaseAddr+0xb30004) = 0x1fb00864;
		VPint(gSlaveVirBaseAddr+0xb3000c) = 0x10000004;
		VPint(gSlaveVirBaseAddr+0xb30008) = ((byte & 0xffff)<<16)| (burst<<3) | (1<<1) | (1<<0);
#ifdef PCIE_TEST_DBG
		printk("VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30000,VPint(gSlaveVirBaseAddr+0xb30000));
		printk("VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30004,VPint(gSlaveVirBaseAddr+0xb30004));
		printk("VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb3000c,VPint(gSlaveVirBaseAddr+0xb3000c));
		printk("VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30008,VPint(gSlaveVirBaseAddr+0xb30008));
#endif

		do{
#if 1
			for(j=0; j<5000; j++){
				if(((VPint(gSlaveVirBaseAddr+0xb30204) & (1<<0)))){
					break;
				}
				mdelay(1);
			}

			if(j == 5000){
				printk("waitCounter=%d \n",waitCounter);
				printk("interrupt ERROR!! VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30204,VPint(gSlaveVirBaseAddr+0xb30204));

				printk("compare slave GDMA dram to reg data \n");
				slaveDramRegCmp(gSlaveVirBaseAddr+SLAVE_REG_SOURCE_OFFSET, byte);
				return -1;
			}else{
				break;
			}
#else
			waitCounter++;
			if(((VPint(gSlaveVirBaseAddr+0xb30204) & (1<<0)))){
				waitCounter = 0;
				break;
			}
			if(waitCounter == 5000){
				printk("waitCounter=%d \n",waitCounter);
				printk("interrupt ERROR!! VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30204,VPint(gSlaveVirBaseAddr+0xb30204));
				waitCounter = 0;

				printk("compare slave GDMA dram to reg data \n");
				slaveDramRegCmp(gSlaveVirBaseAddr+SLAVE_REG_SOURCE_OFFSET, byte);
				return -1;
			}
#endif
		}while(1);

		slaveDramRegCmp(gSlaveVirBaseAddr+SLAVE_REG_SOURCE_OFFSET, byte);



		printk("slave GDMA test slave reg to DRAM \n");
		memset(gUnWdata, 0, sizeof(unsigned long) * 0xffff);

		/* configure slave GDMA register */
		VPint(gSlaveVirBaseAddr+0xb00040) = 0x0; /*release slave chip mc_rst*/
		VPint(gSlaveVirBaseAddr+0xb30000) = 0x1fb00864;
		VPint(gSlaveVirBaseAddr+0xb30004) = ((unsigned long)wdata & 0x1fffffff); /*physical address*/
		VPint(gSlaveVirBaseAddr+0xb3000c) = 0x10000004;
		VPint(gSlaveVirBaseAddr+0xb30008) = ((byte & 0xffff)<<16)| (burst<<3) | (1<<1) | (1<<0);
#ifdef PCIE_TEST_DBG
		printk("VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30000,VPint(gSlaveVirBaseAddr+0xb30000));
		printk("VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30004,VPint(gSlaveVirBaseAddr+0xb30004));
		printk("VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb3000c,VPint(gSlaveVirBaseAddr+0xb3000c));
		printk("VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30008,VPint(gSlaveVirBaseAddr+0xb30008));
#endif

		do{
#if 1
			for(j=0; j<5000; j++){
				if(((VPint(gSlaveVirBaseAddr+0xb30204) & (1<<0)))){
					break;
				}
				mdelay(1);
			}

			if(j == 5000){
				printk("interrupt ERROR!! VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30204,VPint(gSlaveVirBaseAddr+0xb30204));
				printk("compare slave GDMA dram to reg data \n");
				slaveDramRegCmp(gSlaveVirBaseAddr+SLAVE_REG_SOURCE_OFFSET, byte);
				return -1;
			}else{
				break;
			}
#else
			waitCounter++;
			if(((VPint(gSlaveVirBaseAddr+0xb30204) & (1<<0)))){
				waitCounter = 0;
				break;
			}
			if(waitCounter == 5000){
				printk("waitCounter=%d \n",waitCounter);
				printk("interrupt ERROR!! VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30204,VPint(gSlaveVirBaseAddr+0xb30204));
				waitCounter = 0;

				printk("compare slave GDMA dram to reg data \n");
				slaveDramRegCmp(gSlaveVirBaseAddr+SLAVE_REG_SOURCE_OFFSET, byte);
				return -1;
			}
#endif
		}while(1);

		slaveDramRegCmp(gSlaveVirBaseAddr+SLAVE_REG_SOURCE_OFFSET, byte);


		schedule();
	}while(gLongTest);

	kfree(wdata);
	printk("masterSlaveTest() slave GDMA test FINISH !!\n");
	return -1;
}


#ifdef GDMA_INT_MODE //just test slave GDMA interrupt without data compare
static int masterSlaveTest(void)
{
	unsigned long *wdata, *rdata;
	int i;
	unsigned long *wdataAddr;
	unsigned long pat;
	unsigned char burst = 0;

	printk("slave GDMA test interrupt (DRAM to DRAM) \n");

	wdata = (unsigned long *)kmalloc(sizeof(unsigned long) * 0xffff, GFP_KERNEL);
	rdata = (unsigned long *)kmalloc(sizeof(unsigned long) * 0xffff, GFP_KERNEL);
//	printk("wdata=0x%x rdata=0x%x \n", wdata, rdata);
	memset(wdata, 2, sizeof(unsigned long) * 0xffff);
	memset(rdata, 2, sizeof(unsigned long) * 0xffff);

	do {
		if (gBurstSize > 4) { /*random burst size*/
			burst = rand() & 0x7;
			if (burst > 4) {
				burst = 4;
			}
		} else {
			burst = gBurstSize;
		}

		gSlaveBytes =	(rand() % (128 - 8)) + 8;
#ifdef PCIE_TEST_DBG
		printk("slave GDMA transfer bytes=%d (%d words & %d byte)\n", gSlaveBytes, gSlaveBytes/4, gSlaveBytes%4);
#endif
		gUnWdata = (unsigned long *)((unsigned long)wdata | 0x20000000);
		gUnRdata = (unsigned long *)((unsigned long)rdata | 0x20000000);
//		printk("gUnWdata=0x%x gUnWdata=0x%x \n", gUnWdata, gUnWdata);
		memset(gUnWdata, 2, sizeof(unsigned long) * 0xffff);
		memset(gUnRdata, 2, sizeof(unsigned long) * 0xffff);
		for(i=0; i<=gSlaveBytes/4; i++) {
			switch(gPattern) {
				case 0: //data 0x0
					pat = 0x0;
					break;
				case 1: //data 0xff
					pat = 0xffffffff;
					break;
				case 2: //data 0x12345678
					pat = 0x12345678 + i;
					break;
				case 3:
				default:
					pat = rand();
					break;
			}
			wdataAddr = (unsigned long*)(gUnWdata) + i;
			*wdataAddr = pat;
		}

		/* configure slave GDMA register */
		VPint(gSlaveVirBaseAddr+0xb00040) = 0x0;
		VPint(gSlaveVirBaseAddr+0xb30000) = ((unsigned long)wdata & 0x1fffffff);
		VPint(gSlaveVirBaseAddr+0xb30004) = ((unsigned long)rdata & 0x1fffffff);
		VPint(gSlaveVirBaseAddr+0xb3000c) = 0x4;
		VPint(gSlaveVirBaseAddr+0xb30008) = ((gSlaveBytes&0xffff)<<16)| (burst<<3) | (1<<2) | (1<<1) | (1<<0);
#if PCIE_TEST_DBG
		printk("VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30004,VPint(gSlaveVirBaseAddr+0xb30004));
		printk("VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb3000c,VPint(gSlaveVirBaseAddr+0xb3000c));
		printk("VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30008,VPint(gSlaveVirBaseAddr+0xb30008));
#endif


		schedule_timeout(5);
#if PCIE_TEST_DBG
		printk("###gSlaveCmpDone=%d \n",gSlaveCmpDone);
#endif
		while(!gSlaveCmpDone){
			schedule();
		}

	}while(gLongTest);

	kfree(wdata);
	kfree(rdata);
	printk("masterSlaveTest() GDMA test FINISH !!\n");
	return -1;
}


#else //polling mode

#define MASTER_GDMA_TEST
static int masterSlaveTest(void)
{
#ifdef MASTER_GDMA_TEST
#define SLAVE_REG_SOURCE_OFFSET 0Xb00864 //slave scu register
#define SLAVE_REG_DESTI_OFFSET  0Xb008c4
	unsigned long masterBytes;
#endif
	unsigned long *wdata, *rdata;
	int i,j;
	unsigned long *wdataAddr;
	unsigned long pat;
	unsigned char burst = 0;
	static unsigned long waitCounter = 0;



#ifdef MASTER_GDMA_TEST
	printk("slave GDMA test DRAM to DRAM & master GDMA test slave reg to slave reg \n");
#else
	printk("slave GDMA test DRAM to DRAM \n");
#endif
	wdata = (unsigned long *)kmalloc(sizeof(unsigned long) * 0xffff, GFP_KERNEL);
	rdata = (unsigned long *)kmalloc(sizeof(unsigned long) * 0xffff, GFP_KERNEL);
//	printk("wdata=0x%x rdata=0x%x \n", wdata, rdata);
	memset(wdata, 2, sizeof(unsigned long) * 0xffff);
	memset(rdata, 2, sizeof(unsigned long) * 0xffff);

	do {
		if (gBurstSize > 4) { /*random burst size*/
			burst = rand() & 0x7;
			if (burst > 4) {
				burst = 4;
			}
		} else {
			burst = gBurstSize;
		}

		gSlaveBytes =	(rand() % (128 - 8)) + 8;
#ifdef PCIE_TEST_DBG
		printk("slave GDMA transfer bytes=%d (%d words & %d byte)\n", gSlaveBytes, gSlaveBytes/4, gSlaveBytes%4);
#endif
		gUnWdata = (unsigned long *)((unsigned long)wdata | 0x20000000);
		gUnRdata = (unsigned long *)((unsigned long)rdata | 0x20000000);
//		printk("gUnWdata=0x%x gUnRdata=0x%x \n", gUnWdata, gUnRdata);
		memset(gUnWdata, 2, sizeof(unsigned long) * 0xffff);
		memset(gUnRdata, 2, sizeof(unsigned long) * 0xffff);
		for(i=0; i<=gSlaveBytes/4; i++) {
			switch(gPattern) {
				case 0: //data 0x0
					pat = 0x0;
					break;
				case 1: //data 0xff
					pat = 0xffffffff;
					break;
				case 2: //data 0x12345678
					pat = 0x12345678 + i;
					break;
				case 3:
				default:
					pat = rand();
					break;
			}
			wdataAddr = (unsigned long*)(gUnWdata) + i;
			*wdataAddr = pat;
		}

#ifdef MASTER_GDMA_TEST
		masterBytes = (rand() % (96 - 4)) + 4;
		masterBytes = masterBytes & ~(0x3);
#ifdef PCIE_TEST_DBG
		printk("master GDMA transfer bytes=%d (%d words & %d byte)\n", masterBytes, masterBytes/4, masterBytes%4);
#endif
		for(i=0; i<masterBytes/4; i++) {
			switch(gPattern) {
				case 0: //data 0x0
					VPint(gSlaveVirBaseAddr + SLAVE_REG_SOURCE_OFFSET + i*4) = 0x0;
					break;
				case 1: //data 0xff
					VPint(gSlaveVirBaseAddr + SLAVE_REG_SOURCE_OFFSET + i*4) = 0xffffffff;
					break;
				case 2: //data 0x12345678
					VPint(gSlaveVirBaseAddr + SLAVE_REG_SOURCE_OFFSET + i*4) = 0x12345678 + i;
					break;
				case 3:
				default:
					VPint(gSlaveVirBaseAddr + SLAVE_REG_SOURCE_OFFSET + i*4) = rand();
					break;
			}
		}
#endif

		/* configure slave GDMA register */
		VPint(gSlaveVirBaseAddr+0xb00040) = 0x0; /*release slave chip mc_rst*/
		VPint(gSlaveVirBaseAddr+0xb30000) = ((unsigned long)wdata & 0x1fffffff); /*physical address*/
		VPint(gSlaveVirBaseAddr+0xb30004) = ((unsigned long)rdata & 0x1fffffff);
		VPint(gSlaveVirBaseAddr+0xb3000c) = 0x4;
		VPint(gSlaveVirBaseAddr+0xb30008) = ((gSlaveBytes&0xffff)<<16)| (burst<<3) | (1<<1) | (1<<0);
#ifdef PCIE_TEST_DBG
		printk("VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30000,VPint(gSlaveVirBaseAddr+0xb30000));
		printk("VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30004,VPint(gSlaveVirBaseAddr+0xb30004));
		printk("VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb3000c,VPint(gSlaveVirBaseAddr+0xb3000c));
		printk("VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30008,VPint(gSlaveVirBaseAddr+0xb30008));
#endif


#ifdef MASTER_GDMA_TEST
		/* configure master GDMA register */
		VPint(0xbfb30000) = 0x20000000 + SLAVE_REG_SOURCE_OFFSET;
		VPint(0xbfb30004) = 0x20000000 + SLAVE_REG_DESTI_OFFSET;;
		VPint(0xbfb3000c) = 0x4;
		VPint(0xbfb30008) = ((masterBytes&0xffff)<<16)| (burst<<3) | (1<<1) | (1<<0);
#ifdef PCIE_TEST_DBG
		printk("VPint(0xbfb30000)=0x%x\n",VPint(0xbfb30000));
		printk("VPint(0xbfb30004)=0x%x\n",VPint(0xbfb30004));
		printk("VPint(0xbfb3000c)=0x%x\n",VPint(0xbfb3000c));
		printk("VPint(0xbfb30008)=0x%x\n",VPint(0xbfb30008));
#endif
#endif

/* wait for GDMA done bit */
#ifdef MASTER_GDMA_TEST
		do {
#if 1
			for(j=0; j<5000; j++){
				if(((VPint(gSlaveVirBaseAddr+0xb30204) & (1<<0))) &&
						((VPint(0xbfb30204) & (1<<0)))) {
					break;
				}
				mdelay(1);
			}

			if(j == 5000){
				printk("interrupt ERROR!! VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30204,VPint(gSlaveVirBaseAddr+0xb30204));
				printk("interrupt ERROR!! VPint(0x%x)=0x%x\n",0xbfb30204,VPint(0xbfb30204));

				printk("compare slave GDMA test data \n");
				slaveGdmaCmp();
				printk("compare master GDMA test data \n");
				masterCmp(gSlaveVirBaseAddr + SLAVE_REG_SOURCE_OFFSET, gSlaveVirBaseAddr + SLAVE_REG_DESTI_OFFSET, masterBytes);
				return -1;
			}else{
				break;
			}
#else
			waitCounter++;
			if(((VPint(gSlaveVirBaseAddr+0xb30204) & (1<<0))) &&
					((VPint(0xbfb30204) & (1<<0)))) {
				waitCounter = 0;
				break;
			}
			if(waitCounter == 5000) {
				printk("waitCounter=%d \n",waitCounter);
				printk("interrupt ERROR!! VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30204,VPint(gSlaveVirBaseAddr+0xb30204));
				printk("interrupt ERROR!! VPint(0x%x)=0x%x\n",0xbfb30204,VPint(0xbfb30204));
				waitCounter = 0;

				printk("compare slave GDMA test data \n");
				slaveGdmaCmp();
				printk("compare master GDMA test data \n");
				masterCmp(gSlaveVirBaseAddr + SLAVE_REG_SOURCE_OFFSET, gSlaveVirBaseAddr + SLAVE_REG_DESTI_OFFSET, masterBytes);
				return -1;
			}
#endif
		}while(1);
#else
		do{
#if 1
			for(j=0; j<5000; j++){
				if(((VPint(gSlaveVirBaseAddr+0xb30204) & (1<<0)))){
					break;
				}
				mdelay(1);
			}

			if(j == 5000){
				printk("interrupt ERROR!! VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30204,VPint(gSlaveVirBaseAddr+0xb30204));

				printk("compare slave GDMA test data \n");
				slaveGdmaCmp();
				return -1;
			}else{
				break;
			}
#else
			waitCounter++;
			if(((VPint(gSlaveVirBaseAddr+0xb30204) & (1<<0)))){
				waitCounter = 0;
				break;
			}
			if(waitCounter == 5000){
				printk("waitCounter=%d \n",waitCounter);
				printk("interrupt ERROR!! VPint(0x%x)=0x%x\n",gSlaveVirBaseAddr+0xb30204,VPint(gSlaveVirBaseAddr+0xb30204));
				waitCounter = 0;

				printk("compare slave GDMA test data \n");
				slaveGdmaCmp();
				return -1;
			}
#endif
		}while(1);
#endif


		/* slave GDMA data compare */
		VPint(gSlaveVirBaseAddr+0xb30204) = (1<<0);
		if(slaveGdmaCmp() == -1){
			return -1;
		}
#ifdef MASTER_GDMA_TEST
		/* master GDMA data compare */
		VPint(0xbfb30204) = (1<<0);
		if(masterCmp(gSlaveVirBaseAddr + SLAVE_REG_SOURCE_OFFSET, gSlaveVirBaseAddr + SLAVE_REG_DESTI_OFFSET, masterBytes) == -1){
			return -1;
		}
#endif

		schedule();
	}while(gLongTest);

	kfree(wdata);
	kfree(rdata);
	printk("masterSlaveTest() GDMA test FINISH !!\n");
	return 0;
}
#endif



static int gdmaTest(void)
{

	if(gTestCase == 1)
	{
		slaveDramRegTest();
	}
	else
	{
		masterSlaveTest();
	}
	return 0;
}



static int wifiGdmaDataPathReg(void)
{
	int i;
	unsigned int val1,val2,val3,val4;

	printk("\n## slave wifi/gdma traffic counter: (1)##\n");
	printk("Slave RC1: pcie Rx for slave wifi (step1)\n");
	for(i=0; i<3 ;i++){
		printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr+ 0xb83230 + i*4, VPint(gSlaveVirBaseAddr+ 0xb83230 + i*4));
	}
	printk("\nSlave RC1: pcie Rx for slave wifi (step2)\n");
	for(i=0; i<2 ;i++){
		printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr+ 0xb80320 + i*4, VPint(gSlaveVirBaseAddr+ 0xb80320 + i*4));
	}
	printk("\nmaster RC0: pcie Rx for slave wifi/gdma (step1)\n");
	for(i=0; i<3 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb82230 + i*4, VPint(0xbfb82230 + i*4));
	}
	printk("\nmaster RC0: pcie Rx for slave wifi/gdma (step2)\n");
	for(i=0; i<2 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb80220 + i*4, VPint(0xbfb80220 + i*4));
	}
	printk("\nmaster RC0: pcie Tx for wifi/gdma read back\n");
	for(i=0; i<3 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb82210 + i*4, VPint(0xbfb82210 + i*4));
	}
	printk("\nslave EP0: pcie Rx for wifi/gdma read data\n");
	printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr + 0xb82240 ,VPint(gSlaveVirBaseAddr + 0xb82240));
	printk("\n slave RC1: pcie Tx for wifi read back\n");
	for(i=0; i<3 ;i++){
		printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr + 0xb83210 + i*4, VPint(gSlaveVirBaseAddr + 0xb83210 + i*4));
	}

	printk("\n\n ## slave wifi/gdma traffic counter: (2)##\n");
	val1 = VPint(gSlaveVirBaseAddr+ 0xb83230);
	val2 = VPint(gSlaveVirBaseAddr+ 0xb80320);
	val3 = VPint(gSlaveVirBaseAddr+ 0xb83210);
	printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr+ 0xb83230, val1);
	printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr+ 0xb80320, val2);
	printk("reg(0x%x) = 0x%x  \n", gSlaveVirBaseAddr+ 0xb83210, val3);
	printk("1.(slave RC) TRGT1_WR_EOT_CNT_WIFI=0x%x    TRGT1_WR_HV_CNT_WIFI=0x%x \n", val1 & (0xff << 24), val1 & (0xff << 8));
	printk("		 RB1_OUT_WR_REQ_CNT_WIFI=0x%x  RB1_OUT_WR_DONE_WIFI=0x%x \n", val2 & (0xff << 24), val2 & (0xff << 8));
	printk("2.(slave RC1)TRGT1_RD_EOT_CNT_WIFI=0x%x    TRGT1_RD_HV_CNT_WIFI=0x%x \n", val1 & (0xff << 16), val1 & (0xff << 0));
	printk("		 RB1_OUT_RD_REQ_CNT_WIFI=0x%x  RB1_OUT_RD_DONE_WIFI=0x%x \n", val2 & (0xff << 16), val2 & (0xff << 0));
	printk("  (slave EP0)CPL_EOT_CNT=0x%x \n", val4 & (0xff << 4));
	val1 = VPint(0xbfb82230);
	val2 = VPint(0xbfb80220);
	val3 = VPint(0xbfb82210);
	val4 = VPint(gSlaveVirBaseAddr+ 0xb82240);
	printk("reg(0x%x) = 0x%x  ", 0xbfb82230, val1);
	printk("reg(0x%x) = 0x%x  ", 0xbfb80220, val2);
	printk("reg(0x%x) = 0x%x  ", 0xbfb82210, val3);
	printk("reg(0x%x) = 0x%x  \n", 0xbfb82210, val4);
	printk("1.(master RC0) TRGT1_WR_EOT_CNT_WIFI=0x%x    TRGT1_WR_HV_CNT_WIFI=0x%x \n", val1 & (0xff << 8), val1 & (0xff << 24));
	printk("		  RB1_OUT_WR_REQ_CNT_WIFI=0x%x  RB1_OUT_WR_DONE_WIFI=0x%x \n", val2 & (0xff << 8), val2 & (0xff << 24));
	printk("2.(master RC0) TRGT1_RD_EOT_CNT_WIFI=0x%x    TRGT1_RD_HV_CNT_WIFI=0x%x \n", val1 & (0xff << 16), val1 & (0xff << 0));
	printk("		  RB1_OUT_RD_REQ_CNT_WIFI=0x%x  RB1_OUT_RD_DONE_WIFI=0x%x \n", val2 & (0xff << 16), val2 & (0xff << 0));
	printk("		  CL1_WR_EOT_CNT_WIFI=0x%x  	 CL1_WR_HV_CNT_WIFI=0x%x \n", val3 & (0xff << 24), val3 & (0xff << 8));
	val1 = VPint(gSlaveVirBaseAddr+ 0xb83238);
	val2 = VPint(gSlaveVirBaseAddr+ 0xb80324);
	printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr+ 0xb83238, val1);
	printk("reg(0x%x) = 0x%x  \n", gSlaveVirBaseAddr+ 0xb80324, val2);
	printk("5.(slave RC)  TRGT1_ADDR_WIFI=0x%x    RB1_OUT_LS_ADDR_WIFI=0x%x \n", val1, val2);
	val1 = VPint(0xbfb82238);
	val2 = VPint(0xbfb80224);
	printk("reg(0x%x) = 0x%x  ", 0xbfb82238, val1);
	printk("reg(0x%x) = 0x%x  \n", 0xbfb80224, val2);
	printk("5.(master RC0)  TRGT1_ADDR_WIFI=0x%x    RB1_OUT_LS_ADDR_WIFI=0x%x \n", val1, val2);

	printk("\n## master wifi traffic counter: (1)##\n");
	printk("\nmaster RC1: pcie Rx for slave wifi (step1)\n");
	for(i=0; i<3 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb83230 + i*4, VPint(0xbfb83230 + i*4));
	}
	printk("\nmaster RC1: pcie Rx for slave wifi (step2)\n");
	for(i=0; i<2 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb80230 + i*4, VPint(0xbfb80230 + i*4));
	}
	printk("\nmaster RC1: pcie Tx for wifi read back\n");
	for(i=0; i<3 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb83210 + i*4, VPint(0xbfb83210 + i*4));
	}
	printk("\n## master wifi traffic counter: (2)##\n");
	val1 = VPint(0xbfb83230);
	val2 = VPint(0xbfb80320);
	val3 = VPint(0xbfb83210);
	printk("reg(0x%x) = 0x%x  ", 0xbfb83230, val1);
	printk("reg(0x%x) = 0x%x  ", 0xbfb80320, val2);
	printk("reg(0x%x) = 0x%x  \n", 0xbfb83210, val3);
	printk("1.(master RC1) TRGT1_WR_EOT_CNT_WIFI=0x%x    TRGT1_WR_HV_CNT_WIFI=0x%x \n", val1 & (0xff << 24), val1 & (0xff << 8));
	printk("		  RB1_OUT_WR_REQ_CNT_WIFI=0x%x  RB1_OUT_WR_DONE_WIFI=0x%x \n", val2 & (0xff << 24), val2 & (0xff << 8));
	printk("2.(master RC1) TRGT1_RD_EOT_CNT_WIFI=0x%x    TRGT1_RD_HV_CNT_WIFI=0x%x \n", val1 & (0xff << 24), val1 & (0xff << 8));
	printk("		  RB1_OUT_RD_REQ_CNT_WIFI=0x%x  RB1_OUT_RD_DONE_WIFI=0x%x \n", val2 & (0xff << 24), val2 & (0xff << 8));
	printk("		  CL1_WE_EOT_CNT_WIFI=0x%x      CL1_WR_HV_WIFI=0x%x \n", val3 & (0xff << 24), val3 & (0xff << 8));
	val1 = VPint(0xbfb83238);
	val2 = VPint(0xbfb80324);
	printk("reg(0x%x) = 0x%x  ", 0xbfb83238, val1);
	printk("reg(0x%x) = 0x%x  \n", 0xbfb80324, val2);
	printk("3.(master RC1) TRGT1_ADDR_WIFI=0x%x    RB1_OUT_LS_ADDR_WIFI=0x%x \n", val1, val2);

}

static int bondingDataPathReg(void)
{
	int i;
	unsigned int val1,val2,val3,val4,val5,val6;

	printk("\n## bonding TX path: (1)##\n");
	printk("master RC0: pcie Tx for bonding tx (step1)\n");
	for(i=0; i<2 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb80210 + i*4, VPint(0xbfb80210 + i*4));
	}
	printk("\nmaster RC0: pcie Tx for bonding tx (step2)\n");
	for(i=0; i<3 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb82220 + i*4, VPint(0xbfb82220 + i*4));
	}
	printk("\nslave EP0: pcie Rx for bonding tx (step1)\n");
	for(i=0; i<3 ;i++){
		printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr + 0xb82260 + i*4, VPint(gSlaveVirBaseAddr + 0xb82260 + i*4));
	}
	printk("\nslave EP0: pcie Rx for bonding tx (step2)\n");
	for(i=0; i<2 ;i++){
		printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr + 0xb80230 + i*4, VPint(gSlaveVirBaseAddr + 0xb80230 + i*4));
	}

	printk("\n\n## bonding TX path: (2)##\n");
	val1 = VPint(0xbfb80210);
	val2 = VPint(0xbfb82220);
	val3 = VPint(gSlaveVirBaseAddr+ 0xb82260);
	val4 = VPint(gSlaveVirBaseAddr+ 0xb80230);
	printk("reg(0x%x) = 0x%x  ", 0xbfb80210, val1);
	printk("reg(0x%x) = 0x%x  ", 0xbfb82220, val2);
	printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr+ 0xb82260, val3);
	printk("reg(0x%x) = 0x%x  \n", gSlaveVirBaseAddr+ 0xb80230, val4);
	printk("1. RB0_IN_WR_DONE_CNT=0x%x    RB0_IN_WR_REQ_CNT=0x%x \n", val1 & (0xff << 24), val1 & (0xff << 8));
	printk("   CL2_WR_CNT=0x%x  \n", val2 & (0xf << 8));
	printk("   TRGT1_WR_HV_CNT_BONDING=0x%x  RB0_OUT_WR_CNT_BONDING=0x%x \n", val3 & (0xff << 8), val4 & (0xff << 8));
	val1 = VPint(0xbfb80214);
	val2 = VPint(0xbfb82228);
	val3 = VPint(gSlaveVirBaseAddr+ 0xb82268);
	val4 = VPint(gSlaveVirBaseAddr+ 0xb80234);
	printk("reg(0x%x) = 0x%x  ", 0xbfb80214, val1);
	printk("reg(0x%x) = 0x%x  ", 0xbfb82228, val2);
	printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr+ 0xb82268, val3);
	printk("reg(0x%x) = 0x%x  \n", gSlaveVirBaseAddr+ 0xb80234, val4);
	printk("2. RB0_LS_ADDR=0x%x         CL2_ADDR=0x%x \n", val1 , val2);
	printk("   TRGT1_ADDR_BONDING=0x%x  RB0_OUT_LS_BONDING=0x%x \n", val3, val4);

	printk("## bonding RX path: (1)##\n");
	printk("slave EP0: pcie Tx for bonding rx/wifi (step1)\n");
	for(i=0; i<2 ;i++){
		printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr + 0xb80210 + i*4, VPint(gSlaveVirBaseAddr + 0xb80210 + i*4));
	}
	printk("\nslave EP0: pcie Tx for bonding rx/wifi (step2)\n");
	for(i=0; i<3 ;i++){
		printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr + 0xb82200 + i*4, VPint(gSlaveVirBaseAddr + 0xb82200 + i*4));
	}
	printk("\nmaster RC0: pcie Rx for bonding rx (step1)\n");
	for(i=0; i<3 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb82260 + i*4, VPint(0xbfb82260 + i*4));
	}
	printk("\nmaster RC0: pcie Rx for bonding rx (step2)\n");
	for(i=0; i<2 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb80230 + i*4, VPint(0xbfb80230 + i*4));
	}
	printk("\nmaster RC0: pcie Rx for slave wifi (step1)\n");
	for(i=0; i<3 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb82230 + i*4, VPint(0xbfb82230 + i*4));
	}
	printk("\nmaster RC0: pcie Rx for slave wifi (step2)\n");
	for(i=0; i<2 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb80220 + i*4, VPint(0xbfb80220 + i*4));
	}

	printk("\n\n## bonding RX path: (2)##\n");
	val1 = VPint(gSlaveVirBaseAddr+ 0xb80210);
	val2 = VPint(gSlaveVirBaseAddr+ 0xb82200);
	val3 = VPint(0xbfb82260);
	val4 = VPint(0xbfb80230);
	val5 = VPint(0xbfb82230);
	val6 = VPint(0xbfb80220);
	printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr+ 0xb80210, val1);
	printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr+ 0xb82200, val2);
	printk("reg(0x%x) = 0x%x  ", 0xbfb82260, val3);
	printk("reg(0x%x) = 0x%x  ", 0xbfb80230, val4);
	printk("reg(0x%x) = 0x%x  ", 0xbfb82230, val5);
	printk("reg(0x%x) = 0x%x  \n", 0xbfb80220, val6);
	printk("1. RB0_IN_WR_DONE_CNT=0x%x    RB0_IN_WR_REQ_CNT=0x%x \n", val1 & (0xff << 24), val1 & (0xff << 8));
	printk("   CL0_WR_CNT=0x%x  \n", val2 & (0xf << 8));
	printk("   TRGT1_WR_HV_CNT_BONDING + TRGT1_WR_HV_CNT_WIFI=0x%x \n", (val3 & (0xff << 8)) + val5 & (0xff << 8));
	printk("   RB0_OUT_WR_CNT_BONDING + RB0_OUT_WR_REQ_CNT_WIFI=0x%x \n", (val4 & (0xff << 8)) + val6 & (0xff << 8));
	printk("   RB0_OUT_WR_CNT_BONDING + RB0_OUT_WR_DONE_CNT_WIFI=0x%x \n", (val4 & (0xff << 8)) + val6 & (0xff << 24));
	val1 = VPint(gSlaveVirBaseAddr+ 0xb80214);
	val2 = VPint(gSlaveVirBaseAddr+ 0xb82208);
	val3 = VPint(0xbfb82268);
	val4 = VPint(0xbfb80234);
	val5 = VPint(0xbfb82238);
	val6 = VPint(0xbfb80224);
	printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr+ 0xb80214, val1);
	printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr+ 0xb82208, val2);
	printk("reg(0x%x) = 0x%x  ", 0xbfb82268, val3);
	printk("reg(0x%x) = 0x%x  ", 0xbfb80234, val4);
	printk("reg(0x%x) = 0x%x  ", 0xbfb82238, val5);
	printk("reg(0x%x) = 0x%x  \n", 0xbfb80224, val6);
	printk("2. RB0_LS_ADDR=0x%x    CL0_ADDR=0x%x \n", val1, val2);
	printk("   TRGT1_ADDR_BONDING=0x%x / TRGT1_ADDR_WIFI=0x%x \n", val3, val5);
	printk("   RB0_OUT_LS_ADDR_BONDING=0x%x / RB0_OUT_LS_ADR_WIFI=0x%x \n", val4, val6);
}

static int crtlPathReg(void)
{
	int i;
	unsigned int val1,val2,val3,val4,val5;

	printk("\n## control path: (1)##\n");
	printk("master RC0: pcie Tx for control (step1)\n");
	for(i=0; i<2 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb80200 + i*4, VPint(0xbfb80200 + i*4));
	}
	printk("\nmaster RC0: pcie Tx for control (step2)\n");
	for(i=0; i<3 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb82200 + i*4, VPint(0xbfb82200 + i*4));
	}

#if defined(TCSUPPORT_BONDING)
	printk("\nslave EP0: pcie Rx for control\n");
	for(i=0; i<3 ;i++){
		printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr + 0xb82230 + i*4, VPint(gSlaveVirBaseAddr + 0xb82230 + i*4));
	}
	printk("\nslave EP0: read back Tx\n");
	for(i=0; i<3 ;i++){
		printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr + 0xb82210 + i*4, VPint(gSlaveVirBaseAddr + 0xb82210 + i*4));
	}
#endif

	printk("\nslave EP0: read back Rx\n");
	printk("reg(0x%x) = 0x%x  ", 0xbfb82240, VPint(0xbfb82240));
	printk("\nmaster RC1: pcie Tx for contrl\n");
	for(i=0; i<2 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb80300 + i*4, VPint(0xbfb80300 + i*4));
	}
	printk("\nmaster RC0: read back Rx\n");
	printk("reg(0x%x) = 0x%x  ", 0xbfb83240, VPint(0xbfb83240));

	printk("\n\n## control path: (2)##\n");
	val1 = VPint(0xbfb80200);
	val2 = VPint(0xbfb82200);
#if defined(TCSUPPORT_BONDING)
	val3 = VPint(gSlaveVirBaseAddr+ 0xb82230);
	val4 = VPint(gSlaveVirBaseAddr+ 0xb82210);
#endif
	val5 = VPint(0xbfb82240);
	printk("reg(0x%x) = 0x%x  ", 0xbfb80200, val1);
	printk("reg(0x%x) = 0x%x  ", 0xbfb82200, val2);
#if defined(TCSUPPORT_BONDING)
	printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr+ 0xb82230, val3);
	printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr+ 0xb82210, val4);
#endif
	printk("reg(0x%x) = 0x%x  \n", 0xbfb82240, val5);
	printk("1. PB0_IN_WR_REQ_CNT=0x%x    CL0_WR_HV_CNT=0x%x \n", val1 & (0xff << 8), val2 & (0xff << 8));
#if defined(TCSUPPORT_BONDING)
	printk("   P1_TRGT1_WR_EOT_CNT=0x%x  P1_TRGT1_WR_HV_CNT=0x%x \n", val3 & (0xff << 24), val3 & (0xff << 8));
#endif
	printk("2. PB0_IN_RD_REQ_CNT=0x%x    CL0_RD_HV_CNT=0x%x \n", val1 & (0xff << 0), val2 & (0xff << 0));
#if defined(TCSUPPORT_BONDING)
	printk("   P1_TRGT1_RD_EOT_CNT=0x%x  P1_TRGT1_RD_HV_CNT=0x%x \n", val3 & (0xff << 16), val3 & (0xff << 0));
#endif
	printk("3. PB0_IN_RD_REQ_CNT=0x%x    CPL_EOT_CNT=0x%x \n", val1 & (0xff << 0), val5 & (0xff << 4));
#if defined(TCSUPPORT_BONDING)
	printk("   CL1_WR_EOT_CNT=0x%x       CL1_WR_HV_CNT=0x%x \n", val4 & (0xff << 24), val4 & (0xff << 8));
#endif



	val1 = VPint(0xbfb80204);
	val2 = VPint(0xbfb82208);
#if defined(TCSUPPORT_BONDING)
	val3 = VPint(gSlaveVirBaseAddr+ 0xb82238);
#endif
	printk("reg(0x%x) = 0x%x  ", 0xbfb80204, val1);
	printk("reg(0x%x) = 0x%x  ", 0xbfb82208, val2);
#if defined(TCSUPPORT_BONDING)
	printk("reg(0x%x) = 0x%x  \n", gSlaveVirBaseAddr+ 0xb82238, val3);
#endif
	printk("4. PB0_IN_LS_ADDR=0x%x  CL0_LS_ADDR=0x%x \n", val1, val2);
#if defined(TCSUPPORT_BONDING)
	printk("   P1_TRGT1_ADDR=0x%x\n", val3);
#endif
	val1 = VPint(0xbfb80304);
	val2 = VPint(0xbfb83240);
	printk("reg(0x%x) = 0x%x  ", 0xbfb80304, val1);
	printk("reg(0x%x) = 0x%x  \n", 0xbfb83240, val2);
	printk("5. PB1_IN_RD_CNT=0x%x  CPL_EOT_CNT=0x%x\n", val1, val2);
}

static int rbus2PcieDbgReg(void)
{
	int i;

	printk("\n## XP_RB2RQT debug register ##\n");
	printk("master RC0: \n");
	for(i=0; i<5 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb82270 + i*4, VPint(0xbfb82270 + i*4));
	}
	printk("\nmaster RC1: \n");
	for(i=0; i<5 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb83270 + i*4, VPint(0xbfb83270 + i*4));
	}

#if defined(TCSUPPORT_BONDING)
	printk("\nslave EP0: \n");
	for(i=0; i<5 ;i++){
		printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr + 0xb82270 + i*4, VPint(gSlaveVirBaseAddr + 0xb82270 + i*4));
	}
	printk("\nslave RC: \n");
	for(i=0; i<5 ;i++){
		printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr + 0xb83270 + i*4, VPint(gSlaveVirBaseAddr + 0xb83270 + i*4));
	}
#endif
}

static int pcie2RbsDbgReg(void)
{
	int i;

	printk("\n\n## XP_TRG2RB debug register ##\n");
	printk("master RC0: \n");
	for(i=0; i<4 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb82300 + i*4, VPint(0xbfb82300 + i*4));
	}
	printk("\n");
	for(i=0; i<4 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb80340 + i*4, VPint(0xbfb80340 + i*4));
	}
	printk("\nmaster RC1: \n");
	for(i=0; i<4 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb83300 + i*4, VPint(0xbfb83300 + i*4));
	}
	printk("\n");
	for(i=0; i<4 ;i++){
		printk("reg(0x%x) = 0x%x  ", 0xbfb80350 + i*4, VPint(0xbfb80350 + i*4));
	}

#if defined(TCSUPPORT_BONDING)
	printk("\nslave EP0: \n");
	for(i=0; i<4 ;i++){
		printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr + 0xb82300 + i*4, VPint(gSlaveVirBaseAddr + 0xb82300 + i*4));
	}
	printk("\n");
	for(i=0; i<4 ;i++){
		printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr + 0xb80340 + i*4, VPint(gSlaveVirBaseAddr + 0xb80340 + i*4));
	}
	printk("\nslave RC: \n");
	for(i=0; i<4 ;i++){
		printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr + 0xb83300 + i*4, VPint(gSlaveVirBaseAddr + 0xb83300 + i*4));
	}
	printk("\n");
	for(i=0; i<4 ;i++){
		printk("reg(0x%x) = 0x%x  ", gSlaveVirBaseAddr + 0xb80350 + i*4, VPint(gSlaveVirBaseAddr + 0xb80350 + i*4));
	}
#endif
}


static void dumpPCIeDbgReg(void)
{
#if defined(TCSUPPORT_BONDING)
	wifiGdmaDataPathReg();
	bondingDataPathReg();
#endif
	crtlPathReg();
	rbus2PcieDbgReg();
	pcie2RbsDbgReg();
}



static int pcieRegTest(void)
{
#define PCI_VENDOR_ID_RT		0x1814
#define NIC5392_PCIe_DEVICE_ID  0x5392
#define NIC3593_PCIe_DEVICE_ID  0x3593
#define PCI_VENDOR_ID_MTK		0x14c3
#define PCI_DEVICE_ID_MTK		0x0801
	unsigned int val;

	printk("pcie configuration space reg test START \n");
	do{

		if((VPint(0xbfb82050) & 0x1) != 0){
			VPint(0xbfb80020) = 0x0;

			val = VPint((0xbfb80024 & 0xf) + 0xbfb003a0);
			val = VPint(0xbfb80024);
			if( (val == 0xffffffff) || (val != ( PCI_DEVICE_ID_MTK <<16 | PCI_VENDOR_ID_MTK ))) {
				printk("RC0 FAIL !!!!\n");
				printk("VPint(0xbfb80338)=0x%x VPint(0xbfb8033c)=0x%x\n",VPint(0xbfb80338),VPint(0xbfb8033c));
				printk("###VPint(0xbfb80020)=0x%x (0x%x) val=0x%x\n",VPint(0xbfb80020), 0x0, val);
//				printk("VPint(0xbfb80020)=0x%x VPint(0xbfb80024)=0x%x\n",VPint(0xbfb80020),VPint(0xbfb80024));
				return -1;
			}


			VPint(0xbfb80020) = 0x1000000;
			val = VPint((0xbfb80024 & 0xf) + 0xbfb003a0);
			val = VPint(0xbfb80024);
			if( (val == 0xffffffff) || ((val != ( NIC5392_PCIe_DEVICE_ID <<16 | PCI_VENDOR_ID_RT )) && (val != ( NIC3593_PCIe_DEVICE_ID <<16 | PCI_VENDOR_ID_RT ))) ) {
				printk("EP0 FAIL!!!\n");
				printk("VPint(0xbfb80338)=0x%x VPint(0xbfb8033c)=0x%x\n",VPint(0xbfb80338),VPint(0xbfb8033c));
				printk("###VPint(0xbfb80020)=0x%x (0x%x) val=0x%x\n",VPint(0xbfb80020), 0x1000000, val);
//				printk("VPint(0xbfb80020)=0x%x VPint(0xbfb80024)=0x%x\n",VPint(0xbfb80020),VPint(0xbfb80024));
				return -1;
			}
		}
		else{
			printk("RC0 DOESN'T LINK UP!!!\n");
		}

		if((VPint(0xbfb83050) & 0x1) != 0){
			VPint(0xbfb80020) = 0x80000;
			val = VPint((0xbfb80024 & 0xf) + 0xbfb003a0);
			val = VPint(0xbfb80024);
			if( (val == 0xffffffff) || (val != ( PCI_DEVICE_ID_MTK <<16 | PCI_VENDOR_ID_MTK ))) {
				printk("RC1 FAIL !!!!\n");
				printk("VPint(0xbfb80338)=0x%x VPint(0xbfb8033c)=0x%x\n",VPint(0xbfb80338),VPint(0xbfb8033c));
				printk("###VPint(0xbfb80020)=0x%x (0x%x) val=0x%x\n",VPint(0xbfb80020), 0x80000, val);
//				printk("VPint(0xbfb80020)=0x%x VPint(0xbfb80024)=0x%x\n",VPint(0xbfb80020),VPint(0xbfb80024));
				return -1;
			}

			VPint(0xbfb80020) = 0x2000000;
			val = VPint((0xbfb80024 & 0xf) + 0xbfb003a0);
			val = VPint(0xbfb80024);
			if( (val == 0xffffffff) || ((val != ( NIC5392_PCIe_DEVICE_ID <<16 | PCI_VENDOR_ID_RT )) && (val != ( NIC3593_PCIe_DEVICE_ID <<16 | PCI_VENDOR_ID_RT ))) ) {
				printk("EP1 FAIL!!!\n");
				printk("VPint(0xbfb80338)=0x%x VPint(0xbfb8033c)=0x%x\n",VPint(0xbfb80338),VPint(0xbfb8033c));
				printk("###VPint(0xbfb80020)=0x%x (0x%x) val=0x%x\n",VPint(0xbfb80020), 0x2000000, val);
//				printk("VPint(0xbfb80020)=0x%x VPint(0xbfb80024)=0x%x\n",VPint(0xbfb80020),VPint(0xbfb80024));
				return -1;
			}
		}
		else{
			printk("RC0 DOESN'T LINK UP!!!\n");
		}

		schedule();
	}while(gPcieRegTest);

	printk("pcie configuration space reg test SUCCESS! \n");
	return 0;
}



#ifdef GDMA_INT_MODE
static irqreturn_t gdmaIsr(int irq , void *dev)
{
	uint32 val_gdma;

#if PCIE_TEST_DBG
	printk("pcie irq = %d\n", irq);
	printk("clear gdma interrupt\n");
#endif
	val_gdma = VPint((gSlaveVirBaseAddr+0xb30204));
#if PCIE_TEST_DBG
	printk("(gSlaveVirBaseAddr+0xb30204): %x\n", val_gdma);
#endif
	VPint((gSlaveVirBaseAddr+0xb30204)) = val_gdma;


	tasklet_schedule(&gSlaveTasklet);

	return IRQ_HANDLED;
}
#endif

static int __init gdma_init_module(void)
{
    struct proc_dir_entry *gdmaTest_proc;
    struct proc_dir_entry *gdmaEnable_proc;
    struct proc_dir_entry *pcieReset_proc;
    struct proc_dir_entry *pcieDbgReg_proc;
    struct proc_dir_entry *pcieRegTest_proc;
    int err = 0;

#if defined(TCSUPPORT_BONDING)
    gSlaveVirBaseAddr = pcie_virBaseAddr_get();
#endif

    printk("Create GDMA test proc \n");
    gdmaTest_proc = create_proc_entry("gdma_test", 0, NULL);
    gdmaTest_proc->read_proc = gdmaTest_read_proc;
    gdmaTest_proc->write_proc = gdmaTest_write_proc;

    printk("Create GDMA enable long test proc \n");
    gdmaEnable_proc = create_proc_entry("enable_longTest", 0, NULL);
    gdmaEnable_proc->write_proc = gdmaEnable_write_proc;

    printk("Create PCIE reset proc \n");
    pcieReset_proc = create_proc_entry("pci_reset", 0, NULL);
    pcieReset_proc->write_proc = pcieReset_write_proc;
    pcieReset_proc->read_proc = pcieReset_read_proc;

    printk("Create dump PCIe debug reg proc \n");
    pcieDbgReg_proc = create_proc_entry("dumpPcie_dbgReg", 0, NULL);
    pcieDbgReg_proc->read_proc = pcieDbgReg_read_proc;

    printk("Create PCIE configuration space reg test proc \n");
    pcieRegTest_proc = create_proc_entry("pcieReg_test", 0, NULL);
    pcieRegTest_proc->write_proc = pcieRegTest_write_proc;





#ifdef GDMA_INT_MODE
	printk("request slave GDMA irq\n");
	err = request_irq(GDMA_INT, gdmaIsr, 0, "GDMA", NULL);
	if (err) {
		printk("err=%d\n", err);
		printk("request GDMA irq fail \n");
	}

    tasklet_init(&gSlaveTasklet, (void *) slaveGdmaCmp, 0);
#endif



    return 0;
}

//
// Driver module unload function
//
static void __exit gdma_cleanup_module(void)
{
	printk("GDMA driver test program exit\n");
	remove_proc_entry("gdma_test", NULL);

	remove_proc_entry("enable_longTest", NULL);

	remove_proc_entry("pci_reset", NULL);

	remove_proc_entry("dumpPcie_dbgReg", NULL);

	remove_proc_entry("pcieReg_test", NULL);

#ifdef GDMA_INT_MODE
	free_irq(GDMA_INT, NULL);
#endif
}

module_init(gdma_init_module);
module_exit(gdma_cleanup_module);

