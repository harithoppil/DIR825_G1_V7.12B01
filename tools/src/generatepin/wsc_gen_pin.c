/*
根据RTL提供的算法生成WIRELESS WPS PIN 码。
生成PIN码基本算法:
就是先根据mac得到一个7位的十进制数，
然后再计算1位十进制校验和，
然后合起来会得到8位的wps pin码。
*/

#include <sys/time.h>	
#include<stdio.h>
#include<string.h>
#include <stdlib.h>

#if 0
static void convert_bin_to_str(unsigned char *bin, int len, char *out)
{
	int i;
	char tmpbuf[10];

	out[0] = '\0';

	for (i=0; i<len; i++) {
		sprintf(tmpbuf, "%02x", bin[i]);
		strcat(out, tmpbuf);
	}
}
#endif

static void convert_hex_to_ascii(unsigned long code, char *out)
{
	*out++ = '0' + ((code / 10000000) % 10);  
	*out++ = '0' + ((code / 1000000) % 10);
	*out++ = '0' + ((code / 100000) % 10);
	*out++ = '0' + ((code / 10000) % 10);
	*out++ = '0' + ((code / 1000) % 10);
	*out++ = '0' + ((code / 100) % 10);
	*out++ = '0' + ((code / 10) % 10);
	*out++ = '0' + ((code / 1) % 10);
	*out = '\0';
}

static int compute_pin_checksum(unsigned long int PIN)
{
	unsigned long int accum = 0;
	int digit;
	
	PIN *= 10;
	accum += 3 * ((PIN / 10000000) % 10); 	
	accum += 1 * ((PIN / 1000000) % 10);
	accum += 3 * ((PIN / 100000) % 10);
	accum += 1 * ((PIN / 10000) % 10); 
	accum += 3 * ((PIN / 1000) % 10); 
	accum += 1 * ((PIN / 100) % 10); 
	accum += 3 * ((PIN / 10) % 10);

	digit = (accum % 10);
	return (10 - digit) % 10;
}



static int genWscPin(char mac[])
{
	char wsc_pin[100];
	memset(wsc_pin,0,sizeof(wsc_pin));
    char tmp1[]={0x00, 0x1e,0xe3,0x34,0x20,0xa9};  //"00:1E:E3:34:20:A9 ";
    sscanf(mac, "%x:%x:%x:%x:%x:%x", &tmp1[0], &tmp1[1], &tmp1[2], &tmp1[3], &tmp1[4], &tmp1[5]);	
	struct timeval tod;
	unsigned long num;
		
	gettimeofday(&tod , NULL);

	//apmib_get(MIB_HW_NIC0_ADDR, (void *)&tmp1);	//这行不能执行，读取你们自己的mac address
		
	tod.tv_sec += tmp1[4]+tmp1[5];		
	srand(tod.tv_sec);
	num = rand() % 10000000;
	num = num*10 + compute_pin_checksum(num);
	convert_hex_to_ascii((unsigned long)num, wsc_pin);

	printf("GeneratedPIN %s\n", wsc_pin);
	return 0;
}

int main(int argc, char **argv)
{
    char mac[20]={0};
    if(argc<2)
    {
        printf("absent of MAC address.\n");
        return 0;
    }
    else if(argc>2)
    {
        printf("too many parameters\n");
        return 0;
    }
    strcpy(mac, argv[1]);
    genWscPin(mac);
    return 0;

}
