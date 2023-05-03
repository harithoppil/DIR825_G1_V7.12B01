#include <linux/pci.h>
#include <asm/irq.h>
#include <linux/interrupt.h>
#include <linux/etherdevice.h>
#include <asm/tc3162/tc3162.h>
#include <linux/proc_fs.h>

//#ifdef MT7510_PCIE_TEST
//#define DMT_INT_TEST
//#define GDMA_INT_TEST
//#define ERROR_INT_TEST
//#endif


//
// Function declarations
//
static int __devinit bonding_probe(struct pci_dev *pci_dev, const struct pci_device_id  *ent);
static void __exit bonding_cleanup_module(void);
static int __init bonding_init_module(void);

#ifdef DMT_INT_TEST
static int dmtCounter_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data);
static int dmtCounter_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data);
#endif

//
// Ralink PCI device table, include all supported chipsets
//
static struct pci_device_id bonding_pci_tbl[] __devinitdata =
{
	{PCI_DEVICE(0x14c3, 0x7510)},
    {0,}		// terminate list
};
MODULE_DEVICE_TABLE(pci, bonding_pci_tbl);

//
// Our PCI driver structure
//
static struct pci_driver bonding_driver =
{
    name:       "bonding_probe",
    id_table:   bonding_pci_tbl,
    probe:      bonding_probe,
};


/***************************************************************************
 *
 *	PCI device initialization related procedures.
 *
 ***************************************************************************/
unsigned long csr_addr;

#ifdef DMT_INT_TEST
#define ARRAYSIZE  8192 //2048*4
struct net_device *dmtTest_dev = 0;
static unsigned int gDmtCounter = 0;
static unsigned int *gSymCount;
static int gFull = 0;
#endif

#ifdef GDMA_INT_TEST
struct net_device *gdmaTest_dev = 0;
#endif

#ifdef ERROR_INT_TEST
struct net_device *errTest_dev = 0;
#endif


unsigned long pcie_virBaseAddr_get(void)
{
	return csr_addr;
}
EXPORT_SYMBOL(pcie_virBaseAddr_get);

irqreturn_t pcieIsr(int irq , void *dev)
{
#ifdef GDMA_INT_TEST
	uint32 val_gdma;
	printk("pcie irq = %d\n", irq);
	printk("clear gdma interrupt\n");
	val_gdma = VPint((csr_addr+0xb30204));
	printk("(csr_addr+0xb30204): %lx\n", val_gdma);
	VPint((csr_addr+0xb30204)) = val_gdma;

	return IRQ_HANDLED;
#endif

#ifdef DMT_INT_TEST
	uint32 val_dmt;
	static int idx = 0;
	static int full = 0;

	if(gDmtCounter < ARRAYSIZE)
	{
		gSymCount[gDmtCounter] = VPint((csr_addr+0x900c34));
		gDmtCounter++;
	}
	else{
		if(gFull == 0){
			printk("array full \n");
			gFull = 1;
		}
	}

	val_dmt = VPint((csr_addr+0x900004));

	return IRQ_HANDLED;
#endif

#ifdef ERROR_INT_TEST
	uint32 val;
	uint32 status;
	uint32 errStatus;

	printk("pcie slave dmt irq = %d\n", irq);
	val = VPint(0xbfb80060);
	printk("(0xbfb80060): %lx\n", val);
	if((val & (1<<1)) != 0){
		VPint(0xbfb82070) = 1; //clear interrupt status
		VPint(0xbfb82070) = 0; //set interrupt status
		printk("RC0 error \n");

		//check root error status
		VPint(0xbfb80020) = 0x130;
		printk("VPint(0xbfb80020)=%lx \n",VPint(0xbfb80020));
		printk("VPint(0xbfb80024)=%lx \n",VPint(0xbfb80024));
		//check error source ID
		VPint(0xbfb80020) = 0x134;
		printk("VPint(0xbfb80020)=%lx \n",VPint(0xbfb80020));
		printk("VPint(0xbfb80024)=%lx \n",VPint(0xbfb80024));
		//check uncorrectable error status
		VPint(0xbfb80020) = 0x104;
		printk("VPint(0xbfb80020)=%lx \n",VPint(0xbfb80020));
		printk("VPint(0xbfb80024)=%lx \n",VPint(0xbfb80024));
		//check correctable error status
		VPint(0xbfb80020) = 0x110;
		errStatus = VPint(0xbfb80024);
		printk("VPint(0xbfb80020)=%lx \n",VPint(0xbfb80020));
		printk("VPint(0xbfb80024)=%lx \n",errStatus);
		//clear correctable error status
		VPint(0xbfb80024) = errStatus;
		printk("clear VPint(0xbfb80024)=%lx \n",VPint(0xbfb80024));


		//check error counter
		printk("VPint(0xbfb82060)=%lx \n",VPint(0xbfb82060));
		printk("VPint(0xbfb82064)=%lx \n",VPint(0xbfb82064));
		printk("VPint(0xbfb82068)=%lx \n",VPint(0xbfb82068));
		printk("VPint(0xbfb8206c)=%lx \n",VPint(0xbfb8206c));

		//disable ASPM
		VPint(0xbfb80020) = 0x70c;
		VPint(0xbfb80024) = 0x1b105000;
		VPint(0xbfb80020) = 0x100070c;
		VPint(0xbfb80024) = 0x13105001;

		VPint(0xbfb80020) = 0x80;
		VPint(0xbfb80024) = 0x30110008;
		VPint(0xbfb80020) = 0x1000080;
		VPint(0xbfb80024) = 0x10110000;

		//clear root error status
		VPint(0xbfb80020) = 0x130;
		errStatus = VPint(0xbfb80024);
		VPint(0xbfb80024) = errStatus;
		printk("VPint(0xbfb80020)=%lx \n",VPint(0xbfb80020));
		printk("clear VPint(0xbfb80024)=%lx \n",VPint(0xbfb80024));
	}
	if((val & (1<<2)) != 0){
		VPint(0xbfb83070) = 1; //clear interrupt status
		VPint(0xbfb83070) = 0; //set interrupt status
		printk("RC1 error \n");

		//check root error status
		VPint(0xbfb80020) = 0x80130;
		printk("VPint(0xbfb80020)=%lx \n",VPint(0xbfb80020));
		printk("VPint(0xbfb80024)=%lx \n",VPint(0xbfb80024));
		//check error source ID
		VPint(0xbfb80020) = 0x80134;
		printk("VPint(0xbfb80020)=%lx \n",VPint(0xbfb80020));
		printk("VPint(0xbfb80024)=%lx \n",VPint(0xbfb80024));
		//check uncorrectable error status
		VPint(0xbfb80020) = 0x80104;
		printk("VPint(0xbfb80020)=%lx \n",VPint(0xbfb80020));
		printk("VPint(0xbfb80024)=%lx \n",VPint(0xbfb80024));
		//check correctable error status
		VPint(0xbfb80020) = 0x80110;
		errStatus = VPint(0xbfb80024);
		printk("VPint(0xbfb80020)=%lx \n",VPint(0xbfb80020));
		printk("VPint(0xbfb80024)=%lx \n",errStatus);
		//clear correctable error status
		VPint(0xbfb80024) = errStatus;
		printk("clear VPint(0xbfb80024)=%lx \n",VPint(0xbfb80024));


		//check error counter
		printk("VPint(0xbfb83060)=%lx \n",VPint(0xbfb83060));
		printk("VPint(0xbfb83064)=%lx \n",VPint(0xbfb83064));
		printk("VPint(0xbfb83068)=%lx \n",VPint(0xbfb83068));
		printk("VPint(0xbfb8306c)=%lx \n",VPint(0xbfb8306c));

		//disable ASPM
		VPint(0xbfb80020) = 0x8070c;
		VPint(0xbfb80024) = 0x1b105000;
		VPint(0xbfb80020) = 0x300070c;
		VPint(0xbfb80024) = 0x13105001;

		VPint(0xbfb80020) = 0x80080;
		VPint(0xbfb80024) = 0x30110008;
		VPint(0xbfb80020) = 0x3000080;
		VPint(0xbfb80024) = 0x10110000;

		//clear root error status
		VPint(0xbfb80020) = 0x80130;
		errStatus = VPint(0xbfb80024);
		VPint(0xbfb80024) = errStatus;
		printk("VPint(0xbfb80020)=%lx \n",VPint(0xbfb80020));
		printk("clear VPint(0xbfb80024)=%lx \n",VPint(0xbfb80024));
	}
	if((val & (1<<0)) != 0){
		VPint(0xc0d03070) = 1; //clear interrupt status
		VPint(0xc0d03070) = 0; //set interrupt status
		printk("slave RC error \n");

		//check root error status
		VPint(0xbfb80020) = 0x1080130;
		printk("VPint(0xbfb80020)=%lx \n",VPint(0xbfb80020));
		printk("VPint(0xbfb80024)=%lx \n",VPint(0xbfb80024));
		//check error source ID
		VPint(0xbfb80020) = 0x1080134;
		printk("VPint(0xbfb80020)=%lx \n",VPint(0xbfb80020));
		printk("VPint(0xbfb80024)=%lx \n",VPint(0xbfb80024));
		//check uncorrectable error status
		VPint(0xbfb80020) = 0x1080104;
		printk("VPint(0xbfb80020)=%lx \n",VPint(0xbfb80020));
		printk("VPint(0xbfb80024)=%lx \n",VPint(0xbfb80024));
		//check correctable error status
		VPint(0xbfb80020) = 0x1080110;
		errStatus = VPint(0xbfb80024);
		printk("VPint(0xbfb80020)=%lx \n",VPint(0xbfb80020));
		printk("VPint(0xbfb80024)=%lx \n",errStatus);
		//clear correctable error status
		VPint(0xbfb80024) = errStatus;
		printk("clear VPint(0xbfb80024)=%lx \n",VPint(0xbfb80024));


		//check error counter
		printk("VPint(0xc0d03060)=%lx \n",VPint(0xc0d03060));
		printk("VPint(0xc0d03064)=%lx \n",VPint(0xc0d03064));
		printk("VPint(0xc0d03068)=%lx \n",VPint(0xc0d03068));
		printk("VPint(0xc0d0306c)=%lx \n",VPint(0xc0d0306c));

		//disable ASPM
		VPint(0xbfb80020) = 0x108070c;
		VPint(0xbfb80024) = 0x1b105000;
		VPint(0xbfb80020) = 0x200070c;
		VPint(0xbfb80024) = 0x13105001;

		VPint(0xbfb80020) = 0x1080080;
		VPint(0xbfb80024) = 0x30110008;
		VPint(0xbfb80020) = 0x2000080;
		VPint(0xbfb80024) = 0x10110000;

		//clear root error status
		VPint(0xbfb80020) = 0x1080130;
		errStatus = VPint(0xbfb80024);
		VPint(0xbfb80024) = errStatus;
		printk("VPint(0xbfb80020)=%lx \n",VPint(0xbfb80020));
		printk("clear VPint(0xbfb80024)=%lx \n",VPint(0xbfb80024));
	}

	return IRQ_HANDLED;
#endif

}


#ifdef DMT_INT_TEST
static int dmtCounter_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len = 0;
	int i;
	int result = 0;

	printk("dmt counter: %d \n",gDmtCounter);
	for(i=0; i<(ARRAYSIZE-1); i++)
	{
		if( ((gSymCount[i+1]-gSymCount[i]) != 1) && ((gSymCount[i]-gSymCount[i+1]) != 511))
		{
			printk("#gSymCount[%d]=%d gSymCount[%d]=%d \n", i+1, gSymCount[i+1], i, gSymCount[i]);
			result = 1;
		}
	}

	if(result == 1){
		printk("TEST FAIL! \n");
	}else{
		printk("TEST PASS! \n");
	}


	return len;
}

static int dmtCounter_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data){
	char val_string[40];
	int devIdx;
	int tmpValue;

	if (count > sizeof(val_string) - 1)
    		return -EINVAL;
	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;
	val_string[count] = '\0';
	sscanf(val_string,"%d\n",&tmpValue);


	if (tmpValue == 1)
	{
		printk("reset gSymCount \n");
		memset(gSymCount,0,ARRAYSIZE*sizeof(unsigned int));
		VPint(csr_addr + 0x90000c) = 0x01010101;
		gDmtCounter = 0;
		gFull = 0;
	}

	return count;
}
#endif

static int __init bonding_init_module(void)
{
 	struct net_device *dev;
	int err=0, tmp;
#ifdef DMT_INT_TEST
	struct proc_dir_entry *dmtCounter_proc;
#endif
	printk("bonding pcie module start init \n");

#ifdef GDMA_INT_TEST
	dev = alloc_etherdev(sizeof(struct net_device));
	gdmaTest_dev = dev;
	dev->irq = 37+1;	//slave gdma interrupt
	printk("register slave gdma irq %d \n",dev->irq);
	if ((err = request_irq(dev->irq, pcieIsr, 0, "gdma_test", gdmaTest_dev)) != 0) {
		printk("err=%d\n", err);
		printk("can not allocate IRQ.\n");
	    if(gdmaTest_dev){
	    	free_netdev(gdmaTest_dev);
	    }
	}
#endif

#ifdef DMT_INT_TEST
	dev = alloc_etherdev(sizeof(struct net_device));
	dmtTest_dev = dev;
	dev->irq = 36+1;	//slave dmt interrupt
	printk("register dmt irq %d \n",dev->irq);
	if ((err = request_irq(dev->irq, pcieIsr, 0, "dmt_test", dmtTest_dev)) != 0) {
		printk("err=%d\n", err);
		printk("can not allocate IRQ.\n");
	    if(dmtTest_dev){
	    	free_netdev(dmtTest_dev);
	    }
	}

	gSymCount = (unsigned int *)kmalloc(ARRAYSIZE*sizeof(unsigned int), GFP_KERNEL);
	memset(gSymCount,0,ARRAYSIZE*sizeof(unsigned int));

    printk("Create DMT counter proc \n");
    dmtCounter_proc = create_proc_entry("dmtCounter", 0, NULL);
    dmtCounter_proc->read_proc = dmtCounter_read_proc;
    dmtCounter_proc->write_proc = dmtCounter_write_proc;
#endif

#ifdef ERROR_INT_TEST
	dev = alloc_etherdev(sizeof(struct net_device));
	errTest_dev = dev;
	dev->irq = 39+1;	//pcie error interrupt
	printk("register pcie error iqr %d \n",dev->irq);
	if ((err = request_irq(dev->irq, pcieIsr, 0, "pcieError_test", errTest_dev)) != 0) {
		printk("err=%d\n", err);
		printk("can not allocate IRQ.\n");
	    if(errTest_dev){
	    	free_netdev(errTest_dev);
	    }
	}
#endif

	if (!err){
		err = pci_register_driver(&bonding_driver);
	}
	return err;
}


//
// Driver module unload function
//
static void __exit bonding_cleanup_module(void)
{
    pci_unregister_driver(&bonding_driver);

#ifdef DMT_INT_TEST
    remove_proc_entry("dmtCounter", NULL);
    kfree(gSymCount);
    if(dmtTest_dev){
    	free_netdev(dmtTest_dev);
    }
#endif

#ifdef GDMA_INT_TEST
    if(gdmaTest_dev){
    	free_netdev(gdmaTest_dev);
    }
#endif

#ifdef ERROR_INT_TEST
    if(errTest_dev){
    	free_netdev(errTest_dev);
    }
#endif

}

module_init(bonding_init_module);
module_exit(bonding_cleanup_module);



//
// PCI device probe & initialization function
//
static int __devinit   bonding_probe(
    struct pci_dev              *pci_dev, 
    const struct pci_device_id  *pci_id)
{
	char*				print_name;
	int rv = 0;
	unsigned long tmp;

	printk("enter bonding_probe function\n");
	// wake up and enable device
	if ((rv = pci_enable_device(pci_dev))!= 0)
	{
		printk("Enable PCI device failed, errno=%d!\n", rv);
		return rv;
	}

	print_name = pci_dev ? pci_name(pci_dev) : "bonding_probe";

	if ((rv = pci_request_regions(pci_dev, print_name)) != 0)
	{
		printk("Request PCI resource failed, errno=%d!\n", rv);
		goto err_out;
	}
	
	// map physical address to virtual address for accessing register
	csr_addr = (unsigned long) ioremap(pci_resource_start(pci_dev, 0), pci_resource_len(pci_dev, 0));
	if (!csr_addr)
	{
		printk("ioremap failed for device %s, region 0x%lX @ 0x%lX\n",
					print_name, (unsigned long)pci_resource_len(pci_dev, 0), 
					(unsigned long)pci_resource_start(pci_dev, 0));
		goto err_out_free_res;
	}
	else
	{
		pcie_virBaseAddr_set(csr_addr);
		printk("%s: at 0x%lx, VA 0x%lx, IRQ %d. \n",  print_name, 
					(unsigned long)pci_resource_start(pci_dev, 0), 
					(unsigned long)csr_addr, pci_dev->irq);
	}

	// Set DMA master
	pci_set_master(pci_dev);

	if(isFPGA){
		//config SCU to release slave dmt reset before loading dmt driver...
		VPint(csr_addr + 0xb00084) = 0;
	}else{
		//MEMPLL setting for mt7511
		tmp = regRead32(0xbfb0008c);
		if((tmp & (0x01 << 22)) != 0){
			//Xtal input freq 20Mhz
			printk("Xtal input freq 20Mhz \n");

			regWrite32(csr_addr + 0xb20640, 0x00000003);
			printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20640, regRead32(csr_addr + 0xb20640));

			regWrite32(csr_addr + 0xb20604, 0x00000002);
			printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20604, regRead32(csr_addr + 0xb20604));

			regWrite32(csr_addr + 0xb20600, 0x12003C00);
			printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20600, regRead32(csr_addr + 0xb20600));
			udelay(2);

			regWrite32(csr_addr + 0xb20600, 0x32003C00);
			printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20600, regRead32(csr_addr + 0xb20600));
			mdelay(1);

			regWrite32(csr_addr + 0xb20600, 0x32213C10);
			printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20600, regRead32(csr_addr + 0xb20600));
			udelay(20);

			regWrite32(csr_addr + 0xb2061C, 0x0000002F);
			printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb2061C, regRead32(csr_addr + 0xb2061C));
			udelay(1);
			if((tmp & (0x1 << 18)) != 0){
				//DDR3-1000
				regWrite32(csr_addr + 0xb20620, 0x02211900);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20620, regRead32(csr_addr + 0xb20620));
				regWrite32(csr_addr + 0xb2062C, 0x02211900);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb2062C, regRead32(csr_addr + 0xb2062C));
				regWrite32(csr_addr + 0xb20644, 0x02211900);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20644, regRead32(csr_addr + 0xb20644));
				regWrite32(csr_addr + 0xb20624, 0x00020000);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20624, regRead32(csr_addr + 0xb20624));
				regWrite32(csr_addr + 0xb20620, 0x02211910);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20620, regRead32(csr_addr + 0xb20620));
				regWrite32(csr_addr + 0xb20630, 0x00020000);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20630, regRead32(csr_addr + 0xb20630));
				regWrite32(csr_addr + 0xb2062C, 0x02211910);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb2062C, regRead32(csr_addr + 0xb2062C));
				regWrite32(csr_addr + 0xb20648, 0x00020000);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20648, regRead32(csr_addr + 0xb20648));
				regWrite32(csr_addr + 0xb20644, 0x02211910);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20644, regRead32(csr_addr + 0xb20644));
			}else{
				//DDR2-800
				regWrite32(csr_addr + 0xb20620, 0x02211400);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20620, regRead32(csr_addr + 0xb20620));
				regWrite32(csr_addr + 0xb2062C, 0x02211400);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb2062C, regRead32(csr_addr + 0xb2062C));
				regWrite32(csr_addr + 0xb20644, 0x02211400);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20644, regRead32(csr_addr + 0xb20644));
				regWrite32(csr_addr + 0xb20624, 0x00020000);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20624, regRead32(csr_addr + 0xb20624));
				regWrite32(csr_addr + 0xb20620, 0x02211410);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20620, regRead32(csr_addr + 0xb20620));
				regWrite32(csr_addr + 0xb20630, 0x00020000);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20630, regRead32(csr_addr + 0xb20630));

				regWrite32(csr_addr + 0xb2062C, 0x02211410);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb2062C, regRead32(csr_addr + 0xb2062C));
				regWrite32(csr_addr + 0xb20648, 0x00020000);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20648, regRead32(csr_addr + 0xb20648));
				regWrite32(csr_addr + 0xb20644, 0x02211410);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20644, regRead32(csr_addr + 0xb20644));
			}
			udelay(20);
			regWrite32(csr_addr + 0xb20640, 0x00000013);
			printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20640, regRead32(csr_addr + 0xb20640));
			regWrite32(csr_addr + 0xb20640, 0x00000033);
			printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20640, regRead32(csr_addr + 0xb20640));
		}else{
			//Xtal input freq 25Mhz
			printk("Xtal input freq 25Mhz \n");

			regWrite32(csr_addr + 0xb20640, 0x00000023);
			printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20640, regRead32(csr_addr + 0xb20640));

			regWrite32(csr_addr + 0xb20604, 0x00000000);
			printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20604, regRead32(csr_addr + 0xb20604));

			regWrite32(csr_addr + 0xb20600, 0x12003C00);
			printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20600, regRead32(csr_addr + 0xb20600));
			udelay(2);

			regWrite32(csr_addr + 0xb20600, 0x32003C00);
			printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20600, regRead32(csr_addr + 0xb20600));
			mdelay(1);

			regWrite32(csr_addr + 0xb20600, 0x32213C10);
			printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20600, regRead32(csr_addr + 0xb20600));
			udelay(20);

			regWrite32(csr_addr + 0xb2061C, 0x0000002F);
			printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb2061C, regRead32(csr_addr + 0xb2061C));
			udelay(1);

			if((tmp & (0x1 << 18)) != 0){
				//DDR3-1000
				regWrite32(csr_addr + 0xb20620, 0x02210a00);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20620, regRead32(csr_addr + 0xb20620));
				regWrite32(csr_addr + 0xb2062C, 0x02210a00);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb2062C, regRead32(csr_addr + 0xb2062C));
				regWrite32(csr_addr + 0xb20644, 0x02210a00);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20644, regRead32(csr_addr + 0xb20644));
				regWrite32(csr_addr + 0xb20624, 0x00020000);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20624, regRead32(csr_addr + 0xb20624));
				regWrite32(csr_addr + 0xb20620, 0x02210a10);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20620, regRead32(csr_addr + 0xb20620));
				regWrite32(csr_addr + 0xb20630, 0x00020000);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20630, regRead32(csr_addr + 0xb20630));

				regWrite32(csr_addr + 0xb2062C, 0x02210a10);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb2062C, regRead32(csr_addr + 0xb2062C));
				regWrite32(csr_addr + 0xb20648, 0x00020000);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20648, regRead32(csr_addr + 0xb20648));
				regWrite32(csr_addr + 0xb20644, 0x02210a10);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20644, regRead32(csr_addr + 0xb20644));
			}else{
				//DDR2-800
				regWrite32(csr_addr + 0xb20620, 0x02210800);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20620, regRead32(csr_addr + 0xb20620));
				regWrite32(csr_addr + 0xb2062C, 0x02210800);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb2062C, regRead32(csr_addr + 0xb2062C));
				regWrite32(csr_addr + 0xb20644, 0x02210800);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20644, regRead32(csr_addr + 0xb20644));
				regWrite32(csr_addr + 0xb20624, 0x00020000);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20624, regRead32(csr_addr + 0xb20624));
				regWrite32(csr_addr + 0xb20620, 0x02210810);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20620, regRead32(csr_addr + 0xb20620));
				regWrite32(csr_addr + 0xb20630, 0x00020000);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20630, regRead32(csr_addr + 0xb20630));
				regWrite32(csr_addr + 0xb2062C, 0x02210810);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb2062C, regRead32(csr_addr + 0xb2062C));
				regWrite32(csr_addr + 0xb20648, 0x00020000);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20648, regRead32(csr_addr + 0xb20648));
				regWrite32(csr_addr + 0xb20644, 0x02210810);
				printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20644, regRead32(csr_addr + 0xb20644));
			}
			udelay(20);
			regWrite32(csr_addr + 0xb20640, 0x00000033);
			printk("VPint(0x%lx)=0x%lx \n", csr_addr + 0xb20640, regRead32(csr_addr + 0xb20640));

		}

	}

	return 0;

err_out_free_res:
	pci_release_regions(pci_dev);
	
err_out:
	pci_disable_device(pci_dev);

	printk("<=== bonding_probe failed with rv = %d!\n", rv);

	return -ENODEV; /* probe fail */
}
