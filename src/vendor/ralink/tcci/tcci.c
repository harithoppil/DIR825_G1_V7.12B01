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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tcversion.h"   
#include "../tcapi_lib/libtcapi.h"




typedef int (*func_t)(int argc,char *argv[]);

static int doSysMac(int argc, char *argv[]);
static int dummy(int argc, char *argv[]);
#ifdef TC2031_DEBUG
static int tcipaddr(int argc, char *argv[]);
static int tcsetip(int argc, char *argv[]);
static int tcping(int argc, char *argv[]);
#endif
#if defined(CONFIG_TCSUPPORT_LED_BTN_CHECK) || defined(CONFIG_TCSUPPORT_TEST_LED_ALL) 
#if defined(CONFIG_TCSUPPORT_LED_CHECK) || defined(CONFIG_TCSUPPORT_TEST_LED_ALL) 
static int 	doLedCheck(int argc, char *argv[]);
#endif
#if defined(CONFIG_TCSUPPORT_BTN_CHECK)
static int 	doButtonCheck(int argc, char *argv[]);
#endif
#endif
func_t ci_func[] ={
    dummy,
    doSysMac,
#ifdef TC2031_DEBUG
    tcipaddr,
    tcsetip,
    tcping,
#else/* To ensure index */
    NULL,
    NULL,
    NULL,
#endif    
#if !defined(CONFIG_TCSUPPORT_C1_NEW_GUI) 
    NULL,
#endif
#if !defined(CONFIG_TCSUPPORT_BOOTLOADER_MODIFY_PVNAME) 
	NULL,
	NULL,
	NULL,
#endif
#if  defined(CONFIG_TCSUPPORT_LED_BTN_CHECK) || defined(CONFIG_TCSUPPORT_TEST_LED_ALL) 
#if  defined(CONFIG_TCSUPPORT_LED_CHECK) || defined(CONFIG_TCSUPPORT_TEST_LED_ALL) 
	doLedCheck,
#else 
	NULL,
#endif 
#if  defined(CONFIG_TCSUPPORT_BTN_CHECK)
	doButtonCheck,
#else 
	NULL,
#endif 
#else 
	NULL,
	NULL,
#endif 
#if !defined(CONFIG_TCSUPPORT_C2_TRUE) 
	NULL,
#endif

    NULL  
};

static int dummy(int argc, char *argv[]){
    printf("I am dummy!!\n");
    return 0;
}
#ifdef TC2031_DEBUG
/*_____________________________________________________________________________
**      function name: tcipaddr
**      descriptions:
**           show lan ip addr 
**             
**      parameters:
**            argc : argument number
**            argv : argument point 
**		 p     :  no use
**      global:
**            None
**             
**      return:
**            none
**	     
**      call:
**      	
**      revision:
**      
**____________________________________________________________________________
*/
static int tcipaddr(int argc, char *argv[]){
	
	FILE *fp;
	char buf[256];
	char *point;
	char x=0;
	
	system("ifconfig br0 > /tmp/var/ifconfig.tmp");
	fp = fopen("/tmp/var/ifconfig.tmp","r");
	if(fp != NULL){
		while(fgets(buf,256,fp)){
			point=strstr(buf,"inet addr:");
			if(point != NULL){
				printf("%s(set)\n",strtok(point+strlen("inet addr:")," "));
				break;
			}else{
				continue;
			}
			x++;
			if(x>10){
				break;
			}
		}
		fclose(fp);
		unlink("/tmp/var/ifconfig.tmp");
	}else{
		printf("Get ip addr fail!!\n");
	}

	return 0;
}

/*_____________________________________________________________________________
**      function name: tcsetip
**      descriptions:
**           set lan ip addr 
**             
**      parameters:
**            argc : argument number
**            argv : argument point 
**		 p     :  no use
**      global:
**            None
**             
**      return:
**            none
**	     
**      call:
**      	
**      revision:
**      
**____________________________________________________________________________
*/
static int tcsetip(int argc, char *argv[]){
	char buf[256];

	if(argc > 4){
		/*
		sprintf(buf,"ifconfig br0 %s netmask %s",argv[3],argv[4]);
		*/
		sprintf(buf,"ifconfig br0 %s netmask %s;tcapi set Lan_Entry0 IP %s;tcapi save;", \
				argv[3], argv[4],argv[3]);
	}else{
		sprintf(buf,"ifconfig br0 %s ",argv[3]);
	}

	system(buf);
	return 0;
}
/*_____________________________________________________________________________
**      function name: tcping
**      descriptions:
**           do ping action
**             
**      parameters:
**            argc : argument number
**            argv : argument point 
**		 p     :  no use
**      global:
**            None
**             
**      return:
**            none
**	     
**      call:
**      	
**      revision:
**      
**____________________________________________________________________________
*/
static int tcping(int argc, char *argv[]){
	char buf[256];
	int interval;
	
	if(argc >= 5){	
		interval = atoi(argv[4]);
		if(interval < 0){
			interval = 0;
		}
	}else{
		interval = 0;
	}
	
	sprintf(buf,"/userfs/bin/sendicmp %s %s %s %d",argv[1],argv[2],argv[3],interval);
	system(buf);
	return 0;
}

#endif
static int doSysMac(int argc, char *argv[]){
    unsigned char len, tmp[3], i;
    unsigned char mac_addr[6] = {0};
    FILE *fp;    

//    printf("MAC address=%s\n", argv[1]);    
    /* check MAC address is valid or not */
    len = strlen(argv[1]);
	if (len != 12) {
		printf("mac address must be 12 digits\n");
		return -1;
	}	   
    
    /* Read bootloader from flash */
    system("cat /dev/mtd0 > /tmp/boot.bin");
    
    /* Modify the MAC address in the bootloader */  
	tmp[2] = 0;
    for(i = 0; i < 6; i++){
		tmp[0] = argv[1][2*i];
		tmp[1] = argv[1][2*i+1];
		mac_addr[i] = (unsigned char)strtoul(tmp, NULL, 16);
	}
	
	printf("new mac addr = ");
    for(i = 0; i < 5; i++)    
        printf("%x:", mac_addr[i]);
    printf("%x\n", mac_addr[5]);
    
    fp = fopen("/tmp/boot.bin", "r+b");
    if( fp == NULL ){
        printf("open file error!!\n");
        system("rm /tmp/boot.bin");        
        return -1;
    }
    
    fseek(fp, 0x0000ff48L, SEEK_SET);
    fwrite(mac_addr, 6, 1, fp );
    fflush(fp);
    fclose(fp);
        
    /* Write the bootloader back to flash and reboot*/
    system("/userfs/bin/mtd -rf write /tmp/boot.bin bootloader");
   
    return 0;
}
#if  defined(CONFIG_TCSUPPORT_LED_BTN_CHECK) || defined(CONFIG_TCSUPPORT_TEST_LED_ALL) 
#if  defined(CONFIG_TCSUPPORT_LED_CHECK) || defined(CONFIG_TCSUPPORT_TEST_LED_ALL) 
static int doLedCheck(int argc, char *argv[])
{
	if((argc == 2) && ((strcmp(argv[1], "on") == 0) || (strcmp(argv[1], "off") == 0))){
		if(strcmp(argv[1], "off") == 0){
			printf("All led is turned off! \r\n");	
		 }	
		else{
			printf("All led is turned on! \r\n");
		}
	}
	else{
		printf("Usage: sys led [on||off] \r\n");
		return -1;
	}
	return 0;
}	
#endif 
#if  defined(CONFIG_TCSUPPORT_BTN_CHECK)
static int doButtonCheck(int argc, char *argv[])
{
	if(argc != 2 || (strcmp(argv[1], "enable") && strcmp(argv[1], "disable"))){
		printf("Usage: sys button [enable||disable]\r\n");
		return -1;
	}
	if (strcmp(argv[1], "disable") == 0){
		printf("All buttons are disabled! \r\n");
	}	
	else{
		printf("All buttons are enabled! \r\n");	
	 }

	return 0;
}	
#endif 
#endif 


int main(int argc, char **argv) 
{
	FILE *proc_file;
	char cmd[160];
	const char *s;
	const char *applet_name;
	int i;
	char str[160];
	int func_num=0;

	if(argc == 2){
		if(!strcmp(argv[1],"version")){
			printf("\r\n tcci version: %s\n",MODULE_VERSION_TCCI);
			return 0;
		}
	}
	
	applet_name = argv[0];

	for (s = applet_name; *s != '\0';) {
		if (*s++ == '/')
			applet_name = s;
	}

	strcpy(cmd, applet_name);
	for (i = 1; i < argc; i++) {
		strcat(cmd, " ");
		strcat(cmd, argv[i]);
	}

    proc_file = fopen("/proc/tc3162/tcci_cmd", "w");
	if (!proc_file) {
		printf("open /proc/tc3162/tcci_cmd fail\n");
		return 0;
	}

	fprintf(proc_file, "%s", cmd);
	fclose(proc_file);

    proc_file = fopen("/proc/tc3162/tcci_cmd", "r");
	if (!proc_file) {
		printf("open /proc/tc3162/tcci_cmd fail\n");
		return 0;
	}
	fgets(str, 160, proc_file);
	func_num = atoi(str);
//	printf("data=%d\n", func_num);
	fclose(proc_file);
	
	if(func_num == 0 )
	   return 0;
	   
	if( ci_func[func_num] != NULL )	
        ci_func[func_num](argc-1, &argv[1]);
	return 0;
}
