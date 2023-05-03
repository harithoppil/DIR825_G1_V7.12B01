/*****************************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : ph_encrypt.c
 �ļ����� : ʵ���ַ����ļ��ܼ�����

 �����б� :

 �޶���¼ :
            ���� :  Kevin
            ���� :  2009-11-18
            ���� :

******************************************************************************/

#include "ph_encrypt.h"
#include "string.h"

#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif

/******************************************************************************
                        *                               ȫ�ֱ���                               *
 ******************************************************************************/
 /*����ʱ���ÿ�еĳ���*/
static unsigned  int  g_uiLineWidth   =   76;

/*Base64�����*/
static const   char   BASE64_ENCODE_TABLE[64]   =   {
65,     66,     67,     68,     69,     70,     71,     72,     //   00   -   07
73,     74,     75,     76,     77,     78,     79,     80,     //   08   -   15
81,     82,     83,     84,     85,     86,     87,     88,     //   16   -   23
89,     90,     97,     98,     99,   100,   101,   102,     //   24   -   31
103,   104,   105,   106,   107,   108,   109,   110,     //   32   -   39
111,   112,   113,   114,   115,   116,   117,   118,     //   40   -   47
119,   120,   121,   122,     48,     49,     50,     51,     //   48   -   55
52,     53,     54,     55,     56,     57,     43,     47      //   56   -   63
};

/*Base64�����*/
static const   unsigned   char BASE64_DECODE_TABLE[256] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
	0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
	0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
	0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
	0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

/*****************************************************************************
                        *                                    ��������                           *
 ******************************************************************************/
/******************************************************************************
����: ʹ��BASE64�����ַ���
����: psSrc          input          Դ��
             iSize           input          Դ������
             psDest        output        ���ܴ�(����)
����: ���ܴ���ʵ�ʳ���
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
int  Base64_Encode(char *psSrc, unsigned int uiSize, char *psDest)
{
    unsigned int uiByte = 0;
    unsigned int uiInMax3 =0 ;
    unsigned int uiLen = 0;
    char *psInPtr = NULL;
    char *psInLimitPtr = NULL;
    char *psOutPtr = NULL  ;

    if ((psSrc   ==   NULL)   ||   (uiSize   <=   0))   
        return   0;

    psInPtr = psSrc;
    uiInMax3 = uiSize/3*3;
    psOutPtr = psDest;
    psInLimitPtr = psInPtr + uiInMax3;

    while (psInPtr != psInLimitPtr)
    {
        uiLen = 0;
        while ((uiLen < g_uiLineWidth)  &&  (psInPtr != psInLimitPtr))
        {
            uiByte = (unsigned char) *psInPtr++;
            uiByte = uiByte << 8;

            uiByte = uiByte |(unsigned   char) *psInPtr++;
            uiByte = uiByte << 8;
            uiByte = uiByte | (unsigned   char)   *psInPtr++;

            /*��4   byte����д���������*/
            psOutPtr[3] = BASE64_ENCODE_TABLE[uiByte & 0x3F];
            uiByte = uiByte   >>   6;
            psOutPtr[2] = BASE64_ENCODE_TABLE[uiByte & 0x3F];
            uiByte = uiByte>>   6;
            psOutPtr[1] = BASE64_ENCODE_TABLE[uiByte & 0x3F];
            uiByte = uiByte   >>   6;
            psOutPtr[0] = BASE64_ENCODE_TABLE[uiByte];
            psOutPtr += 4;
            uiLen += 4;
        }/*end of while ((uiLen < g_uiLineWidth) && (psInPtr != psInLimitPtr))*/
       
        if (uiLen >= g_uiLineWidth)
        {
            *psOutPtr++ =  '\r';   //���ϻس����з�
            *psOutPtr++ =  '\n';
        }
    }/*end of   while   (psInPtr   !=   psInLimitPtr)*/
    
    /*����β��*/
    switch (uiSize - uiInMax3)
    {
        case 1:
            uiByte = (unsigned char) *psInPtr;
            uiByte = uiByte << 4;
            psOutPtr[1] = BASE64_ENCODE_TABLE[uiByte & 0x3F];
            uiByte = uiByte >> 6;
            psOutPtr[0]  =  BASE64_ENCODE_TABLE[uiByte];
            psOutPtr[2]  =  '=';   //��'='Ҳ����64�����ʣ�ಿ��
            psOutPtr[3]  =  '=';
            psOutPtr += 4;
            break;
        case   2:
            uiByte = (unsigned  char) *psInPtr++;
            uiByte = uiByte << 8;
            uiByte = uiByte |(unsigned char) *psInPtr;
            uiByte = uiByte << 2;
            psOutPtr[2]  =  BASE64_ENCODE_TABLE[uiByte & 0x3F];
            uiByte = uiByte >> 6;
            psOutPtr[1] = BASE64_ENCODE_TABLE[uiByte & 0x3F];
            uiByte = uiByte >> 6;
            psOutPtr[0] = BASE64_ENCODE_TABLE[uiByte];
            psOutPtr[3] =  '=';
            psOutPtr +=4;
            break;
    }
    
    return  (unsigned int) (psOutPtr  - psDest);
}
/******************************************************************************
����: ʹ��BASE64�����ַ���
����: psSrc          input          ���ܴ�
             iSize           input          ���ܴ�����
             psDest        output        ���ܴ�(����)
����: ���ܴ���ʵ�ʳ���
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
int   Base64_Decode(char *psSrc, int iSize, char *psDest)
{
    unsigned int uiByteBuffer = 0;
    unsigned int uiByteBufferSpace = 0;
    unsigned int uiTemp = 0;
    char *psInPtr = NULL;
    char *psInLimitPtr = NULL;
    char  *psOutPtr = NULL;
    int   iRealLen = 0;

    if ((psSrc == NULL) ||(psDest == NULL) ||(iSize   <=   0)) 
        return 0;

    uiByteBuffer = 0;
    uiByteBufferSpace = 4;

    psInPtr = psSrc;
    psInLimitPtr = psInPtr + iSize;
    psOutPtr = psDest;

    while (psInPtr != psInLimitPtr)
    {
        uiTemp   =   BASE64_DECODE_TABLE[*psInPtr];
        psInPtr++;
        if (uiTemp ==   0xFF)   
            continue;   //����255�Ƿ��ַ�
        uiByteBuffer   =   uiByteBuffer   <<   6   ;
        uiByteBuffer   =   uiByteBuffer   |   uiTemp;
        uiByteBufferSpace--;

        if (uiByteBufferSpace   !=   0)   
            continue;   //һ�ζ���4���ֽ�
           
        /*����д��3���ֽڵ�����*/
        psOutPtr[2]   =   uiByteBuffer;
        uiByteBuffer   =   uiByteBuffer   >>   8;
        psOutPtr[1]   =   uiByteBuffer;
        uiByteBuffer   =   uiByteBuffer   >>   8;
        psOutPtr[0]   =   uiByteBuffer;

        /*׼��д���3λ*/
        psOutPtr+=   3;   
        uiByteBuffer   =   0;   
        uiByteBufferSpace   =   4;
    }/*end of  while (psInPtr != psInLimitPtr)*/
   
    iRealLen   =   (unsigned   int)psOutPtr   -   (unsigned   int)psDest;
    /*����β��   ����ʵ�ʳ���*/
    switch   (uiByteBufferSpace)
    {
    case   1:
        uiByteBuffer   =   uiByteBuffer   >>   2;
        psOutPtr[1]   =   uiByteBuffer;
        uiByteBuffer   =   uiByteBuffer   >>   8;
        psOutPtr[0]   =   uiByteBuffer;
        return   iRealLen   +   2;
    case   2:
        uiByteBuffer   =   uiByteBuffer   >>   4;
        psOutPtr[0]   =   uiByteBuffer;
        return   iRealLen   +   1;
    default:
        return   iRealLen;
    }
}
/******************************************************************************
����: ʹ��MD5�����ַ���
����: pszSrcString          input           ԭ��
             pszDesString         output          ����
����: ��
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
void MD5String(char *pszSrcString, char *pszDesString)
{
    MD5_CTX mdContext;
    int i = 0;
    unsigned char uszHash[16] = {0};
    char temp[2] = {0};
   
    MD5Init(&mdContext);
    MD5Update(&mdContext,pszSrcString,strlen(pszSrcString));
    MD5Final(uszHash,&mdContext);

    for(i=0;i<16;i++)  
    {
        sprintf(temp,"%02x", uszHash[i]);
        strcat(pszDesString,temp);
    }

    return ;
}


#ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif
