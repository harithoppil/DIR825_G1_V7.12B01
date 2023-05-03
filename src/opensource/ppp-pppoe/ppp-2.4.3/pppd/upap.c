/*
 * upap.c - User/Password Authentication Protocol.
 *
 * Copyright (c) 1984-2000 Carnegie Mellon University. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name "Carnegie Mellon University" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For permission or any legal
 *    details, please contact
 *      Office of Technology Transfer
 *      Carnegie Mellon University
 *      5000 Forbes Avenue
 *      Pittsburgh, PA  15213-3890
 *      (412) 268-4387, fax: (412) 268-7395
 *      tech-transfer@andrew.cmu.edu
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Computing Services
 *     at Carnegie Mellon University (http://www.cmu.edu/computing/)."
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define RCSID	"$Id: upap.c,v 1.29 2002/12/04 23:03:33 paulus Exp $"

/*
 * TODO:
 */

#include <stdio.h>
#include <string.h>
#ifdef PPP_SPECIAL_WAN
#include <stdlib.h>
#endif

#include "pppd.h"
#include "upap.h"

static const char rcsid[] = RCSID;

static bool hide_password = 1;

/*
 * Command-line options.
 */
static option_t pap_option_list[] = {
    { "hide-password", o_bool, &hide_password,
      "Don't output passwords to log", OPT_PRIO | 1 },
    { "show-password", o_bool, &hide_password,
      "Show password string in debug log messages", OPT_PRIOSUB | 0 },

    { "pap-restart", o_int, &upap[0].us_timeouttime,
      "Set retransmit timeout for PAP", OPT_PRIO },
    { "pap-max-authreq", o_int, &upap[0].us_maxtransmits,
      "Set max number of transmissions for auth-reqs", OPT_PRIO },
    { "pap-timeout", o_int, &upap[0].us_reqtimeout,
      "Set time limit for peer PAP authentication", OPT_PRIO },

    { NULL }
};

/*
 * Protocol entry points.
 */
static void upap_init __P((int));
static void upap_lowerup __P((int));
static void upap_lowerdown __P((int));
static void upap_input __P((int, u_char *, int));
static void upap_protrej __P((int));
static int  upap_printpkt __P((u_char *, int,
			       void (*) __P((void *, char *, ...)), void *));

struct protent pap_protent = {
    PPP_PAP,
    upap_init,
    upap_input,
    upap_protrej,
    upap_lowerup,
    upap_lowerdown,
    NULL,
    NULL,
    upap_printpkt,
    NULL,
    1,
    "PAP",
    NULL,
    pap_option_list,
    NULL,
    NULL,
    NULL
};

upap_state upap[NUM_PPP];		/* UPAP state; one for each unit */

static void upap_timeout __P((void *));
static void upap_reqtimeout __P((void *));
static void upap_rauthreq __P((upap_state *, u_char *, int, int));
static void upap_rauthack __P((upap_state *, u_char *, int, int));
static void upap_rauthnak __P((upap_state *, u_char *, int, int));
static void upap_sauthreq __P((upap_state *));
static void upap_sresp __P((upap_state *, int, int, char *, int));

#ifdef PPP_SPECIAL_WAN
#define MAX_SEC_MODE 6
unsigned char sec_phase = 1;

#if 1 //星空极速

/*
  星空极速V3.3         
*/
	char keyMaps[93][17]=
	{"E397D1AD9CA2B78E",	
	 "E094D2AE9FA1B48D",	
	 "E195D3AF9EA0B58C",	
	 "E692D4A899A7B28B",	
	 "E793D5A998A6B38A",	
	 "E490D6AA9BA5B089",	
	 "E591D7AB9AA4B188",	
	 "EA9ED8A495ABBE87",	
	 "EB9FD9A594AABF86",	
	 "E296D0AC9DA3B68F",	
	 "A3D791EDDCE2F7CE",	
	 "A5D197EBDAE4F1C8",	
	 "B7C385F9C8F6E3DA",	
	 "A0D492EEDFE1F4CD",	
	 "A6D294E8D9E7F2CB",	
	 "ABDF99E5D4EAFFC6",	
	 "A7D395E9D8E6F3CA",	
	 "BBCF89F5C4FAEFD6",	
	 "BDC98FF3C2FCE9D0",	
	 "A2D690ECDDE3F6CF",	
	 "B3C781FDCCF2E7DE",	
	 "A1D593EFDEE0F5CC",	
	 "B6C284F8C9F7E2DB",	
	 "B4C086FACBF5E0D9",	
	 "B5C187FBCAF4E1D8",	
	 "BACE88F4C5FBEED7",	
	 "B8CC8AF6C7F9ECD5",	
	 "B9CD8BF7C6F8EDD4",	
	 "BECA8CF0C1FFEAD3",	
	 "A8DC9AE6D7E9FCC5",	
	 "AADE98E4D5EBFEC7",	
	 "B1C583FFCEF0E5DC",	
	 "A4D096EADBE5F0C9",	
	 "B0C482FECFF1E4DD",	
	 "BCC88EF2C3FDE8D1",	
	 "BFCB8DF1C0FEEBD2",	
	 "83F7B1CDFCC2D7EE",	
	 "85F1B7CBFAC4D1E8",	
	 "97E3A5D9E8D6C3FA",	
	 "80F4B2CEFFC1D4ED",	
	 "86F2B4C8F9C7D2EB",	
	 "8BFFB9C5F4CADFE6",
	 "87F3B5C9F8C6D3EA",
	 "9BEFA9D5E4DACFF6",
	 "9DE9AFD3E2DCC9F0",
	 "82F6B0CCFDC3D6EF",
	 "93E7A1DDECD2C7FE",
	 "81F5B3CFFEC0D5EC",
	 "96E2A4D8E9D7C2FB",	
	 "94E0A6DAEBD5C0F9",
	 "95E1A7DBEAD4C1F8",
	 "9AEEA8D4E5DBCEF7",
	 "98ECAAD6E7D9CCF5",
	 "99EDABD7E6D8CDF4",
	 "9EEAACD0E1DFCAF3",
	 "88FCBAC6F7C9DCE5",
	 "8AFEB8C4F5CBDEE7",	
	 "91E5A3DFEED0C5FC",
	 "84F0B6CAFBC5D0E9",
	 "90E4A2DEEFD1C4FD",
	 "9CE8AED2E3DDC8F1",
	 "9FEBADD1E0DECBF2",
	 "ACD89EE2D3EDF8C1",
	 "F387C1BD8CB2A79E",
	 "92E6A0DCEDD3C6FF",	
	 "F185C3BF8EB0A59C",
	 "F682C4B889B7A29B",
	 "F783C5B988B6A39A",
	 "8CF8BEC2F3CDD8E1",
	 "F480C6BA8BB5A099",
	 "F88CCAB687B9AC95",
	 "FA8EC8B485BBAE97",
	 "FB8FC9B584BAAF96",	
	 "8DF9BFC3F2CCD9E0",
	 "F98DCBB786B8AD94",
	 "A9DD9BE7D6E8FDC4",
	 "AFDB9DE1D0EEFBC2",
	 "AEDA9CE0D1EFFAC3",
	 "E89CDAA697A9BC85",
	 "F084C2BE8FB1A49D",
	 "EE9ADCA091AFBA83",	
	 "EC98DEA293ADB881",
	 "ED99DFA392ACB980",
	 "89FDBBC7F6C8DDE4",
	 "8FFBBDC1F0CEDBE2",
	 "8EFABCC0F1CFDAE3",
	 "E99DDBA796A8BD84",
	 "F581C7BB8AB4A198",
	 "FE8ACCB081BFAA93",	
	 "FC88CEB283BDA891",
	 "FD89CFB382BCA990",
	 "FF8BCDB180BEAB92",
	 "EF9BDDA190AEBB82"
	 };

int ChinaNetClientV3_3(char *user,char*pwd,char *newUser,char*newPwd)
{
     int i;	
	 int n;	
	 int m;	
	 char *in = user;
	 char *out = newUser;
	 //======================输入的密码在密码表中的位置=====================================================//	
	 char a[93]="1234567890qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";	
	 a[62]='~';
	 a[63]='!';
	 a[64]='@';
	 a[65]='#';
	 a[66]='$';
	 a[67]='%';
	 a[68]='^';	
	 a[69]='&';
	 a[70]='*';
	 a[71]='(';
	 a[72]=')';
	 a[73]='_';
	 a[74]='+';
	 a[75]='{';	
	 a[76]='}';
	 a[77]='|';
	 a[78]=':';
	 a[79]='\"';
	 a[80]='<';
	 a[81]='>';
	 a[82]='\?';	
	 a[83]='[';
	 a[84]=']';
	 a[85]='\\';
	 a[86]=';';
	 a[87]='\'';
	 a[88]=',';	
	 a[89]='.';	
	 a[90]='/';
	 a[91]='-';
	 a[92]='=';	
	 if(strlen(user)>128)
		return 1;
	 //================================end==================================//	
      //===========破解方法==========//	
	 for(i=0;in[i] != '\0';i++)	
	 {		
	     for(n = 0;n < 93; n++)		
	     {			
	         if(in[i] == a[n])  //查找字符在密码表中的位置			
	         {				
	            m = i % 8;  //超过8位的密码，密码位置向前覆盖//第8位的密码，对应位置是第0位				
	            out[2 * i] = keyMaps[n][m * 2];				
	            out[2 * i + 1] = keyMaps[n][m * 2 + 1];			
	         }		
	     }	
	 }	
	 out[2*i] = '\0';	
	 return 0;
}
/*
   星空极速湖南湖北1      
   以下是星空极速客户端中使用的另两种加密方式，在湖南、
   湖北、福建都有出现，在其他地区偶尔使用，所以我们的
   设备必须集成进去，这两种加密比较简单，主要就是在原有
   的PPPOE用户名的头部加入一些特殊字符来形成真正的用户名
*/

/*************************************************************************
功能: 实现星空极速PPPOE+ 用户名加密算法
参数: strName :PPPOE 拨号用户名
返回: 无
备注:
*************************************************************************/

char *  PPPOE_Plus_Username_Encrypt(char  *strName)
{
		//加密开始
		char *string =  strName;  /* 接收用户输入的用户名*/
		char cas_str[]="9012345678abcdeABCDEFGHIJKLMNfghijklmnUVWXYZxyzuvwopqrstOPQRST";
		int cas_str_buffer[16];
		int cas_esi = 37;
		char src_str[128],dec_str[128]= {0};
		int k=0;
		int cas_eax,cas_edx;
		div_t x;
		int  i,j;
		//PPPOE_TRACE("\r\nstrname=%s\r\n",string);
		cas_str_buffer[15]=25;
		cas_str_buffer[14]=35;
		cas_str_buffer[13]=182;
		cas_str_buffer[12]=236;
		cas_str_buffer[11]=43;
		cas_str_buffer[10]=41;
		cas_str_buffer[9]=53;
		cas_str_buffer[8]=18;
		cas_str_buffer[7]=226;
		cas_str_buffer[6]=215;
		cas_str_buffer[5]=24;
		cas_str_buffer[4]=117;
		cas_str_buffer[3]=35;
		cas_str_buffer[2]=201;
		cas_str_buffer[1]=52;
		cas_str_buffer[0]=17;
	
		sprintf(src_str,"%s",string);
				
		for ( i=0;i<strlen(src_str);i++)
		{
			for(j=0;j<strlen(cas_str);j++)
			{

			    if (src_str[i]==cas_str[j])
				{
        			  	    if(i<16)
        			    	      cas_eax=cas_str_buffer[i];
            			    else 
            			    {
            			      x=div(i,16);
            			      cas_eax=cas_str_buffer[x.rem] ;
            			    }
									
            			    cas_edx=cas_esi+cas_esi*2 ;
            			    cas_eax=cas_eax^cas_edx;
            			    cas_eax=cas_eax^k;
            			    cas_eax=cas_eax+j;
            			    x = div(cas_eax,62);
            			    dec_str[i]=cas_str[x.rem];
            			    cas_edx=x.rem ;
            			    cas_esi=cas_esi^(cas_edx+9433);
			             break;
			         }
			 }

			if( dec_str[i]==0)
			dec_str[i]=src_str[i];
			k=k+5;
		}
		//加密结束
		sprintf(string,"2:%s",dec_str);/* string 为真正的PPPOE用户名*/
                  
                   //PPPOE_TRACE("Encrypt name(string) is :%s\n", string);
                   //PPPOE_TRACE("Encrypt name(strName) is :%s\n", strName);
                   return string;
}

int ChinaNetClientVhunanHubei1(char *user,char*pwd,char *newUser,char*newPwd)
{
    if(strlen(user)>250)
		return 1;
	
    newUser[0]=13;
    newUser[1]=10;
    newUser[2]='\0';
    strcat(newUser,user);    
	return 0;

}
/*
  星空极速湖南湖北2   
   以下是星空极速客户端中使用的另两种加密方式，在湖南、
   湖北、福建都有出现，在其他地区偶尔使用，所以我们的
   设备必须集成进去，这两种加密比较简单，主要就是在原有
   的PPPOE用户名的头部加入一些特殊字符来形成真正的用户名
*/

int ChinaNetClientVhunanHubei2(char *user,char*pwd,char *newUser,char*newPwd)
{
    if(strlen(user)>250)
		return 1;
	
    newUser[0]=94;
    newUser[1]=94;
    newUser[2]='\0';
    strcat(newUser,user);    
	return 0;

}

/*************************************************************************
	功能: 实现星空极速动态PPPOE用户名加密算法
	             星空极速江西南昌
	参数: strName :PPPOE 拨号用户名
	返回: 无
	备注:
*************************************************************************/
static void mult(long op1, long op2, long *prod_hi, long *prod_lo)
{
	long op1_hi = (op1 >> 16) & 0xffff;
	long op1_lo = op1 & 0xffff;
	long op2_hi = (op2 >> 16) & 0xffff;
	long op2_lo = op2 & 0xffff;
	long cross_prod = op1_lo * op2_hi + op1_hi * op2_lo;
	*prod_hi = op1_hi * op2_hi + ((cross_prod >> 16) & 0xffff);
	*prod_lo = op1_lo * op2_lo + ((cross_prod << 16) & 0xffff0000);
}  

int ChinaNetClientVnanchang(char *user,char*pwd,char *newUser,char*newPwd)
{
	unsigned long totalInt[16]={0};
	char bufShare[40];
	char bufRealKey2[8];
	char *ourEncryptKey = newUser;		
	char *strName = user;
	char username[256]={0};
	char globalName[256]={0};
	strcpy(globalName, strName);
	char sharename[7]="radius";
	int usernamelen=0; 
	int totallen=usernamelen+6;//16
	unsigned long timeMark;
	unsigned long timeMarkLow;
	unsigned long timeMarkHigh;
	unsigned long timeMarkHighShiftRight1;
	unsigned long timeMarkUse;
	unsigned long timeMarkUse1;
	//uint64_t timeMark64;
	unsigned long timeMarkUseLow;
	unsigned long timeMarkUseHigh;
	int tempi1;
	int tempi2;
	//int tempk;
	int tempi;
	int tempj;
	int k;
	int m;
	//int i;
	unsigned char my2_dl,my2_cl;
	unsigned char my2_char_arr[7];
	unsigned long tempUse1=0;
	//unsigned char frontKey[30];
	int countKey_sign;
	unsigned char shiftCharValue;
	unsigned long my_esi;
	unsigned long my_eax,temp_eax,my_ebx,my_ecx,my_edx,my_ecx2,my_ebp,my_edi;

	unsigned long my2_esi,zhi_eax,zhi_ecx;
	unsigned long my2_eax,my2_edx,my2_ecx;
	//char *string = strName;
	unsigned char shiftKey[16]={
							0x07,0x0C,0x11,0x16,0x05,0x09,0x0e,0x14,
							0x04,0x0b,0x10,0x17,0x06,0x0a,0x0f,0x15
							 };
	unsigned char valueKey[48]={
							 0x01,0x06,0x0b,0x00,0x05,0x0a,0x0f,0x04,
							 0x09,0x0e,0x03,0x08,0x0d,0x02,0x07,0x0c,
							 0x05,0x08,0x0b,0x0e,0x01,0x04,0x07,0x0a,
							 0x0d,0x00,0x03,0x06,0x09,0x0c,0x0f,0x02,
							 0x00,0x07,0x0e,0x05,0x0c,0x03,0x0a,0x01,
							 0x08,0x0f,0x06,0x0d,0x04,0x0b,0x02,0x09
							   };
	unsigned int globalKey[64]={
							0xD76AA478,0xE8C7B756,0x242070DB,0xC1BDCEEE,
							0xF57C0FAF,0x4787C62A,0xA8304613,0xFD469501,
							0x698098D8,0x8B44F7AF,0xFFFF5BB1,0x895CD7BE,
							0x6B901122,0xFD987193,0xA679438E,0x49B40821,
							0xF61E2562,0xC040B340,0x265E5A51,0xE9B6C7AA,
							0xD62F105D,0x02441453,0xD8A1E681,0xE7D3FBC8,
							0x21E1CDE6,0xC33707D6,0xF4D50D87,0x455A14ED,
							0xA9E3E905,0xFCEFA3F8,0x676F02D9,0x8D2A4C8A,
							0xFFFA3942,0x8771F681,0x6D9D6122,0xFDE5380C,
							0xA4BEEA44,0x4BDECFA9,0xF6BB4B60,0xBEBFBC70,
							0x289B7EC6,0xEAA127FA,0xD4EF3085,0x04881D05,
							0xD9D4D039,0xE6DB99E5,0x1FA27CF8,0xC4AC5665,
							0xF4292244,0x432AFF97,0xAB9423A7,0xFC93A039,
							0x655B59C3,0x8F0CCC92,0xFFEFF47D,0x85845DD1,
							0x6FA87E4F,0xFE2CE6E0,0xA3014314,0x4E0811A1,
							0xF7537E82,0xBD3AF235,0x2AD7D2BB,0xEB86D391
							  };
	unsigned long countKey[4]={0x67452301,0xEFCDAB89,0x98BADCFE,0x10325476};
	unsigned long countKeyTemp[4]={0x67452301,0xEFCDAB89,0x98BADCFE,0x10325476};

	if(strlen(user)>128)
		return 1;
	

	strcat(globalName,sharename);
	//PPPOE_TRACE("\r\n%d---%d\r\n",sizeof(unsigned long),sizeof(unsigned long long));
	time(&timeMark);   /* 获得系统当前时间*/
	//PPPOE_TRACE("\r\ntime=%0lx",timeMark);
	mult(timeMark,0x66666667,&timeMarkHigh,&timeMarkLow);
	//PPPOE_TRACE("\r\ntimeH=%0lx,timeL=%0lx,shift=%0lx\r\n",timeMarkHigh,timeMarkLow,timeMarkHigh>>1);
   // if((timeMarkHigh>>31)==1)
   if((timeMarkHigh&&0x80000000)==0x80000000)
		timeMarkHighShiftRight1=(timeMarkHigh>>1)+0x10000000;
	else
		timeMarkHighShiftRight1=timeMarkHigh>>1;
	//timeMarkHighShiftRight1=0EC1E867
	//PPPOE_TRACE("\r\nshift=%0lx\r\n",timeMarkHighShiftRight1);
	timeMarkHighShiftRight1=timeMarkHighShiftRight1+(timeMarkHighShiftRight1>>31);
	//PPPOE_TRACE("\r\nshift=%0lx\r\n",timeMarkHighShiftRight1);
	//timeMarkHighShiftRight1=0EC1E867

	timeMarkUse=ntohl(timeMarkHighShiftRight1);//in little ed
	//PPPOE_TRACE("eeee\r\n--%0lx",timeMarkUse);
	//timeMarkUse=htonl(timeMarkHighShiftRight1);//in big ed
	//PPPOE_TRACE("\r\nkkk --%0lx--%0lx--%0lx--%0lx--",timeMarkHighShiftRight1<<24,timeMarkHighShiftRight1<<16,timeMarkHighShiftRight1<<8,timeMarkHighShiftRight1);
	timeMarkUse=((timeMarkHighShiftRight1<<24)&0xff000000)+((timeMarkHighShiftRight1<<8)&0x00ff0000)+((timeMarkHighShiftRight1>>8)&0x0000ff00)+((timeMarkHighShiftRight1>>24)&0x000000ff);
	timeMarkUse1=timeMarkUse;
	//timeMarkUSE=0x67E8C10E
	timeMarkUseLow=timeMarkUse&0x0000ffff;//=0000C10E
	timeMarkUseHigh=timeMarkUse&0xffff0000;//=67E80000
	//PPPOE_TRACE("\r\nuse=%0lx,uselow=%0x,usehi=%0x\r\n",timeMarkUse,timeMarkUseLow,timeMarkUseHigh);

	my2_edx=timeMarkUseLow;
	my2_eax=3;
	my2_ecx=0;
	my2_esi=my2_esi^my2_esi;
	for(k=0;k<0x20;k++)
	{
		zhi_eax=my2_eax;
		zhi_ecx=my2_ecx;
		//PPPOE_TRACE("--zhi=%0lx,%0lx",zhi_eax,zhi_ecx);
		my2_eax=(my2_eax&0xffffff00)+((timeMarkUse1>>8*my2_eax)&0x000000ff);
		my2_ecx=(my2_ecx&0xffffff00)+((tempUse1>>8*my2_ecx)&0x000000ff);
		//PPPOE_TRACE("\r\ntest 1 -- %0lx---%0lx",my2_eax,my2_ecx);
		my2_edx=(my2_edx&0xffffff00)+ (my2_eax&0x000000ff);
		//PPPOE_TRACE("---%0lx",my2_edx);
		my2_ecx=(my2_ecx&0xffffff00)+(((my2_ecx<<24)<<1)>>24);
		//PPPOE_TRACE("-%0lx",my2_ecx);
		my2_edx=(my2_edx&0xffffff00)+((my2_edx&0x000000ff)&0x00000001);
		//PPPOE_TRACE("-%0lx",my2_edx);
		my2_ecx=(my2_ecx&0xffffff00)+((my2_ecx&0x000000ff)|(my2_edx&0x000000ff));
		//PPPOE_TRACE("-%0lx",my2_ecx);
		my2_eax=(my2_eax&0xffffff00)+((my2_eax&0x000000ff)>>1);
		my2_esi++;
		//PPPOE_TRACE("-%0lx-%0lx",my2_eax,my2_esi);
		if(zhi_eax==0)
			timeMarkUse1=(timeMarkUse1&0xffffff00)+(my2_eax&0x000000ff);
		else if(zhi_eax==1)
			timeMarkUse1=(timeMarkUse1&0xffff00ff)+((my2_eax<<8)&0x0000ff00);
		else if(zhi_eax==2)
			timeMarkUse1=(timeMarkUse1&0xff00ffff)+((my2_eax<<16)&0x00ff0000);
		else
			timeMarkUse1=(timeMarkUse1&0x00ffffff)+((my2_eax<<24)&0xff000000);
		//PPPOE_TRACE("-%0lx",timeMarkUse1);
		if(zhi_ecx==0)
			tempUse1=(tempUse1&0xffffff00)+(my2_ecx&0x000000ff);
		else if(zhi_ecx==1)
			tempUse1=(tempUse1&0xffff00ff)+((my2_ecx<<8)&0x0000ff00);
		else if(zhi_ecx==2)
			tempUse1=(tempUse1&0xff00ffff)+((my2_ecx<<16)&0x00ff0000);
		else
			tempUse1=(tempUse1&0x00ffffff)+((my2_ecx<<24)&0xff000000);

		//PPPOE_TRACE("-%0lx",tempUse1);
		my2_eax=my2_esi;
		//PPPOE_TRACE("-&&%0lx",my2_eax);

		//my2_edx=my2_edx+((my2_eax>>31)<<31);	//cdq调用
		my2_edx=0;
		my2_edx=my2_edx&0x00000007;
		my2_ecx=3;
		//PPPOE_TRACE("-%0lx-%0lx",my2_edx,my2_ecx);
		my2_eax=my2_eax+my2_edx;
		//PPPOE_TRACE("-RR%0lx",my2_eax);
		my2_edx=my2_esi;
		//PPPOE_TRACE("-%0lx",my2_edx);
		my2_eax=my2_eax/8;
		//if((my2_eax>>31)==1)
			 //my2_eax=(my2_eax>>3)+0x80000000;
		//else
			//my2_eax=(my2_eax>>3);
		//PPPOE_TRACE("zhongyao--%0lx-%0lx",my2_ecx,my2_eax);

		my2_ecx=my2_ecx-my2_eax;
		//PPPOE_TRACE("-%0lx",my2_ecx);
		my2_edx=my2_edx&0x80000003;
		my2_eax=my2_ecx;
		if((my2_eax>>31)==1)
		{
			//PPPOE_TRACE("-jinqu??()**&^%-----");
			my2_edx--;
			my2_edx=my2_edx|0xfffffffc;
			my2_edx++;
		}
		//PPPOE_TRACE("-%0lx-%0lx",my2_edx,my2_eax);
		my2_ecx=my2_edx;
		//PPPOE_TRACE("-%0lx",my2_ecx);
		//PPPOE_TRACE("\r\n nmy2---sign1=%0lx,sign2=%0lx\r\n",timeMarkUse1,tempUse1);
	}
	//PPPOE_TRACE("\r\nguanjian timeUse=%0lx",tempUse1);

	my2_cl=tempUse1&0x000000ff;
	my2_cl=my2_cl>>2;
	my2_char_arr[0]=my2_cl;
	my2_cl=tempUse1&0x000000ff;
	my2_cl=my2_cl&0x03;
	my2_cl=my2_cl<<4;
	my2_char_arr[1]=my2_cl;
	my2_dl=(tempUse1>>8)&0x000000ff;
	my2_dl=my2_dl>>4;
	my2_dl=my2_dl|my2_cl;
	my2_char_arr[1]=my2_dl;
	my2_cl=(tempUse1>>8)&0x000000ff;
	my2_cl=my2_cl&0x0f;
	my2_cl=my2_cl<<2;
	my2_char_arr[2]=my2_cl;
	my2_dl=(tempUse1>>16)&0x000000ff;
	my2_dl=my2_dl>>6;
	my2_dl=my2_dl|my2_cl;
	my2_char_arr[2]=my2_dl;
	my2_cl=(tempUse1>>16)&0x000000ff;
	my2_cl=my2_cl&0x3f;
	my2_char_arr[3]=my2_cl;
	my2_dl=(tempUse1>>24)&0x000000ff;
	my2_dl=my2_dl>>2;
	my2_char_arr[4]=my2_dl;
	my2_cl=(tempUse1>>24)&0x000000ff;
	my2_cl=my2_cl&0x03;
	my2_cl=my2_cl<<4;
	my2_char_arr[5]=my2_cl;

	for(m=0;m<6;m++)
	{
		my2_dl=my2_char_arr[m];
		my2_dl=my2_dl+0x20;
		my2_char_arr[m]=my2_dl;
		if(my2_dl>=0x40)
		{
			my2_dl++;
			my2_char_arr[m]=my2_dl;
		}

	}
	//PPPOE_TRACE("\r\nfront key:");
#if 0
	for(m=0;m<6;m++)
	{
		PPPOE_TRACE("\r\n%c--%0x",my2_char_arr[m],my2_char_arr[m]);
	}
	PPPOE_TRACE("\r\n");
	PPPOE_TRACE("\r\nlen=%d,%s\r\n",totallen,globalName);
#endif

	totalInt[0]=timeMarkUse;
	tempi1=totallen/4;//4
	tempi2=totallen%4;//0
	int i;
	int j=-1;
	for(i=1;i<=tempi1;i++)
	{
		j+=4;
		totalInt[i]=totalInt[i]+globalName[j];
		totalInt[i]=(totalInt[i]<<8)+globalName[j-1];
		totalInt[i]=(totalInt[i]<<8)+globalName[j-2];
		totalInt[i]=(totalInt[i]<<8)+globalName[j-3];

	}
	if(tempi2==0)
		totalInt[i]=0x80;
	else if(tempi2==1)
	{
		totalInt[i]=0x80;
		totalInt[i]=(totalInt[i]<<8)+globalName[totallen-1];
	}
	else if(tempi2==2)
	{
		totalInt[i]=0x80;
		totalInt[i]=(totalInt[i]<<8)+globalName[totallen-1];
		totalInt[i]=(totalInt[i]<<8)+globalName[totallen-2];
	}
	else
	{
		totalInt[i]=0x80;
		totalInt[i]=(totalInt[i]<<8)+globalName[totallen-1];
		totalInt[i]=(totalInt[i]<<8)+globalName[totallen-2];
		totalInt[i]=(totalInt[i]<<8)+globalName[totallen-3];
	}
	//totalInt[14]=totalInt[14]+0xA0;
	totalInt[14]=totalInt[14]+(usernamelen+0x0a)*0x08;
#if 0
	for(tempk=0;tempk<16;tempk++)
	{
		PPPOE_TRACE("\r\n%08x",totalInt[tempk]);
	}
#endif
	countKey_sign=0;
	for(tempi=0;tempi<4;tempi++)
	{
		tempj=0;
		//countKey_sign=countKey_sign+tempi*16;
		countKey_sign=tempi*16;
		for(tempj=0;tempj<0x10;tempj++)
		{
			my_eax=tempj;
#if 0
			PPPOE_TRACE("--count=%d--",tempj);
#endif
			temp_eax=my_eax%4;
#if 0
			PPPOE_TRACE("--temp_eax=%d--",temp_eax);
#endif
			if(temp_eax==1)
				my_edx=temp_eax+2;
			else if(temp_eax==3)
				my_edx=temp_eax-2;
			else
				my_edx=temp_eax;

			if(temp_eax==0)
			{
				my_eax=2;
				my_ecx=1;
			}
			else if(temp_eax==1)
			{
				my_eax=1;
				my_ecx=0;
			}
			else if(temp_eax==2)
			{
				my_eax=0;
				my_ecx=3;
			}
			else
			{
				my_eax=3;
				my_ecx=2;
			}

			if(temp_eax==0)
				my_ecx2=3;
			else if(temp_eax==1)
				my_ecx2=2;
			else if(temp_eax==2)
				my_ecx2=1;
			else
				my_ecx2=0;

#if 0
			PPPOE_TRACE("--eax=%d,ecx=%d,ecx2=%d",my_eax,my_ecx,my_ecx2);
#endif
			shiftCharValue=shiftKey[temp_eax+tempi*4];
#if 0
			PPPOE_TRACE("\r\n%0x",shiftCharValue);
#endif
			my_ebp=countKey[my_ecx];
			my_edi=countKey[my_eax];
			my_ecx2=countKey[my_ecx2];
			switch(tempi)
			{
			case 0:
				my_eax=my_ebp;
				my_edi=my_edi&my_ebp;
				my_eax=~my_eax;
				my_eax=my_eax&my_ecx2;
				my_eax=my_eax|my_edi;
				my_edi=totalInt[tempj];
		
				my_eax=my_eax+my_edi;
				my_edi=countKey[my_edx];
				my_ecx2=globalKey[countKey_sign];
				my_ecx2=my_ecx2+my_edi;
				my_eax=my_eax+my_ecx2;
				my_ebx=my_eax;
				my_ebx=my_ebx>>(0x20-shiftCharValue);
				my_eax=my_eax<<(0x0+shiftCharValue);
				my_ebx=my_ebx|my_eax;
				my_ebx=my_ebx+my_ebp;
				my_eax=my_ebx;
				countKey[my_edx]=my_eax;
#if 0
				PPPOE_TRACE("\r\nmy_edx=%0x,value=%0lx",my_edx,countKey[my_edx]);
#endif
				countKey_sign++;
				break;
			case 1:
				my_eax=my_ecx2;
				my_ecx2=my_ecx2&my_ebp;
				my_eax=~my_eax;
				my_eax=my_eax&my_edi;
				my_eax=my_eax|my_ecx2;
				my_ecx2=my_ecx2^my_ecx2;
				my_ecx2=valueKey[tempj+(tempi-1)*16];
				my_edi=totalInt[my_ecx2];
		
				my_eax=my_eax+my_edi;
				my_edi=countKey[my_edx];
				my_ecx2=globalKey[countKey_sign];
				my_ecx2=my_ecx2+my_edi;
				my_eax=my_eax+my_ecx2;
				my_ebx=my_eax;
				my_ebx=my_ebx>>(0x20-shiftCharValue);
				my_eax=my_eax<<(0x0+shiftCharValue);
				my_ebx=my_ebx|my_eax;
				my_ebx=my_ebx+my_ebp;
				my_eax=my_ebx;
				countKey[my_edx]=my_eax;
#if 0
				PPPOE_TRACE("\r\nmy_edx=%0x,value1=%0lx",my_edx,countKey[my_edx]);
#endif
				countKey_sign++;
				break;
			case 2:
				my_eax=my_eax^my_eax;
				my_ecx2=my_ecx2^my_edi;
				my_eax=valueKey[tempj+(tempi-1)*16];
#if 0
				PPPOE_TRACE("vk=%0x",my_eax);
#endif
				my_ecx2=my_ecx2^my_ebp;
				my_eax=totalInt[my_eax];
				my_eax=my_eax+my_ecx2;
		
		
				//my_eax=my_eax+my_edi;
				my_edi=countKey[my_edx];
				my_ecx2=globalKey[countKey_sign];
				my_ecx2=my_ecx2+my_edi;
				my_eax=my_eax+my_ecx2;
				my_ebx=my_eax;
				my_ebx=my_ebx>>(0x20-shiftCharValue);
				my_eax=my_eax<<(0x0+shiftCharValue);
				my_ebx=my_ebx|my_eax;
				my_ebx=my_ebx+my_ebp;
				my_eax=my_ebx;
				countKey[my_edx]=my_eax;
#if 0
				PPPOE_TRACE("\r\nmy_edx=%0x,value2=%0lx",my_edx,countKey[my_edx]);
#endif
				countKey_sign++;
				break;
			default:
				my_eax=my_eax^my_eax;
				my_eax=valueKey[tempj+(tempi-1)*16];
				my_ecx2=~my_ecx2;
				my_eax=totalInt[my_eax];
				my_ecx2=my_ecx2|my_ebp;
				my_ecx2=my_ecx2^my_edi;
				my_eax=my_eax+my_ecx2;
		
		
				my_edi=countKey[my_edx];
				my_ecx2=globalKey[countKey_sign];
				my_ecx2=my_ecx2+my_edi;
				my_eax=my_eax+my_ecx2;
				my_ebx=my_eax;
				my_ebx=my_ebx>>(0x20-shiftCharValue);
				my_eax=my_eax<<(0x0+shiftCharValue);
				my_ebx=my_ebx|my_eax;
				my_ebx=my_ebx+my_ebp;
				my_eax=my_ebx;
				countKey[my_edx]=my_eax;
#if 0
				PPPOE_TRACE("\r\nmy_edx=%0x,value3=%0lx",my_edx,countKey[my_edx]);
#endif
				countKey_sign++;
				break;	
			}
		}
	}
#if 0
	PPPOE_TRACE("\r\nkey1 value");
	for(i=0;i<4;i++)
	{
		PPPOE_TRACE("\r\n%08lx",countKey[i]);
	} 
#endif
	for(i=0;i<4;i++)
	{
		my_esi=countKeyTemp[i];
		my_ebp=countKey[i];
		my_ebp=my_ebp+my_esi;
		countKey[i]=my_ebp;
	}
#if 0
	PPPOE_TRACE("\r\nkey2 value");
	for(i=0;i<4;i++)
	{
		PPPOE_TRACE("\r\n%08lx",countKey[i]);
	} 
	PPPOE_TRACE("\r\n");
#endif
	sprintf(bufShare,"%02x%02x%02x%02x",(char)(countKey[0]&0x000000ff), 
   (char) ((countKey[0]>>8)&0x000000ff),(char)((countKey[0]>>16)&0x000000ff), 
	(char)((countKey[0]>>24)&0x000000ff));
	printf("----------------zzzzzzz  username:%s-----------\n",username);/////////////strName
	printf("----------------zzzzzzz  strName:%s-----------\n",strName);
	for(i=1;i<4;i++)
	{
		sprintf(bufShare,"%s%02x%02x%02x%02x",bufShare,
											(char)(countKey[i]&0x000000ff),
											 (char)((countKey[i]>>8)&0x000000ff),
											(char)((countKey[i]>>16)&0x000000ff),
											 (char)((countKey[i]>>24)&0x000000ff)
											);
	}
	/*此处比较奇怪，若将strcpy(username, strName);放到函数的开头，运行到这
里的时候username值会发生变化, 变成另外的莫名其妙的值而非当初
strName拷贝过去的值，而 此前也没有看到谁用到username ，并且我在
PC上运行测试这个函数也没发现有此问题,可能和堆栈有关系*/
	strcpy(username, strName);
	usernamelen=strlen(username);
	printf("----------------here  username:%s-----------\n",username);
	printf("----------------here  strName:%s-----------\n",strName);
#if 0
	PPPOE_TRACE("\r\nshareKey=%s",bufShare);
#endif
	strncpy(bufRealKey2,bufShare,4);
#if 0
	PPPOE_TRACE("\r\nbufRealKey2=%s",bufRealKey2);
#endif
	ourEncryptKey[0]=0x0d;
	ourEncryptKey[1]=0x0a;
	for(m=0;m<6;m++)
	{
		ourEncryptKey[m+2]=my2_char_arr[m];;
	}
	ourEncryptKey[8]=bufRealKey2[0];
	ourEncryptKey[9]=bufRealKey2[1];
	for(m=0;m<usernamelen;m++)
	{
		ourEncryptKey[10+m]=username[m];
		printf("ourEncryptKey[10+m]):%c, username[m]:%c\n", ourEncryptKey[10+m],username[m]);
	}
	ourEncryptKey[10+usernamelen]='\0';
	printf("\r\nkey is \r\n");
	for(m=0;m<10+usernamelen;m++)
	{
		printf("%c---%0x\r\n",ourEncryptKey[m],ourEncryptKey[m]);
	}
	//strName=ourEncryptKey;
	printf("ourEncryptKey:%s\n", ourEncryptKey);
	return 0;
	//ourEncryptKey 这个字符型数组就是加密后得到的真实pppoe用户名，用这个进行拨号
}
#endif
#endif


/*
 * upap_init - Initialize a UPAP unit.
 */
static void
upap_init(unit)
    int unit;
{
    upap_state *u = &upap[unit];

    u->us_unit = unit;
    u->us_user = NULL;
    u->us_userlen = 0;
    u->us_passwd = NULL;
    u->us_passwdlen = 0;
    u->us_clientstate = UPAPCS_INITIAL;
    u->us_serverstate = UPAPSS_INITIAL;
    u->us_id = 0;
    u->us_timeouttime = UPAP_DEFTIMEOUT;
    u->us_maxtransmits = 10;
    u->us_reqtimeout = UPAP_DEFREQTIME;
}


/*
 * upap_authwithpeer - Authenticate us with our peer (start client).
 *
 * Set new state and send authenticate's.
 */
void
upap_authwithpeer(unit, user, password)
    int unit;
    char *user, *password;
{
    upap_state *u = &upap[unit];

    /* Save the username and password we're given */
    u->us_user = user;
    u->us_userlen = strlen(user);
    u->us_passwd = password;
    u->us_passwdlen = strlen(password);
    u->us_transmits = 0;

    /* Lower layer up yet? */
    if (u->us_clientstate == UPAPCS_INITIAL ||
	u->us_clientstate == UPAPCS_PENDING) {
	u->us_clientstate = UPAPCS_PENDING;
	return;
    }

    upap_sauthreq(u);			/* Start protocol */
}


/*
 * upap_authpeer - Authenticate our peer (start server).
 *
 * Set new state.
 */
void
upap_authpeer(unit)
    int unit;
{
    upap_state *u = &upap[unit];

    /* Lower layer up yet? */
    if (u->us_serverstate == UPAPSS_INITIAL ||
	u->us_serverstate == UPAPSS_PENDING) {
	u->us_serverstate = UPAPSS_PENDING;
	return;
    }

    u->us_serverstate = UPAPSS_LISTEN;
    if (u->us_reqtimeout > 0)
	TIMEOUT(upap_reqtimeout, u, u->us_reqtimeout);
}


/*
 * upap_timeout - Retransmission timer for sending auth-reqs expired.
 */
static void
upap_timeout(arg)
    void *arg;
{
    upap_state *u = (upap_state *) arg;

    if (u->us_clientstate != UPAPCS_AUTHREQ)
	return;

    if (u->us_transmits >= u->us_maxtransmits) {
	/* give up in disgust */
	error("No response to PAP authenticate-requests");
	u->us_clientstate = UPAPCS_BADAUTH;
	auth_withpeer_fail(u->us_unit, PPP_PAP);
	return;
    }

    upap_sauthreq(u);		/* Send Authenticate-Request */
}


/*
 * upap_reqtimeout - Give up waiting for the peer to send an auth-req.
 */
static void
upap_reqtimeout(arg)
    void *arg;
{
    upap_state *u = (upap_state *) arg;

    if (u->us_serverstate != UPAPSS_LISTEN)
	return;			/* huh?? */

    auth_peer_fail(u->us_unit, PPP_PAP);
    u->us_serverstate = UPAPSS_BADAUTH;
}


/*
 * upap_lowerup - The lower layer is up.
 *
 * Start authenticating if pending.
 */
static void
upap_lowerup(unit)
    int unit;
{
    upap_state *u = &upap[unit];

    if (u->us_clientstate == UPAPCS_INITIAL)
	u->us_clientstate = UPAPCS_CLOSED;
    else if (u->us_clientstate == UPAPCS_PENDING) {
	upap_sauthreq(u);	/* send an auth-request */
    }

    if (u->us_serverstate == UPAPSS_INITIAL)
	u->us_serverstate = UPAPSS_CLOSED;
    else if (u->us_serverstate == UPAPSS_PENDING) {
	u->us_serverstate = UPAPSS_LISTEN;
	if (u->us_reqtimeout > 0)
	    TIMEOUT(upap_reqtimeout, u, u->us_reqtimeout);
    }
}


/*
 * upap_lowerdown - The lower layer is down.
 *
 * Cancel all timeouts.
 */
static void
upap_lowerdown(unit)
    int unit;
{
    upap_state *u = &upap[unit];

    if (u->us_clientstate == UPAPCS_AUTHREQ)	/* Timeout pending? */
	UNTIMEOUT(upap_timeout, u);		/* Cancel timeout */
    if (u->us_serverstate == UPAPSS_LISTEN && u->us_reqtimeout > 0)
	UNTIMEOUT(upap_reqtimeout, u);

    u->us_clientstate = UPAPCS_INITIAL;
    u->us_serverstate = UPAPSS_INITIAL;
}


/*
 * upap_protrej - Peer doesn't speak this protocol.
 *
 * This shouldn't happen.  In any case, pretend lower layer went down.
 */
static void
upap_protrej(unit)
    int unit;
{
    upap_state *u = &upap[unit];

    if (u->us_clientstate == UPAPCS_AUTHREQ) {
	error("PAP authentication failed due to protocol-reject");
	auth_withpeer_fail(unit, PPP_PAP);
    }
    if (u->us_serverstate == UPAPSS_LISTEN) {
	error("PAP authentication of peer failed (protocol-reject)");
	auth_peer_fail(unit, PPP_PAP);
    }
    upap_lowerdown(unit);
}


/*
 * upap_input - Input UPAP packet.
 */
static void
upap_input(unit, inpacket, l)
    int unit;
    u_char *inpacket;
    int l;
{
    upap_state *u = &upap[unit];
    u_char *inp;
    u_char code, id;
    int len;

    /*
     * Parse header (code, id and length).
     * If packet too short, drop it.
     */
    inp = inpacket;
    if (l < UPAP_HEADERLEN) {
	UPAPDEBUG(("pap_input: rcvd short header."));
	return;
    }
    GETCHAR(code, inp);
    GETCHAR(id, inp);
    GETSHORT(len, inp);
    if (len < UPAP_HEADERLEN) {
	UPAPDEBUG(("pap_input: rcvd illegal length."));
	return;
    }
    if (len > l) {
	UPAPDEBUG(("pap_input: rcvd short packet."));
	return;
    }
    len -= UPAP_HEADERLEN;

    /*
     * Action depends on code.
     */
    switch (code) {
    case UPAP_AUTHREQ:
	upap_rauthreq(u, inp, id, len);
	break;

    case UPAP_AUTHACK:
	upap_rauthack(u, inp, id, len);
	break;

    case UPAP_AUTHNAK:
	upap_rauthnak(u, inp, id, len);
	break;

    default:				/* XXX Need code reject */
	break;
    }
}


/*
 * upap_rauth - Receive Authenticate.
 */
static void
upap_rauthreq(u, inp, id, len)
    upap_state *u;
    u_char *inp;
    int id;
    int len;
{
    u_char ruserlen, rpasswdlen;
    char *ruser, *rpasswd;
    char rhostname[256];
    int retcode;
    char *msg;
    int msglen;

    if (u->us_serverstate < UPAPSS_LISTEN)
	return;

    /*
     * If we receive a duplicate authenticate-request, we are
     * supposed to return the same status as for the first request.
     */
    if (u->us_serverstate == UPAPSS_OPEN) {
	upap_sresp(u, UPAP_AUTHACK, id, "", 0);	/* return auth-ack */
	return;
    }
    if (u->us_serverstate == UPAPSS_BADAUTH) {
	upap_sresp(u, UPAP_AUTHNAK, id, "", 0);	/* return auth-nak */
	return;
    }

    /*
     * Parse user/passwd.
     */
    if (len < 1) {
	UPAPDEBUG(("pap_rauth: rcvd short packet."));
	return;
    }
    GETCHAR(ruserlen, inp);
    len -= sizeof (u_char) + ruserlen + sizeof (u_char);
    if (len < 0) {
	UPAPDEBUG(("pap_rauth: rcvd short packet."));
	return;
    }
    ruser = (char *) inp;
    INCPTR(ruserlen, inp);
    GETCHAR(rpasswdlen, inp);
    if (len < rpasswdlen) {
	UPAPDEBUG(("pap_rauth: rcvd short packet."));
	return;
    }
    rpasswd = (char *) inp;

    /*
     * Check the username and password given.
     */
    retcode = check_passwd(u->us_unit, ruser, ruserlen, rpasswd,
			   rpasswdlen, &msg);
    BZERO(rpasswd, rpasswdlen);

    /*
     * Check remote number authorization.  A plugin may have filled in
     * the remote number or added an allowed number, and rather than
     * return an authenticate failure, is leaving it for us to verify.
     */
    if (retcode == UPAP_AUTHACK) {
	if (!auth_number()) {
	    /* We do not want to leak info about the pap result. */
	    retcode = UPAP_AUTHNAK; /* XXX exit value will be "wrong" */
	    warn("calling number %q is not authorized", remote_number);
	}
    }

    msglen = strlen(msg);
    if (msglen > 255)
	msglen = 255;
    upap_sresp(u, retcode, id, msg, msglen);

    /* Null terminate and clean remote name. */
    slprintf(rhostname, sizeof(rhostname), "%.*v", ruserlen, ruser);

    if (retcode == UPAP_AUTHACK) {
	u->us_serverstate = UPAPSS_OPEN;
	notice("PAP peer authentication succeeded for %q", rhostname);
	auth_peer_success(u->us_unit, PPP_PAP, 0, ruser, ruserlen);
    } else {
	u->us_serverstate = UPAPSS_BADAUTH;
	warn("PAP peer authentication failed for %q", rhostname);
	auth_peer_fail(u->us_unit, PPP_PAP);
    }

    if (u->us_reqtimeout > 0)
	UNTIMEOUT(upap_reqtimeout, u);
}


/*
 * upap_rauthack - Receive Authenticate-Ack.
 */
static void
upap_rauthack(u, inp, id, len)
    upap_state *u;
    u_char *inp;
    int id;
    int len;
{
    u_char msglen;
    char *msg;

    if (u->us_clientstate != UPAPCS_AUTHREQ) /* XXX */
	return;

    /*
     * Parse message.
     */
    if (len < 1) {
	UPAPDEBUG(("pap_rauthack: ignoring missing msg-length."));
    } else {
	GETCHAR(msglen, inp);
	if (msglen > 0) {
	    len -= sizeof (u_char);
	    if (len < msglen) {
		UPAPDEBUG(("pap_rauthack: rcvd short packet."));
		return;
	    }
	    msg = (char *) inp;
	    PRINTMSG(msg, msglen);
	}
    }

    u->us_clientstate = UPAPCS_OPEN;

    notice("PAP authentication succeeded");
    auth_withpeer_success(u->us_unit, PPP_PAP, 0);
}


/*
 * upap_rauthnak - Receive Authenticate-Nak.
 */
static void
upap_rauthnak(u, inp, id, len)
    upap_state *u;
    u_char *inp;
    int id;
    int len;
{
    u_char msglen;
    char *msg;

    if (u->us_clientstate != UPAPCS_AUTHREQ) /* XXX */
	return;

    /*
     * Parse message.
     */
    if (len < 1) {
	UPAPDEBUG(("pap_rauthnak: ignoring missing msg-length."));
    } else {
	GETCHAR(msglen, inp);
	if (msglen > 0) {
	    len -= sizeof (u_char);
	    if (len < msglen) {
		UPAPDEBUG(("pap_rauthnak: rcvd short packet."));
		return;
	    }
	    msg = (char *) inp;
	    PRINTMSG(msg, msglen);
	}
    }

    u->us_clientstate = UPAPCS_BADAUTH;

    error("PAP authentication failed");
    auth_withpeer_fail(u->us_unit, PPP_PAP);
}

#ifdef PPP_SPECIAL_WAN
void encrypt_XKJS(char* user,char* password,char* new_user,char* new_password,unsigned char state)
{
    switch (state)
    {
      case MAX_SEC_MODE:
			strcpy(new_user,user);
			strcpy(new_password,password);
			break;
	 case 1:
	 	    ChinaNetClientVhunanHubei2(user,NULL,new_user,NULL);	
			ChinaNetClientV3_3(password,NULL,new_password,NULL);
			break;
	 case 2:
	 	    ChinaNetClientVhunanHubei1(user,NULL,new_user,NULL);
			strcpy(new_password,password);
			break;
	 case 3:
	 	    ChinaNetClientVhunanHubei2(user,NULL,new_user,NULL);
			strcpy(new_password,password);
			break;
	 case 4:
	 	    ChinaNetClientVnanchang(user,NULL,new_user,NULL);
			strcpy(new_password,password);
			break;
	 case 5:
			strcpy(new_user,user);
	 	    PPPOE_Plus_Username_Encrypt(new_user);
			strcpy(new_password,password);
			break;
		
	 default:
 	       strcpy(new_user,user);
	       strcpy(new_password,password);
 	       break;
}	 	
		
}
#endif

/*
 * upap_sauthreq - Send an Authenticate-Request.
 */
static void
upap_sauthreq(u)
    upap_state *u;
{
    u_char *outp;
    int outlen;

    outlen = UPAP_HEADERLEN + 2 * sizeof (u_char) +
	u->us_userlen + u->us_passwdlen;
    outp = outpacket_buf;
    
    MAKEHEADER(outp, PPP_PAP);

    PUTCHAR(UPAP_AUTHREQ, outp);
    PUTCHAR(++u->us_id, outp);
    PUTSHORT(outlen, outp);
    PUTCHAR(u->us_userlen, outp);
    BCOPY(u->us_user, outp, u->us_userlen);
    INCPTR(u->us_userlen, outp);
    PUTCHAR(u->us_passwdlen, outp);
    BCOPY(u->us_passwd, outp, u->us_passwdlen);

    output(u->us_unit, outpacket_buf, outlen + PPP_HDRLEN);

    TIMEOUT(upap_timeout, u, u->us_timeouttime);
    ++u->us_transmits;
    u->us_clientstate = UPAPCS_AUTHREQ;
}


/*
 * upap_sresp - Send a response (ack or nak).
 */
static void
upap_sresp(u, code, id, msg, msglen)
    upap_state *u;
    u_char code, id;
    char *msg;
    int msglen;
{
    u_char *outp;
    int outlen;

    outlen = UPAP_HEADERLEN + sizeof (u_char) + msglen;
    outp = outpacket_buf;
    MAKEHEADER(outp, PPP_PAP);

    PUTCHAR(code, outp);
    PUTCHAR(id, outp);
    PUTSHORT(outlen, outp);
    PUTCHAR(msglen, outp);
    BCOPY(msg, outp, msglen);
    output(u->us_unit, outpacket_buf, outlen + PPP_HDRLEN);
}

/*
 * upap_printpkt - print the contents of a PAP packet.
 */
static char *upap_codenames[] = {
    "AuthReq", "AuthAck", "AuthNak"
};

static int
upap_printpkt(p, plen, printer, arg)
    u_char *p;
    int plen;
    void (*printer) __P((void *, char *, ...));
    void *arg;
{
    int code, id, len;
    int mlen, ulen, wlen;
    char *user, *pwd, *msg;
    u_char *pstart;

    if (plen < UPAP_HEADERLEN)
	return 0;
    pstart = p;
    GETCHAR(code, p);
    GETCHAR(id, p);
    GETSHORT(len, p);
    if (len < UPAP_HEADERLEN || len > plen)
	return 0;

    if (code >= 1 && code <= sizeof(upap_codenames) / sizeof(char *))
	printer(arg, " %s", upap_codenames[code-1]);
    else
	printer(arg, " code=0x%x", code);
    printer(arg, " id=0x%x", id);
    len -= UPAP_HEADERLEN;
    switch (code) {
    case UPAP_AUTHREQ:
	if (len < 1)
	    break;
	ulen = p[0];
	if (len < ulen + 2)
	    break;
	wlen = p[ulen + 1];
	if (len < ulen + wlen + 2)
	    break;
	user = (char *) (p + 1);
	pwd = (char *) (p + ulen + 2);
	p += ulen + wlen + 2;
	len -= ulen + wlen + 2;
	printer(arg, " user=");
	print_string(user, ulen, printer, arg);
	printer(arg, " password=");
	if (!hide_password)
	    print_string(pwd, wlen, printer, arg);
	else
	    printer(arg, "<hidden>");
	break;
    case UPAP_AUTHACK:
    case UPAP_AUTHNAK:
	if (len < 1)
	    break;
	mlen = p[0];
	if (len < mlen + 1)
	    break;
	msg = (char *) (p + 1);
	p += mlen + 1;
	len -= mlen + 1;
	printer(arg, " ");
	print_string(msg, mlen, printer, arg);
	break;
    }

    /* print the rest of the bytes in the packet */
    for (; len > 0; --len) {
	GETCHAR(code, p);
	printer(arg, " %.2x", code);
    }

    return p - pstart;
}
