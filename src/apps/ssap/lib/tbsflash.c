/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : tbs_flash.c
 文件描述 : 配置信息条目化保存应用层接口


 函数列表 :
				app_item_get
				app_item_save

 修订记录 :
          1 创建 : 轩光磊
            日期 : 2008-10-27
            描述 :

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


/* 检查对象ID的合法性 */
#define CHECK_OBJ_ID(enObjId) \
        ((enObjId) < OBJ_LOGGER || (enObjId) > OBJ_ETHLAN ? 0 : 1)

/* 测试联合semun是否已经定义 */
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

    /* 计算键值 */
    iKey = ftok(ORIGINAL_DATA_PATH, 1);
    if (iKey < 0)
    {
        FLASH_TRACE("ftok error \n");
        return TBS_FAILED;
    }

    /* 获得信号量ID */
    s_iSemId = semget(iKey, 1, iFlag | IPC_EXCL);
    if (s_iSemId >= 0)
    {   /* 初始化信号量 */
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
        /* 获取已存在的信号量ID */
        s_iSemId = semget(iKey, 1, iFlag);
        if (s_iSemId < 0)
        {
            FLASH_TRACE("semget error \n");
            return TBS_FAILED;
        }

        /* 检查信号量是否已经被初始化 */
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
        {   /* 信号量已经被初始化 */
            FLASH_TRACE("semaphore has been initialized by other process\n");
            return TBS_SUCCESS;
        }
        else
        {
            /* 信号量还没有被初始化 */
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
* 函数名称: InitOrigiDataBuf
* 函数功能: 用于对original_data 文件数据缓冲区进行初始化
* 输入:     存储original_data 文件数据的缓冲区指针
* 输出:     成功 - 返回0
*           失败 - 返回-1
* 创建时间:
* 创建者:   徐晓东
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
* 函数名称: GetFlashFileData
* 函数功能: 从original_data 文件中读取所有的数据到动态缓冲区中
* 输入:     iFd - original_data的文件描述符，调用该函数前，
*           文件保证已经被打开
*
*           ppstFlashDataInfo - 从original_data文件中读出的数据
*           存放在*ppstFlashDataInfo指向的动态缓冲区中
*
* 输出:     成功 - 返回0
*           失败 - 返回-1
* 创建时间:
* 创建者:   徐晓东
**********************************************************************/
static int GetFlashFileData(const char *pcPath, ST_FLASH_DATA_INFO **ppstFlashDataInfo)
{
    int   iRet = -1;
    int   iFd = -1;

    /* 参数检查 */
    if (NULL == pcPath || NULL == ppstFlashDataInfo)
    {
        FLASH_TRACE("Error: invalid parameters \n");
        return TBS_FAILED;
    }

    /* 打开文件 */
    iFd = open(pcPath, O_RDONLY);
    if (iFd < 0)
    {
        FLASH_TRACE("Error: open file %s error \n", pcPath);
        return TBS_FAILED;
    }

    /* 分配动态存储区用于存储整个original_data文件中的数据*/
    *ppstFlashDataInfo = (ST_FLASH_DATA_INFO *)malloc(MAX_FLASH_LEN);
    if (NULL == *ppstFlashDataInfo)
    {
        FLASH_TRACE("malloc error \n");
        close(iFd);
        return TBS_FAILED;
    }

    memset((void *)(*ppstFlashDataInfo), 0, MAX_FLASH_LEN);

    /*从flash中读出数据到动态存储区中 */
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
* 函数名称: SaveFlashFileData
* 函数功能: 将数据写入到original_data文件中
* 输入:     iFd - original_data的文件描述符，调用该函数前，
*           文件保证已经被打开
*
*           pstFlashDataInfo - 写入数据的缓冲区指针
*
* 输出:     成功 - 返回0
*           失败 - 返回-1
* 创建时间:
* 创建者:   徐晓东
***********************************************************************/
static int SaveFlashFileData(const char *pcPath, const ST_FLASH_DATA_INFO *pstFlashDataInfo)
{
    int           iFd  = -1;
    int           iRet = -1;
    unsigned long ulLen = 0;

    /* 参数检查 */
    if (NULL == pcPath || NULL == pstFlashDataInfo)
    {
        FLASH_TRACE("Error: invalid parameters \n");
        return TBS_FAILED;
    }

    /* 打开文件 */
    iFd = open(pcPath, O_WRONLY|O_TRUNC);
    if (iFd < 0)
    {
        FLASH_TRACE("Error: open file %s error \n", pcPath);
        return TBS_FAILED;
    }

    /* 将数据写入到文件中 */
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
* 函数名称: FindObjData
* 函数功能: 在文件数据缓冲区中寻找对象数据
* 输入:     pstFlashDataInfo - 文件数据缓冲区指针
*           enObjId  - 要寻找的对象ID
*           ppstObjDataInfo -  *ppstObjDataInfo 最后被设置为指向对象数据的头部
*
* 输出:     成功 - 返回0
*           失败 - 返回-1
* 创建时间:
* 创建者:   徐晓东
***********************************************************************/
static int FindObjData(const ST_FLASH_DATA_INFO *pstFlashDataInfo, EN_OBJ_ID enObjId,
                          ST_OBJ_DATA_INFO **ppstObjDataInfo)
{
    int  iLoop = 0;

    /* 参数检查 */
    if (NULL == pstFlashDataInfo || NULL == ppstObjDataInfo ||
        0 == enObjId || !CHECK_OBJ_ID(enObjId))
    {
        FLASH_TRACE("Error: invalid parameters \n");
        return TBS_FAILED;
    }

    /* 寻找访问对象的数据 */
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
* 函数名称: InsertObjData
* 函数功能: 将对象数据插入到original_data文件数据的缓冲区中
* 输入:     pstFlashDataInfo - original_data文件数据缓冲区指针
*           pstObjDataInfo -   在文件数据缓冲区中，对象数据头部的指针
*           pcObjData - 对象数据缓冲区指针
*           ulLen   -   要写入的对象数据长度
*           iFlag  -  对象数据写入的方式
*
* 输出:     成功 - 返回0
*           失败 - 返回-1
* 创建时间:
* 创建者:   徐晓东
***********************************************************************/
static int InsertObjData(ST_FLASH_DATA_INFO *pstFlashDataInfo, ST_OBJ_DATA_INFO *pstObjDataInfo,
                             const char *pcObjData, unsigned long ulLen, int iFlag)
{
    char *pcShiftMargin = NULL;
    char *pcTmpPointer  = NULL;
    int         iOffset = 0;
    int  iSpareSpaceLen = 0;
    int        iIncrLen = 0;


    /* 参数检查 */
    if (NULL == pstFlashDataInfo || NULL == pstObjDataInfo || NULL == pcObjData ||
        0 == ulLen || (iFlag != FLAG_TRUNC && iFlag != FLAG_APPEND))
    {
        FLASH_TRACE("Error: invalid parameters \n");
        return TBS_FAILED;
    }

    /* 计算对象数据头部的偏移量 */
    iOffset = (char *)pstObjDataInfo - pstFlashDataInfo->acData;

    if(iOffset == pstFlashDataInfo->ulDataLen)
    {   /* 访问对象数据添加到original_data文件尾部,说明访问对象数据是新添加的 */

        /* 计算剩余空间长度 */
        iSpareSpaceLen = MAX_FLASH_LEN - (sizeof(ST_FLASH_DATA_INFO) +
                         pstFlashDataInfo->ulDataLen + sizeof(ST_OBJ_DATA_INFO));
        if (iSpareSpaceLen < (int)ulLen)
        {   /*剩余空间不足*/

            FLASH_TRACE("Error: spare space is too small \n");
            return TBS_FAILED;
        }

        /* 更新Flash头部信息 */
        pstFlashDataInfo->iObjNum++;
        pstFlashDataInfo->ulDataLen += sizeof(ST_OBJ_DATA_INFO) + ulLen;

        /* 更新对象数据 */
        memcpy(pstObjDataInfo->acData, pcObjData, ulLen);

        /* 更新对象数据头部信息 */
        pstObjDataInfo->ulDataLen = ulLen;

        return TBS_SUCCESS;

    }
    else if (iOffset < (int)pstFlashDataInfo->ulDataLen  &&
             iOffset >= 0)
    {   /* 访问对象数据已经存在 */

        /* 计算剩余空间长度 */
        iSpareSpaceLen = MAX_FLASH_LEN - (sizeof(ST_FLASH_DATA_INFO) + pstFlashDataInfo->ulDataLen);

        /* 计算新增字节数，即需要移动的字节数 */
        iIncrLen = (FLAG_TRUNC == iFlag) ? (ulLen - pstObjDataInfo->ulDataLen) : ulLen;

        if(iIncrLen > 0 && iSpareSpaceLen < iIncrLen)
        {   /* 剩余空间不足 */
            FLASH_TRACE("Error: spare space is too small \n");
            return TBS_FAILED;
        }

        if (0 == iIncrLen)
        {
             ; //空语句
        }
        else if (iIncrLen > 0)
        {   /* 新增字节数为正值 */

            pcTmpPointer = pstFlashDataInfo->acData + pstFlashDataInfo->ulDataLen - 1;
            pcShiftMargin  = pstObjDataInfo->acData + pstObjDataInfo->ulDataLen;

            /* 移动数据，为新增的对象数据留出足够的存储空间*/
            while (pcTmpPointer >= pcShiftMargin)
            {
                *(pcTmpPointer + iIncrLen) = *pcTmpPointer;
                pcTmpPointer--;
            }
        }
        else
        {   /* 新增字节数为负值，即对象数据长度变短了 */

            FLASH_TRACE("IncrLen is smaller than 0 \n");
            pcTmpPointer = pstObjDataInfo->acData + pstObjDataInfo->ulDataLen;
            pcShiftMargin = pstFlashDataInfo->acData + pstFlashDataInfo->ulDataLen -1;

            while (pcTmpPointer <= pcShiftMargin)
            {
                *(pcTmpPointer + iIncrLen) = *pcTmpPointer;
                pcTmpPointer++;
            }
        }

        /* 更新Flash头部信息 */
        pstFlashDataInfo->ulDataLen += iIncrLen;

        /* 更新对象数据 */
        (FLAG_TRUNC == iFlag) ? memcpy(pstObjDataInfo->acData, pcObjData, ulLen) : \
            memcpy(pstObjDataInfo->acData+pstObjDataInfo->ulDataLen, pcObjData, ulLen);

        /* 更新对象数据头部信息 */
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
* 函数名称: ReadOrigiData
* 函数功能: 从/proc/llconfig/original_data文件中读取属于访问对象的数据
* 输入:  enObjId - 访问对象ID，用于标识访问original_data的某个对象
*        buf     - 存储读取数据的缓冲区
*        ulCount - 期望读取的数据长度
* 输出:  成功 - 返回实际读取到的数据字节数目
*        失败 - 返回 -1
* 创建时间: 2008-6-12
* 创建者:   徐晓东
***********************************************************************/
int ReadOrigiData(EN_OBJ_ID enObjId, char *pcBuf, unsigned long ulCount)
{
    ST_FLASH_DATA_INFO *pstFlashDataInfo = NULL;
    ST_OBJ_DATA_INFO   *pstObjDataInfo = NULL;
    struct sembuf      stAskForRes = {0, -1, SEM_UNDO};
    struct sembuf      stFreeRes = {0, 1, SEM_UNDO};
    int                iRet = -1;

    /*参数检查 */
    if (NULL == pcBuf || 0 == ulCount || !CHECK_OBJ_ID(enObjId))
    {
        FLASH_TRACE("Error: Invalid parameters \n");
        return TBS_FAILED;
    }

    FLASH_TRACE("semaphore ID = %d \n",s_iSemId);
    /* 进程是否已经获得了信号量ID */
    if (s_iSemId < 0)
    {
        FLASH_TRACE("process has not got semaphore ID \n");
        iRet = InitSem();  //初始化信号量
        if (RET_FAILED(iRet))
        {
            FLASH_TRACE("InitSem error \n");
            return TBS_FAILED;
        }
    }

    /* 请求信号量资源 */
    iRet = semop(s_iSemId, &stAskForRes, 1);
    if (iRet < 0)
    {
        FLASH_TRACE("semop--ask for resource error \n");
        return TBS_FAILED;
    }

    FLASH_TRACE("semop--ask for resource successfully \n");

    /* 从original_data 文件中获取数据放入动态缓冲区中 */
    iRet = GetFlashFileData(ORIGINAL_DATA_PATH, &pstFlashDataInfo);
    if (RET_FAILED(iRet))
    {
        FLASH_TRACE("Error: calling function \"GetFlashFileData\" failure \n");
        goto FREE_SEM;
    }

    /* 从文件数据动态缓冲区中寻找对象数据 */
    iRet = FindObjData(pstFlashDataInfo, enObjId, &pstObjDataInfo);
    if (iRet < 0)
    {
        FLASH_TRACE("Error: calling function \"FindObjData\" failure \n");
        goto FREE_MEMORY;
    }
    else if (TRUE == (BOOL)iRet)
    {   /* 找到对象数据 */

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
    {   /* 没有找到对象数据 */
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
* 函数名称: WriteOrigiData
* 函数功能: 将访问对象的数据写入/proc/llconfig/original_data文件
* 输入:  enObjId -  访问对象ID，用于标识访问original_data的某个对象
*        buf     -  将要写入数据的缓冲区
*        ulCount -  将要写入数据的长度
*        iFlag   -  用于指明写入数据方法的标志
* 输出:  成功 - 返回0
*        失败 - 返回-1
* 创建时间: 2008-6-13
* 创建者:   徐晓东
***********************************************************************/
int WriteOrigiData(EN_OBJ_ID enObjId, const char *pcBuf, unsigned long ulCount, int iFlag)
{
    ST_FLASH_DATA_INFO *pstFlashDataInfo = NULL;
    ST_OBJ_DATA_INFO   *pstObjDataInfo = NULL;
    struct sembuf      stAskForRes = {0, -1, SEM_UNDO};
    struct sembuf      stFreeRes = {0, 1, SEM_UNDO};
    int                iRet = -1;

    /* 参数检查 */
    if (NULL == pcBuf || 0 == ulCount || !CHECK_OBJ_ID(enObjId) ||
        (iFlag != FLAG_APPEND && iFlag != FLAG_TRUNC))
    {
        FLASH_TRACE("Error: Invalid parameters \n");
        return TBS_FAILED;
    }

    FLASH_TRACE("semaphore ID = %d \n",s_iSemId);
    /* 进程是否已经获得了信号量ID */
    if (s_iSemId < 0)
    {
        FLASH_TRACE("process has not got semaphore ID \n");
        iRet = InitSem();  //初始化信号量
        if (RET_FAILED(iRet))
        {
            FLASH_TRACE("InitSem error \n");
            return TBS_FAILED;
        }
    }

    /* 请求信号量资源 */
    iRet = semop(s_iSemId, &stAskForRes, 1);
    if (iRet < 0)
    {
        FLASH_TRACE("semop--ask for resource error \n");
        return TBS_FAILED;
    }

    FLASH_TRACE("semop--ask for resource successfully \n");

    /* 从original_data 文件中获取数据放入动态缓冲区中 */
    iRet = GetFlashFileData(ORIGINAL_DATA_PATH, &pstFlashDataInfo);
    if (RET_FAILED(iRet))
    {
        FLASH_TRACE("Error: calling fuction GetFlashFileData failure\n");
        goto FREE_SEM;
    }

    /* 从文件数据动态缓冲区中寻找对象数据 */
    iRet = FindObjData(pstFlashDataInfo, enObjId, &pstObjDataInfo);
    if (iRet < 0)
    {
        FLASH_TRACE("Error: calling function \"FindObjData\" failure \n");
        goto FREE_MEMORY;
    }

    /* 将对象数据插入到original_data 数据缓冲区中 */
    iRet = InsertObjData(pstFlashDataInfo, pstObjDataInfo, pcBuf, ulCount, iFlag);
    if (RET_FAILED(iRet))
    {
        FLASH_TRACE("calling function \"InsertObjData\" failure \n");
        goto FREE_MEMORY;
    }

    /* 更新对象数据头部 */
    pstObjDataInfo->enObjID = enObjId;

    /* 将修改后的数据写回original_data文件中 */
    iRet = SaveFlashFileData(ORIGINAL_DATA_PATH, pstFlashDataInfo);
    if (RET_FAILED(iRet))
    {
        FLASH_TRACE("Error: calling fuction \"SaveFlashFileData\" failure \n");
    }


FREE_MEMORY:
    free(pstFlashDataInfo);
FREE_SEM:
    semop(s_iSemId, &stFreeRes, 1); //释放信号量资源

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

 Description:		获取指定条目的数据
 Calls:			无
 Data Accessed:
 Data Updated:
 Input:			*item_name:	 条目名称
 				*data:		条目数据的存放指针
 Output:			*len:		条目有效数据的长度
 
 Return:			ERROR_ITEM_OK 
				ERROR_MALLOC
				ERROR_NOT_FIND
				ERROR_BIG
				ERROR_ITEM_MTD
				ERROR_ITEM_IOCTRL
 Others:
 		上层调用此函数时*len传下来的值是上层能接收到最大长度。
 如果 有效数据实际长度大于*len，data就不赋值，并且返回错误，并将
 条目有 效数据的长度通过*len告诉上层。
 如果有效数据实际长度不大于*len，就将数据赋给data，并将实际长度通
 过*len告诉上层
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

 Description:		获取指定条目的数据

 Data Accessed:
 Data Updated:
 Input:			*item_name:	 条目名称
 				*data:		要存放的条目数据的指针 				
				len:		条目有效数据的长度
 Output:			无
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

 Description:		获取指定端口的MAC地址
 Calls:			无
 Data Accessed:
 Data Updated:
 Input:			id:		指定端口
 				offset:	在指定端口上的偏移量

 Output:			*mac:	指定端口的MAC地址

 Return:			0:	获取成功
 				-1:	获取失败
 Others:
 		调用此函数请先为*mac分配内存空间
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

    /*pinandother预备64位，这里取出前8位作为PIN码*/
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

 Description:		升级uboot
 Calls:			无
 Data Accessed:
 Data Updated:
 Input:			
 Output:			
 Return:			0:	获取成功
 				-1:	获取失败
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

