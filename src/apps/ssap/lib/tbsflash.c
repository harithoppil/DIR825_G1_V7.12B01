/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : tbs_flash.c
 �ļ����� : ������Ϣ��Ŀ������Ӧ�ò�ӿ�


 �����б� :
				app_item_get
				app_item_save

 �޶���¼ :
          1 ���� : ������
            ���� : 2008-10-27
            ���� :

=========================================================================*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include "tbsflash.h"
#include "tbstype.h"
#include "tbserror.h"
#include <sys/ioctl.h>
#include "tbs_ioctl.h"
#include "flash_layout.h"


/* ������ID�ĺϷ��� */
#define CHECK_OBJ_ID(enObjId) \
        ((enObjId) < OBJ_LOGGER || (enObjId) > OBJ_ETHLAN ? 0 : 1)

/* ��������semun�Ƿ��Ѿ����� */
#ifdef _SEM_SEMUN_UNDEFINED
    union semun
    {
        int val;
        struct semid_ds *buf;
        unsigned short int *array;
        struct seminfo *__buf;
    };
#endif


static int s_iSemId = -1;

/********************************************************************/

int InitSem(void)
{
    BOOL bInitOk = FALSE;
    int iKey = 0;
    int iTmpError = 0;
    int iFlag = IPC_CREAT | 00666;
    int iMaxTries = 3;
    int i;
    struct semid_ds  stSemInfo;
    union  semun     unSemArg;

    /* �����ֵ */
    iKey = ftok(ORIGINAL_DATA_PATH, 1);
    if (iKey < 0)
    {
        FLASH_TRACE("ftok error \n");
        return TBS_FAILED;
    }

    /* ����ź���ID */
    s_iSemId = semget(iKey, 1, iFlag | IPC_EXCL);
    if (s_iSemId >= 0)
    {   /* ��ʼ���ź��� */
        FLASH_TRACE("create semaphore \n");
        unSemArg.val = 1;
        if(semctl(s_iSemId, 0, SETVAL, unSemArg) < 0)
        {
            FLASH_TRACE("semctl SETVAL error \n");
            return TBS_FAILED;
        }

        return TBS_SUCCESS;
    }
    else
    {
        iTmpError = errno;
        if (iTmpError != EEXIST)
        {
            FLASH_TRACE("semget error \n");
            return TBS_FAILED;
        }

        FLASH_TRACE("open semaphore \n");
        /* ��ȡ�Ѵ��ڵ��ź���ID */
        s_iSemId = semget(iKey, 1, iFlag);
        if (s_iSemId < 0)
        {
            FLASH_TRACE("semget error \n");
            return TBS_FAILED;
        }

        /* ����ź����Ƿ��Ѿ�����ʼ�� */
        unSemArg.buf = &stSemInfo;
        for(i=0; i < iMaxTries; i++)
        {
            if(semctl(s_iSemId, 0, IPC_STAT, unSemArg) < 0)
            {
                FLASH_TRACE("semctl error \n");
                return TBS_FAILED;
            }

            if(unSemArg.buf->sem_otime != 0)
            {
                bInitOk = TRUE;
                break;
            }

            sleep(1);
        }

        if (TRUE == bInitOk)
        {   /* �ź����Ѿ�����ʼ�� */
            FLASH_TRACE("semaphore has been initialized by other process\n");
            return TBS_SUCCESS;
        }
        else
        {
            /* �ź�����û�б���ʼ�� */
            unSemArg.val =1;
            if (semctl(s_iSemId, 0, SETVAL,  unSemArg) < 0)
            {
                FLASH_TRACE("semctl SETVAL error \n");
                return TBS_FAILED;
            }

            return TBS_SUCCESS;
        }

    }

}


/*********************************************************************
* ��������: InitOrigiDataBuf
* ��������: ���ڶ�original_data �ļ����ݻ��������г�ʼ��
* ����:     �洢original_data �ļ����ݵĻ�����ָ��
* ���:     �ɹ� - ����0
*           ʧ�� - ����-1
* ����ʱ��:
* ������:   ������
**********************************************************************/
static int InitOrigiDataBuf(ST_FLASH_DATA_INFO *pstFlashDataInfo)
{
    ST_FLASH_DATA_INFO  stFlashDataInfo = {0, 0};

    if (NULL == pstFlashDataInfo)
    {
        return TBS_FAILED;
    }

    *pstFlashDataInfo = stFlashDataInfo;
    return TBS_SUCCESS;
}

/*********************************************************************
* ��������: GetFlashFileData
* ��������: ��original_data �ļ��ж�ȡ���е����ݵ���̬��������
* ����:     iFd - original_data���ļ������������øú���ǰ��
*           �ļ���֤�Ѿ�����
*
*           ppstFlashDataInfo - ��original_data�ļ��ж���������
*           �����*ppstFlashDataInfoָ��Ķ�̬��������
*
* ���:     �ɹ� - ����0
*           ʧ�� - ����-1
* ����ʱ��:
* ������:   ������
**********************************************************************/
static int GetFlashFileData(const char *pcPath, ST_FLASH_DATA_INFO **ppstFlashDataInfo)
{
    int   iRet = -1;
    int   iFd = -1;

    /* ������� */
    if (NULL == pcPath || NULL == ppstFlashDataInfo)
    {
        FLASH_TRACE("Error: invalid parameters \n");
        return TBS_FAILED;
    }

    /* ���ļ� */
    iFd = open(pcPath, O_RDONLY);
    if (iFd < 0)
    {
        FLASH_TRACE("Error: open file %s error \n", pcPath);
        return TBS_FAILED;
    }

    /* ���䶯̬�洢�����ڴ洢����original_data�ļ��е�����*/
    *ppstFlashDataInfo = (ST_FLASH_DATA_INFO *)malloc(MAX_FLASH_LEN);
    if (NULL == *ppstFlashDataInfo)
    {
        FLASH_TRACE("malloc error \n");
        close(iFd);
        return TBS_FAILED;
    }

    memset((void *)(*ppstFlashDataInfo), 0, MAX_FLASH_LEN);

    /*��flash�ж������ݵ���̬�洢���� */
    iRet = read(iFd, (void *)(*ppstFlashDataInfo), MAX_FLASH_LEN);
    close(iFd);

    if (iRet < 0)
    {
        FLASH_TRACE("read error \n");
        free(*ppstFlashDataInfo);
        return TBS_FAILED;
    }

    FLASH_TRACE("the data length reading from flash is : %d\n", iRet);
    FLASH_TRACE("objnum= %d, datalen = %lu \n",
                (*ppstFlashDataInfo)->iObjNum, (*ppstFlashDataInfo)->ulDataLen);

    if (iRet == (int)(sizeof(ST_FLASH_DATA_INFO) + (*ppstFlashDataInfo)->ulDataLen))
    {
        FLASH_TRACE("file data length is correct \n");
    }
    else
    {
        FLASH_TRACE("file data length is error \n");
        iRet = InitOrigiDataBuf(*ppstFlashDataInfo);
        if (RET_FAILED(iRet))
        {
            FLASH_TRACE("Error: calling fuction \"InitOrigiDataBuf\" failure \n");
            free(*ppstFlashDataInfo);
            return TBS_FAILED;
        }
    }

    return TBS_SUCCESS;

}


/**********************************************************************
* ��������: SaveFlashFileData
* ��������: ������д�뵽original_data�ļ���
* ����:     iFd - original_data���ļ������������øú���ǰ��
*           �ļ���֤�Ѿ�����
*
*           pstFlashDataInfo - д�����ݵĻ�����ָ��
*
* ���:     �ɹ� - ����0
*           ʧ�� - ����-1
* ����ʱ��:
* ������:   ������
***********************************************************************/
static int SaveFlashFileData(const char *pcPath, const ST_FLASH_DATA_INFO *pstFlashDataInfo)
{
    int           iFd  = -1;
    int           iRet = -1;
    unsigned long ulLen = 0;

    /* ������� */
    if (NULL == pcPath || NULL == pstFlashDataInfo)
    {
        FLASH_TRACE("Error: invalid parameters \n");
        return TBS_FAILED;
    }

    /* ���ļ� */
    iFd = open(pcPath, O_WRONLY|O_TRUNC);
    if (iFd < 0)
    {
        FLASH_TRACE("Error: open file %s error \n", pcPath);
        return TBS_FAILED;
    }

    /* ������д�뵽�ļ��� */
    ulLen = sizeof(ST_FLASH_DATA_INFO) + pstFlashDataInfo->ulDataLen;
    FLASH_TRACE("---write data length = %lu \n", ulLen);

    iRet = write(iFd, (void *)pstFlashDataInfo, ulLen);
    close(iFd);

    if (iRet < 0 || iRet != ulLen)
    {
        FLASH_TRACE("write error \n");
        return TBS_FAILED;
    }

    FLASH_TRACE("after write, return data length = %d \n", iRet);

    return TBS_SUCCESS;
}

/**********************************************************************
* ��������: FindObjData
* ��������: ���ļ����ݻ�������Ѱ�Ҷ�������
* ����:     pstFlashDataInfo - �ļ����ݻ�����ָ��
*           enObjId  - ҪѰ�ҵĶ���ID
*           ppstObjDataInfo -  *ppstObjDataInfo �������Ϊָ��������ݵ�ͷ��
*
* ���:     �ɹ� - ����0
*           ʧ�� - ����-1
* ����ʱ��:
* ������:   ������
***********************************************************************/
static int FindObjData(const ST_FLASH_DATA_INFO *pstFlashDataInfo, EN_OBJ_ID enObjId,
                          ST_OBJ_DATA_INFO **ppstObjDataInfo)
{
    int  iLoop = 0;

    /* ������� */
    if (NULL == pstFlashDataInfo || NULL == ppstObjDataInfo ||
        0 == enObjId || !CHECK_OBJ_ID(enObjId))
    {
        FLASH_TRACE("Error: invalid parameters \n");
        return TBS_FAILED;
    }

    /* Ѱ�ҷ��ʶ�������� */
    *ppstObjDataInfo = (ST_OBJ_DATA_INFO *)pstFlashDataInfo->acData;

    for (iLoop = 0; iLoop < pstFlashDataInfo->iObjNum; iLoop++)
    {
        if (enObjId == (*ppstObjDataInfo)->enObjID)
        {
            FLASH_TRACE("object data has been found in the file \n");
            return TRUE;
        }
        else
        {
            *ppstObjDataInfo = (ST_OBJ_DATA_INFO *)((*ppstObjDataInfo)->acData + (*ppstObjDataInfo)->ulDataLen);
        }
    }

    FLASH_TRACE("object data doesn't exist in the file \n");
    return FALSE;

}

/**********************************************************************
* ��������: InsertObjData
* ��������: ���������ݲ��뵽original_data�ļ����ݵĻ�������
* ����:     pstFlashDataInfo - original_data�ļ����ݻ�����ָ��
*           pstObjDataInfo -   ���ļ����ݻ������У���������ͷ����ָ��
*           pcObjData - �������ݻ�����ָ��
*           ulLen   -   Ҫд��Ķ������ݳ���
*           iFlag  -  ��������д��ķ�ʽ
*
* ���:     �ɹ� - ����0
*           ʧ�� - ����-1
* ����ʱ��:
* ������:   ������
***********************************************************************/
static int InsertObjData(ST_FLASH_DATA_INFO *pstFlashDataInfo, ST_OBJ_DATA_INFO *pstObjDataInfo,
                             const char *pcObjData, unsigned long ulLen, int iFlag)
{
    char *pcShiftMargin = NULL;
    char *pcTmpPointer  = NULL;
    int         iOffset = 0;
    int  iSpareSpaceLen = 0;
    int        iIncrLen = 0;


    /* ������� */
    if (NULL == pstFlashDataInfo || NULL == pstObjDataInfo || NULL == pcObjData ||
        0 == ulLen || (iFlag != FLAG_TRUNC && iFlag != FLAG_APPEND))
    {
        FLASH_TRACE("Error: invalid parameters \n");
        return TBS_FAILED;
    }

    /* �����������ͷ����ƫ���� */
    iOffset = (char *)pstObjDataInfo - pstFlashDataInfo->acData;

    if(iOffset == pstFlashDataInfo->ulDataLen)
    {   /* ���ʶ���������ӵ�original_data�ļ�β��,˵�����ʶ�������������ӵ� */

        /* ����ʣ��ռ䳤�� */
        iSpareSpaceLen = MAX_FLASH_LEN - (sizeof(ST_FLASH_DATA_INFO) +
                         pstFlashDataInfo->ulDataLen + sizeof(ST_OBJ_DATA_INFO));
        if (iSpareSpaceLen < (int)ulLen)
        {   /*ʣ��ռ䲻��*/

            FLASH_TRACE("Error: spare space is too small \n");
            return TBS_FAILED;
        }

        /* ����Flashͷ����Ϣ */
        pstFlashDataInfo->iObjNum++;
        pstFlashDataInfo->ulDataLen += sizeof(ST_OBJ_DATA_INFO) + ulLen;

        /* ���¶������� */
        memcpy(pstObjDataInfo->acData, pcObjData, ulLen);

        /* ���¶�������ͷ����Ϣ */
        pstObjDataInfo->ulDataLen = ulLen;

        return TBS_SUCCESS;

    }
    else if (iOffset < (int)pstFlashDataInfo->ulDataLen  &&
             iOffset >= 0)
    {   /* ���ʶ��������Ѿ����� */

        /* ����ʣ��ռ䳤�� */
        iSpareSpaceLen = MAX_FLASH_LEN - (sizeof(ST_FLASH_DATA_INFO) + pstFlashDataInfo->ulDataLen);

        /* ���������ֽ���������Ҫ�ƶ����ֽ��� */
        iIncrLen = (FLAG_TRUNC == iFlag) ? (ulLen - pstObjDataInfo->ulDataLen) : ulLen;

        if(iIncrLen > 0 && iSpareSpaceLen < iIncrLen)
        {   /* ʣ��ռ䲻�� */
            FLASH_TRACE("Error: spare space is too small \n");
            return TBS_FAILED;
        }

        if (0 == iIncrLen)
        {
             ; //�����
        }
        else if (iIncrLen > 0)
        {   /* �����ֽ���Ϊ��ֵ */

            pcTmpPointer = pstFlashDataInfo->acData + pstFlashDataInfo->ulDataLen - 1;
            pcShiftMargin  = pstObjDataInfo->acData + pstObjDataInfo->ulDataLen;

            /* �ƶ����ݣ�Ϊ�����Ķ������������㹻�Ĵ洢�ռ�*/
            while (pcTmpPointer >= pcShiftMargin)
            {
                *(pcTmpPointer + iIncrLen) = *pcTmpPointer;
                pcTmpPointer--;
            }
        }
        else
        {   /* �����ֽ���Ϊ��ֵ�����������ݳ��ȱ���� */

            FLASH_TRACE("IncrLen is smaller than 0 \n");
            pcTmpPointer = pstObjDataInfo->acData + pstObjDataInfo->ulDataLen;
            pcShiftMargin = pstFlashDataInfo->acData + pstFlashDataInfo->ulDataLen -1;

            while (pcTmpPointer <= pcShiftMargin)
            {
                *(pcTmpPointer + iIncrLen) = *pcTmpPointer;
                pcTmpPointer++;
            }
        }

        /* ����Flashͷ����Ϣ */
        pstFlashDataInfo->ulDataLen += iIncrLen;

        /* ���¶������� */
        (FLAG_TRUNC == iFlag) ? memcpy(pstObjDataInfo->acData, pcObjData, ulLen) : \
            memcpy(pstObjDataInfo->acData+pstObjDataInfo->ulDataLen, pcObjData, ulLen);

        /* ���¶�������ͷ����Ϣ */
        pstObjDataInfo->ulDataLen = (FLAG_TRUNC == iFlag) ? ulLen : (pstObjDataInfo->ulDataLen + ulLen);

        return TBS_SUCCESS;
    }
    else
    {
        FLASH_TRACE("offset error \n");
        return TBS_FAILED;
    }

}

/**********************************************************************
* ��������: ReadOrigiData
* ��������: ��/proc/llconfig/original_data�ļ��ж�ȡ���ڷ��ʶ��������
* ����:  enObjId - ���ʶ���ID�����ڱ�ʶ����original_data��ĳ������
*        buf     - �洢��ȡ���ݵĻ�����
*        ulCount - ������ȡ�����ݳ���
* ���:  �ɹ� - ����ʵ�ʶ�ȡ���������ֽ���Ŀ
*        ʧ�� - ���� -1
* ����ʱ��: 2008-6-12
* ������:   ������
***********************************************************************/
int ReadOrigiData(EN_OBJ_ID enObjId, char *pcBuf, unsigned long ulCount)
{
    ST_FLASH_DATA_INFO *pstFlashDataInfo = NULL;
    ST_OBJ_DATA_INFO   *pstObjDataInfo = NULL;
    struct sembuf      stAskForRes = {0, -1, SEM_UNDO};
    struct sembuf      stFreeRes = {0, 1, SEM_UNDO};
    int                iRet = -1;

    /*������� */
    if (NULL == pcBuf || 0 == ulCount || !CHECK_OBJ_ID(enObjId))
    {
        FLASH_TRACE("Error: Invalid parameters \n");
        return TBS_FAILED;
    }

    FLASH_TRACE("semaphore ID = %d \n",s_iSemId);
    /* �����Ƿ��Ѿ�������ź���ID */
    if (s_iSemId < 0)
    {
        FLASH_TRACE("process has not got semaphore ID \n");
        iRet = InitSem();  //��ʼ���ź���
        if (RET_FAILED(iRet))
        {
            FLASH_TRACE("InitSem error \n");
            return TBS_FAILED;
        }
    }

    /* �����ź�����Դ */
    iRet = semop(s_iSemId, &stAskForRes, 1);
    if (iRet < 0)
    {
        FLASH_TRACE("semop--ask for resource error \n");
        return TBS_FAILED;
    }

    FLASH_TRACE("semop--ask for resource successfully \n");

    /* ��original_data �ļ��л�ȡ���ݷ��붯̬�������� */
    iRet = GetFlashFileData(ORIGINAL_DATA_PATH, &pstFlashDataInfo);
    if (RET_FAILED(iRet))
    {
        FLASH_TRACE("Error: calling function \"GetFlashFileData\" failure \n");
        goto FREE_SEM;
    }

    /* ���ļ����ݶ�̬��������Ѱ�Ҷ������� */
    iRet = FindObjData(pstFlashDataInfo, enObjId, &pstObjDataInfo);
    if (iRet < 0)
    {
        FLASH_TRACE("Error: calling function \"FindObjData\" failure \n");
        goto FREE_MEMORY;
    }
    else if (TRUE == (BOOL)iRet)
    {   /* �ҵ��������� */

        FLASH_TRACE("object data len = %lu \n", pstObjDataInfo->ulDataLen);
        if (ulCount >= pstObjDataInfo->ulDataLen)
        {
            memcpy(pcBuf, (void *)pstObjDataInfo->acData, pstObjDataInfo->ulDataLen);
            free(pstFlashDataInfo);
            semop(s_iSemId, &stFreeRes, 1);
            return pstObjDataInfo->ulDataLen;
        }
        else
        {
            memcpy(pcBuf, (void *)pstObjDataInfo->acData, ulCount);
            free(pstFlashDataInfo);
            semop(s_iSemId, &stFreeRes, 1);
            return ulCount;
        }
    }
    else
    {   /* û���ҵ��������� */
        free(pstFlashDataInfo);
        semop(s_iSemId, &stFreeRes, 1);
        return TBS_FAILED;
    }

FREE_MEMORY:
    free(pstFlashDataInfo);
FREE_SEM:
    semop(s_iSemId, &stFreeRes, 1);

    if (iRet < 0)
    {
        return TBS_FAILED;
    }
    else
    {
        return TBS_SUCCESS;
    }

}

/**********************************************************************
* ��������: WriteOrigiData
* ��������: �����ʶ��������д��/proc/llconfig/original_data�ļ�
* ����:  enObjId -  ���ʶ���ID�����ڱ�ʶ����original_data��ĳ������
*        buf     -  ��Ҫд�����ݵĻ�����
*        ulCount -  ��Ҫд�����ݵĳ���
*        iFlag   -  ����ָ��д�����ݷ����ı�־
* ���:  �ɹ� - ����0
*        ʧ�� - ����-1
* ����ʱ��: 2008-6-13
* ������:   ������
***********************************************************************/
int WriteOrigiData(EN_OBJ_ID enObjId, const char *pcBuf, unsigned long ulCount, int iFlag)
{
    ST_FLASH_DATA_INFO *pstFlashDataInfo = NULL;
    ST_OBJ_DATA_INFO   *pstObjDataInfo = NULL;
    struct sembuf      stAskForRes = {0, -1, SEM_UNDO};
    struct sembuf      stFreeRes = {0, 1, SEM_UNDO};
    int                iRet = -1;

    /* ������� */
    if (NULL == pcBuf || 0 == ulCount || !CHECK_OBJ_ID(enObjId) ||
        (iFlag != FLAG_APPEND && iFlag != FLAG_TRUNC))
    {
        FLASH_TRACE("Error: Invalid parameters \n");
        return TBS_FAILED;
    }

    FLASH_TRACE("semaphore ID = %d \n",s_iSemId);
    /* �����Ƿ��Ѿ�������ź���ID */
    if (s_iSemId < 0)
    {
        FLASH_TRACE("process has not got semaphore ID \n");
        iRet = InitSem();  //��ʼ���ź���
        if (RET_FAILED(iRet))
        {
            FLASH_TRACE("InitSem error \n");
            return TBS_FAILED;
        }
    }

    /* �����ź�����Դ */
    iRet = semop(s_iSemId, &stAskForRes, 1);
    if (iRet < 0)
    {
        FLASH_TRACE("semop--ask for resource error \n");
        return TBS_FAILED;
    }

    FLASH_TRACE("semop--ask for resource successfully \n");

    /* ��original_data �ļ��л�ȡ���ݷ��붯̬�������� */
    iRet = GetFlashFileData(ORIGINAL_DATA_PATH, &pstFlashDataInfo);
    if (RET_FAILED(iRet))
    {
        FLASH_TRACE("Error: calling fuction GetFlashFileData failure\n");
        goto FREE_SEM;
    }

    /* ���ļ����ݶ�̬��������Ѱ�Ҷ������� */
    iRet = FindObjData(pstFlashDataInfo, enObjId, &pstObjDataInfo);
    if (iRet < 0)
    {
        FLASH_TRACE("Error: calling function \"FindObjData\" failure \n");
        goto FREE_MEMORY;
    }

    /* ���������ݲ��뵽original_data ���ݻ������� */
    iRet = InsertObjData(pstFlashDataInfo, pstObjDataInfo, pcBuf, ulCount, iFlag);
    if (RET_FAILED(iRet))
    {
        FLASH_TRACE("calling function \"InsertObjData\" failure \n");
        goto FREE_MEMORY;
    }

    /* ���¶�������ͷ�� */
    pstObjDataInfo->enObjID = enObjId;

    /* ���޸ĺ������д��original_data�ļ��� */
    iRet = SaveFlashFileData(ORIGINAL_DATA_PATH, pstFlashDataInfo);
    if (RET_FAILED(iRet))
    {
        FLASH_TRACE("Error: calling fuction \"SaveFlashFileData\" failure \n");
    }


FREE_MEMORY:
    free(pstFlashDataInfo);
FREE_SEM:
    semop(s_iSemId, &stFreeRes, 1); //�ͷ��ź�����Դ

    if (iRet < 0)
    {
        return TBS_FAILED;
    }
    else
    {
        return TBS_SUCCESS;
    }

}

/*=========================================================================
 Function:		int app_item_get( void *data , char *item_name ,unsigned short *len )

 Description:		��ȡָ����Ŀ������
 Calls:			��
 Data Accessed:
 Data Updated:
 Input:			*item_name:	 ��Ŀ����
 				*data:		��Ŀ���ݵĴ��ָ��
 Output:			*len:		��Ŀ��Ч���ݵĳ���
 
 Return:			ERROR_ITEM_OK 
				ERROR_MALLOC
				ERROR_NOT_FIND
				ERROR_BIG
				ERROR_ITEM_MTD
				ERROR_ITEM_IOCTRL
 Others:
 		�ϲ���ô˺���ʱ*len��������ֵ���ϲ��ܽ��յ���󳤶ȡ�
 ��� ��Ч����ʵ�ʳ��ȴ���*len��data�Ͳ���ֵ�����ҷ��ش��󣬲���
 ��Ŀ�� Ч���ݵĳ���ͨ��*len�����ϲ㡣
 �����Ч����ʵ�ʳ��Ȳ�����*len���ͽ����ݸ���data������ʵ�ʳ���ͨ
 ��*len�����ϲ�
=========================================================================*/
#ifndef _TBS_ENV_PC
int app_item_get( void *data , char *item_name ,unsigned short *len )
{
	static int		fp = 0;
	item_app_t	item_app;

	item_app.len = *len;
	item_app.name = item_name;
	item_app.data = data;

	fp = open( "/dev/mtd0" , O_RDONLY );
	if( fp < 0 )
	{
		printf("Faild to open /dev/mtd0.\n");
		return ERROR_ITEM_MTD;
	}

	if( ioctl( fp , TBS_IOCTL_ITEM_GET , &item_app ) < 0 )
	{
		printf("Faild to get item.\n");
		close( fp );
		return ERROR_ITEM_IOCTRL;
	}

	*len = item_app.len;
	close(fp);

    //printf("app_item_get: %s=%p(%d)\n", item_name, data, *len);
	return item_app.item_error;
}
#else
int app_item_get( void *data , char *item_name , unsigned short *len )
{
    long lFd = 0;
    struct stat stStat;
    unsigned long ulLen = *len;
    char *pcBuf = data;
    char acFileName[64];

    stStat.st_size = 0;

    sprintf(acFileName, "/etc/%s", item_name);
    lFd = open(acFileName, O_RDONLY);
    if (lFd < 0)
    {
        return -1;
    }

    (void)fstat(lFd, &stStat);
    if (ulLen < stStat.st_size)
    {
        *len = (unsigned short)stStat.st_size;
        return -1;
    }

    ulLen = (unsigned long)read(lFd, pcBuf, stStat.st_size);
    if ((unsigned long)(-1) == ulLen)
    {
        return -1;
    }

    return 0;
}
#endif

/*=========================================================================
 Function:		int app_item_save( void *data , char  *item_name  , unsigned short len ) 

 Description:		��ȡָ����Ŀ������

 Data Accessed:
 Data Updated:
 Input:			*item_name:	 ��Ŀ����
 				*data:		Ҫ��ŵ���Ŀ���ݵ�ָ�� 				
				len:		��Ŀ��Ч���ݵĳ���
 Output:			��
 Return:			ERROR_ITEM_OK 
				ERROR_ITEM_MALLOC				
				ERROR_ITEM_BIG
				ERROR_ITEM_REPEAT_FAIL
				ERROR_ITEM_MTD
				ERROR_ITEM_IOCTRL
 Others:
=========================================================================*/

#ifndef _TBS_ENV_PC
int app_item_save( void *data , char  *item_name  , unsigned short len )
{
	static int		fp = 0;
	item_app_t	item_app;

	item_app.len = len;
	item_app.name = item_name;
	item_app.data = data;

    //printf("app_item_save: %s=%p(%d)\n", item_name, data, len);

	fp = open( "/dev/mtd0" , O_RDONLY );

	if( fp < 0 )
	{
		printf("Faild to open /dev/mtd0.\n");
		return ERROR_ITEM_MTD;
	}

	if( ioctl( fp , TBS_IOCTL_ITEM_SAVE, &item_app) < 0 )
	{
		printf("Faild to set item.\n");
		close( fp );
		return ERROR_ITEM_IOCTRL;
	}

	close(fp);

	return item_app.item_error;
}
#else
int app_item_save( void *data , char *item_name ,unsigned short len )
{
    long lFd = 0;
    long lOffset = 0;
    char *pcBuf = data;
    unsigned long ulLen = len;
    char acFileName[64];

    sprintf(acFileName, "/etc/%s", item_name);
    lFd = open(acFileName, O_WRONLY | O_CREAT, 0666);
    if ( 0 > lFd )
    {
        return -1;
    }

    while (ulLen > 0)
    {
        lOffset = write(lFd, pcBuf, ulLen);
        if (lOffset == -1 )
        {
            close(lFd);

            return -1;
        }
        else if (lOffset < (long)ulLen )
        {
            pcBuf += lOffset;
        }
        else
        {
            ;
        }
        ulLen -= lOffset;
    }

    close(lFd);
    return 0;
}
#endif


/*=========================================================================
 Function:		int app_tbs_read_mac( int id , int offset , unsigned char *mac )

 Description:		��ȡָ���˿ڵ�MAC��ַ
 Calls:			��
 Data Accessed:
 Data Updated:
 Input:			id:		ָ���˿�
 				offset:	��ָ���˿��ϵ�ƫ����

 Output:			*mac:	ָ���˿ڵ�MAC��ַ

 Return:			0:	��ȡ�ɹ�
 				-1:	��ȡʧ��
 Others:
 		���ô˺�������Ϊ*mac�����ڴ�ռ�
=========================================================================*/

int app_tbs_read_mac( int id , int offset , unsigned char *mac )
{
#ifdef _TBS_ENV_PC
    char pBaseMac[] = {0x00,0x11, 0x22, 0x33, 0x44, 0x66};
    memcpy(mac, pBaseMac, 6);

    mac[4] = id % 255;
    mac[5] = offset % 255;
    return 0;
#else
	static int		fp = 0;
	mac_t		mac_app;
	unsigned char	i;

	fp = open( "/dev/mtd0" , O_RDONLY );
	if( fp < 0 )
	{
		printf("Faild to open /dev/mtd0.\n");
		return -1;
	}

	mac_app.id = id;
	mac_app.offset= offset;

	if( ioctl( fp , TBS_IOCTL_MAC_READ, &mac_app) < 0 )
	{
		printf("Faild to get mac.\n");
		close( fp );
		return -1;
	}
	close(fp);

	for( i = 0 ; i < 6 ; i++ )
	{
		mac[i] = mac_app.mac[i];
	}

	return 0;
#endif
}



//get region code
int app_tbs_read_region(unsigned char *region)
{
	static int		fp = 0;
	region_t		region_app;
	unsigned char	i;

	fp = open( "/dev/mtd0" , O_RDONLY );
	if( fp < 0 )
	{
		printf("Faild to open /dev/mtd0.\n");
		return -1;
	}

	if( ioctl( fp , TBS_IOCTL_REGION_READ, &region_app) < 0 )
	{
		printf("Faild to get region.\n");
		close( fp );
		return -1;
	}
	close(fp);

	for( i = 0 ; i < 2 ; i++ )
	{
		region[i] = region_app.region[i];
	}

	return 0;
}

#ifdef PINANDOTHER_ENABLED
//get pin code
int app_tbs_read_pinandother(unsigned char *pinandother)
{
    static int      fp = 0;
    pin_t        pin_app;
    unsigned char   i;
    fp = open( "/dev/mtd0" , O_RDONLY );
    if( fp < 0 )
    {
        printf("Faild to open /dev/mtd0.\n");
        return -1;
    }

    if( ioctl( fp , TBS_IOCTL_PIN_READ, &pin_app) < 0 )
    {
        printf("Faild to get region.\n");
        close( fp );
        return -1;
    }
    close(fp);

    /*pinandotherԤ��64λ������ȡ��ǰ8λ��ΪPIN��*/
    for( i = 0 ; i < PINANDOTHER_LEN ; i++ )
    {
        pinandother[i] = pin_app.pinandother[i];
        //printf("apps, pinandother[%d]=%d\n", i,pinandother[i]);
    }
    return 0;
}
#endif

#ifdef CONFIG_APPS_UPDATE_UBOOT //add by wyh start at 2016-01-26 to support the function that update the uboot while the kernel up
/*=========================================================================
 Function:		int app_tbs_update_uboot( )

 Description:		����uboot
 Calls:			��
 Data Accessed:
 Data Updated:
 Input:			
 Output:			
 Return:			0:	��ȡ�ɹ�
 				-1:	��ȡʧ��
 Others:
 		
=========================================================================*/

int app_tbs_update_uboot()
{

	static int		fp = 0;
	int		ret = 0;

	fp = open( "/dev/mtd0" , O_RDONLY );
	if( fp < 0 )
	{
		printf("Faild to open /dev/mtd0.\n");
		return -1;
	}

	if( ioctl( fp , TBS_IOCTL_UPDATE_UBOOT, &ret) < 0 )
	{
		printf("Faild to update uboot.\n");
		close( fp );
		return -1;
	}
	close(fp);

	return ret;
}
#endif

