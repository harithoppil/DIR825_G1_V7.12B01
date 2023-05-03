
/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : cfg_get_usb_path.c
 �ļ����� : cfg ��ʼ��ʱ��ȡusb�ָ����õ��ļ�·��
 �����б� :

 �޶���¼ :
          1 ���� : ��Ծ��
            ���� : 2009-04-06
            ���� :

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

#define CFG_USB_BACK_DIR "e8_Config_Backup"  /* �̶��������ļ����Ŀ¼�� */

#define CFG_USB_RESTORE_PATH "InternetGatewayDevice.DeviceInfo.X_TWSZ-COM_Restore.Enable"
#define CFG_PRODUCT_TYPE "InternetGatewayDevice.DeviceInfo.ModelName"

#define CFG_USB_MOUNT_FLAG  "/var/run/usb/sda"
#define CFG_USB_MOUNT_OK_FLAG  "/var/run/hotplug_mass.pid"
#define CFG_USB_INIT_WAITE_TIME  6



/*************************************************************************
Function:      CFG_RET CFG_GetUsbInitPath(char *pcPath)
Description:   ��ȡusb���е������ļ�·��(���ļ���)
Calls:         ��
Data Accessed:
Data Updated:
Input:
Output:        pcPath, ���·���Ļ�����
Return:        0,�ɹ�;
               ����, ʧ��
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

    /* ��ȡ InternetGatewayDevice.DeviceInfo.X_TWSZ-COM_Restore */
    ret = CFG_GetNodeValPtr(CFG_USB_RESTORE_PATH, &pcVal, NULL);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    if ('0' == pcVal[0] || '\0' == pcVal[0])  /* ֵΪ0 */
    {
        /* ֱ�ӷ��� */
        pcPath[0] = '\0';
        return CFG_OK;
    }

    /* ȡ�豸���� */
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

    /* ȡ /mnt/������Ŀ¼ */
    pstDir = opendir(CFG_USB_PATH);
    if (NULL == pstDir)
    {
        CFG_ERR(ERR_CFG_FILE_OPEN);
        return ERR_CFG_FILE_OPEN;
    }

    while (pstDirEnt = readdir(pstDir), NULL != pstDirEnt)  /* ��һ��Ŀ¼ */
    {
        if (pstDirEnt->d_reclen > 3
            && 'u' == pstDirEnt->d_name[0] && 's' == pstDirEnt->d_name[1]
            && 'b' == pstDirEnt->d_name[2])  /* ��usb��ͷ��Ŀ¼ */
        {
            /* ȡ ������Ϊ e8_Config_Backup ��Ŀ¼ */
            sprintf(acPath, CFG_USB_PATH "/%s/" CFG_USB_BACK_DIR, pstDirEnt->d_name);

            /* ����Ƿ����ļ��� */
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

            while (pstDirEnt2 = readdir(pstDir2), NULL != pstDirEnt2)  /* ��һ���ļ� */
            {

                if (0 == strcmp(pstDirEnt2->d_name, acExpectFile)) /* �ļ����Ʒ��� */
                {
                    /* ƴװ����·�� */
                    sprintf(pcPath, CFG_USB_PATH "/%s/" CFG_USB_BACK_DIR "/%s",
                            pstDirEnt->d_name, pstDirEnt2->d_name);

                    /* ����Ƿ����ļ��� */
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


