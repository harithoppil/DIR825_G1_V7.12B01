/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : mktree.c
 �ļ����� :
            1���ù�����Ҫ�������ı���ʽ�Ľڵ��б�ת���ɽڵ����ĸ�ʽ,һ�������,pdf
               �ļ��еĽڵ��б����ת�����ı���ʽ.
            2��
               �ı��ĸ�ʽӦ��Ϊ:
               Name                             Type            Writable    Default
               ����:
               SRTPKeyingMethods                string(256)     -           -
               PSTNFailOver                     boolean         W           False
               .VoiceService.{i}.Capabilities.  object          -           -
               û�е�ֵ�Լ��Ŵ���,ֵ�м���TAB���߿ո�ָ�
            3������PRE_TAB_COUNT�����ʹ�õ����������������ӳ�ʼ������TAB����
            4����Ҫע���ֹ�ɾ���ɱ༭��������·���ͽڵ�������ӵ�"-"�����±�
            5��
               ʹ�÷���:
                 ./mktree filenamein.txt filenameout.xml [PreTabCount]
 �����б� :
 �޶���¼ :
          1 ���� : huangjidong
            ���� : 2008-7-15
            ���� : mktree 1.0

**********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE_COUNT 1024
#define MAX_OBJ_LEN 512
#define PRE_TAB_COUNT 0   /*�ʼǰ���TAB��,�ɸ���*/

typedef struct _ST_NODE
{
    char *pcNode;
    struct _ST_NODE *pstPre;
    struct _ST_NODE *pstNxt;
}ST_NODE;


#define APPEND_LIST(head, tail, node) \
{ \
    if (NULL != tail) \
    { \
        tail->pstNxt = node; \
        node->pstPre = tail; \
    } \
    else \
    { \
        head = node; \
    } \
    tail = node; \
}

void free_chain (ST_NODE * chain)
{
    ST_NODE *current = NULL;

    while (chain)
    {
        if(chain->pcNode)
            free(chain->pcNode);
        current = chain;
        chain = chain->pstNxt;
        free (current);
    }
    chain = NULL;
}

int safe_strcmp(const char *s1, const char *s2)
{
    if(!s1 && !s2)
        return 0;
    if(!s1)
        return -1;
    if(!s2)
        return 1;
    return strcmp(s1, s2);
}

ST_NODE *parse_node(const char *ccNodeStr)
{
    char *pcSrc;
    char *pcNode;
    ST_NODE *pstHead = NULL;
    ST_NODE *pstTail = NULL;
    ST_NODE *pstNode = NULL;

    if(!ccNodeStr)
        return NULL;

    pcSrc= strdup(ccNodeStr);
    pcNode = strtok(pcSrc, ".{}");

    if(pcNode)
    do
    {
        pstNode = malloc(sizeof(ST_NODE));
        memset(pstNode, 0, sizeof(ST_NODE));
        APPEND_LIST(pstHead, pstTail, pstNode);
        pstNode->pcNode = strdup(pcNode);
    }
    while(NULL != (pcNode = strtok(NULL, ".{}")));

    free(pcSrc);
    return pstHead;
}

int main(int argc, char **argv)
{
    char *pcName = NULL;
    char *pcType = NULL;
    char *pcWritable = NULL;
    char *pcDefault = NULL;
    char acBuf[MAX_LINE_COUNT+1] = {0};
    char acOutBuf[MAX_LINE_COUNT+1] ={0};
    FILE *fpIn = NULL, *fpOut = NULL;
    int i = 0, j = 0;
    int iPreTab = PRE_TAB_COUNT;
    char acObjectOld[MAX_OBJ_LEN+1] = {0};
    char acObjectNew[MAX_OBJ_LEN+1]={0};
    ST_NODE *pstOld = NULL, *pstNew = NULL, *pstFreeOld = NULL, *pstFreeNew = NULL;
    int iFileEnd = 0;

    if(argc < 3 || argc > 4)
    {
        printf("Usage:\n\tmktree filenamein.txt filenameout.xml\n\t\n\tText node file shoud include Name,Type,Writable,and Default.\n");
        exit(0);
    }
    else
    {
        /*����������ļ�*/
        fpIn = fopen(argv[1], "r");
        fpOut = fopen(argv[2], "w");
    }
    if(argc == 4)
    {
        iPreTab = atol(argv[3]);
    }

    if(!fpIn || !fpOut)
    {
        printf("File open error, Please check and try again.\n");
        exit(0);
    }

    while(fgets(acBuf, MAX_LINE_COUNT, fpIn))
    {
        /*�ý�������ͬʱȥ����ȫ���ַ�������*/
        /*��ýڵ���*/
        LAB_FILE_END:
        pcName = strtok(acBuf, " \t\r\n塰塱");
        if(pcName)
        {
            /*��ýڵ�����*/
            pcType = strtok(NULL, " \t\r\n塰塱");
            if(pcType)
            {
                /*��ýڵ��д����*/
                pcWritable = strtok(NULL, " \t\r\n塰塱");
                if(pcWritable)
                {
                    /*��ýڵ�Ĭ��ֵ*/
                    if(strcasecmp(pcType, "object") != 0)
                    {
                        pcDefault = strtok(NULL, " \t\r\n塰塱");
                        if(!pcDefault)
                        {
                            continue;
                        }
                    }
                    else
                    {
                        /*����Ƕ���ڵ�Ļ�,���ж����������*/
                        pcDefault = "";
                        strcpy(acObjectNew, pcName);
                        /*�Ƚ��¶���;ɶ���Ĳ�ͬ*/
                        pstFreeNew = pstNew = parse_node(acObjectNew);
                        pstFreeOld = pstOld = parse_node(acObjectOld);
                        if(pstOld != NULL)
                        {
                            while(pstOld && pstNew)
                            {
                                if(safe_strcmp(pstNew->pcNode, pstOld->pcNode) != 0)break;
                                pstNew=pstNew->pstNxt;
                                pstOld=pstOld->pstNxt;
                            }
                        }

                        if(pstOld)
                        {
                            /*���������·��,�򽫼��ٵĶ���ر�*/
                            for(i = 0; pstOld->pstNxt != NULL; i++)pstOld=pstOld->pstNxt;
                            for(; i >= 0 ; i--)
                            {
                                iPreTab--;
                                for(j = 0; j < iPreTab; j++)acOutBuf[j]='\t';
                                sprintf(acOutBuf+iPreTab, "</%s>\n", pstOld->pcNode);
                                printf("%s", acOutBuf);
                                /*������ļ�*/
                                if(fputs(acOutBuf, fpOut)<0)
                                {
                                    printf("file write error.\n");
                                }
                                pstOld=pstOld->pstPre;
                            }
                        }
                        if(pstNew)
                        {
                            if(strcasecmp(pcWritable, "w") == 0)
                            {
                                pcWritable = " Writable=\"1\"";
                            }
                            else if(strcasecmp(pcWritable, "0") == 0)
                            {
                                pcWritable = " Writable=\"0\"";
                            }
                            else
                            {
                                /*����ú�ɽ�û����ȷ����ΪW��0���ԵĽڵ㲻����Writable����*/
                                #ifdef EXPLICIT_WRITABLE
                                pcWritable = "";
                                #else
                                pcWritable = " Writable=\"0\"";
                                #endif
                            }
                            /*���������·��,�����Ϊ����*/
                            while(pstNew != NULL)
                            {
                                for(j = 0; j < iPreTab; j++)acOutBuf[j]='\t';
                                sprintf(acOutBuf+iPreTab, "<%s type=\"%s\"%s>\n", \
                                    pstNew->pcNode, pcType, pcWritable);
                                printf("%s", acOutBuf);
                                /*������ļ�*/
                                if(fputs(acOutBuf, fpOut)<0)
                                {
                                    printf("file write error.\n");
                                }
                                pstNew=pstNew->pstNxt;
                                iPreTab++;
                            }
                        }
                        free_chain(pstFreeNew);
                        free_chain(pstFreeOld);
                        strcpy(acObjectOld, acObjectNew);
                        continue;
                    }
                }
                else
                {
                    continue;
                }
            }
            else
            {
                continue;
            }
        }
        else
        {
            continue;
        }

        /*ת��Ĭ��ֵ*/
        if(strcasecmp(pcDefault, "false") == 0)
        {
            pcDefault = "0";
        }
        else if(strcasecmp(pcDefault, "true") == 0)
        {
            pcDefault = "1";
        }
        else if(strcasecmp(pcDefault, "<empty>") == 0)
        {
            pcDefault = "";
        }

        if(strcasecmp(pcWritable, "w") == 0)
        {
            pcWritable = " Writable=\"1\"";
        }
        else if(strcasecmp(pcWritable, "0") == 0)
        {
            pcWritable = " Writable=\"0\"";
        }
        else
        {
            /*����ú�ɽ�û����ȷ����ΪW��0���ԵĽڵ㲻����Writable����*/
            #ifdef EXPLICIT_WRITABLE
            pcWritable = "";
            #else
            pcWritable = " Writable=\"0\"";
            #endif
        }

        /*����ڵ�̶���ʽ*/
        for(j = 0; j < iPreTab; j++)acOutBuf[j]='\t';
        sprintf(acOutBuf+iPreTab, "<%s type=\"%s\"%s>%s</%s>\n", \
            pcName, pcType, pcWritable,strcmp(pcDefault, "-") == 0?"":pcDefault, pcName);
        printf("%s", acOutBuf);

        /*������ļ�*/
        if(fputs(acOutBuf, fpOut)<0)
        {
            printf("file write error.\n");
        }
    }

    if(iFileEnd == 0)
    {
        iFileEnd = 1;
        strcpy(acBuf, ".    object  -   -");
        goto LAB_FILE_END;
    }

    if(fpOut)
    {
        fclose(fpOut);
    }
    if(fpIn)
    {
        fclose(fpIn);
    }
    exit(0);
}
