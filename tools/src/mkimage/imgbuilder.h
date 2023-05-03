/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : imgbuilder.h
 �ļ����� : �������ͷ�ļ�


 �����б� :


 �޶���¼ :
          1 ���� : ������
            ���� : 2008-2-21
            ���� :

          2 �޶�: ������
             ����: 2008-9-12
             ����:���Ӵ�ӡ����

**********************************************************************/

#ifndef IMGBUILDER_H
#define IMGBUILDER_H

#include <inttypes.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "flash_layout.h"

//#define DEBUG 1    

#ifdef	DEBUG
#define debug(fmt,args...)	printf (fmt ,##args)
#else
#define debug(fmt,args...)
#endif	

#define DEFAULT_BINFILE 		"image.bin"
#define DEFAULT_IMGFILE 		"image.img"

typedef struct
{
	int		tagArgRequited;          					/* record how many requited parameters have got */
	char		BootloaderName[256];
	char		KernelName[256];
	char		RootFileSystemName[256];
    char        ExtFileSystemName[256];
	char		OutputBinName[256];
	char		OutputImgName[256];
	char		AppDefaultCfgItemName[256];
	char		AppDefaultCfgFileName[256];
	char		transfer_mode;							/* sysconfig transfer mode. 0: use flash, 1: use sdram */
    char        iMultiLang;
	char		endian;
	int		iBlockSize;             						/* Block size of the flash,kernel code will start with this address. */
    unsigned int    iMlSize;  
    int		tagOutputBin;
	int		tagOutputImg;
}ImageInfo;

typedef struct
{
	unsigned long bin_image_checksum ;
	unsigned long bin_file_checksum ;
	unsigned long img_image_checksum ;
	unsigned long img_file_checksum ;
}crc_checksum_t;

extern crc_checksum_t	crc_checksum;

int				item_generate( void *data , char *item_name ,unsigned short data_len ,item_t *item , int *item_len , ImageInfo *stImgInfo );
int				tbs_crc_file(FILE *fp , unsigned int offset , unsigned long *crc);
void				tbs_crc_addr(unsigned char *cp, unsigned long *crc,unsigned int size);
int				tbs_set_sum( FILE*, unsigned long );
int				tbs_get_sum(FILE*, unsigned long*);
unsigned short 	tbs_crc16(unsigned char *buf, int len);


int				AnalyseParameter(int argc, char *argv[], ImageInfo *stImgInfo , sys_config_t *stSysConfig, update_hdr_t *update_hdr);
int				CreateBinFile(ImageInfo *stImgInfo,  sys_config_t *stSysConfig);
int				CreateImgFile(ImageInfo *stImgInfo,  sys_config_t *stSysConfig, update_hdr_t *update_hdr);
int				ConvertEndian(int val,int endian);
short			ConvertEndian16(short val,int endian);
void 			PrintHelp(void);
void				image_info(void);


#endif
