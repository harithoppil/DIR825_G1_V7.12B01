#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tbserror.h"
//#include "tbsutil.h"
#include "config.h"
#include <unistd.h>

extern ST_TBS_ERR_TABLE g_astTbsErrTable[]; //defined in tbserror.c file

int itrm_cmp(const void *pv1, const void *pv2)
{
    const ST_TBS_ERR_TABLE *pst1 = *(ST_TBS_ERR_TABLE **)pv1;
    const ST_TBS_ERR_TABLE *pst2 = *(ST_TBS_ERR_TABLE **)pv2;

    if (TBS_ERR_INTERNAL(pst1->iErrNo) > TBS_ERR_INTERNAL(pst2->iErrNo))
    {
        return 1;
    }
    else if (TBS_ERR_INTERNAL(pst1->iErrNo) < TBS_ERR_INTERNAL(pst2->iErrNo))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}


#define MAX_SINGLE_LEN  1024
#define MAX_JS_LEN   0xffff

#ifndef STANDARD_DIR
#define STANDARD_DIR "./standard/languages/"
#endif
#define EN_DIR     "en_us"
#define CN_DIR     "zh_cn"

#define STANDARD_JS_NAME "error.js"

#define MID_JS_NAME "mid_"
#define UPDATED_JS_NAME "updated_"




#define JS_FORMAT(pstItem, member) \
    "\t%04u : '%s',\n", TBS_ERR_INTERNAL(pstItem->iErrNo), pstItem->member


#define CAT_CONTENTS(pcBuf, pstItem, member) \
    ulLen = snprintf(pcBuf, MAX_SINGLE_LEN, JS_FORMAT(pstItem, member)); \
    pcBuf += ulLen;


#define WRITE_FILE(file, buf, len) \
{ \
    if ('\0' != buf[0]) \
    { \
        fp = fopen(file, "w"); \
        if (NULL == fp) \
        { \
            printf("File Open Failed!\n"); \
            return -1; \
        } \
        fwrite(buf, 1, len, fp); \
        fclose(fp); \
    } \
}

#define MEM_ALLOC(acBuf, ulLen, pcBuf) \
{ \
    acBuf = malloc(ulLen); \
    if (NULL == acBuf) \
    { \
        printf("malloc failed\n"); \
        return -1; \
    } \
    pcBuf = acBuf; \
    pcBuf[0] = '\0'; \
}

static inline int safe_strcmp(const char *s1, const char *s2)
{
    if (!s1 && !s2)
        return 0;
    if (!s1)
        return -1;
    if (!s2)
        return 1;
    return strcmp(s1, s2);
}

int make_file(void)
{
    ST_TBS_ERR_TABLE *pstErrItem = g_astTbsErrTable;
    ST_TBS_ERR_TABLE **ppstIndex = NULL;
    unsigned long ulCount = 0;
    unsigned long i = 0;
//    unsigned short usInteralErr = 0;
    char *aacCommBuf[2] = {NULL, NULL};
    char *apcCommBuf[2] = {aacCommBuf[0], aacCommBuf[1]};
    unsigned long ulLen = 0;
    FILE *fp = NULL;

    MEM_ALLOC(aacCommBuf[0], MAX_JS_LEN, apcCommBuf[0]);
    MEM_ALLOC(aacCommBuf[1], MAX_JS_LEN, apcCommBuf[1]);

    while (pstErrItem->iErrNo != 0)
    {
        pstErrItem++;
    }
    ulCount = pstErrItem - g_astTbsErrTable;
    pstErrItem = g_astTbsErrTable;
    ppstIndex = malloc(ulCount * sizeof(ST_TBS_ERR_TABLE *));
    if (NULL == ppstIndex)
    {
        printf("malloc failed\n");
        return -1;
    }
    for (i = 0; i < ulCount; i++)
    {
        ppstIndex[i] = &pstErrItem[i];
    }
    qsort(ppstIndex, ulCount, sizeof(ST_TBS_ERR_TABLE *), itrm_cmp);

    for (i = 0; i < ulCount; i++)
    {
//        usInteralErr = TBS_ERR_INTERNAL(ppstIndex[i]->iErrNo);
        CAT_CONTENTS(apcCommBuf[0], ppstIndex[i], pcEnglish);
        CAT_CONTENTS(apcCommBuf[1], ppstIndex[i], pcChinese);
    }

    WRITE_FILE(MID_JS_NAME EN_DIR ".js", aacCommBuf[0],
               apcCommBuf[0] - aacCommBuf[0]);

    WRITE_FILE(MID_JS_NAME CN_DIR ".js", aacCommBuf[1],
               apcCommBuf[1] - aacCommBuf[1]);

    free(ppstIndex);

    free(aacCommBuf[0]);
    free(aacCommBuf[1]);

    return 0;
}




int update_to_js(const char *pcStandard, const char *pcBuild, char *pcUpdated)
{
    FILE *fpJs = NULL;
    FILE *fp = NULL;
    char *acJsBuf = NULL;
    char *acMIDBuf = NULL;
    char *pcBuf = NULL;
    unsigned long ulLen = 0;
    unsigned long ulJsLen = 0;
    unsigned long ulMIDLen = 0;
    unsigned char ucErrFlag = 0;

    MEM_ALLOC(acJsBuf, MAX_JS_LEN, pcBuf);
    MEM_ALLOC(acMIDBuf, MAX_JS_LEN, pcBuf);

    /* 打开标准文件 */
    fpJs = fopen(pcStandard, "r");
    if (NULL == fpJs)
    {
        printf("File Open Failed: %s\n", pcStandard);
        return -1;
    }

    pcBuf = acJsBuf;
    while (1)
    {
        if (NULL == fgets(pcBuf, MAX_SINGLE_LEN, fpJs))
        {
            break;
        }
        if (NULL != strstr(pcBuf, "var UEcode = {"))
        {
            ucErrFlag = 1;
            pcBuf += strlen(pcBuf);
            break;
        }
        pcBuf += strlen(pcBuf);
    }
    fclose(fpJs);

    ulJsLen = pcBuf - acJsBuf;

    /* 打开中间文件 */
    pcBuf = acMIDBuf;
    fp = fopen(pcBuild, "r");
    if (NULL == fp)
    {
        printf("File Open Failed: %s\n", pcBuild);
        return -1;
    }
    ulLen = fread(pcBuf, 1, MAX_JS_LEN, fp);
    pcBuf += ulLen;

    /* 关闭文件 */
    fclose(fp);

    if (pcBuf == acMIDBuf)/* 没有内容 */
    {
        free(acJsBuf);
        free(acMIDBuf);
        return 0;
    }
    pcBuf[0] = '}';
    pcBuf[1] = '\0';
    ulMIDLen = pcBuf - acMIDBuf + 1;
    while (',' != *pcBuf)
    {
        pcBuf--;
    }
    *pcBuf = ' ';

    /* 写入更新文件 */
    fpJs = fopen(pcUpdated, "w");
    if (NULL == fpJs)
    {
        printf("File Open Failed: %s\n", pcUpdated);
        return -1;
    }

    /* 重新写入内容 */
    ulLen = fwrite(acJsBuf, 1, ulJsLen, fpJs);
    ulLen = fwrite(acMIDBuf, 1, ulMIDLen, fpJs);

    /* 关闭文件 */
    fclose(fpJs);

    free(acJsBuf);
    free(acMIDBuf);

    return 0;
}



int main(int argc, char **argv)
{
    int ret = 0;
    int argn = 1;
    char pcJsPath[256] = {0}; // 保存生成error.js 的基础js路径.

    ret = make_file();
    if (0 != ret)
    {
        printf("Make file failed\n");
        return -1;
    }
	printf("STANDARD_DIR:%s\n",STANDARD_DIR);
    system("find -name 'mid_zh_cn.js' | while read line; "
            "do  iconv -f GB2312 -t UTF-8 $line > /tmp/tmp.conv;if [ $? -eq 0 ];"
            " then mv -f /tmp/tmp.conv $line; "
            "else echo \"convert $line failed\";fi ; done;");

    while ( argn < argc && argv[argn][0] == '-' )
    {
        /*********************************************************************
            新加一个运行参数, 依据-p的路径去重新生成 error.js, 如果没有指定
            路径, 依然使用原本的逻辑
        **********************************************************************/
		if ( safe_strcmp( argv[argn], "-p" ) == 0 )
        {      
			++argn;
            printf( "argv[%i]=%s\n", argn, argv[argn]);
            sprintf( pcJsPath, "%s%s/%s", argv[argn] ,EN_DIR,STANDARD_JS_NAME);
            ret = update_to_js(pcJsPath,  MID_JS_NAME EN_DIR ".js", UPDATED_JS_NAME EN_DIR ".js");  // 生成 en error.js
            sprintf( pcJsPath, "%s%s/%s", argv[argn] ,CN_DIR,STANDARD_JS_NAME);
            ret = update_to_js(pcJsPath,  MID_JS_NAME CN_DIR ".js", UPDATED_JS_NAME CN_DIR ".js");  // 生成 cn error.js
            printf("OK!!!\n");
            exit(0);
		}
    }
    
    ret = update_to_js(STANDARD_DIR EN_DIR "/" STANDARD_JS_NAME,
                       MID_JS_NAME EN_DIR ".js", UPDATED_JS_NAME EN_DIR ".js");
    if (0 != ret)
    {
        printf("Update en failed\n");
    }

    ret = update_to_js(STANDARD_DIR CN_DIR "/" STANDARD_JS_NAME,
                       MID_JS_NAME CN_DIR ".js", UPDATED_JS_NAME CN_DIR ".js");
    if (0 != ret)
    {
        printf("Update zh failed\n");
    }

    printf("OK!!!\n");

    return 0;
}



