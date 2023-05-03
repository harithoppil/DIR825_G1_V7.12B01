#include "access_module_cl.h"

#include <linux/kernel.h>
#include <linux/atmdev.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/tc3162/tc3162.h>	
#include <asm/tc3162/TCIfSetQuery_os.h>	
#include <asm/tc3162/ledcetrl.h>
#include <asm/tc3162/cmdparse.h>	


/*
extern int subcmd(const cmds_t tab[], int argc, char *argv[], void *p);
extern int cmd_register(cmds_t *cmds_p);
*/

#define amc_read_reg_word(reg) 			regRead32(reg)
#define amc_write_reg_word(reg, wdata) 	regWrite32(reg, wdata)


int doAmcWriteVerify(int argc, char *argv[], void *p);
int doAmcReadVerify(int argc, char *argv[], void *p);
int doAmcAllVevify(int argc, char *argv[], void *p);
int amc_func(unsigned int chipid,char *modulename,unsigned int addr,unsigned int value,unsigned int direction,int flag);


static const cmds_t amcv_cmd[] = {	
	{"xw",		doAmcWriteVerify,		0x02,	0,	NULL},	
	{"xr",		doAmcReadVerify,		0x02,	0,	NULL},
	{"all",		doAmcAllVevify,		0x02,	0,	NULL},	
	{NULL,	NULL,	0x10,	0,	NULL},	
};

amc_para amc_info_chipmt7510[]=
{
#if 0//for fpga verify
	{"pcie",{0xbfb82034,0x6040001},{0xbfb82050,0},{0xbfb80000,0},{0xbfb00834,0},1},//external 29,27,26
	{"pciephy",{0xbfb82034,0x6040001},{0xbfaf2000,0},{0xbfb80000,0},{0xbfb00834,0},1},//external 29,27,26
#else//for asic verify
	{"pcie",{0xbfb82034,0x6040001},{0xbfb82050,0},{0,0},{0xbfb00834,0},0},//external 29,27,26
	{"pciephy",{0xbfaf2000,0x7f7f0000},{0xbfaf2000,0},{0xbfb00088,15},{0,0},3},//bit 15 
#endif
	{"usb2.0",{0xbfbb0054,0x00001000},{0xbfbb0054,0},{0xbfbb0010,1},{0xbfb00834,0},4},//external 25,22
	{"usb1.1",{0xbfba0054,0x00001000},{0xbfba0054,0},{0xbfba0008,0},{0xbfb00834,0},4},//external 25,22
	{"usbphy",{0xbfaf1800,0x0048086a},{0xbfaf1800,0},{0,0},{0xbfb00834,0},4},//external 25,22
	{"gdma",{0xbfb30204,0x0},{0xbfb30200,0},{0,0},{0xbfb00834,14},0},
	{"i2c",{0xbfbf8040,0x0},{0xbfbf8040,0},{0,0},{0xbfb00834,16},0},
	{"gpon",{0xbfb640b0,0x0},{0xbfb640b0,0},{0,0},{0xbfb00834,31},0},
	{"epon",{0xbfb6610c,0x10},{0xbfb66010,0},{0,0},{0xbfb00834,31},0},
	{"sar",{0xbfb60008,0x003fffff},{0xbfb60004,0},{0xbfb60000,0},{0xbfb00834,7},0},
	//{"gsw",{0xbfb5f004,0x0},{0xbfb5f008,0},{0,0},{0xbfb00834,23},0},//not use internal reset in mt7510,so no need to verify
	{"cryptoengine",{0xbfb70004,0x0},{0xbfb70000,0},{0xbfb70100,0},{0xbfb00834,6},0},
	{"pcm",{0xbfbd0024,0x0},{0xbfbd0000,0},{0xbfbd0000,24},{0xbfb00834,11},3},
	{"uart2",{0xbfbf0304,0x0},{0xbfbf0308,0},{0,0},{0xbfb00834,19},0},
	{"gpio",{0xbfbf0204,0x0},{0xbfbf0200,0},{0,0},{0xbfb00834,13},0},
	{"ptm",{0xbfb62004,0x02010101},{0xbfb62004,0},{0xbfb62000,1},{0xbfb00834,5},0},
	{"nfc",{0xbfbe000c,0x44433},{0xbfbe000c,0},{0xbfbe0008,1},{0xbfb00834,15},2},
	{"spi",{0xbfbc0000,0x00160000},{0xbfbc0034,0},{0xbfbc0038,0},{0xbfb00834,18},0},
	//{"dmc",{0xbfb20000,0x0},{0xbfb20000,0},{0,0},{0xbfb00040,0},0},//not need to test
	{"linebond",{0xbfb6f000,0x00010001},{0xbfb6f018,0},{0,0},{0xbfb00834,10},0},
	{"dmt",{0xbf901800,0x0},{0xbf901800,0},{0,0},{0xbfb00084,0},0},
	{"pdma",{0xbfb50800,0x0},{0xbfb50800,0},{0,0},{0xbfb00834,1},0},
	{"qdma",{0xbfb51800,0x0},{0xbfb51800,0},{0,0},{0xbfb00834,21},0},
	{"",{0,0},{0,0},{0,0},{0,0}}
};


int amc_func(unsigned int chipid,char *modulename,unsigned int addr,unsigned int value,unsigned int direction,int flag)
{
	printk("\r\nStart Test!");
	switch(chipid)
	{
		case 1://mt7510
		{
			int i;
			unsigned char tempflag;
			unsigned long tempaddrxr,tempaddrxw,tempvalue;
			for(i = 0;amc_info_chipmt7510[i].modulename[0] != 0;i++)
			{
				if(strncmp(amc_info_chipmt7510[i].modulename,modulename,MAXAMCMODULELENGTH) == 0)
				{
					tempflag = 0;
					//reset to default
					printk("\r\nreset to default value!");
					if(direction)
					{
						if(amc_info_chipmt7510[i].out_reset.addr != 0)
						{	
							printk("\r\nexternal:resetaddr=0x%x",amc_info_chipmt7510[i].out_reset.addr);
							tempvalue = amc_read_reg_word(amc_info_chipmt7510[i].out_reset.addr);
							printk("\r\nresetcurrent=0x%x",tempvalue);
							#if 0//mt7510 pcie fpga modify:1 reset;
							if(1 == amc_info_chipmt7510[i].flag)
								tempvalue |= (1<<26|1<<27);
							#else
							if(1 == amc_info_chipmt7510[i].flag)
								tempvalue &= ~(1<<26|1<<27);
							#endif
							else if(4 == amc_info_chipmt7510[i].flag)
								tempvalue &= ~(1<<22|1<<25);
							else
								tempvalue &= ~(1<<amc_info_chipmt7510[i].out_reset.value);
							printk("\r\nresetdefault=0x%x",tempvalue);
							amc_write_reg_word(amc_info_chipmt7510[i].out_reset.addr,tempvalue);
						}
						else
						{
							printk("\r\nnot support external reset! return!");
							return;
						}
					}
					else
					{
						if(amc_info_chipmt7510[i].in_reset.addr != 0)
						{
							printk("\r\ninternal:resetaddr=0x%x",amc_info_chipmt7510[i].in_reset.addr);
							tempvalue = amc_read_reg_word(amc_info_chipmt7510[i].in_reset.addr);
							printk("\r\nresetcurrent=0x%x",tempvalue);
							if(3 == amc_info_chipmt7510[i].flag)
								tempvalue |= 1<<amc_info_chipmt7510[i].in_reset.value;
							else if(1 == amc_info_chipmt7510[i].flag)
								tempvalue &= ~(1<<1|1<<2);
							else
								tempvalue &= ~(1<<amc_info_chipmt7510[i].in_reset.value);
							printk("\r\nresetdefault=0x%x",tempvalue);
							amc_write_reg_word(amc_info_chipmt7510[i].in_reset.addr,tempvalue);
						}
						else
						{
							printk("\r\nnot support internal reset!return!");
							return;
						}
					}

					mdelay(1);	
					
					//first do reset
					printk("\r\ntest module:%s",modulename);
					printk("\r\ndo reset operation:use %s reset way",direction>0?"scu":"module");
					printk("\r\n\r\nstep1:do reset");
					if(direction)
					{
						if(amc_info_chipmt7510[i].out_reset.addr != 0)
						{	
							printk("\r\nresetaddr=0x%x",amc_info_chipmt7510[i].out_reset.addr);
							tempvalue = amc_read_reg_word(amc_info_chipmt7510[i].out_reset.addr);
							printk("\r\nresetvalue=0x%x before do reset operation",tempvalue);
							#if 0//mt7510 pcie fpga modify:1 reset;
							if(1 == amc_info_chipmt7510[i].flag)
								tempvalue &= ~(1<<26|1<<27);
							#else
							if(1 == amc_info_chipmt7510[i].flag)
								tempvalue |= (1<<26|1<<27);
							#endif
							else if(4 == amc_info_chipmt7510[i].flag)
								tempvalue |= (1<<22|1<<25);
							else
								tempvalue |= 1<<amc_info_chipmt7510[i].out_reset.value;
							printk("\r\nresetvalue=0x%x after do reset operation",tempvalue);
							amc_write_reg_word(amc_info_chipmt7510[i].out_reset.addr,tempvalue);
						}
						else
						{
							printk("\r\nnot support external reset!return!");
							return;
						}
					}
					else
					{
						if(amc_info_chipmt7510[i].in_reset.addr != 0)
						{

							printk("\r\nresetaddr=0x%x",amc_info_chipmt7510[i].in_reset.addr);
							tempvalue = amc_read_reg_word(amc_info_chipmt7510[i].in_reset.addr);
							printk("\r\nresetvalue=0x%x before do reset operation",tempvalue);
							if(3 == amc_info_chipmt7510[i].flag)
								tempvalue &= ~(1<<amc_info_chipmt7510[i].in_reset.value);
							else if(1 == amc_info_chipmt7510[i].flag)
								tempvalue |= (1<<1|1<<2);
							else
								tempvalue |= 1<<amc_info_chipmt7510[i].in_reset.value;
							printk("\r\nresetvalue=0x%x after do reset operation",tempvalue);
							amc_write_reg_word(amc_info_chipmt7510[i].in_reset.addr,tempvalue);
						}
						else
						{
							printk("\r\nnot support internal reset!");
							return;
						}
					}
	
					//second read or write register
					printk("\r\nstep2:do read or write");
					if(addr != 0 && addr != 0xffffffff)
					{
						tempaddrxr = tempaddrxw = addr;
						tempflag = 1;
					}
					else
					{
						tempaddrxr = amc_info_chipmt7510[i].read.addr;
						tempaddrxw = amc_info_chipmt7510[i].write.addr;
					}

					
					if(flag)
					{
						tempvalue = (tempflag == 1? (value):(amc_info_chipmt7510[i].write.value));
						printk("\r\nWrite:VPint(%x)=0x%x",tempaddrxw,tempvalue);
						amc_write_reg_word(tempaddrxw,tempvalue);
					}
					else
					{
						printk("\r\nRead:VPint(%x) = 0x%x",tempaddrxr,amc_read_reg_word(tempaddrxr));
					}

					//at end clear reset
					printk("\r\n\r\nstep3:clear reset");
				
					if(direction)
					{
						if(amc_info_chipmt7510[i].out_reset.addr != 0)
						{	
							printk("\r\nresetaddr=0x%x",amc_info_chipmt7510[i].out_reset.addr);
							tempvalue = amc_read_reg_word(amc_info_chipmt7510[i].out_reset.addr);
							printk("\r\nresetvalue=0x%x before clear reset operation",tempvalue);
						#if 0//mt7510 pcie fpga modify:1 reset;
							if(1 == amc_info_chipmt7510[i].flag)
								tempvalue |= (1<<26|1<<27);
						#else
							if(1 == amc_info_chipmt7510[i].flag)
								tempvalue &= ~(1<<26|1<<27);
						#endif
							else if(4 == amc_info_chipmt7510[i].flag)
								tempvalue &= ~(1<<22|1<<25);
							else
								tempvalue &= ~(1<<amc_info_chipmt7510[i].out_reset.value);
							printk("\r\nresetvalue=0x%x after clear reset operation",tempvalue);
							amc_write_reg_word(amc_info_chipmt7510[i].out_reset.addr,tempvalue);
						}
						else
						{
							printk("\r\nnot support external reset! return!");
							return;
						}
					}
					else
					{
						if(2 != amc_info_chipmt7510[i].flag)
						{
							if(amc_info_chipmt7510[i].in_reset.addr != 0)
							{
								printk("\r\nresetaddr=0x%x",amc_info_chipmt7510[i].in_reset.addr);
								tempvalue = amc_read_reg_word(amc_info_chipmt7510[i].in_reset.addr);
								printk("\r\nresetvalue=0x%x before clear reset operation",tempvalue);
								if(3 == amc_info_chipmt7510[i].flag)
									tempvalue |= 1<<amc_info_chipmt7510[i].in_reset.value;
								else if(1 == amc_info_chipmt7510[i].flag)
									tempvalue &= ~(1<<1|1<<2);
								else
									tempvalue &= ~(1<<amc_info_chipmt7510[i].in_reset.value);
								printk("\r\nresetvalue=0x%x after clear reset operation",tempvalue);
								amc_write_reg_word(amc_info_chipmt7510[i].in_reset.addr,tempvalue);
							}
							else
							{
								printk("\r\nnot support internal reset!return!");
								return;
							}
						}	
					}
					break;
				}
			}

			if(amc_info_chipmt7510[i].modulename[0] == 0)
			{
				printk("\r\nnot support such module(name:%s)!",modulename);
			}

			break;
		}
		default:
			printk("\r\nnot support this chip!");
			break;
	}
}

int doAmcWriteVerify(int argc, char *argv[], void *p)
{
	unsigned int chipid,direction,value = 0,writedaddr = 0;
	char modulename[MAXAMCMODULELENGTH];
	if(argc != 6 && argc != 4)	
	{		
		goto errorHandle;
	}

	chipid = simple_strtoul(argv[1],NULL,16);	
	strncpy(modulename,argv[2],MAXAMCMODULELENGTH);
	direction = simple_strtoul(argv[3],NULL,16);
	if(argc == 6)
	{
		writedaddr = simple_strtoul(argv[4],NULL,16);
		value =  simple_strtoul(argv[5],NULL,16);
	}

	amc_func(chipid,modulename,writedaddr,value,direction,WRITETEST);
	
	return 0;
		
errorHandle:
	printk("\r\ncorrect cmd:xr [chipid] [modulename] [direction] [writeaddr] [value]");
	return -1;

}

int doAmcReadVerify(int argc, char *argv[], void *p)
{
	unsigned int chipid,direction,readaddr = 0;
	char modulename[MAXAMCMODULELENGTH];
	if(argc != 5 && argc != 4)	
	{		
		goto errorHandle;
	}

	chipid = simple_strtoul(argv[1],NULL,16);	
	strncpy(modulename,argv[2],MAXAMCMODULELENGTH);
	direction = simple_strtoul(argv[3],NULL,16);
	if(argc == 5)
		readaddr = simple_strtoul(argv[4],NULL,16);

	amc_func(chipid,modulename,readaddr,0,direction,READTEST);
	
	return 0;
	
errorHandle:
	printk("\r\ncorrect cmd:xr [chipid] [modulename] [direction] [readaddr]");
	return -1;

}

int doAmcAllVevify(int argc, char *argv[], void *p)
{
	int i;
	for(i = 0;amc_info_chipmt7510[i].modulename[0] != 0;i++)
	{
		printk("\r\n======module read test=========");
		amc_func(1,amc_info_chipmt7510[i].modulename,0,0,0,READTEST);
		printk("\r\n======moduke read test=========");
		amc_func(1,amc_info_chipmt7510[i].modulename,0,0,0,WRITETEST);	

		printk("\r\n======scu read test=========");
		amc_func(1,amc_info_chipmt7510[i].modulename,0,0,1,READTEST);
		printk("\r\n======scu write test=========");
		amc_func(1,amc_info_chipmt7510[i].modulename,0,0,1,WRITETEST);
	}
	return 0;
}

#if 0
int doAmcV(int argc, char *argv[], void *p)
{	
	return subcmd(amcv_cmd, argc, argv, p);
}

void amc_verify_init(void)
{
	cmds_t	amcv_cmd;	/*Register amcv ci-cmd*/
	printk("\r\nRegister amc verify cmd\r\n");	
	amcv_cmd.name="amcv";	
	amcv_cmd.func=doAmcV;	
	amcv_cmd.flags=0x12;	
	amcv_cmd.argcmin=0;	
	amcv_cmd.argc_errmsg=NULL;
	cmd_register(&amcv_cmd);	
	return;
}
#endif

