 /************************************************************************
 *
 *	Copyright (C) 2006 Trendchip Technologies, Corp.
 *	All Rights Reserved.
 *
 * Trendchip Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of Trendchip Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * Trendchip Technologeis, Co.
 *
 *************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "global.h"
#include	"prolinecmd.h"
#include	"../../../version/tcversion.h"

#ifdef TCSUPPORT_PRODUCTIONLINE
static int doProductClass(int argc, char *argv[], void *p);
static int doManufacturerOUI(int argc, char *argv[], void *p);
static int doSerialnum(int argc, char *argv[], void *p);
static int doClear(int argc, char *argv[], void *p);
static int doProLineVer(int argc, char *argv[], void *p);
static int doProLineHelp(int argc, char *argv[], void *p);

static cmds_t ProLineCmds[] = {
	{"productclass", doProductClass, 0x10,   0,  NULL},/*set or display productclass*/	
	{"manufacturerOUI", doManufacturerOUI, 0x10,   0,  NULL},/*set or display manufacturerOUI*/	
	{"serialnum", doSerialnum, 0x10,   0,  NULL},/*set or display serialnum*/
	{"clear", doClear, 0x10,   0,  NULL},/*clear para*/		
	{"version", doProLineVer, 0x10,   0,  NULL},/*show proline cmd version*/	
	{"help", doProLineHelp, 0x10,   0,  NULL},/*show proline cmd help*/	
	{NULL,          NULL,               0x10,   0,  NULL},
};

/*------------------------/
/		 common function	   /										
/------------------------*/
int subcmd  (
    const cmds_t tab[], int argc, char *argv[], void *p
)
{
    register const cmds_t *cmdp;
    int found = 0;
    int i;

    /* Strip off first token and pass rest of line to subcommand */
    if (argc < 2) {
        if (argc < 1)
            printf("SUBCMD - Don't know what to do?\n");
    	else{
            argv++;
            goto print_out_cmds;
    	}
    	return -1;
    }
	argc--;
	argv++;
	for(cmdp = tab;cmdp->name != NULL;cmdp++){
	    if(strncmp(argv[0],cmdp->name,strlen(argv[0])) == 0){
	       found = 1;
	       break;
		}
	}
	if(!found){
        char buf[66];

print_out_cmds:
        printf("valid subcommands of %s:\n", (argv-1)[0]);
        memset(buf,' ',sizeof(buf));
        buf[64] = '\n';
        buf[65] = '\0';
        for(i=0,cmdp = tab;cmdp->name != NULL;cmdp++){
            strncpy(&buf[i*16],cmdp->name,strlen(cmdp->name));
            if(i == 3){
		       printf(buf);
		       memset(buf,' ',sizeof(buf));
		       buf[64] = '\n';
		       buf[65] = '\0';
		    }
		    i = (i+1)%4;
		}
		if(i != 0)
		   printf(buf);
		return -1;
	}
	if(argc <= cmdp->argcmin){
	    if(cmdp->argc_errmsg != NULL)
	       printf("Usage: %s\n",cmdp->argc_errmsg);
	    return -1;
	}
	return (*cmdp->func)(argc,argv,p);
}

static void showInfo(int argc, char *argv[], void *p)
{
#if 0
	int i;
	printf("\r\nthe argc is %d",argc);
	for(i = 0; i < argc; i++)
	{
		printf("\r\nthe argv[%d] is %s",i,argv[i]);
	}
#endif
}

//int getProLineCwmpPara(proline_cwmpPara*buf)
static int getProLinePara(proLineFlag *plpara,void*buf)
{
	char cmdbuf[200] = {0};
	FILE *fp = NULL;
	sprintf(cmdbuf,TC_FLASH_READ_CMD,plpara->para.readfile,plpara->para.flashsize,plpara->para.flashoffset,plpara->para.zonename);
	system(cmdbuf);
			
	//open file
	fp = fopen(plpara->para.readfile, "r");
	if (fp == NULL) 
	{
		printf("\r\ngetProLineCwmpPara:open file(%s) fail!",plpara->para.readfile);
		return FAIL;
	}
	
	switch(plpara->type)
	{
		case PL_CP:
			//read cwmp para 
			fread((proline_cwmpPara*)buf, sizeof(proline_cwmpPara), 1, fp);
			break;
			
		default:
			printf("\r\n[getProLinePara]Not support this para!!");
			break;
	}

	//close file and unlink it
	fclose(fp);
	unlink(plpara->para.readfile);
	return SUCCESS;	
}
static int clearProLinePara(int flag)
{	
	FILE *fp = NULL;
	char cmdbuf[200] = {0};
	switch(flag)
	{
		case 1:
		{
			proline_cwmpPara temp;	
			memset(&temp,0,sizeof(temp));
			//open file and read para to struct
			fp = fopen(PROLINECMD_CWMPPARA_FILE, "w+");
			if (fp == NULL) 
			{
				printf("\r\nsetProLineCwmpPara:open file(%s) fail!",PROLINECMD_CWMPPARA_FILE);
				return FAIL;
			}
			fwrite(&temp, sizeof(proline_cwmpPara), 1, fp);
			fclose(fp);
			sprintf(cmdbuf,TC_FLASH_WRITE_CMD,PROLINECMD_CWMPPARA_FILE,PROLINE_CWMPPARA_RA_SIZE,PROLINE_CWMPPARA_RA_OFFSET,RESERVEAREA_NAME);
			system(cmdbuf);
			unlink(PROLINECMD_CWMPPARA_FILE);
			break;
		}
		default:
			printf("\r\nclearProLinePara:not support this type!");
			break;
	}
	
	return SUCCESS;
}

//int setProLineCwmpPara(void*buf,int len,int flag)
static int setProLinePara(proLineFlag *plpara,void*buf,int len)
{
	char cmdbuf[200] = {0};
	proline_cwmpPara temp;
	FILE *fpread = NULL;
	FILE *fpwrite = NULL;
	fprintf(stderr,"buf is %s",buf);
	fprintf(stderr,"len is %d",len);
	
	memset(&temp,0,sizeof(temp));
	//read para from reservearea to file first
	sprintf(cmdbuf,TC_FLASH_READ_CMD,plpara->para.readfile,plpara->para.flashsize,plpara->para.flashoffset,plpara->para.zonename);
	system(cmdbuf);
	
	//open file and read para to struct
	fpread = fopen(plpara->para.readfile, "r");
	if (fpread == NULL) 
	{
		printf("\r\nsetProLineCwmpPara:open file(%s) fail!",plpara->para.readfile);
		return FAIL;
	}

	//open file and read para to struct
	fpwrite = fopen(plpara->para.writefile, "w+");
	if (fpwrite == NULL) 
	{
		printf("\r\nsetProLineCwmpPara:open file(%s) fail!",plpara->para.writefile);
		fclose(fpread);
		return FAIL;
	}
	
	//fprintf(stderr,"plpara->type is %d",plpara->type);
	//fprintf(stderr,"plpara->flag is %d",plpara->flag);
	switch(plpara->type)
	{
		case PL_CP:
			//read all cwmpara and replace any other para according to flag
			fread(&temp, sizeof(proline_cwmpPara), 1, fpread);
			switch(plpara->flag)
			{
				case PL_CP_PRODUCTCLASS:
					memset(temp.productclass,0,sizeof(temp.productclass));
					strncpy(temp.productclass,(char*)buf,len);
					temp.flag |= PL_CP_PRODUCTCLASS_FLAG;
					break;
					
				case PL_CP_MANUFACUREROUI:
					memset(temp.manufacturerOUI,0,sizeof(temp.manufacturerOUI));
					strncpy(temp.manufacturerOUI,(char*)buf,len);
					temp.flag |= PL_CP_MANUFACUREROUI_FLAG;
					break;
					
				case PL_CP_SERIALNUM:
					memset(temp.serialnum,0,sizeof(temp.serialnum));
					strncpy(temp.serialnum,(char*)buf,len);
					temp.flag |= PL_CP_SERIALNUM_FLAG;
					break;
				default:
					printf("\r\n[setProLinePara]Not support this para in cwmp para!!");
					break;		
			}
			temp.magic = CWMPPARAMAGIC;
			fwrite(&temp, sizeof(proline_cwmpPara), 1, fpwrite);
			break;
			
		default:
			printf("\r\n[setProLinePara]Not support this para!!");
			break;
	}

	fclose(fpwrite);
	fclose(fpread);
	
	//write para from reservearea to file first
	sprintf(cmdbuf,TC_FLASH_WRITE_CMD,plpara->para.writefile,plpara->para.flashsize,plpara->para.flashoffset,plpara->para.zonename);
	system(cmdbuf);

	unlink(plpara->para.writefile);
	unlink(plpara->para.readfile);
	return SUCCESS;
}

static void setPLCwmpParaDefault(proLineFlag*paraptr)
{
	paraptr->type = PL_CP;
	paraptr->flag = PL_CP_PRODUCTCLASS;
	paraptr->para.flashsize = PROLINE_CWMPPARA_RA_SIZE;
	paraptr->para.flashoffset = PROLINE_CWMPPARA_RA_OFFSET;
	strcpy(paraptr->para.zonename,RESERVEAREA_NAME);
	strcpy(paraptr->para.readfile,PROLINECMD_CWMPPARAREAD_FILE);
	strcpy(paraptr->para.writefile,PROLINECMD_CWMPPARAWRITE_FILE);
	return;
}
/*------------------------/
/		cmd function		   /										
/------------------------*/
static int doProductClass(int argc, char *argv[], void *p)
{
	proline_cwmpPara cwmppara;
	proLineFlag plpara;

	showInfo(argc,argv,p);
	memset(&plpara,0,sizeof(proLineFlag));
	memset(&cwmppara,0,sizeof(proline_cwmpPara));
	
	setPLCwmpParaDefault(&plpara);
	
	if((argc == 2)&&(!strcmp(argv[1] , "display") ))
	{
		getProLinePara(&plpara,&cwmppara);
		if(CWMPPARAMAGIC == cwmppara.magic)
		{
			printf("productclass:%s\r\n",cwmppara.productclass);
		}
		else
		{
			printf("NotValue\r\n");
		}
	}
	else if((argc == 3)&&(!strcmp(argv[1] , "set") ))
	{
		if(strlen(argv[2]) > PRDDUCTCLASSLEN-1)
			goto errorHandle;
		strcpy(cwmppara.productclass,argv[2]);
		cwmppara.magic = CWMPPARAMAGIC;
		setProLinePara(&plpara,argv[2],strlen(argv[2]));
	}
	else
	{
		goto errorHandle;
	}

	return SUCCESS;

errorHandle:
	printf("\r\nUsage:prolinecmd productclass set <value(1~63 characters)>\n");
	return FAIL;
}

static int doManufacturerOUI(int argc, char *argv[], void *p)
{
	proline_cwmpPara cwmppara;
	proLineFlag plpara;

	showInfo(argc,argv,p);
	memset(&plpara,0,sizeof(proLineFlag));
	memset(&cwmppara,0,sizeof(proline_cwmpPara));
	
	setPLCwmpParaDefault(&plpara);
	plpara.flag = PL_CP_MANUFACUREROUI;
	if((argc == 2)&&(!strcmp(argv[1] , "display") ))
	{
		getProLinePara(&plpara,&cwmppara);
		if(CWMPPARAMAGIC == cwmppara.magic)
		{
			printf("ManufacturerOUI:%s\r\n",cwmppara.manufacturerOUI);
		}
		else
		{
			printf("NotValue\r\n");
		}
	}
	else if((argc == 3)&&(!strcmp(argv[1] , "set") ))
	{
		if(strlen(argv[2]) > MANUFACUREROUILEN-1)
			goto errorHandle;
		strcpy(cwmppara.manufacturerOUI,argv[2]);
		cwmppara.magic = CWMPPARAMAGIC;
		setProLinePara(&plpara,argv[2],strlen(argv[2]));
	}
	else
	{
		goto errorHandle;
	}

	return SUCCESS;

errorHandle:
	printf("\r\nUsage:prolinecmd ManufacturerOUI set <value(1~63 characters)>\n");
	return FAIL;
}

static int doSerialnum(int argc, char *argv[], void *p)
{
	proline_cwmpPara cwmppara;
	proLineFlag plpara;

	showInfo(argc,argv,p);
	memset(&plpara,0,sizeof(proLineFlag));
	memset(&cwmppara,0,sizeof(proline_cwmpPara));
	
	setPLCwmpParaDefault(&plpara);
	plpara.flag = PL_CP_SERIALNUM;
	if((argc == 2)&&(!strcmp(argv[1] , "display") ))
	{
		getProLinePara(&plpara,&cwmppara);
		if(CWMPPARAMAGIC == cwmppara.magic)
		{
			printf("SerialNum:%s\r\n",cwmppara.serialnum);
		}
		else
		{
			printf("NotValue\r\n");
		}
	}
	else if((argc == 3)&&(!strcmp(argv[1] , "set") ))
	{
		if(strlen(argv[2]) > SERIALNUMLEN-1)
			goto errorHandle;
		strcpy(cwmppara.serialnum,argv[2]);
		cwmppara.magic = CWMPPARAMAGIC;
		setProLinePara(&plpara,argv[2],strlen(argv[2]));
	}
	else
	{
		goto errorHandle;
	}

	return SUCCESS;

errorHandle:
	printf("\r\nUsage:prolinecmd serialnum set <value(1~127 characters)>\n");
	return FAIL;
}

static int doClear(int argc, char *argv[], void *p)
{
	int flag = 0;
	
	showInfo(argc,argv,p);
	if((argc == 2)&&(!strcmp(argv[0] , "clear") ))
	{
		flag = atoi(argv[1]);
		clearProLinePara(flag);
	}
	else
	{
		goto errorHandle;
	}

	return SUCCESS;

errorHandle:
	printf("\r\nUsage:prolinecmd clear <paratype(1:cwmp)>\n");
	return FAIL;
}

static int doProLineVer(int argc, char *argv[], void *p)
{
	showInfo(argc,argv,p);
	if((argc == 2)&&(!strcmp(argv[1] , "display") ))
	{
		printf("prolinecmd version: %s\r\n",MODULE_VERSION_PROLINECMD);
		return SUCCESS;
	}
	else
	{
		printf("\r\nUsage:prolinecmd version display\n");
		return FAIL;
	}
}

static int doProLineHelp(int argc, char *argv[], void *p)
{
	printf("\r\nUsage: \r\n"
		  		"\tprolinecmd productclass set <value>"
		  		"\n\t\tvalue:{1~63characters}\r\n"
				"\tprolinecmd productclass display\r\n"
				"\tprolinecmd ManufacturerOUI set <value>"
		  		"\n\t\tvalue:{1~63characters,now only support max characters are 6!}\r\n"
				"\tprolinecmd ManufacturerOUI display\r\n"
				"\tprolinecmd serialnum set <value>"
		  		"\n\t\tvalue:{1~63characters}\r\n"
				"\tprolinecmd serialnum display\r\n"
				"\tprolinecmd clear <value>"
				"\n\t\tvalue:{1:cwmp}\r\n"
				"\tprolinecmd help\r\n"
				"\tprolinecmd version display\r\n");
			
	return 0;
}

 int main(int argc, char **argv) 
{
	void *p;
	int ret = -1;
	int pidfd;
	pidfd =open(PROLINECMD_SOCK_PATH,O_RDWR | O_CREAT);
	if(pidfd < 0)
	{
		printf("\r\nopen lock file error!");
		ret = subcmd(ProLineCmds, argc, argv, p);
	}
	else
	{
		writew_lock(pidfd,0,SEEK_SET,0);
		ret = subcmd(ProLineCmds, argc, argv, p);
		un_lock(pidfd,0,SEEK_SET,0);
		//close(pidfd);
	}
	return ret;
}


 #else
  int main(int argc, char **argv) 
 {
 	return 0;
 }
#endif


