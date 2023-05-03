
#include "cfg_comm.h"
#include "cfg_file.h"

#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif

int ByteNeedCvt = 0;

int main(int argc, char *argv[])
{
    char *pcBuf = NULL;
    unsigned long ulLen = 0;
    CFG_RET ret = CFG_OK;

    if (argc < 3)
    {
        CFG_ERR(-1, "Please Input the source xml file...");
        return -1;
    }

    if (!strcmp(argv[3], "-BYTE_NEED_CVT"))
    {
        ByteNeedCvt = 1;
    }
    else
    {
        ByteNeedCvt = 0;
    }

    /* ��ȡ�ļ� */
    ret = CFG_ReadFile(argv[1], &pcBuf, 0, 0, &ulLen);
    CFG_COMM_ERR_PROC(ret, "%s", argv[1]);

    /* ���ú���д��flash�ļ� */
    ret = CFG_SaveCurCfg(argv[2], pcBuf, ulLen);
    free(pcBuf);
    CFG_COMM_ERR_PROC(ret, "%s", argv[3]);

    return 0;
}






#ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif


