/* commhandle.c */

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>
#include <errno.h>
#include <ctype.h>

#include "commhandle.h"
#include "cmmif.h"
#include "tbsutil.h"

/*
*	函数名称: 	CMM_GetStrColumn
*	函数功能: 	从CMM获取字符串列值。
*	参数:		cInstancePrefix:[in] 全对象实例名称。
*				cNode:			[in] 字段对应的TR069节点名。
*				cValue:			[out] 存放节点值的缓冲区。
*				uSize:			[in] 缓冲区大小。
*				pulLen:			[out] 实际获取到的字符串长度(不包括结尾的0字符)
*	返回值  :	CMM_SUCCESS - 成功
*				其他错误代码。
*	创建    :	刘章平 / 2008-05-29
*/
int CMM_GetStrColumn(const char* cInstancePrefix, const char* cNode,
	char* cValue, unsigned int uSize, size_t* pulLen, char **pv, int pvLen)
{
	char* pcBuff = NULL;
	int iRet = CMM_SUCCESS;
	char cNodeName[CMM_MAX_NODE_LEN];
	size_t iStrLen = 0;

	if (uSize==0) {
		return CMM_FAIL;
	}
	if(!cValue) return CMM_FAIL;
	memset(cValue, 0, uSize);

	/*
	* 注意:CMM_GetStr()要求值缓冲区大小包含0字符结尾，所以此处分配内存来
	* 保存包含0字符结尾的字符串。SNMP消息中字符串是没有0字符结尾的。
	*/
	pcBuff = (char*)malloc(uSize+1);
	if (!pcBuff) {
		iRet = CMM_ERR_MEM;
		TR64FE_TRACE("err: not enough memory!\n");
		goto EXIT1;
	}

	sprintf(cNodeName, "%s%s", cInstancePrefix, cNode);
	iRet = CMM_GetStr(cNodeName, pcBuff, uSize+1, pv, pvLen);
	if (iRet!=CMM_SUCCESS) {
		goto EXIT2;
	}
	iStrLen = strlen(pcBuff);
	if(pulLen) *pulLen = iStrLen;
	memcpy(cValue, pcBuff, iStrLen);

EXIT2:
	free(pcBuff);
EXIT1:
	return iRet;
}


/*
*	函数名称: 	CMM_SetStrColumn
*	函数功能: 	设置字符串列值到CMM。
*	参数:		cInstancePrefix:[in] 全对象实例名称。
*				cNode:			[in] 字段对应的TR069节点名。
*				cValue:			[in] 字符串列值。
*				uSize:			[in] 字符串长度(不包括结尾的0字符)
*	返回值  :	CMM_SUCCESS - 成功
*				其他错误代码。
*	创建    :	刘章平 / 2008-05-29
*/
int CMM_SetStrColumn(const char* cInstancePrefix, const char* cNode,
	const char* cValue, size_t uSize, char **pv, int pvLen)
{
	char* pcBuff = NULL;
	int iRet = CMM_SUCCESS;
	char cNodeName[CMM_MAX_NODE_LEN];

	/*
	* 注意:由SNMP消息传进来的字符串类型，不是0字符结尾的，所以
	* 此处需要拷贝出来，并加上0字符结尾，然后再调用CMM_SetStr()。
	*/
	pcBuff = (char*)malloc(uSize+1);
	if (!pcBuff) {
		iRet = CMM_ERR_MEM;
		TR64FE_TRACE("err: not enough memory!\n");
		goto EXIT1;
	}
	memcpy(pcBuff, cValue, uSize);
	pcBuff[uSize] = 0;

	sprintf(cNodeName, "%s%s", cInstancePrefix, cNode);
	iRet = CMM_SetStr(cNodeName, pcBuff, pv, pvLen);
	free(pcBuff);

EXIT1:
	return iRet;
}

/*
*	函数名称: 	CMM_GetIntColumn
*	函数功能: 	从CMM获取整数型列值。
*	参数:		cInstancePrefix:[in] 全对象实例名称。
*				cNode:			[in] 字段对应的TR069节点名。
*				pulValue:		[out] 存放整数型节点值的缓冲区。
*	返回值  :	CMM_SUCCESS - 成功
*				其他错误代码。
*	创建    :	刘章平 / 2008-05-29
*/
int CMM_GetIntColumn(const char* cInstancePrefix, const char* cNode, u_long* pulValue, char **pv, int pvLen)
{
	int iRet;
	char cNodeName[CMM_MAX_NODE_LEN];
	char cNodeValue[256];

	*pulValue = 0;
	sprintf(cNodeName, "%s%s", cInstancePrefix, cNode);
	iRet = CMM_GetStr(cNodeName, cNodeValue, sizeof(cNodeValue), pv, pvLen);
	if (iRet!=CMM_SUCCESS) {
		return CMM_FAIL;
	}
	*pulValue = atoi(cNodeValue);
	return iRet;
}

/*
*	函数名称: 	CMM_SetIntColumn
*	函数功能: 	设置整数型列值到CMM。
*	参数:		cInstancePrefix:[in] 全对象实例名称。
*				cNode:			[in] 字段对应的TR069节点名。
*				ulValue:		[in] 整数值。
*	返回值  :	CMM_SUCCESS - 成功
*				其他错误代码。
*	创建    :	刘章平 / 2008-05-29
*/
int CMM_SetIntColumn(const char* cInstancePrefix, const char* cNode,
	u_long ulValue, char **pv, int pvLen)
{
	int iRet = CMM_SUCCESS;
	char cNodeName[CMM_MAX_NODE_LEN];
	char cNodeValue[256];

	sprintf(cNodeName, "%s%s", cInstancePrefix, cNode);

	sprintf(cNodeValue, "%lu", ulValue);
	iRet = CMM_SetStr(cNodeName, cNodeValue, pv, pvLen);
	return iRet;
}

/*
 ***************************************************************************
 *
 * LIST Handle Method
 *
 ***************************************************************************/
/*
  parameter: l -  pointer to the list in which data will be inserted
                   data - ponter to the data to be inserted
                   len - data size
                   refBytes - how many bytes to be compared
*/
void TW_addList (char *l, char *data, int len, int refBytes)
{
  lstList_t* list = (lstList_t*)l;

  // create a new node and the data that goes in it
  nwlNode = malloc(sizeof(struct lstNode));
  nwlNode->data = malloc(len);
  memcpy (nwlNode->data, data, len);

  // this deals with an empty list
  if( LIST_EMPTY ( list )) 
  {
    LIST_INSERT_HEAD (list, nwlNode, nodes);
    return;
  }

  // this deals with UIDs that match
  for(hdlNode=LIST_FIRST(list); hdlNode!=NULL; hdlNode=LIST_NEXT(hdlNode, nodes)) 
  {
    if(memcmp(data, hdlNode->data, refBytes) == 0 ) // found matching UID
    {
      LIST_INSERT_AFTER (hdlNode, nwlNode, nodes);
      if (hdlNode->data) free(hdlNode->data);
      LIST_REMOVE (hdlNode, nodes);
      free (hdlNode);
      return;
    }
  }

  // this deals with inserting a new UID in the list
  for(hdlNode=LIST_FIRST(list); hdlNode!=NULL; hdlNode=LIST_NEXT(hdlNode, nodes))
  {
    lstNode = hdlNode;
    if (memcmp(hdlNode->data, data, refBytes) > 0 ) // old ID > new ID AND
    {
      LIST_INSERT_BEFORE (hdlNode, nwlNode, nodes);
      return;
    }
  }

  // this deals with a UID that needs to go on the end of the list
  LIST_INSERT_AFTER (lstNode, nwlNode, nodes);

  return;
} 

void TW_flushList ( char *l )
{
    lstList_t*	list;
    list = (lstList_t*)l;

    while ( !LIST_EMPTY ( list ))
    {
        hdlNode = LIST_FIRST ( list );
        if ( hdlNode->data )
        {
            free ( hdlNode->data );
        }
        LIST_REMOVE ( hdlNode, nodes );
        free ( hdlNode );
    }
}

void TW_copyList( char *d, char *s, int dataLen )
{
    lstList_t *dList = (lstList_t*)d;
    lstList_t *sList = (lstList_t*)s;

    if(LIST_EMPTY(sList) || dataLen<1 ||!d || !s)
        return;

    for(hdlNode=LIST_FIRST(sList); hdlNode!=NULL; hdlNode=LIST_NEXT(hdlNode, nodes)) 
    {
        if(hdlNode->data)
        {
            nwlNode = malloc(sizeof(struct lstNode));
            nwlNode->data = malloc(dataLen);
            memcpy (nwlNode->data, hdlNode->data, dataLen);
            
            // this deals with an empty list
            if( LIST_EMPTY ( dList )) 
            {
                LIST_INSERT_HEAD (dList, nwlNode, nodes);
            }
            else LIST_INSERT_AFTER (lstNode, nwlNode, nodes);
            lstNode = nwlNode;
        }
    }
}

