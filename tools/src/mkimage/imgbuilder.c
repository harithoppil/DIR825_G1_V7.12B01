/*************************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : imgbuilder.c
 文件描述 : 打包工具主程序入口


 函数列表 :
				main

 修订记录 :
          1 创建 : 张喻
            日期 : 200-7-18
            描述 :

          2 修订: 轩光磊
             日期: 2008-2-21
             描述: 为image增加CRC校验和

*************************************************************************/

#include "imgbuilder.h"

/*************************************************************************
 *                                STRUCT                                      *
*************************************************************************/



/*************************************************************************
 *                               GLOBAL VAR                                   *
*************************************************************************/
crc_checksum_t	crc_checksum;

/*************************************************************************
 *                               FUNCTION                                     *
*************************************************************************/


/*************************************************************************
 Function:      int AnalyseParameter(int argc, char *argv[], ImageInfo *stImgInfo, 
                    sys_config_t *stSysConfig,update_hdr_t *update_hdr) 

 Description:		analyse parameter,save result to stImgInfo,stImgHdr,stSysConfig 
 Calls:			无
 Data Accessed:
 Data Updated:
 Input:			argc: 输入参数个数
 				*argv[]:参数字符串指针
 Output:			
 Return:			1: 成功
				0: 失败
 Others:
 ************************************************************************/

int AnalyseParameter(int argc, char *argv[], ImageInfo *stImgInfo, sys_config_t *stSysConfig,update_hdr_t *update_hdr) 
{
	char			strCurrentDir[256];
	char			strTempPath[256];
	char			strSlash[2] = "/";
	unsigned int	iTempIP[4];
	unsigned int	iTempMAC[6];
	int			i;
	int			j;

	union 
	{  
		unsigned int iIpVal;  
		char   ch[4];  
	}iptmp = {0}; 


    for(i=1;i<argc;i++)
    {
         if(strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"--help") == 0)
         {
              PrintHelp();
              return 2;
         }
         else if(strcmp(argv[i],"-b") == 0)            /* deal with bootload name parameter */
         {
              i++;

              if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
              {
                   printf("The -b parameter is wrong.\n");
                   return 0;
              }

              stImgInfo->tagArgRequited ++;              /* a requited parameter have got */

              strcpy(stImgInfo->BootloaderName, argv[i]);
         }
         else if(strcmp(argv[i],"-k") == 0)            /* deal with kernel name parameter */
         {
              i++;

              if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
              {
                   printf("The -k parameter is wrong.\n");
                   return 0;
              }

              stImgInfo->tagArgRequited ++;              /* a requited parameter have got */

              strcpy(stImgInfo->KernelName, argv[i]);
         }
         else if(strcmp(argv[i],"-r") == 0)            /* deal with rootfs name parameter */
         {
              i++;

              if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
              {
                   printf("The -r parameter is wrong.\n");
                   return 0;
              }

              stImgInfo->tagArgRequited ++;              /* a requited parameter have got */
              strcpy(stImgInfo->RootFileSystemName, argv[i]);
         }
         else if(strcmp(argv[i],"-blk") == 0)            /* deal with block size parameter */
         {
              i++;
                   
              if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
              {
                   printf("The -blk parameter is wrong.\n");
                   return 0;
              }
         
              if(sscanf(argv[i],"%d",&(stImgInfo->iBlockSize)) != 1)
              {
                   printf("The -blk parameter is wrong.example \"-blk 64\",it means 64k.\n");
                   return 0;
              }

              if(stImgInfo->iBlockSize < 64)
              {
                   printf("The -blk parameter must >= 64.\n");
                   return 0;
              }
 
              stImgInfo->iBlockSize *= 1024;
     
         }              
         else if(strcmp(argv[i],"-obin") == 0)            /* deal with output bin file name parameter */
         {
              i++;

              if(i >= argc || *argv[i] == '-')            /* if the next parameter is wrong */
              {
                   printf("The -obin parameter is wrong.\n");
                   return 0;
              }

              stImgInfo->tagOutputBin = 1;               /* set a tag:a output bin file name parameter have got */

              strcpy(stImgInfo->OutputBinName, argv[i]);
         }
         else if(strcmp(argv[i],"-oimg") == 0)            /* deal with output img file name parameter */
         {
              i++;

              if(i >= argc || *argv[i] == '-')            /* if the next parameter is wrong */
              {
                   printf("The -oimg parameter is wrong.\n");
                   return 0;
              }

              stImgInfo->tagOutputImg = 1;               /* set a tag:a output img file name parameter have got */

              strcpy(stImgInfo->OutputImgName, argv[i]);
         }
         else if(strcmp(argv[i],"-mac") == 0)            /* deal with MAC address parameter */
         {
              i++;
    
              if(i >= argc || *argv[i] == '-')            /* if the next parameter is wrong */
              {
                   printf("The -mac parameter is wrong.\n");
                   return 0;
              }


              if(sscanf(argv[i],"%x:%x:%x:%x:%x:%x",&iTempMAC[0],&iTempMAC[1],&iTempMAC[2],&iTempMAC[3],&iTempMAC[4],&iTempMAC[5]) != 6)
              {
                   printf("The -mac parameter is wrong.\n");
                   return 0;
              }


              for(j=0;j<6;j++)
              {
                   if(iTempMAC[j] < 256)
                   {
                        stSysConfig->mac[j] = iTempMAC[j];
                   }
                   else
                   {
                        printf("The -mac parameter is wrong.\n");
                        return 0;
                   }
              }

         }

#ifdef PINANDOTHER_ENABLED
         else if(strcmp(argv[i],"-pin") == 0)            /* deal with Wireless PIN coder */
         {
              i++;
              if(i >= argc || *argv[i] == '-')            
              {
                   printf("The -pin parameter is wrong.\n");
                   return 0;
              }

              //stSysConfig->pinandother的前8位是PIN码
              for(j=0;j<8;j++)
              {
                   stSysConfig->pinandother[j] = argv[i][j];
              } 
#ifdef CONFIG_GENERATE_SSID_PASSPHRASE
              //stSysConfig->pinandother的8-9位为ssid后最后两位.  For example:NETGEARxx (xx is 2 random digit 0~9)
              for(j=8;j<10;j++)
              {
                   stSysConfig->pinandother[j] = argv[i][j];
              }
              //stSysConfig->pinandother的随机位为Passphrase占用位数.For example:  luckyflower348 
              for(j=10;;j++)
              {
              	  
              	  if(argv[i][j] == '\0')
                  	  break;
				  stSysConfig->pinandother[j] = argv[i][j];
              }
              //stSysConfig->pinandother的后56位是预留作后续其他用途;默认自字符x
              for(; j<64; j++)
              {
                   stSysConfig->pinandother[j] = 'x';
              }
#else 

			 for(j=8; j<64; j++)
			 {
			 	  stSysConfig->pinandother[j] = 'x';
			 }

#endif
         }
#endif

         else if(strcmp(argv[i],"-ip") == 0)            /* deal with IP address parameter */
         {
              i++;

              if(i >= argc || *argv[i] == '-')            /* if the next parameter is wrong */
              {
                   printf("The -ip parameter is wrong.\n");
                   return 0;
              }

              if(sscanf(argv[i],"%d.%d.%d.%d",&iTempIP[0],&iTempIP[1],&iTempIP[2],&iTempIP[3]) != 4)   
              {
                   printf("The -ip parameter is wrong.\n");
                   return 0;
              }

              for(j=0;j<4;j++)
              {
                   if(iTempIP[j] < 255)
                   {
                        iptmp.ch[j] = iTempIP[j];
                   }
                   else
                   {
                        printf("The -ip parameter is wrong.\n");
                        return 0;
                   }
              }
               
              stSysConfig->ip = iptmp.iIpVal;
         }

	  else if(strcmp(argv[i],"-p") == 0)            /* deal with product name parameter */
         {
		i++;

              if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
              {
                   printf("The -p parameter is wrong.\n");
                   return 0;
              }

		if( strlen( argv[i] ) > ( PRODUCT_NAME_LEN - 1 ) )
		{
			printf("product name is too long ,please use less than %d characters !\n",( PRODUCT_NAME_LEN - 1 ));
			return 0;
		}

		strcpy( stSysConfig->product , argv[i]  );
		strcpy( update_hdr->product , argv[i]  );
         }
	  
	  else if(strcmp(argv[i],"-v") == 0)            /* deal with versionparameter */
         {
		i++;

              if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
              {
                   printf("The -v parameter is wrong.\n");
                   return 0;
              }
		
		if( strlen( argv[i] ) > ( TBS_VERSION_NAME_LEN - 1 ) )
		{
			printf("version name is too long ,please use less than %d characters !\n", ( TBS_VERSION_NAME_LEN - 1 ) );
			return 0;
		}
		strcpy( stSysConfig->version, argv[i]  );
		strcpy( update_hdr->version, argv[i]  );
	  }

	  else if(strcmp(argv[i],"-img_type") == 0)            /* deal with image type parameter */
         {
		i++;

              if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
              {
                   printf("The -imgmark parameter is wrong.\n");
                   return 0;
              }
		
		if( strlen( argv[i] ) > ( IMAGE_TYPES_NAME_LEN - 1 ) )
		{
			printf("imgmark  is too long ,please use less than %d characters !\n" , ( IMAGE_TYPES_NAME_LEN - 1 ) );
			return 0;
		}
		strcpy( update_hdr->img_type, argv[i]  );
	  } 

	  else if(strcmp(argv[i],"-id") == 0)            /* deal with system id name parameter */
         {
		i++;

              if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
              {
                   printf("The -id parameter is wrong.\n");
                   return 0;
              }
		
		if( strlen( argv[i] ) > ( BOARD_ID_NAME_LEN - 1 ) )
		{
			printf("board_id  is too long ,please use less than %d characters !\n" , ( BOARD_ID_NAME_LEN - 1 ) );
			return 0;
		}
		strcpy( stSysConfig->board_id, argv[i]  );
		strcpy( update_hdr->board_id, argv[i]  );
	  }         


//for netgear
	  else if(strcmp(argv[i],"-region") == 0)            /* deal with system id name parameter */
         {
		i++;

              if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
              {
                   printf("The -region parameter is wrong.\n");
                   return 0;
              }
		
		if( strlen( argv[i] ) > ( REGION_LEN - 1 ) )
		{
			printf("region  is too long ,please use less than %d characters !\n" , ( REGION_LEN - 1 ) );
			return 0;
		}
		strcpy( stSysConfig->region, argv[i]  );
		strcpy( update_hdr->region, argv[i]  );
	  }    


      	  else if(strcmp(argv[i],"-sw") == 0)            /* deal with system id name parameter */
         {
		i++;

              if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
              {
                   printf("The -swversion parameter is wrong.\n");
                   return 0;
              }
		
		if( strlen( argv[i] ) > ( VERSION_LEN - 1 ) )
		{
			printf("swversion  is too long ,please use less than %d characters !\n" , ( VERSION_LEN - 1 ) );
			return 0;
		}
		strcpy( stSysConfig->swversion, argv[i]  );
		strcpy( update_hdr->swversion, argv[i]  );
	  }    

          else if(strcmp(argv[i],"-model") == 0)            /* deal with system id name parameter */
         {
		i++;

              if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
              {
                   printf("The -model parameter is wrong.\n");
                   return 0;
              }
		
		if( strlen( argv[i] ) > ( MODEL_LEN - 1 ) )
		{
			printf("board_id  is too long ,please use less than %d characters !\n" , ( MODEL_LEN - 1 ) );
			return 0;
		}
		strcpy( stSysConfig->model_name, argv[i]  );
		strcpy( update_hdr->model_name, argv[i]  );
	  }    

//end for netgear
              
	  else if(strcmp(argv[i],"-t") == 0)            /* deal with cofig repeat mode parameter */
         {
		i++;

              if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
              {
                   printf("The -id parameter is wrong.\n");
                   return 0;
              }
		
		if( (strcmp(argv[i],"flash") == 0) )
		{
			stImgInfo->transfer_mode = 0;
		}
		else if( (strcmp(argv[i],"sdram") == 0) )
		{
			stImgInfo->transfer_mode = 1;
		}
	  }         

         else if(strcmp(argv[i],"-be") == 0)            /* deal with big-endian parameter */
         {                         
              stSysConfig->endian = 1;
		stImgInfo->endian =1;
         }
         else if(strcmp(argv[i],"-le") == 0)            /* deal with little-endian parameter */
         {                             
              stSysConfig->endian = 0;
		stImgInfo->endian =0;
         }

	  else if(strcmp(argv[i],"-app_cfg") == 0)            /* deal with app default config parameter */
         {
		i++;

              if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
              {
                   printf("The -app_cfg parameter is wrong.\n");
                   return 0;
              }

		strcpy( stImgInfo->AppDefaultCfgItemName, argv[i]  );

		i++;
		
		if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
              {
                   printf("The -app_cfg parameter is wrong.\n");
                   return 0;
              }
		
		strcpy( stImgInfo->AppDefaultCfgFileName , argv[i]  );

	  }


      		 else if(strcmp(argv[i],"-f") == 0)            /* deal with external filesystem img name parameter */
         {
              i++;

              if(i >= argc || *argv[i] == '-')         /* if the next parameter is wrong */
              {
                   printf("The -r parameter is wrong.\n");
                   return 0;
              }

              stImgInfo->tagArgRequited ++;              /* a requited parameter have got */
              strcpy(stImgInfo->ExtFileSystemName, argv[i]);
         }

		 else if(strcmp(argv[i],"-multi_lang") == 0)
		 {
		 	  stImgInfo->iMultiLang = 1;
		 }

		 else if(strcmp(argv[i],"-fss") == 0)            /* deal with external jffs2 file system img size parameter */
         {
              i++;

              if(i >= argc || *argv[i] == '-')            /* if the next parameter is wrong */
              {
                   printf("The -oimg parameter is wrong.\n");
                   return 0;
              }

              if(sscanf(argv[i],"%d",&(stImgInfo->iMlSize)) != 1)
              {
                   printf("The -blk parameter is wrong.example \"-blk 64\",it means 64k.\n");
                   return 0;
              }

              stImgInfo->iMlSize *= 1024;

         }

    }  /* end of for(i=1;i<argc;i++) */

    if(stImgInfo->tagOutputBin == 0)
    {
         strcpy(stImgInfo->OutputBinName, DEFAULT_BINFILE);
    }

    if(stImgInfo->tagOutputImg == 0)
    {
         strcpy(stImgInfo->OutputImgName, DEFAULT_IMGFILE);
    }

    /* Generate absolute path */

    getcwd(strCurrentDir,256);           /* Get current dir name */
 
    if(*(stImgInfo->BootloaderName) != '/')
    {
         strcpy(strTempPath,strCurrentDir);  /* Check bootloader file */
         strcat(strTempPath,strSlash);
    }
    else
    {
         strTempPath[0] = '\0';
    }

    strcat(strTempPath,stImgInfo->BootloaderName);
    strcpy(stImgInfo->BootloaderName,strTempPath);


    if(access(stImgInfo->BootloaderName,F_OK))
    {
         printf("The bootloader file is not exist.\n");
         return 0;
    }

    if(*(stImgInfo->KernelName) != '/')
    {
         strcpy(strTempPath,strCurrentDir);  /* Check kernel file */
         strcat(strTempPath,strSlash);
    }
    else
    {
         strTempPath[0] = '\0';
    }

    strcat(strTempPath,stImgInfo->KernelName);
    strcpy(stImgInfo->KernelName,strTempPath);


    if(access(stImgInfo->KernelName,F_OK))
    {
         printf("The KernelName file is not exist.\n");
         return 0;
    }

    if(*(stImgInfo->RootFileSystemName) != '/')
    {
         strcpy(strTempPath,strCurrentDir);  /* Check rootfs file */
         strcat(strTempPath,strSlash);
    }
    else
    {
         strTempPath[0] = '\0';
    }

    strcat(strTempPath,stImgInfo->RootFileSystemName);
    strcpy(stImgInfo->RootFileSystemName,strTempPath);


    if(access(stImgInfo->RootFileSystemName,F_OK))
    {
         printf("The rootfs file is not exist.\n");
         return 0;
    }

    if(*(stImgInfo->AppDefaultCfgFileName) != '/')
    {
         strcpy(strTempPath,strCurrentDir);  /* Check app default config file */
         strcat(strTempPath,strSlash);
    }
    else
    {
         strTempPath[0] = '\0';
    }

    strcat(strTempPath,stImgInfo->AppDefaultCfgFileName);
    strcpy(stImgInfo->AppDefaultCfgFileName,strTempPath);

    if(access(stImgInfo->AppDefaultCfgFileName,F_OK))
    {
         printf("The app default config file is not exist.\n");
         return 0;
    }
    
    if(*(stImgInfo->OutputBinName) != '/')
    {
         strcpy(strTempPath,strCurrentDir);  /*  BIN file */
         strcat(strTempPath,strSlash);
    }
    else
    {
         strTempPath[0] = '\0';
    }

    strcat(strTempPath,stImgInfo->OutputBinName);
    strcpy(stImgInfo->OutputBinName,strTempPath);


    if(*(stImgInfo->OutputImgName) != '/')
    {              
         strcpy(strTempPath,strCurrentDir);  /*  IMG file */
         strcat(strTempPath,strSlash);
    }
    else
    {
         strTempPath[0] = '\0';
    }

    strcat(strTempPath,stImgInfo->OutputImgName);
    strcpy(stImgInfo->OutputImgName,strTempPath);


    /* Check MAC,IP and endian configuration */

    if(stSysConfig->mac[0] == 0xff)
    {
         printf("Please input MAC address configuration.\n");
         return 0;
         
    }

    if(stSysConfig->ip == 0)
    {
         printf("Please input IP address configuration.\n");
         return 0;
         
    }

#ifdef PINANDOTHER_ENABLED
    if(stSysConfig->pinandother== 0)
    {
         printf("Please input pin code configuration.\n");
         return 0;
         
    }
#endif

    if(stSysConfig->endian == 0xf)
    {
         printf("Please input endian configuration.\n");
         return 0;
         
    }

    if( ( update_hdr->product[0] == 0x0 ) || ( stSysConfig->product[0] == 0x0 ) )
    {
         printf("Please input product name.\n");
         return 0;
    }
	
    if( ( update_hdr->version[0] == 0x0 ) || ( stSysConfig->version[0] == 0x0 ) )
    {
         printf("Please input version.\n");
         return 0;
    }  
	
    if( update_hdr->img_type[0] == 0x0 )
    {
         printf("Please input image type.\n");
         return 0;
    }

    if( stImgInfo->AppDefaultCfgItemName[0] == 0x0 )
    {
         printf("Please input app default config item name.\n");
         return 0;
    }
    /* if it is big-endian,convert endian */

   // stSysConfig->ip = ConvertEndian(stSysConfig->ip,stSysConfig->endian);

    return 1;     /* succeed */
}

/*=========================================================================
 Function:		int CreateBinFile(ImageInfo *stImgInfo, sys_config_t *stSysConfig)
 Description:		生成用于出厂烧录到flash的image
 Calls:			无
 Data Accessed:
 Data Updated:
 Input:			*stImgInfo
 				*stSysConfig
 				
 Output:			生成用于出厂烧录到flash的image
 Return:			1: 成功
				0: 失败
 Others:			打出的image默认名称为image.bin.可以在打包时加-obin file_name修改
 				烧录image包含bootloader kernel和rootfs数据
=========================================================================*/

int CreateBinFile(ImageInfo *stImgInfo, sys_config_t *stSysConfig)
{
	char				buf[4096];
	FILE				*pfin;
	FILE				*pfout;
	FILE				*pAppconfigfile;
	int				ret;
	char				*buff;

	int				iBinFileLength = 0;      /* count how many bytes have been write to BIN file */
	int				iReadCount;
	int				iWriteCount;
	int				tmp;
	int				i;
	unsigned long		checksum_result;
	unsigned long		kernel_offset;
	item_t			*item;
	int				item_len;
	int				item_len_sum = 0;
	image_bin_tail_t 	image_bin_tail;
	
	debug("Open output BIN file: %s\n",stImgInfo->OutputBinName);
	debug("Open bootloader file: %s\n",stImgInfo->BootloaderName);


/******************************************************************
				initialize image.bin tail     
******************************************************************/
	strcpy( image_bin_tail.product , stSysConfig->product  );
	strcpy( image_bin_tail.bin_type, IMAGE_BIN );

/******************************************************************
				Copy bootloader file              
******************************************************************/

	(stSysConfig->layout).zone_offset[ZONE_BOOTLOADER] = 0;

	pfin = fopen(stImgInfo->BootloaderName,"r");                          
	if(pfin == NULL)
	{
		printf("Can't open bootloader file: %s\n",stImgInfo->BootloaderName);
		return 0;
	}

	pfout = fopen(stImgInfo->OutputBinName,"wb+");
	if(pfout == NULL)
	{
		printf("Can't open output file: %s\n",stImgInfo->OutputBinName);
		fclose(pfin);

		return 0;
	} 
    
	while(1)
	{
		iReadCount = fread(buf,1,4096,pfin); 
		iWriteCount = fwrite(buf,1,iReadCount,pfout);
		if(iReadCount != iWriteCount && iReadCount > 0)  /* fail to write ? */
		{
			printf("Fail to write code to BIN file.\n");
			fclose(pfin);
			fclose(pfout);
			remove(stImgInfo->OutputBinName);
			return 0;
		}

		iBinFileLength += iWriteCount ;

		if(iReadCount < 4096)
		{
			if(!feof(pfin))        /* fail to read ? */
			{
				printf("Fail to read bootloader file.\n");
				fclose(pfin);
				fclose(pfout);
				remove(stImgInfo->OutputBinName);
				return 0;
			}

			fclose(pfin);
			break;
		}
	}

	if( iBinFileLength > BOOTCODE_LENGTH)     /* Fix bootloader length to 64K */
	{
		printf("The bootloader file is too big( bigger then 64K ).\n");
		fclose(pfout);
		remove(stImgInfo->OutputBinName);
		return 0;
	} 
	else
	{
		for(; iBinFileLength < stImgInfo->iBlockSize; iBinFileLength++)
            fputc(0xffff,pfout);
	}


/******************************************************************
				Fix config space length to flash sector size             
******************************************************************/

	for( i = 0; i < stImgInfo->iBlockSize; i++ , iBinFileLength++ )
	{
		fputc(0xffff,pfout);
	}

/******************************************************************
				Fix transfer space length to flash sector size              
******************************************************************/

	if( stImgInfo->transfer_mode == 0 )
	{
		for( i = 0; i < stImgInfo->iBlockSize; i++ , iBinFileLength++ )
		{
			fputc(0xffff,pfout);
		}
	}

/******************************************************************
				Copy kernel file              
******************************************************************/

	debug("Open kernel file: %s,start address=0x%x\n",stImgInfo->KernelName,iBinFileLength);

	kernel_offset = iBinFileLength;
	
	(stSysConfig->layout).zone_offset[ZONE_KERNEL_FIRST] = iBinFileLength;
 
	pfin = fopen(stImgInfo->KernelName,"r");  
	if(pfin == NULL)
	{
		printf("Can't open kernel file: %s\n",stImgInfo->KernelName);
		fclose(pfout);
		remove(stImgInfo->OutputBinName);

		return 0;
	}

	while(1)
	{
		iReadCount = fread(buf,1,4096,pfin);
		iWriteCount = fwrite(buf,1,iReadCount,pfout);
		if(iReadCount != iWriteCount && iReadCount > 0)  /* fail to write ? */
		{
			printf("Fail to write kernel to BIN file.\n");
			fclose(pfin);
			fclose(pfout);
			remove(stImgInfo->OutputBinName);
			return 0;
		}

		iBinFileLength += iWriteCount ;

		if(iReadCount < 4096)
		{
			if(!feof(pfin))        /* fail to read ? */
			{
				printf("Fail to read kernel file.\n");
				fclose(pfin);
				fclose(pfout);
				remove(stImgInfo->OutputBinName);
				return 0;
			}

			fclose(pfin);
			break;
		}
	}

    
/******************************************************************
				Align rootfs image start address to 0x100             
******************************************************************/

	tmp = iBinFileLength % 0x100;
	tmp = 0x100 - tmp;

	if(tmp)
	{
		for(i=0; i<tmp; i++)
		{
		fputc(0xffff,pfout);
		iBinFileLength++;
		}
	}

/******************************************************************
				Copy rootfs file              
******************************************************************/

	debug("Open rootfs file: %s,start address=0x%x\n",stImgInfo->RootFileSystemName,iBinFileLength);

	(stSysConfig->layout).zone_offset[ZONE_ROOTFS_FIRST] = iBinFileLength;

	pfin = fopen(stImgInfo->RootFileSystemName,"r");
	if(pfin == NULL)
	{
		printf("Can't open rootfs file: %s\n",stImgInfo->RootFileSystemName);
		fclose(pfout);
		remove(stImgInfo->OutputBinName);

		return 0;
	}


	while(1)
	{
		iReadCount = fread(buf,1,4096,pfin);
		iWriteCount = fwrite(buf,1,iReadCount,pfout);
		if(iReadCount != iWriteCount && iReadCount > 0)  /* fail to write ? */
		{
			printf("Fail to write rootfs to BIN file.\n");
			fclose(pfin);
			fclose(pfout);
			remove(stImgInfo->OutputBinName);
			return 0;
		}

		iBinFileLength += iWriteCount ;

		if(iReadCount < 4096)
		{
			if(!feof(pfin))        /* fail to read ? */
			{
				printf("Fail to read rootfs file.\n");
				fclose(pfin);
				fclose(pfout);
				remove(stImgInfo->OutputBinName);
				return 0;
			}

			fclose(pfin);
			break;
		}
	}

#if 0
/*fix bug:升级兼容性问题，防止大的img升级后启动失败

# cat /proc/mtd 
dev:    size   erasesize  name
mtd0: 00030000 00010000 "boot"
mtd1: 000e6900 00010000 "kernel"
mtd2: 002b9700 00010000 "rootfs"  
mtd3: 00020000 00010000 "multi_lang"

kernel+rootfs=0x3a0000 -->3712k
boot+kernel+rootfs=0x3d0000

flash擦除是以0x10000(64k)为单位的，因此img文件不能再大了。
这样，新的rootfs和kernel分区大小总是固定的，img文件最大可以是3712k

boot+kernel+rootfs+multilang=0x3f0000
0x400000-0x3f0000=0x10000 这剩下的64k用来Deal with system config    ，我们的当前配置就保存在这个位置。

*/
#if 0
/******************************************************************
                Align multi_lang.img image start address to 0x10000             
******************************************************************/

    tmp = iBinFileLength % 0x10000;
    tmp = 0x10000 - tmp;
#else
    tmp=0x3d0000-iBinFileLength;    
#endif

    if(tmp)
    {
        for(i=0; i<tmp; i++)
        {
        fputc(0xffff,pfout);
        iBinFileLength++;
        }
    }

(stSysConfig->layout).zone_offset[ZONE_MULTI_LANG] = iBinFileLength;
/*reserve caldata*/
//iBinFileLength+=512*1024;


//for(tmp=0; tmp < 512*1024; tmp++)
        //fputc(0xffff,pfout);

//iBinFileLength+=512*1024;


#if 1
/******************************************************************
                Copy multi_lang.img file
******************************************************************/
    tmp=0;
    if(stImgInfo->iMultiLang == 1){
        pfin = fopen(stImgInfo->ExtFileSystemName,"r");
        if(pfin == NULL)
        {
            printf("Can't open jffs2 file: %s\n",stImgInfo->ExtFileSystemName);
            fclose(pfout);
            remove(stImgInfo->OutputBinName);
            return 0;
        }

        debug("Open jffs2 file: %s,start address=0x%x\n",stImgInfo->ExtFileSystemName,iBinFileLength);

        while(1)
        {
            iReadCount = fread(buf,1,4096,pfin);
            iWriteCount = fwrite(buf,1,iReadCount,pfout);
            if(iReadCount != iWriteCount && iReadCount > 0)  /* fail to write ? */
            {
                printf("Fail to write rootfs to BIN file.\n");
                fclose(pfin);
                fclose(pfout);
                remove(stImgInfo->OutputBinName);
                return 0;
            }

           iBinFileLength += iWriteCount ;
           tmp+= iWriteCount;

            if(iReadCount < 4096)
            {
                if(!feof(pfin))        /* fail to read ? */
                {
                    printf("Fail to read rootfs file.\n");
                    fclose(pfin);
                    fclose(pfout);
                    remove(stImgInfo->OutputBinName);
                    return 0;
                }

                fclose(pfin);
                break;
            }
        }
    }
#else

iBinFileLength+=320*1024;


#endif


//(stSysConfig->layout).zone_offset[ZONE_CAL_DATA] = stImgInfo->iFlashSize - stImgInfo->iCalDataSize;
#endif

/******************************************************************
				Deal with sys_config_t             
******************************************************************/

	

	stSysConfig->first_image_len = ConvertEndian( iBinFileLength - kernel_offset , stSysConfig->endian );

	debug("\tkernel offset = 0x%x \n",(stSysConfig->layout).zone_offset[ZONE_KERNEL_FIRST]);
	debug("\trootfs offset = 0x%x \n",(stSysConfig->layout).zone_offset[ZONE_ROOTFS_FIRST]);  
    //debug("\tcaldata offset = 0x%x \n",(stSysConfig->layout).zone_offset[ZONE_MULTI_LANG]); 
    

    
	(stSysConfig->layout).zone_offset[ZONE_MULTI_LANG] = 0x800000 - MULTILANG_LENGTH /*- DEFLANG_LENGTH*/; //flash 8M 大小
	(stSysConfig->layout).zone_offset[ZONE_KERNEL_FIRST] = ConvertEndian( (stSysConfig->layout).zone_offset[ZONE_KERNEL_FIRST] , stSysConfig->endian);
	(stSysConfig->layout).zone_offset[ZONE_ROOTFS_FIRST] = ConvertEndian( (stSysConfig->layout).zone_offset[ZONE_ROOTFS_FIRST] , stSysConfig->endian);
    (stSysConfig->layout).zone_offset[ZONE_MULTI_LANG] = ConvertEndian( (stSysConfig->layout).zone_offset[ZONE_MULTI_LANG] , stSysConfig->endian);


	if( tbs_crc_file(pfout , kernel_offset , &checksum_result ) == 0 )
	{
		printf("Fail to calculate crc for image.bin image.\n");
		fclose(pfout);
		remove(stImgInfo->OutputBinName);

		return 0;
	}

	crc_checksum.bin_image_checksum = checksum_result;
	stSysConfig->first_image_checksum= ConvertEndian( checksum_result , stSysConfig->endian);


/******************************************************************
				Deal with system config          
******************************************************************/

	if( fseek( pfout , stImgInfo->iBlockSize , SEEK_SET ) == -1 )
	{
	      printf("Fail to point config addr.\n");
	      fclose(pfout);
	      remove(stImgInfo->OutputBinName);
	      return 0;                         /* fail to lseek */
	}

	iWriteCount = fwrite( CONFIG_MARK , CONFIG_MARK_LEN , 1 , pfout );
	if(iWriteCount != 1)  /* fail to write ? */
	{
	     printf("Fail to write config mark to BIN file.\n");
	     free( item );
	     fclose(pfout);
	     remove(stImgInfo->OutputBinName);
		 
	     return 0;
	}

	item_len_sum += CONFIG_MARK_LEN;

	item = malloc( stImgInfo->iBlockSize );
	if( item == NULL )
	{
		printf("Can't malloc space for item.\n");
		fclose(pfout);
		remove(stImgInfo->OutputBinName);
		
		return 0;
	}

	/* Deal with llconfig item */
	
	ret = item_generate( stSysConfig , LLCONFIG_NAME ,SYSCFG_SIZE , item , &item_len , stImgInfo );
	if( ret == 0 )
	{
		printf("Generate %s item fail!\n",LLCONFIG_NAME);
		free( item );
		fclose(pfout);
		remove(stImgInfo->OutputBinName);

		return 0;
	}

	item_len_sum += item_len;
	if( item_len_sum > stImgInfo->iBlockSize )
	{
		printf("System config space is full!\n");
		free( item );
		fclose(pfout);
		remove(stImgInfo->OutputBinName);

		return 0;
	}

	iWriteCount = fwrite( item , item_len , 1 , pfout );
	if(iWriteCount != 1)  /* fail to write ? */
	{
	     printf("Fail to write %s item to BIN file.\n",LLCONFIG_NAME);
	     free( item );
	     fclose(pfout);
	     remove(stImgInfo->OutputBinName);
		 
	     return 0;
	}

	/* Deal with app default config item */

	pAppconfigfile= fopen(stImgInfo->AppDefaultCfgFileName,"r");                          
	if(pAppconfigfile == NULL)
	{
		printf("Can't open app default config file: %s\n",stImgInfo->AppDefaultCfgFileName);
		return 0;
	}

	fseek(pAppconfigfile, 0 , SEEK_SET);
	fseek(pAppconfigfile, 0 , SEEK_END);
	tmp = ftell(pAppconfigfile);

	if( tmp > ( stImgInfo->iBlockSize - item_len_sum ) )
	{
	     printf("App default config is too long !\n");
	     free( item );
            fclose(pAppconfigfile);
	     fclose(pfout);
	     remove(stImgInfo->OutputBinName);
		 
	     return 0;
	}

	buff = malloc( stImgInfo->iBlockSize );

	fseek(pAppconfigfile, 0 , SEEK_SET);
	fread(buff,1,tmp,pAppconfigfile);

	ret = item_generate( buff , TBS_APP_CFG ,tmp , item , &item_len , stImgInfo );
	if( ret == 0 )
	{
		printf("Generate %s item fail!\n",TBS_APP_CFG);
		free( item );
		free(buff);
		fclose(pAppconfigfile);
		fclose(pfout);
		remove(stImgInfo->OutputBinName);

		return 0;
	}

	item_len_sum += item_len;
	if( item_len_sum > stImgInfo->iBlockSize )
	{
		printf("System config space is full!\n");
		free( item );
		free( buff );
		fclose(pAppconfigfile);
		fclose(pfout);
		remove(stImgInfo->OutputBinName);

		return 0;
	}

	iWriteCount = fwrite( item , item_len , 1 , pfout );
	if(iWriteCount != 1)  /* fail to write ? */
	{
		printf("Fail to write %s item to BIN file.\n",TBS_APP_CFG);
		free( item );
		free( buff );
		fclose(pAppconfigfile);
		fclose(pfout);
		remove(stImgInfo->OutputBinName);

		return 0;
	}
	
	fclose(pAppconfigfile);
	free( buff );
	free( item );


/******************************************************************
				Add BIN file tail             
******************************************************************/

	if( fseek( pfout , 0 , SEEK_END ) == -1 )
	{
	      printf("Fail point to BIN file end.\n");
	      fclose(pfout);
	      remove(stImgInfo->OutputBinName);
	      return 0;                        
	}
	
	iWriteCount = fwrite( &image_bin_tail, sizeof( image_bin_tail_t ) , 1 , pfout );
	if(iWriteCount != 1)  
	{
	     printf("Fail to write image_bin_tail to BIN file.\n");
	     fclose(pfout);
	     remove(stImgInfo->OutputBinName);
		 
	     return 0;
	}
	iBinFileLength += sizeof( image_bin_tail_t ) ;

	if( tbs_crc_file( pfout , 0 , &checksum_result ) == 0 )
	{
		printf("Fail to calculate crc for BIN file.\n");
		fclose(pfout);
		remove(stImgInfo->OutputBinName);

		return 0;
	}

	crc_checksum.bin_file_checksum = checksum_result;
	checksum_result = ConvertEndian( checksum_result , stSysConfig->endian);

	if( tbs_set_sum( pfout , checksum_result ) == 0 )
	{
		printf("Fail write crc to BIN file.\n");
		fclose(pfout);
		remove(stImgInfo->OutputBinName);
		return 0;                
	}

	fclose(pfout);
	return 1;
}

/*=========================================================================
 Function:      int CreateImgFile(ImageInfo *stImgInfo, sys_config_t *stSysConfig ,update_hdr_t *update_hdr) 

 Description:		生成用于升级的image 
 Calls:			无
 Data Accessed:
 Data Updated:
 Input:			*stImgInfo
 				*stSysConfig
 				*update_hdr
 Output:			生成用于升级image
 Return:			1: 成功
				0: 失败
 Others:			打出的image默认名称为image.img.可以在打包时加-oimg file_name修改
 				升级image只包含kernel和rootfs数据
=========================================================================*/

int CreateImgFile(ImageInfo *stImgInfo, sys_config_t *stSysConfig ,update_hdr_t *update_hdr)
{
	char			buf[4096];
	update_hdr_t	image_header;

	FILE 		*pfin;
	FILE 		*pfout;

	int 			iImgFileLength = 0;      /* count how many bytes have been write to IMG file */
	int 			iReadCount;
	int			iWriteCount;
	int			tmp;
	int			i;
	unsigned long checksum_result;


/******************************************************************
				初始化升级文件头部数据             
******************************************************************/
    
	memset(&image_header,0,sizeof(update_hdr_t));

	strcpy( image_header.product , update_hdr->product);
	strcpy( image_header.version,  update_hdr->version);
	strcpy( image_header.img_type,  update_hdr->img_type);
	strcpy( image_header.board_id,  update_hdr->board_id );

        //for netgear
        strcpy( image_header.region,  update_hdr->region);
        strcpy( image_header.model_name,  update_hdr->model_name);
        strcpy( image_header.swversion,  update_hdr->swversion);

	pfout = fopen(stImgInfo->OutputImgName,"wb+");
	if(pfout == NULL)
	{
		printf("Can't open output file: %s\n",stImgInfo->OutputImgName);
		return 0;
	} 

/******************************************************************
					留出头部空间             
******************************************************************/

	for(i=0; i<sizeof(update_hdr_t); i++)
	{
		fputc(0xffff,pfout);
		iImgFileLength++;
	}
  
	
/******************************************************************
				Copy kernel file              
******************************************************************/

	image_header.kernel_offset = iImgFileLength;

	debug("Open kernel file: %s\n",stImgInfo->KernelName);

	pfin = fopen(stImgInfo->KernelName,"r");  
	if(pfin == NULL)
	{
		printf("Can't open kernel file: %s\n",stImgInfo->KernelName);
		fclose(pfout);
		remove(stImgInfo->OutputImgName);
		return 0;
	}

	while(1)
	{
		iReadCount = fread(buf,1,4096,pfin);
		iWriteCount = fwrite(buf,1,iReadCount,pfout);
		if(iReadCount != iWriteCount && iReadCount > 0)  /* fail to write ? */
		{
			printf("Fail to write rootfs to IMG file .\n");
			fclose(pfin);
			fclose(pfout);
			remove(stImgInfo->OutputImgName);
			return 0;
		}

		iImgFileLength += iWriteCount ;

		if(iReadCount < 4096)
		{
			if(!feof(pfin))        /* fail to read ? */
			{
				printf("Fail to read rootfs file.\n");
				fclose(pfin);
				fclose(pfout);
				remove(stImgInfo->OutputImgName);
				return 0;
			}

			fclose(pfin);
			break;
		}
	}

/******************************************************************
				Copy rootfs  file              
******************************************************************/

    /* Align rootfs image start address to 0x100 */

	tmp = iImgFileLength % 0x100;
	tmp = 0x100 - tmp;

	if(tmp)
	{
		for(i=0; i<tmp; i++)
		{
			fputc(0xffff,pfout);
			iImgFileLength++;
		}
	}

	image_header.rootfs_offset = iImgFileLength;
	image_header.kernel_size= image_header.rootfs_offset - image_header.kernel_offset;

	debug("Open rootfs file: %s\n",stImgInfo->RootFileSystemName);

	pfin = fopen(stImgInfo->RootFileSystemName,"r");
	if(pfin == NULL)
	{
		printf("Can't open rootfs file: %s\n",stImgInfo->RootFileSystemName);
		fclose(pfout);
		remove(stImgInfo->OutputImgName);
		return 0;
	}

	while(1)
	{
		iReadCount = fread(buf,1,4096,pfin);
		iWriteCount = fwrite(buf,1,iReadCount,pfout);
		if(iReadCount != iWriteCount && iReadCount > 0)  /* fail to write ? */
		{
			printf("Fail to write kernel to IMG file.\n");
			fclose(pfin);
			fclose(pfout);
			remove(stImgInfo->OutputImgName);
			return 0;
		}

		iImgFileLength += iWriteCount ;

		if(iReadCount < 4096)
		{
			if(!feof(pfin))        /* fail to read ? */
			{
				printf("Fail to read kernel file.\n");
				fclose(pfin);
				fclose(pfout);
				remove(stImgInfo->OutputImgName);
				return 0;
			}

			fclose(pfin);
			break;
		}
	}

	image_header.rootfs_size= iImgFileLength - image_header.rootfs_offset;
	image_header.image_len = iImgFileLength - sizeof( update_hdr_t );


/******************************************************************
				Deal with image header            
******************************************************************/

	debug("image kernel_offset=0x%x\n image rootfs_offset=0x%x\nimage_len=0x%x\n",image_header.kernel_offset,image_header.rootfs_offset,image_header.image_len);

	image_header.rootfs_offset = ConvertEndian(image_header.rootfs_offset, stSysConfig->endian);
	image_header.rootfs_size = ConvertEndian(image_header.rootfs_size, stSysConfig->endian);
	image_header.kernel_offset = ConvertEndian(image_header.kernel_offset, stSysConfig->endian);
	image_header.kernel_size = ConvertEndian(image_header.kernel_size, stSysConfig->endian);
	image_header.image_len = ConvertEndian(image_header.image_len, stSysConfig->endian);

 	if( tbs_crc_file(pfout , sizeof(update_hdr_t) , &checksum_result ) )
	{
		crc_checksum.img_image_checksum = checksum_result;
		image_header.image_checksum = ConvertEndian( checksum_result , stSysConfig->endian);

		if(fseek(pfout,0,SEEK_SET) == -1)
		{
		      printf("Fail to point image header.\n");
		      fclose(pfout);
		      remove(stImgInfo->OutputImgName);
		      return 0;                         /* fail to lseek */
		}

		iWriteCount = fwrite(&image_header,sizeof(update_hdr_t),1,pfout);
		if(iWriteCount != 1)  /* fail to write ? */
		{
		     printf("Fail to write checksum to IMG file.\n");
		     fclose(pfout);
		     remove(stImgInfo->OutputImgName);
		     return 0;
		}
	}


/******************************************************************
				set image.img file crc           
******************************************************************/
	
	tbs_crc_file( pfout , 0 , &checksum_result );
	crc_checksum.img_file_checksum = checksum_result;
	checksum_result = ConvertEndian( checksum_result, stSysConfig->endian );

	if(fseek(pfout,0,SEEK_END) == -1)
	{
		printf("Fail to point IMG file tail.\n");
		fclose(pfout);
		remove(stImgInfo->OutputImgName);
		return 0;                         /* fail to lseek */
	}

	if(fwrite( &checksum_result , 1, 4 , pfout )  <  4 )
	{
		printf("Fail to write file_checksum to IMG file.\n");
		fclose(pfout);
		remove(stImgInfo->OutputImgName);
		return 0;
	}
	
	fclose(pfout);
	
	return 1;
}

/*=========================================================================
 Function:		int ConvertEndian(int val,int endian)

 Description:		转换长度为4字节的字节序
 Calls:			无
 Data Accessed:
 Data Updated:
 Input:			val:		要转换的数据
 				endian:	大小端标识
 				
 Output:			i:		转换的结果
 Return:			i
			
 Others:			
=========================================================================*/

int ConvertEndian(int val,int endian)
{
	int i;
	unsigned char *p;
	unsigned char *q;

	i = val;
	p = ( unsigned char * ) &val;
	q = ( unsigned char * ) &i;

	if(endian == 0x1)
	{
		*q = *(p+3);
		*(q+1) = *(p+2);
		*(q+2) = *(p+1);
		*(q+3) = *p;
	}

	return i;
}

/*=========================================================================
 Function:		short ConvertEndian16(short val,int endian)

 Description:		转换长度为2字节的字节序
 Calls:			无
 Data Accessed:
 Data Updated:
 Input:			val:		要转换的数据
 				endian:	大小端标识
 				
 Output:			i:		转换的结果
 Return:			i
			
 Others:			
=========================================================================*/

short ConvertEndian16(short val,int endian)
{
	short i;
	unsigned char *p;
	unsigned char *q;

	i = val;
	p = ( unsigned char * ) &val;
	q = ( unsigned char * ) &i;

	if(endian == 0x1)
	{
		*q = *(p+1);
		*(q+1) = *p;
	}    
	return i;
}

int item_generate( void *data , char *item_name ,unsigned short data_len ,item_t *item , int *item_len , ImageInfo *stImgInfo )
{
	if( ( data == NULL ) || ( item_name == NULL ) || ( item == NULL ) || ( item_len ==NULL ) || ( stImgInfo == NULL ) )
	{
		return 0;
	}

	if( data_len >= stImgInfo->iBlockSize )
	{
		return 0;
	}

	item->hdr.avail = ITEM_AVAIL;			/* 值条目有效位为11*/
	memcpy( item->data , item_name , strlen( item_name ) );
	memcpy( item->data + strlen( item_name ) , "\0" , 1 );	
	memcpy( item->data + strlen( item_name ) + 1 , data , data_len );	
	item->hdr.len = strlen( item_name ) + 1 + data_len;
	*item_len = ITEM_SIZE( item->hdr.len );

	crc16( item->data , item->hdr.len , &( item->hdr.crc ) );
	item->hdr.len = ConvertEndian16( item->hdr.len , stImgInfo->endian );
	item->hdr.crc = ConvertEndian16( item->hdr.crc , stImgInfo->endian );

	return 1;
}


/*=========================================================================
 Function:		void PrintHelp(void)

 Description:		输出打包工具使用方法
 Calls:			无
 Data Accessed:
 Data Updated:
 Input:			无
  				
 Output:			无
 Return:			无
			
 Others:			
=========================================================================*/

void PrintHelp(void)
{
	printf("This utility generates a BIN file and a IMG file \
	        \nThe generated BIN file is used to write in flash. \
	        \nThe generated IMG file is used to upgrade the system. \n\n"); 
	printf("Usage:  mkimage [-h,--help] \n"); 
	printf("\t\t[-b filename] [-k filename] [-r filename] [-app_cfg item_name filename] [-obin filename] [-oimg filename] \n");
	printf("\t\t[-p product] [-v version] [-id board_id] [-img_type image_type]\n");
	printf("\t\t[-eb,-el] [-mac address] [-ip address] [-blk flash_erasesize] [-t repeat mode]\n\n");
	printf("   -h,--help\tPrint this infomation\n");
	printf("   -b\t\tThe name of bootloader file (required)\n"); 
	printf("   -k\t\tThe name of Linux kernel file (required)\n");
	printf("   -r\t\tThe name of root file system file (required)\n");
	printf("   -app_cfg\tThe name of app default config item and file (required)\n");
	
	printf("   -obin\tThe name of output BIN file \n");
	printf("   -oimg\tThe name of output IMG file \n");
	printf("   -p\t\tThe name of product (required)\n");
	printf("   -v\t\tThe version of product (required)\n");
	printf("   -id\t\tThe board_id of product \n");
	printf("   -img_type\tThe image type of product (required)\n");

	printf("\n");
	printf("Configuration information:\n");
	printf("   -be,-le\tBig-endian or little-endian (required)\n");
	printf("   -blk\t\tThe system flash erasesize\n");
	printf("   -mac\t\tMAC address(required)\n");
	printf("   -ip\t\tIP address(required)\n");
	printf("   -t\t\tConfig repeat mode\n");

	printf("\n");
	return;
}

void image_info( void )
{
	
}

