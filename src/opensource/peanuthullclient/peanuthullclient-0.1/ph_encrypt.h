/*****************************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : ph_encrypt.h
 �ļ����� : �����ַ����ļ��ܼ�����

 �����б� :

 �޶���¼ :
            ���� :  Kevin
            ���� :  2009-11-18
            ���� :

******************************************************************************/

#ifndef __PH_ENCRYPT_H
#define __PH_ENCRYPT_H

#include "md5.h"
#include "hmac_md5.h"
#include "blowfish.h"


/******************************************************************************
                        *                               ��������                                   *
 ******************************************************************************/
void MD5String(char *pszSrcString, char *pszDesString);
int   Base64_Decode(char   *psSrc,  int   iSize,   char   *psDest);
int  Base64_Encode(char *psSrc, unsigned int uiSize, char *psDest);

#endif /* __PH_ENCRYPT_H */

