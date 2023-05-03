
/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : cfg_get_usb_path.c
 文件描述 : cfg 初始化时获取usb恢复配置的文件路径
 函数列表 :

 修订记录 :
          1 创建 : 陈跃东
            日期 : 2009-04-06
            描述 :

**********************************************************************/

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cfg_prv.h"

#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif


#define CFG_USB_PATH   "/mnt"

#define CFG_USB_BACK_DIR "e8_Config_Backup"  /* 固定的配置文件存放目录名 */

#define CFG_USB_RESTORE_PATH "InternetGatewayDevice.DeviceInfo.X_TWSZ-COM_Restore.Enable"
#define CFG_PRODUCT_TYPE "InternetGatewayDevice.DeviceInfo.ModelName"

#define CFG_USB_MOUNT_FLAG  "/var/run/usb/sda"
#define CFG_USB_MOUNT_OK_FLAG  "/var/run/hotplug_mass.pid"
#define CFG_USB_INIT_WAITE_TIME  6



/*************************************************************************
Function:      CFG_RET CFG_GetUsbInitPath(char *pcPath)
Description:   获取usb盘中的配置文件路径(含文件名)
Calls:         无
Data Accessed:
Data Updated:
Input:
Output:        pcPath, 输出路径的缓冲区
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_GetUsbInitPath(char *pcPath)
{
    const char *pcVal = NULL;
    CFG_RET ret = CFG_OK;
    char acPath[CFG_MAX_PATH_LEN];
    char acExpectFile[32];
    DIR *pstDir = NULL;
    struct dirent *pstDirEnt= NULL;
    struct stat stStat;

    DIR *pstDir2 = NULL;
    struct dirent *pstDirEnt2= NULL;
    unsigned long ulTime = 0;

    /* 读取 InternetGatewayDevice.DeviceInfo.X_TWSZ-COM_Restore */
    ret = CFG_GetNodeValPtr(CFG_USB_RESTORE_PATH, &pcVal, NULL);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    if ('0' == pcVal[0] || '\0' == pcVal[0])  /* 值为0 */
    {
        /* 直接返回 */
        pcPath[0] = '\0';
        return CFG_OK;
    }

    /* 取设备类型 */
    ret = CFG_GetNodeValPtr(CFG_PRODUCT_TYPE, &pcVal, NULL);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    sprintf(acExpectFile, "ctce8_%s.cfg", pcVal);

    while (ulTime++, ulTime < CFG_USB_INIT_WAITE_TIME)
    {
        if (0 == access(CFG_USB_MOUNT_FLAG, F_OK))
        {
            if (0 != access(CFG_USB_MOUNT_OK_FLAG, F_OK))
            {
                //break;
            }
        }
        (void)sleep(1);
    }
    if (ulTime > CFG_USB_INIT_WAITE_TIME)
    {
        CFG_ERR(ERR_CFG_TIME_OUT);
        pcPath[0] = '\0';
    }

    /* 取 /mnt/下所有目录 */
    pstDir = opendir(CFG_USB_PATH);
    if (NULL == pstDir)
    {
        CFG_ERR(ERR_CFG_FILE_OPEN);
        return ERR_CFG_FILE_OPEN;
    }

    while (pstDirEnt = readdir(pstDir), NULL != pstDirEnt)  /* 下一个目录 */
    {
        if (pstDirEnt->d_reclen > 3
            && 'u' == pstDirEnt->d_name[0] && 's' == pstDirEnt->d_name[1]
            && 'b' == pstDirEnt->d_name[2])  /* 以usb打头的目录 */
        {
            /* 取 其下名为 e8_Config_Backup 的目录 */
            sprintf(acPath, CFG_USB_PATH "/%s/" CFG_USB_BACK_DIR, pstDirEnt->d_name);

            /* 检查是否是文件夹 */
            ret = stat(acPath, &stStat);
            if (-1 == ret)
            {
                continue;
            }

            if (!S_ISDIR(stStat.st_mode))
            {
                continue;
            }

            pstDir2 = opendir(acPath);
            if (NULL == pstDir2)
            {
                continue;
            }

            while (pstDirEnt2 = readdir(pstDir2), NULL != pstDirEnt2)  /* 下一个文件 */
            {

                if (0 == strcmp(pstDirEnt2->d_name, acExpectFile)) /* 文件名称符合 */
                {
                    /* 拼装完整路径 */
                    sprintf(pcPath, CFG_USB_PATH "/%s/" CFG_USB_BACK_DIR "/%s",
                            pstDirEnt->d_name, pstDirEnt2->d_name);

                    /* 检查是否是文件夹 */
                    ret = stat(pcPath, &stStat);
                    if (-1 == ret)
                    {
                        continue;
                    }

                    if (S_ISDIR(stStat.st_mode))
                    {
                        continue;
                    }
                    if(pstDir != NULL)
                    (void)closedir(pstDir);
                    if(pstDir2 != NULL)
                    (void)closedir(pstDir2);
                    return CFG_OK ;
                }
            }
        }
    }
    if(pstDir != NULL)
    (void)closedir(pstDir);
    if(pstDir2 != NULL)
    (void)closedir(pstDir2);

    CFG_ERR(ERR_CFG_FILE_OPEN);
    pcPath[0] = '\0';
    return CFG_OK;
}




#ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif


