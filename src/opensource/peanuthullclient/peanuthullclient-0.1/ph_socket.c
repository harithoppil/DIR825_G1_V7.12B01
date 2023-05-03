/*****************************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : ph_socket.c
 �ļ����� : Socket������ʵ��

 �����б� :

 �޶���¼ :
            ���� :  Kevin
            ���� :  2009-11-18
            ���� :

******************************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "ph_socket.h"
#include <unistd.h>
#include <fcntl.h>

#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif

 /******************************************************************************
                        *                                    ��������                           *
 ******************************************************************************/
/******************************************************************************
����: ��UDP���ݱ���ָ����socket�����ַ��ͳ�ȥ
����: fd                          input          socket������
             pvData                   input          �������ݵ�ָ��
             iLen                       input           ���ݵĳ���
             pstSocketAddrTo     input          Ŀ��IP��ַ��ָ��
����: �������ݱ��ĳ���/��������
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
int UDP_SendTo(int fd, void *pvData, int iLen, struct sockaddr_in *pstSocketAddrTo)
{
    struct sockaddr_in stSocketAddr;
   
    stSocketAddr = *pstSocketAddrTo;
   
    return sendto(fd, pvData, iLen, 0, (struct sockaddr *)&stSocketAddr, sizeof(stSocketAddr));
}
/******************************************************************************
����: ��ָ����socket��������ȡȫ��������
����: fd                           input            socket������
             pvBuf                      output          ������������ָ��
             iLen                        input             �����������ĳ���
             pstSocketAddrFrom    input            Ŀ��IP��ָ��
����: ��ȡ�����ݵĳ���/��������
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
int UDP_ReadFromServer(int fd, char *pvBuf, int iLen, struct sockaddr_in *pstSocketAddrFrom)	
{
    struct sockaddr_in stSocketAddr = *pstSocketAddrFrom;
    int iAddrLen = 0;
    int iRet = 0;
    int iTry = 0;

    PH_TRACE_INTO_FUNC;

    PH_TRACE("host:[%s], port:%d, sock:%d\n", inet_ntoa(stSocketAddr.sin_addr), ntohs(stSocketAddr.sin_port), fd);

    iAddrLen = sizeof(stSocketAddr);
    for (iTry = 0; iTry < 15; iTry++)
    {
        iRet = recvfrom(fd, pvBuf, iLen, 0, (struct sockaddr *)&stSocketAddr, &iAddrLen);
        PH_TRACE("LINE=%d,try = %d,iRet=%d\n",__LINE__,iTry,iRet);

        if (iRet > 0)
        {
            break;
        }
       
        usleep(200000); /* 200 * 3msec */
    }

    if(-1 == iRet)
    {
        perror( "recv" );
    }

    PH_TRACE_OUT_FUNC;
   
    return iRet;
}

/******************************************************************************
����: ��ָ����socket��������ȡȫ��������
����: fd              input          socket������
             psBuf         input          �������ָ��
             iBufLen       input          һ�ζ�ȡ����󳤶�
����: ��ȡ�����ݵĳ���/��������
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
int TCP_ReadFromServer(int fd, char *psBuf, int iBufLen)
{
    char *pCurrent = psBuf;
    int iBytes = 0;
    int iCurLen = 0;

    PH_TRACE_INTO_FUNC;

    /*��һ�ζ�ȡ����ʱ4��*/
    iBytes = TCP_ReadFrom( fd, pCurrent, iBufLen-iCurLen, SOCK_READ_WAIT_SEC);
    while ( (iBytes > 0) && (iCurLen < iBufLen))
    {
        pCurrent += iBytes;
        iCurLen += iBytes;
        /*��ȡʣ������ݣ�����Ҫ��ʱ*/
        iBytes = TCP_ReadFrom( fd, pCurrent, iBufLen-iCurLen, SOCK_READ_NOWAIT);
    }
   
    psBuf[iCurLen] = '\0';
    PH_TRACE("Read from server:%s\n",psBuf);

    PH_TRACE_OUT_FUNC;

    return iCurLen;
}
/******************************************************************************
����: ��ָ����socket�����ֶ�ȡ����
����: fd              input          socket������
             psBuf         input          �������ָ��
             iBufLen       input          һ�ζ�ȡ����󳤶�
             iTimeout     input          ��ʱʱ��
����: ��ȡ�����ݵĳ���/��������
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
 int TCP_ReadFrom(int fd, char *psBuf, int iBufLen, int iTimeout)
{
    fd_set fdReadfds;
    struct timeval stTimeOut;
    int fdMax = 0;
    int iRet = 0;
    int iLen = -1;

    /*��ʼ�������ּ���*/
    FD_ZERO(&fdReadfds);
    FD_SET(fd, &fdReadfds);
    fdMax = fd;
    stTimeOut.tv_sec = iTimeout;	
    stTimeOut.tv_usec = 0;

    PH_TRACE_INTO_FUNC;

    iRet = select(fdMax + 1, &fdReadfds, NULL, NULL, &stTimeOut);

    if (-1 == iRet)
    {
        perror( "select" );
        PH_TRACE("Select failed.\n");
        return -1;
    }
    else 
    {
        if(iTimeout && ( 0 == iRet ) )
        {
            PH_TRACE("Select timeout.\n");
            return -1;
        }
        else
       {
            /*����ָ����socket�������Ƿ�ɶ� */
            if (FD_ISSET(fd, &fdReadfds))
            {
                iLen = recv(fd, psBuf, iBufLen-1, 0);
                if (iLen >= 0)
                    psBuf[iLen] = '\0';
                else
                {
                    perror( "recv" );
                    PH_TRACE("Receive failed.\n");
                    return -1;
                }
            }
            else
            {
                PH_TRACE("Do not come here.\n");
                return -1;
            }/*end of if (FD_ISSET(fd, &fdReadfds))*/
           
        }/*end of  if(iTimeout && ( 0 == iRet ) )*/
       
    }/*end of  if (-1 == iRet)*/

    PH_TRACE_OUT_FUNC;

    return iLen;
}
/******************************************************************************
����: ��TCP���ݱ���ָ����socket�����ַ��ͳ�ȥ
����: fd              input          socket������
             pvBuf         input          �������ݵ�ָ��
����: �������ݵĳ���/��������
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
int TCP_SendTo( int fd, void *pvBuf ) 
{
    fd_set fdWritefds;
    int fdMax = 0;
    int iRet = 0;
    struct timeval stTimeOut;

    PH_TRACE_INTO_FUNC;

    /*�ļ������ּ��ϼ���ʱ�ĳ�ʼ��*/
    FD_ZERO(&fdWritefds);
    FD_SET(fd, &fdWritefds);
    fdMax = fd; 
    stTimeOut.tv_sec = 2;
    stTimeOut.tv_usec = 0;

    /*����Socket�������Ƿ��д*/
    iRet = select(fdMax + 1, NULL, &fdWritefds, NULL, &stTimeOut);

    if (-1 == iRet)
    {
        perror( "select" );
        PH_TRACE("select failed.\n");
        return -1;
    }
    else 
    {
        if ( 0 == iRet )
        {
            PH_TRACE("select timeout.\n");
            return -1;
        }
        else
        {
            /* �ڹ涨ʱ���ڣ�socket�������Ƿ��д*/
            if (FD_ISSET(fd, &fdWritefds))
            {
                if (-1 == send(fd, pvBuf, strlen(pvBuf), 0) )
                {
                    perror( "send" );
                    return -1;
                }
                
                PH_TRACE("pvBuf=%s\n",(char *)pvBuf);
            }/*end of if (FD_ISSET(fd, &fdWritefds))*/
           
        }/*end of if ( 0 == iRet )*/
       
    }/*end of if (-1 == iRet)*/

    PH_TRACE_OUT_FUNC;

    return 0;
}
/******************************************************************************
����: �뻨���Ƿ���������UDP����
����: iPort         input     �˿ں�
����: socket�ӿ�/��������
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
int UDP_connect(int iPort)
{
    int fd = 0;
    long lFlags = 0;
    struct sockaddr_in stSocketAddr;

    PH_TRACE_INTO_FUNC;

    memset(&stSocketAddr, 0, sizeof(stSocketAddr));
    stSocketAddr.sin_family = AF_INET;
    stSocketAddr.sin_addr.s_addr = htonl( INADDR_ANY );
    stSocketAddr.sin_port = htons( iPort );

    /*����socket�׽���*/
    fd = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( -1 == fd )
    {
        PH_TRACE( "UDP EnableNetwork socket call failed..\n" );
        return -1;
    }
   
    if ( -1 == bind( fd, ( struct sockaddr * )&stSocketAddr, sizeof( stSocketAddr ) ) )
    {
        close(fd);
        PH_TRACE( "UDP bind call failed..\n" );
        return -1;
    }
   
    /*��Socket ���óɷ�����״̬*/
    lFlags = fcntl( fd, F_GETFL );
    lFlags = lFlags | O_NONBLOCK;
   
    if ( -1 == fcntl( fd, F_SETFL, lFlags ) )
    {
        close(fd);
        PH_TRACE( "UDP set input socket to non-blocking fail..\n" );
        return -1;
    }

    PH_TRACE_OUT_FUNC;
   
    return fd;
}
/******************************************************************************
����: �뻨���Ƿ���������TCP����
����: pcsHost         input     �����ǵ���ַ
             pcsPort         input     Э���ַ���
����: socket�ӿ�/��������
����: Kevin 
ʱ��: 2009-11-18
*******************************************************************************/
int TCP_connect(const char *pcsHost,const char *pcsPort)
{
    int iLen = 0;
    int iRet = 0;
    int iSocket = 0;
    struct hostent *pstHost;
    struct servent *pstServer;
    struct sockaddr_in stSocketAddr;

    PH_TRACE_INTO_FUNC;

    /*����socket�׽���*/
    iSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == iSocket ) 
    {
        perror("socket");
        PH_TRACE("Create socket failed.\n");
        return -1;
    }
    
    stSocketAddr.sin_family = AF_INET;
    
    /*��ȡ�����Ƿ�������IP��ַ*/
    pstHost = gethostbyname(pcsHost);
    if (0 == pstHost) 
    {
        PH_TRACE("gethostbyname server failed.\n");
        close(iSocket);
        return -1;
    }   
   
    stSocketAddr.sin_addr = *(struct in_addr *)*pstHost->h_addr_list;

    /*��ȡ�����Ƿ������Ķ˿ں�*/
    pstServer = getservbyname(pcsPort, TCP_PROTOCOL_PORT);
    if (NULL != pstServer) 
    {
        stSocketAddr.sin_port = pstServer->s_port;
    }
    else 
    {
        stSocketAddr.sin_port = htons(atoi(pcsPort));
    }

    /*����socket*/
    iLen = sizeof(stSocketAddr);
    iRet = connect(iSocket, (struct sockaddr *)&stSocketAddr, iLen);
    if ( -1 == iRet)
    {
        PH_TRACE("Connect socket failed.\n");
        close(iSocket);
        return -1;
    }

    PH_TRACE("connected to %s (%s) on port %d.\n",pcsPort,inet_ntoa(stSocketAddr.sin_addr),
			ntohs(stSocketAddr.sin_port));
			
    PH_TRACE_OUT_FUNC;

    return iSocket;
}


 #ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif
