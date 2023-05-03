#include <stdio.h>
#include <stdlib.h>
#include "tbserror.h"

typedef struct __error_info
{
  int errnum;
  const char *const msg;
}error_info;


/*
����Ҫ��:
1. ��Ӧ��TR069
2. Ӣ�Ľ���
3. ���Ľ���

*/


static const error_info error_table[] =
{
    {TR069_ERRNO_UNSUP_FUNC,      "Method not supported"},
    {TR069_ERRNO_REJECT,          "Request denied"},
    {TR069_ERRNO_CPE_ERR,         "Internal error"},
    {TR069_ERRNO_INVALID_PARAM,   "Invalid arguments"},
    {TR069_ERRNO_NO_RESOURCE,     "Resources exceeded"},
    {TR069_ERRNO_INVALID_NAME,    "Invalid parameter name"},
    {TR069_ERRNO_INVALID_TYPE,    "Invalid parameter type"},
    {TR069_ERRNO_INVALID_VAL,     "Invalid parameter value"},
    {TR069_ERRNO_WRITE_FORBID,    "Attempt to set a non-writable parameter"},
    {TR069_ERRNO_NOTI_REQ_REJ,    "Notification request rejected"},
    {TR069_ERRNO_DOWNLOAD_FAIL,   "unable to connect to the server"},
    {TR069_ERRNO_UPLOAD_FAIL,     "unable to connect to the server"},
    {TR069_ERRNO_FS_AUTHFAIL,     "File transfer server authentication failure"},
    {TR069_ERRNO_FS_NOTSUPP,      "Unsupported protocol for file transfer"},
    {TR069_ERRNO_INVALID_IGMP,      "Download failure: unable to join multicast group"},
    {TR069_ERRNO_INVALID_URL,     "Download failure: unable to contact file server"},

    {TBS_FAILED,                  "failure"},
    {TBS_MSG_SEND_ERR,            "message sending failed"},
    {TBS_OUT_OF_MEM,              "memory not enough"},
    {TBS_PARAM_ERR,               "Invalid parameter"},
    {TBS_OUT_OF_SRC,              "Resources not enough"},
    {TBS_TIME_OUT,                "timeout"},
    {TBS_COLLISION,               "collision"},
    {TBS_NULL_PTR,                "NULL pointer"},
    {TBS_LIST_NOT_EMPTY,          "List is not empty"},
    {TBS_LIST_EMPTY,              "List is empty"},
    {TBS_INVALID_INDEX,           "Invalid index"},
    {TBS_INDEX_OVERFLOW,          "Index overflow"},
    {TBS_CORRUPT_STRUCT,          "Corrupt struct"},
    {TBS_NO_INSTANCE,             "Do not found the instance"},
    {TBS_MODULE_BUSY,             "Setting up. Wait a moument"},

};

#define num_error_names  (sizeof(error_table)/sizeof(error_info))

/*=========================================================================*/
/*  ��������: tbsstrerr                                                    */
/*  ��������: ���Ҵ������Ӧ���ַ���������                                 */
/*  ��  ��  : errnum: �����룬ӦΪ����                                     */
/*  ��  ��  : huangce / 2008-03-19                                             */
/*=========================================================================*/

const char *tbsstrerr (int errnum)
{
    int i;
    const char *msg = "";
    static char buf[32];

    for(i = 0; i < num_error_names; i ++)
    {
        if(error_table[i].errnum == errnum)
        {
            msg = error_table[i].msg;
            break;
        }
    }
    if(i == num_error_names)
    {
        sprintf (buf, "Error %d", errnum);
        msg = buf;
    }
    return msg;
}


int tbserr2tr069(int tbserrno)
{
    int i;
    for(i = 0; i < num_error_names; i ++)
    {
        //TBS�д������Ǹ���
        if((error_table[i].errnum == tbserrno) && (error_table[i].errnum < 0))
        {
            //TR069���Զ���Ĵ����뷶ΧΪ9800 - 9899
            return 9900 + error_table[i].errnum;
            break;
        }
    }
    return tbserrno;
}



const ST_TBS_ERR_TABLE g_astTbsErrTable[] =
{
    {ERR_NOT_SUPPORT_FUNCTION,    "This function is not supported right now", "��ʱ��֧�����ַ���"},
    {ERR_IP_INVALID_FORMAT,       "Invalid IP format", "IP��ʽ����"},
    {ERR_PORT_INVALID_FORMAT,     "Invalid port format", "�˿ڸ�ʽ����"},
    {ERR_PORT_TOO_SMALL,          "Port cant be zero or negative,should be between 1-65535",
                                  "�˿�ֵ����Ϊ0����, Ӧ��Ϊ1-65535"},
    {ERR_PORT_TOO_BIG,            "Port should be between 1-65535",  "�˿�ֵ��Ч��Ӧ����1-65535"},
    {ERR_PRIORITY_INVALID_VALUE,  "Invalid priority value",  "���ȼ�������1-8֮��"},
    {ERR_PRIORITY_TOO_SMALL,      "Priority too low,should be between 1-8",  "���ȼ�������1-8֮��"},
    {ERR_PRIORITY_TOO_BIG,        "Priority too high,should be between 1-8",  "���ȼ�������1-8֮��"},
    {ERR_MASK_INVALID_FORMAT,     "Net mask invalid format",  "���������ʽ����"},
    {ERR_MASK_EMPTY,              "Net mask can not be emty",  "�������벻��Ϊ��"},
    {ERR_MAKS_ALL_ZERO,           "Net mask can not be zero",  "�������벻��Ϊȫ0"},
    {ERR_MASK_INVALID_VALUE,      "the left part of net mask must be continuous binary 1",  "�����������Ӧ����������1"},
    {ERR_ENABLE_EMPTY,            "enable should not be zero", "enable ����Ϊ��"},
    {ERR_ENABLE_INVALID_VALUE,    "bool val must be \"0\" or \"1\"", "����Ϊ\"0\" ����\"1\""},
    {ERR_INT_INVALID_VALUE,       "must be int value",  "����������"},
    {ERR_UINT_INVALID_VALUE,      "must be unsigned int", "������������"},
    //ԭ������Ϣ���������˲���Ϊ�鲥��ַ
    {ERR_MAC_INVALID_VALUE,       "mac must be \"xx:xx:xx:xx:xx:xx\" format and must not be broadcast,multicast and all 0 ",  "mac��ַӦ����xx:xx:xx:xx:xx:xx���ָ�ʽ�Ҳ����ǹ㲥,�鲥��ȫ0"},
    {ERR_IPLIST_TOO_MANY,         "ip number more than max limit",  "IP��������"},
    {ERR_IPLIST_INVALID_FORMAT,   "valid ip list format",  "IP �б��ʽ����"},
    {ERR_MACLIST_INVALID_FORMAT,  "valid mac list format",  "MAC �б��ʽ����"},
    {ERR_MACLIST_TOO_MANY,        "Too many MAC-adresses",  "mac ��������"},
    {ERR_INSTANCE_TOO_MANY,       "Too many instances,cant add new one",  "�Ѵ�ʵ�����ޣ����������"},
    {ERR_INSTANCE_IMMUTABLE,      "This instance can not be deleted",  "�����ò�����ɾ��"},
    {ERR_INNER_MSG_INVALID,       "inner msg format error",  "�ڲ���Ϣ��������ϵ������"},
    {ERR_INNER_MSG_REPEATED_NAME, "inner msg has repeated name",  "�ڲ���Ϣ����ͬ�����ͣ�����ϵ������"},
    {ERR_INNER_CACHE_INVALID,     "inner cache error",  "�ڲ��ṹ��������ϵ������"},
    {ERR_IP_MASK_NOT_MATCH,       "IP address does not match with the subnet mask ",  "IP��ַ���������벻ƥ��"},
    {ERR_IP_IS_HOST_ADDR,         "IP address can not be host address ",  "����Ϊ���ص�ַ"},
    {ERR_IP_IS_CAST_ADDR,         "IP address can not be broadcast address ",  "����Ϊ�㲥��ַ"},
    {ERR_IP_ADDR_START_MINOR_END, "IP end address lower then start address",  "������ַ����С�ڿ�ʼ��ַ"},
    {ERR_IP_ADDR_POOL_OVERLAP,    "IP address pool overlaps ",  "IP��ַ�س�ͻ"},
    {ERR_STR_TOO_LONG,            "String is too long ",  "�ַ�������"},
    {ERR_CAN_NOT_EMPTY,           "can not be empty ",  "����Ϊ��"},
    {ERR_GET_NODE_VALUE_FAIL,     "get node value failed ",  "��ȡ�ڵ�ֵʧ��"},
    {ERR_SET_NODE_VALUE_FAIL,     "Set node value failed ",  "���ýڵ�ֵʧ��"},
    {ERR_INVALID_DOMAIN_NAME,     "Domain name must be \"xxxxx.xxxxxx.xxxx.xxx\" format",
                                  "����Ӧ����\" xxxx.xxxx.xxxx.xxxx\" ���ָ�ʽ"},
    {ERR_INVALID_SERVER_IP,       "Invalid server IP address",
                                  "������IP��ַ��Ч"},            
	{ERR_SERVER_IP_CAN_NOT_EMPTY, "server IP can not be empty",
                                  "������IP����Ϊ��"},   
    {ERR_OUT_OF_RANGE,            "the value out of range", "ֵ����ָ���ķ�Χ"},
    {ERR_IPLIST_HAS_REPEAT,       "IP list has to repeat", "�б������ظ�IP��ַ"},
    {ERR_IP_IS_WRONG_NET_SECTION,         "wrong net section", "IP��ַ���δ���"},
    {ERR_IP_IS_CAST_OR_NET_ADDR,         "IP address can not be broadcast address or net address",  "����Ϊ�㲥��ַ�������ַ"},
    {ERR_ENABLE_DHCPS_CONFLICT_DHCPR, "DHCP couldnt perform as server and relay simultaneously", "DHCP����ͬʱ������Server��Relay"},
    {ERR_PORT_RANGE_INVALID,      "invalid portrange (min > max)", "��Ч�Ķ˿ڷ�Χ(�����˿�>��ʼ�˿�)"},
    {ERR_INVALID_VAL,           "invalid value", "��Чֵ"},
    {ERR_LIST_HAS_REPEAT,       "The list are duplicates", "�б������ظ���"},

    {ERR_MSG_MID_INVALID,       "ERR_MSG_MID_INVALID","ERR_MSG_MID_INVALID"},
    {ERR_MSG_SOCKET,            "ERR_MSG_SOCKET","ERR_MSG_SOCKET"},
    {ERR_MSG_PROC_NOT_FOUND,    "ERR_MSG_PROC_NOT_FOUND","ERR_MSG_PROC_NOT_FOUND"},
    {ERR_MSG_SEND_FAIL,         "ERR_MSG_SEND_FAIL","ERR_MSG_SEND_FAIL"},
    {ERR_MSG_DSTMID_UNREGED,    "ERR_MSG_DSTMID_UNREGED","ERR_MSG_DSTMID_UNREGED"},
    {ERR_MSG_NOT_FULL,          "ERR_MSG_NOT_FULL","ERR_MSG_NOT_FULL"},
    {ERR_MSG_PART_NOEFFECT,     "ERR_MSG_PART_NOEFFECT","ERR_MSG_PART_NOEFFECT"},
    {ERR_MSG_PART_INVALID,      "ERR_MSG_PART_INVALID","ERR_MSG_PART_INVALID"},
    {ERR_MSG_TIMEOUT,           "ERR_MSG_TIMEOUT","ERR_MSG_TIMEOUT"},
    {ERR_MSG_PART_LIST_FULL,    "ERR_MSG_PART_LIST_FULL","ERR_MSG_PART_LIST_FULL"},

    {ERR_MALLOC_FAILED,         "ERR_MALLOC_FAILED","ERR_MALLOC_FAILED"},
    {ERR_BUF_NOT_ENOUGH,        "ERR_BUF_NOT_ENOUGH","ERR_BUF_NOT_ENOUGH"},
    {ERR_INTERNAL_ERR,          "ERR_INTERNAL_ERR","ERR_INTERNAL_ERR"},
    {ERR_PARA_INVALID,          "ERR_PARA_INVALID","ERR_PARA_INVALID"},
    {ERR_FILE_OPEN,             "ERR_FILE_OPEN","ERR_FILE_OPEN"},
    {ERR_FILE_READ,             "ERR_FILE_READ","ERR_FILE_READ"},
    {ERR_FILE_WRITE,            "ERR_FILE_WRITE","ERR_FILE_WRITE"},
    {ERR_FILE_CHKSUM,           "ERR_FILE_CHKSUM","ERR_FILE_CHKSUM"},
    {ERR_BUSY,                  "ERR_BUSY","ERR_BUSY"},
    {ERR_CREATE_MSG_FAIL,       "ERR_CREATE_MSG_FAIL","ERR_CREATE_MSG_FAIL"},
    {ERR_MSG_EXT_MID_LIST_FULL, "ERR_MSG_EXT_MID_LIST_FULL","ERR_MSG_EXT_MID_LIST_FULL"},

    {ERR_CFG_ATTR_STAT,         "Stat data not support set noti",             "ͳ�����ݲ�֧������noti����"},
    {ERR_CFG_PATH_NOT_EXSITED,  "Path not exist",                           "·��������"},
    {ERR_CFG_INVALID_OBJ_VAL,   "Attemp to set val to obj",                   "��ͼ�Զ���·������ֵ"},
    {ERR_CFG_INVALID_STRING,    "Invalid string val",                         "�Ƿ��ַ���"},
    {ERR_CFG_INVALID_INT_0,     "0 must expressed by \"0\", \"+0\"\"-0\"\"00\" are all invalid", "0ֻ����\"0\"��ʾ, \"+0\"\"-0\"\"00\"����Ϊ�Ƿ�"},
    {ERR_CFG_INVALID_INT_01,    "non-zero int must begin with \"+\" or non-zero number", "��0�����������Է��Ż�������ֵ��ͷ"},
    {ERR_CFG_INVALID_INT_A,     "include invalid char",                       "���зǷ��ַ�"},
    {ERR_CFG_INVALID_UINT_0,    "0 must expressed by \"0\", \"+0\"\"-0\"\"00\" are all invalid", "0ֻ����\"0\"��ʾ, \"+0\"\"-0\"\"00\"����Ϊ�Ƿ�"},
    {ERR_CFG_INVALID_UINT_01,   "non-zero unit must begin non-zero number",   "��0�޷������������Է��Ż�������ֵ��ͷ"},
    {ERR_CFG_INVALID_UINT_A,    "include invalid char",                       "���зǷ��ַ�"},
    {ERR_CFG_INVALID_BOOL,      "bool val must be \"0\" or \"1\"",                "����Ϊ\"0\" ����\"1\""},
    {ERR_CFG_INVALID_TIME,      "dateTime val must be like \"0000-00-02T03:04:05\"", "�������� \"0000-00-02T03:04:05\""},
    {ERR_CFG_INVALID_HEX,       "Hex val must be number or A-F a-f",          "ʮ�����������������ֻ���a-f A-F"},
    {ERR_CFG_NOT_WRITABLE,      "The path is not writable",                      "��·������д"},
    {ERR_CFG_NOT_IN_ACCESSLIST, "Not in accesslist",                          "���ڿɷ����б���"},
    {ERR_CFG_PATH_NOT_INST,     "The path is not a valid inst",               "��·������һ���Ϸ��Ķ���ʵ��"},
    {ERR_CFG_NOT_STANDARD,      "The path is not a standard path",            "��·������һ����׼��·��"},
    {ERR_CFG_INVALID_TYPE,      "The node type is invalid",                   "�ڵ����ͷǷ�"},
    {ERR_CFG_ITEM_OPT_FAIL,     "App config flash opt failed",                "Flash�������ݲ���ʧ��"},
    {ERR_CFG_WILDCARD_QUE_FULL, "Wildcard queue is full",               "ͨ��·����������"},
    {ERR_CFG_INVALID_PATH,      "Invalid path",                               "�Ƿ�·����"},
    {ERR_CFG_OBJ_INST_FULL,     "The object instance count has reached the max allowed count",    "����ʵ�������Ѿ��ﵽ������������"},
    {ERR_CFG_REJECT_ATTR_SET,     "This node reject to set of active notication",    "�˽ڵ㲻������Ϊ�����ϱ�"},

    {ERR_CMM_NOMID,             "Not support the opt to this path",           "��ǰ��֧�ָ�·�����в���"},
    {ERR_CMM_INVALID_INDEX,     "Invalid index",           "��Ч������"},
    {ERR_CMM_NOLEAF,            "Invalid leaf node",           "��Ч��Ҷ�ӽڵ�"},
    {ERR_CMM_NOITEM,            "Invalid item",           "��Чʵ��"},
    {ERR_CMM_INVALID_PATH,      "Invalid path",           "��Ч��·��"},
    {ERR_CMM_TIMEOUT,           "time out",           "��ʱ"},
#ifdef CONFIG_LAST_SAVE_CFG
    {ERR_JFS_NOT_OPEN,           "JFFS2 file systems unmounted,Operation fail",           "JFFS2�ļ�ϵͳδ������,����ʧ��"},
    {ERR_SAVE_CFG,           "Save config faild",           "��������ʧ��"},
    {ERR_RECOVER_CFG,           "Recover config faild",           "�ָ�����ʧ��"},
#endif
    {ERR_DGN_INVALID_STATE,    "invalid state (must be \"Requested\")",        "�Ƿ�״ֵ̬(������\"Requested\")"},
    {ERR_DGN_INVALID_INTF,     "invalid intf (not exsited)",                 "�Ƿ��ӿ�(������)"},
    {ERR_DGN_INVALID_HOST_IP_PART, "invalid host (not support partly ip)",  "�Ƿ�����(��ǰ��֧�ֲ���ip��ַ)"},
    {ERR_DGN_INVALID_HOST_IP_OCT,  "invalid host (not support oct format ip)", "�Ƿ�����(��ǰ��֧�ְ˽��Ƶ�ip��ַ)"},
    {ERR_DGN_INVALID_HOST_BEGIN,   "invalid host (must begin with number,char or \"_\")", "�Ƿ�����(������������������ĸ����\"_\"��ʼ)"},
    {ERR_DGN_INVALID_TIMES,    "invalid Repeate times,should be 1 ~ 50",                      "�Ƿ��ظ�����"},
    {ERR_DGN_INVALID_TO,       "invalid timeout,should be 1 ~ 20",                            "�Ƿ���ʱʱ��"},
    {ERR_DGN_INVALID_SIZE,     "invalid data size,should be 1 ~ 5000",                          "�Ƿ����ݰ���С,����С��5000"},
    {ERR_DGN_INVALID_DSCP,     "invalid DSCP",                               "�Ƿ�DSCPֵ"},
    {ERR_DGN_NOHOST,          "not set host",                               "��ǰ��δ��������"},
    {ERR_DGN_INVALID_HOST_A,   "invalid host (must not contain char except \".\", \"_\", \"-\", \"@\", a-z A-Z 0-9)",    "�Ƿ�����(���������ܰ���\".\", \"_\", \"-\", \"@\", ��ĸ������֮����ַ�)"},
    {ERR_DGN_INVALID_HOST_LEN, "invalid host (too long, exceeding 256)",    "������̫��(����256���ַ�)"},
    {ERR_INVALID_FILTER_MODE,       "Invalid filter mode",                      "��Ч�Ĺ���ģʽ"},
    {ERR_INVALID_SCHEDULE_PATH,     "Invalid schedule path",                    "��Ч��schdule·��"},


    /* AUTH */
    {ERR_AUTH_CANNOT_CHANGE_USER,   "cannot modify user name","�������޸��û���"},
    {ERR_AUTH_CANNOT_CHANGE_LEVEL,  "cannot modify user name","�������޸��û�����"},
    {ERR_AUTH_CANNOT_REMOVE_ADMIN,  "cannot remove admin","������ɾ��admin�û�"},
    {ERR_AUTH_HAVE_SAME_USER,       "have same user","������ͬ�û���"},
    {ERR_AUTH_PASSWORD_TOO_LONG,    "password length must less than 32","���볤�ȱ���С��32"},
#ifdef CONFIG_APPS_LOGIC_DLink_AC750    
    {ERR_AUTH_USERNAME_TOO_LONG,    "user name length must less than 16","�û������ȱ���С��16"},
#else
    {ERR_AUTH_USERNAME_TOO_LONG,    "user name length must less than 32","�û������ȱ���С��32"},
#endif
    {ERR_AUTH_SESSION_FULL,         "Too many users logged on, please wait a moment","��¼�û�����"},
    {ERR_AUTH_WRONG_USER,           "Username wrong","������û���"},
    {ERR_AUTH_WRONG_PASSWORD,       "Password wrong","���������"},
    {ERR_AUTH_WRONG_USER_OR_PWD,    "Username or Password wrong","������û���������"},
    {ERR_AUTH_SESSION_TIMEOUT,      "Session is timeout","�Ự��ʱ"},
    {ERR_AUTH_SESSION_ILLEGAL,      "Inadequate access","����Ȩ�޲���"},
    {ERR_AUTH_LOGIN_FREQUENT,       "Please login in one minute","��һ���Ӻ��ٵ�¼"},
    {ERR_AUTH_LOGOUT_SUCCESS,       "Logout success","ע���ɹ�"},
    {ERR_AUTH_OLDPASSWORD_INVALID,  "Old password is wrong!","���������"},
    {ERR_AUTH_HAVE_SAME_PASSWORD,   "The new password is same as the old one","�������������һ��"},

    /* WAN */
    {ERR_WAN_INVALID_INNER_MSG, "inner msg error",                            "�ڲ���Ϣ����"},
    {ERR_WAN_INVALID_VAL,       "get invalid value",                          "��ȡֵ��Ч"},
    {ERR_WAN_INVALID_SVC_LIST,   "Invalid Service List",                          "�Ƿ��ķ�������"},
    {ERR_WAN_INVALID_CONN_TYPE,  "Invalid Connection Type",                       "�Ƿ�����������"},
    {ERR_WAN_INVALID_LAN_INTF,   "Invalid Lan Intf",                              "�Ƿ���lan�ڰ�"},
    {ERR_WAN_LAN_INTF_REPEAT,     "The lan intf mutil bind",                       "lan���ظ���"},
    {ERR_WAN_TOO_MORE_BRIDGE,    "Not allowed to create more than one bridge on the same vlan", "һ��wan vlan�ϲ����������������"},
    {ERR_WAN_BRIDGE_ROUTE_MIX,   "Not allowed to mix bridge and route on the same vlan",        "һ��wan vlan�ϲ��������ź�·�����ӹ�������ӵ����"},
    {ERR_WAN_TR069_BRIDGE,       "The TR069 service type is not allowed to config on a bridge conn",  "TR069�������͵�wan���Ӳ��ܲ���������"},
    {ERR_WAN_TR069_LAN,          "Not allowed to bind lan to a TR069 wan conn",   "TR069�������͵�wan���Ӳ������lan��"},
    {ERR_WAN_NOT_EXIST,          "please select a wan connection",   "��ѡ��һ��wan����"},
    {ERR_WAN_INVALID_VLAN_ID,    "Invalid Vlan ID", ""},
    {ERR_WAN_NOREOA_LINK_EXIST,  "For one PVC, only one  can be configured", ""},
    {ERR_WAN_EOA_LINK_EXIST,  "For one PVC, MER, PPPOE and Bridging are only allowed with IPOA and PPPOA", ""},
    {ERR_WAN_TOO_MANY_PVC,    "CPE can support up to 8 PVCs", ""},
    {ERR_WAN_CANNOT_CHNG_LINK_TYPE, "For one PVC, more than one MER, PPPOE or Bridging connetion exist, can not change protocol to IPOA or PPPOA", ""},
    {ERR_WAN_ETH_LINK_CONFLICT, "For one VLAN,only 1 IPOE, 1 Bridging and 4 PPPOE connection can be configured at the same time", ""},
    {ERR_WAN_DSL_LINK_CONFLICT, "For one (PVC,VLAN), only 1 MER, 1 Bridging and 4 PPPOE connections can be configured at the same time", "һ����PVC,VLAN����ֻ��ͬʱ����һ���ţ�һ��MER��������PPPOE���ӡ�"},
    {ERR_WAN_TOO_MANY_ENABLED_DSL_PPP, "CPE can support up to 4 enabled PPPOA and PPPOE connections", ""},
    {ERR_WAN_TOO_MANY_ENABLED_ETH_PPP, "CPE can support up to 4 enabled PPPOE connections", ""},
    {ERR_DSL_TOO_MANY_IP_INST, "For one (PVC,VLAN), only 1 MER, 1 Bridging connections can be configured at the same time", ""},
    {ERR_ETH_TOO_MANY_IP_INST, "For one VLAN, only 1 IPOE, 1 Bridging connections can be configured at the same time", ""},
    {ERR_WAN_UNKNOWN_DSL_TYPE, "Invalid DSL type", "��Ч��DSL����"},
    {ERR_WAN_REPEAT_VLANID, "Vlan ID repeated", "Vlan ID�ظ���"},


    /* LAN */
    {ERR_LAN_INVALID_IP_FORMAT, "Invalid IP format",             "�Ƿ�IP��ַ��ʽ"},
    {ERR_LAN_INVALID_LAN_IP, "Invalid LAN IP",             "�Ƿ���LAN IP��ַ"},
    {ERR_LAN_MACLIST_TOO_LONG, "Mac address list too long",             "mac��ַ�б�������������"},
    {ERR_LAN_INVALID_MACLIST, "Invalid Mac address",             "�����mac��ַ"},
    {ERR_LAN_IP_MASK_CONFLICT, "Net Mask conflict with IP",             "IP��ַ�����������ͻ"},
    {ERR_LAN_IP_CONFLICT, "Repeated IP",             "IP��ַ������IP�����ظ�"},
    {ERR_LAN_LAST_INSTANCE, "This is first IP config,cannot delete",             "��һ��IP���ò�����ɾ��"},
    {ERR_LAN_TOO_MANY_INSTANCE, "Too many LAN IP config",             "LAN IP�����Ѿ��дﵽ���ޣ��޷������"},
    {ERR_LAN_SAME_REGION_CONFLICT, "same net region with other lan device,will bring route error",             "LAN IP����������LAN����������ͬ���ᵼ��VLAN����"},
    {ERR_LAN_NOT_EXIST, "please select a lan interface",             "��ѡ��һ��lan����"},
    {ERR_LAN_INVALID_IPV6_FORMAT, "Invalid IPv6 address format", "�Ƿ�IPv6��ַ��ʽ"},


    /* DNS */
    {ERR_DNS_VALID_DNS_ADDR,        "invalid DNS address",            "DNS��ַ��ʽ����"},
    {ERR_DNS_HAS_REPEAT,            "DNS address repeat",             "DNS��ַ���ظ�"},
    {ERR_DNS_CUSTOM_BE_EMPTY,       "When allowed a custom DNS, DNS addresses can not be empty",
                                    "�������Զ���DNSʱ, DNS��ַ����Ϊ��"},

    /* ETHLAN & ETHWAN */
    {ERR_ETH_INVALID_BITRATE,    "invalid max bit rate",     "�Ƿ�������"},
    {ERR_ETH_INVALID_DUPLEXMODE, "invalid duplex mode",     "�Ƿ���˫��ģʽ"},

    /* LSVLAN */
    {ERR_LSVLAN_INVALID_BRIDGENAME,  "invalid bridge name",  "�Ƿ�������"},
    {ERR_LSVLAN_INVALID_VLANID,      "invalid Vlan ID",      "�Ƿ�VLAN ID"},
    {ERR_LSVLAN_INVALID_FILTERBRREF, "invalid Filter Bridge Reference",  "�Ƿ��������ù��˹���"},
    {ERR_LSVLAN_INVALID_FILTERIF,    "invalid Filter Interface",  "�Ƿ��ӿ����ù��˹���"},
    {ERR_LSVLAN_DEL_FIRST_BRIDGE,    "the first bridge should not be deleted",  "��һ���Ų���ɾ��"},
    {ERR_LSVLAN_INSTANCE_IN_USE,     "the instance which is in use should not be deleted", "����ʹ��ʵ������ɾ��"},
    {ERR_LSVLAN_BRNAME_CONFLICT,     "Bridge name conflict", "�����Ƴ�ͻ"},
    {ERR_LSVLAN_BRVLAN_CONFLICT,     "Vlan id conflict", "VLAN ID��ͻ"},
    {ERR_LSVLAN_IF_CONFLICT,         "One interface must belong to only one LAN Group", "һ���ӿ�ֻ������һ��LAN Group"},
    {ERR_LSVLAN_WLAN_IF_TAGGED,      "VAP interface should not be tagged mode", "VAP�ӿڲ�����taggedģʽ"},
    {ERR_LSVLAN_BEGIN_NAME_NON,      "Bridge name cannot be empty", "�����ֲ���Ϊ��"},

    /*LPVLAN*/
    {ERR_LPVLAN_INVALID_VLAN_ID,     "invalid Vlan ID", "�Ƿ�VLAN ID"},
    {ERR_LPVLAN_INVALID_VLAN_ID_LIST,"invalid Vlan ID list", "�Ƿ�VLAN ID�б�"},
    {ERR_LPVLAN_TOO_MANY_VLAN,       "too many Vlan ID in list", "�б���VLAN ID����"},


    /* IPCONN */
    {ERR_IPCONN_NAME_TOO_LONG,      "the length of connection name can not over 256",
                                    "�������Ƴ��Ȳ��ܳ���256"},
    {ERR_IPCONN_NAME_INVALID,       "invalid connection name (only contains the following characters: [-_.@0-9a-zA-Z] ) ",
                                    "��Ч��������(ֻ���������ַ�: [-_.@0-9a-zA-Z])"},
    {ERR_IPCONN_ADDR_TYPE_INVALID,  "invalid address type value",
                                    "��Ч�ĵ�ַ����"},
    {ERR_IPCONN_ACTIVE_CONN_OVER,   "the number of active connections to the ceiling",
                                    "��ǰ����Ӹ����ѵ�����"},
    {ERR_IPCONN_WAN_CONN_OVER,      "the number of WAN connections to the ceiling",
                                    "��ǰWAN���Ӹ����ѵ�����"},
    {ERR_IPCONN_WAN_IPCONN_OVER,    "Each WAN Device allow to create just a IP Connection",
                                    "һ��WAN�豸ֻ������һ��IP����"},
    {ERR_IPCONN_IP_GATEWAY_SAME,    "Static IP address and default gateway address can not be the same",
                                    "��̬IP��ַ��Ĭ�����ص�ַ����һ��"},
    {ERR_IPCONN_NAME_EXISTS,        "Connection Name has exists",
                                    "�����������ظ�"},
    {ERR_IPCONN_CONNTYPE_INVALID,   "invalid connection type",
                                    "��Ч����������"},
    {ERR_PPPOE_ACNAME_INVALID,   "invalid AC name",
                                    "��Ч��AC��"},
    {ERR_PPPOE_SERVICENAME_INVALID,   "invalid service name",
                                    "��Ч�ķ�����"},
    {ERR_IPCONN_IP_GATEWAY_NOT_MATCH,    "Static IP address and default gateway address is not in the same network",
                                    "��̬IP��ַ��Ĭ�����ص�ַ����ͬһ����"},
    {ERR_IPCONN_MTU_INVALID,                "invalid MTU value,the valid range is 616 to 1500 bytes",
                                            "��Ч��MTUֵ,��Чֵ��Χ��616��1500�ֽ�"},
    {ERR_IPCONN_MTU_INVALID_FORMAT, "Invalid MTU format", "MTU��ʽ����"},
	
    {ERR_IPCONN_INVALID_HOSTNAME,    "invalid host name", "��Ч��������"},
    
	{ERR_IPCONN_INVALID_RENEWRELEASE,       "invalid release or renew action",
                                            "��Ч��Release��Renew����"},

    /* DHCPS */
    {ERR_DHCPS_INVALID_CLASS_ID, "Unknown class id ", "δ֪�ͻ�����"},
    {ERR_DHCPS_EXIST_CLASS_ID, "Already exist class id ", "�Ѵ��ڵĿͻ�����"},
    {ERR_DHCPS_IFIP_ADDRPOOL_UNMATCH, "Interface address and dhcp address pool not match", "�ӿڵ�ַ���ַ�ز�ƥ��"},
    {ERR_DHCPS_LEASE_TIME_OUT_RANGE, "The lease time is out of range, should be higher than or equal to 10","��Чʱ�䳬����Χ��Ӧ�ô��ڵ���10"},
    {ERR_DHCPS_CPEIP_CONFLICT_DHCP_POLL_IPADDR, "CPE IP conflicts with DHCP pool IP addr", "����IP��DHCP��ַ�س�ͻ"},

    /* PPPOE */
    {ERR_PPPOE_NO_RESOURSE, "no resource",  "û����Դ����"},
    {ERR_PPPOE_INVALID_VAL,  "invalid value",  "��Чֵ"},
    {ERR_PPPOE_INVALID_PATH,	"invalid path", "��Ч·��"},
    {ERR_PPPOE_EXCEED_CONNECTION_NUM,	"exceeded number of connections", "����������"},
    {ERR_PPPOE_INVALID_INSTANCE,	"invalid instance", "��Чʵ��"},
    {ERR_PPPOE_ENABLE_INVALID,	"invalid enable", "��Чenable"},
    {ERR_PPPOE_USERNAME_INVALID,	"invalid  username", "��Ч�û���"},
    {ERR_PPPOE_USERNAME_OVERLENTH,	"username  over  length", "�û�������"},
    {ERR_PPPOE_USERNAME_EMPTY,	"username empty", "�û���Ϊ��"},
    {ERR_PPPOE_PASSWD_INVALID,	"invalid passwd", "��Ч����"},
    {ERR_PPPOE_PASSWD_OVERLENTH,	"passwd over length", "���볬��"},
    {ERR_PPPOE_PASSWD_EMPTY,	"passwd empty", "����Ϊ��"},
    {ERR_PPPOE_CONNETNAME_INVALID,	"connected name invalid", "��Ч����������"},
    {ERR_PPPOE_CONNETNAME_OVERLENTH,	"connected name over length", "�������Ƴ���"},
    {ERR_PPPOE_AUTH_INVALID,	"invalid auth", "��Ч�ļ�Ȩ��ʽ"},
    {ERR_PPPOE_MRU_INVALID,	"MRU invalid(576~1492)", "��Ч��MRU"},
    {ERR_PPPOE_TRIGGER_INVALID,	"invalid trigger", "��Ч�Ĵ�����ʽ"},
    {ERR_PPPOE_CONNACTION_INVALID,	"connect button invalid", "�ֶ�������ʽ�İ�ť��Ч"},
    {ERR_PPPOE_LCPECHO_INVALID,	"LCPECHO invalid, should be 5 ~ 65535", "��Ч��LCPECHO��Ӧ��Ϊ 5 ~ 65535"},
    {ERR_PPPOE_LCPECHO_RETRY_INVALID,	"LCPECHO retry invalid, should be 0 ~ 65535", "��Ч��LCPECHO���Դ�����Ӧ��Ϊ 0 ~ 65535"},
    {ERR_PPPOE_ACTIVE_CONN_OVER,	"exceed active connect", "�������õ�������"},
    {ERR_PPPOE_INVALID_INNER_MSG,	"inner message invalid", "�ڲ���Ϣ����"},
    {ERR_PPPOE_INVALID_MATCH_INDEX,	"the index of message invalid", "��Ϣ��������"},
    {ERR_PPPOE_NO_PPPSESSION, "no pppsession in flash", "û��pppsession ����flash��"},
    {ERR_PPPOE_NO_PPPSESSION_IDX, "invalid pppsession idx", "pppsession idx ����"},
    {ERR_PPPOE_ERROR_PPPSESSION_VAL, "pppsession  value error", "pppsession ����ֵ"},
    {ERR_PPPOE_NAME_REPEATED,  "WAN connected name repeated",  "WAN���������ظ�"},
    {ERR_PPPOE_IDLEDISCONNTIME_INVALID,  "invalid idle disconnect time",  "��Ч�����ó�ʱʱ��"},
    {ERR_PPPOE_APN_INVALID,                 "invalid apn",  "��Ч��APN"},
    {ERR_PPPOE_APN_EMPTY,	                "apn empty",  "APNΪ��"},
    {ERR_PPPOE_APN_OVERLENTH,	            "apn over  length",  "APN����"},
    {ERR_PPPOE_DIALNUMBER_INVALID,          "invalid dialnumber",  "��Ч�Ĳ��ź���"},
    {ERR_PPPOE_DIALNUMBER_EMPTY,            "dialnumber empty",  "���ź���Ϊ��"},
    {ERR_PPPOE_DIALNUMBER_OVERLENTH,	    "dialnumber over length",  "���ź��볬��"},
    {ERR_PPPOE_MTU_INVALID,	                "MTU invalid(576~1492)", "��Ч��MTU"},
    {ERR_PPPOE_TOO_MANY_PPPOU_INST,         "CPE can support up to 1 PPPoU connections", "CPEֻ����һ��3G����"},
    {ERR_PPPOE_IDLEDELAYTIME_INVALID,	    "Backup delay time invalid, should be 0~600", "��Ч�ı����ӳ�ʱ�䣬Ӧ��Ϊ0~600������Ҫ�ܱ�15����"},
    {ERR_PPPOE_IDLEDELAYTIME_EMPTY,	        "Backup delay time empty", "�����ӳ�ʱ��Ϊ��"},
    {ERR_PPPOE_IDLEDELAYTIME_DIVIDE,	    "Backup delay time  must be divisible by 15", "�����ӳ�ʱ������ܱ�15����"},
    {ERR_PPPOE_IP6_MTU_INVALID,	      "IPv6 MTU invalid(1280~1492)", "��Ч��IPv6  MTU"},
    {ERR_PPPOE_IP6_MRU_INVALID,	      "IPv6 MRU invalid(1280~1492)", "��Ч��IPv6  MRU"},
	{ERR_PPPOE_ADDR_MODE_INVALID,	   "invalid pppoe address mode", "��Ч��PPPOE��ַģʽ"},
	
	


    /* QOS */
#ifdef CONFIG_APPS_LOGIC_QOS_IMQ
    {ERR_QOS_INVALID_COMMITTED_RATE,    "Invalid Bandwidth value, should be 100-102400Kbps",
                                        "��Ч�Ĵ���ֵ,Ӧ��Ϊ100-102400Kbps"},
    {ERR_QOS_INVALID_PEAK_RATE,         "Invalid peak rate value, should be 100-102400Kbps",
                                        "��Ч�ķ�ֵ����ֵ,Ӧ��Ϊ100-102400Kbps"},
    {ERR_QOS_INVALID_BURST_SIZE,        "Invalid burst size, should be 5-100bytes", "��Ч��ͻ��������,Ӧ��Ϊ5-100bytes"},
    {ERR_QOS_INVALID_QUE_LEN,           "Queue length should be 10-150packets or 15000-225000bytes",
                                        "���г���Ӧ��Ϊ10-150packets��15000-225000bytes"},
    {ERR_QOS_INVALID_QUE_WEIGHT,        "Invalid queue weight value,should be 0-100", "��Ч�Ķ���Ȩ��ֵ,Ӧ��Ϊ0-100"},
    {ERR_QOS_INVALID_QUE_CAR,           "Invalid queue bandwidth value, should be 0-102400Kbps", "��Ч�Ķ��д���ֵ,Ӧ��Ϊ0-102400Kbps"},
    {ERR_QOS_INVALID_CLASS_IF,          "Invalid ingress interface", "��Ч������ӿ�"},
    {ERR_QOS_INVALID_VLAN_ID,           "Invalid vlan id, shoud be 1-4094", "��Ч��VLAN ID,Ӧ��Ϊ1-4094"},
    {ERR_QOS_INVALID_IP_LEN,            "Invalid ip length value,should be 20-1500", "��Ч��IP���ݳ���,Ӧ��Ϊ20-1500"},
    {ERR_QOS_INVALID_CLASS_QUE,         "Invalid index of queue instance", "��Ч�Ķ���ʵ������"},
    {ERR_QOS_QUEUE_NOT_EXIST,           "The queue instance does not exist", "����ʵ��������"},
    {ERR_QOS_QUEUE_CAR_CONFLICT,        "The bandwidth sum of all enabled queues shouled be less than or equal to the global bandwidth value",
                                        "���������ö��еĴ���ֵ֮��Ӧ��С�ڻ�����ܴ���ֵ"},
    {ERR_QOS_SHAPE_RATE_CONFLICT,       "Committed rate should be less than or equal to peak rate", "��ŵ����Ӧ��С�ڵ��ڷ�ֵ����"},
    {ERR_QOS_QUEUE_LEN_CONFLICT,        "Queue length should be 20-150packets or 30000-225000bytes",
                                        "���г���Ӧ��Ϊ20-150packets��30000-225000bytes"},
    {ERR_QOS_QUEUE_WEIGHT_CONFLICT,     "The weighted sum of all enabled queues should be less than or equal to 100%",
                                        "���������ö��е�Ȩ��֮��Ӧ��С�ڻ����100%"},
    {ERR_QOS_QUEUE_PRIO_CONFLICT,       "Two enabled queues binding the same SP interface should not have the same priority",
                                        "�����󶨵�ͬһSP�ӿڵ������ö��е����ȼ�������ͬ"},
    {ERR_QOS_IP_LENGTH_CONFLICT,        "The minimum ip length should be less than or equal to the maximum ip length",
                                        "IP������СֵӦ��С�ڻ����IP�������ֵ"},
    {ERR_QOS_PORT_CONFLICT,             "The minimum port value should be less than or equal to the maximum port value",
                                        "�˿���СֵӦ��С�ڻ���ڶ˿����ֵ"},
    {ERR_QOS_APP_NAME_CONFLICT,         "The configuration with same app name has existed", "������ͬҵ�����Ƶ������Ѿ�������"},
    {ERR_QOS_INVALID_IP_ADDR,           "Invalid IP format", "IP��ַ��ʽ����ȷ"},
    {ERR_QOS_INVALID_PREFIX_LEN,        "Invalid IPV6 format,prefix length should be 0-128", "��Ч��IPv6��ַǰ׺����,Ӧ��Ϊ0-128"},
    {ERR_QOS_SRCIP_CONFLICT,            "Invalid source IP format", "ԴIP��ַ��ʽ����ȷ"},
    {ERR_QOS_DSTIP_CONFLICT,            "Invalid destination IP format", "Ŀ��IP��ַ��ʽ����ȷ"},
    {ERR_QOS_INVALID_FLOW_LABEL,        "Invalid Flow Label value,should 0-1048575", "��Ч��Flow Labelֵ,Ӧ��Ϊ0-1048575"},
    {ERR_QOS_INVALID_TRAFFIC_CLASS,     "Invalid value of the Traffic Class,should be 0-255", "��Ч��Traffic Classֵ,Ӧ��Ϊ0-255"},
    {ERR_QOS_CLASS_CONFLICT,     "The same Classification Rules has existed", "����������ظ�"},
    {ERR_QOS_CLASS_EMPTY,          "Classification Rules should not be empty","���������Ϊ��"},
#else
    {ERR_QOS_INVALID_IF_PATH,           "Invalid egress interface" , "��Ч������ӿ�"},
#if 0
    {ERR_QOS_INVALID_COMMITTED_RATE,    "To ethernet interface,rate value should be 100-100000Kbps and to VAP interface it should be 100-30000Kbps",
                                        "������̫���ӿ�,����ֵӦ��Ϊ100-100000Kbps,����VAP�ӿ�Ӧ��Ϊ100-30000Kbps"},
    {ERR_QOS_INVALID_PEAK_RATE,         "To ethernet interface,rate value should be 100-100000Kbps and to VAP interface it should be 100-30000Kbps",
                                        "������̫���ӿ�,����ֵӦ��Ϊ100-100000Kbps,����VAP�ӿ�Ӧ��Ϊ100-30000Kbps"},
    {ERR_QOS_INVALID_COMMITTED_RATE,    "Invalid committed rate value, should be 100-100000Kbps",
                                        "��Ч�ĳ�ŵ����ֵ��Ӧ��Ϊ100-100000Kbps"},
    {ERR_QOS_INVALID_PEAK_RATE,         "Invalid peak rate value, should be 100-100000Kbps",
                                        "��Ч�ķ�ֵ����ֵ��Ӧ��Ϊ100-100000Kbps"},
    {ERR_QOS_INVALID_BURST_SIZE,        "Invalid burst size, should be 2-100", "��Ч��ͻ��������,Ӧ��Ϊ2-100"},
#endif
    {ERR_QOS_INVALID_QUE_SCH_ALGORITHM, "Invalid queue scheduler Algorithm, should be DWRR or SP",
                                        "��Ч�Ķ��е��ȷ�ʽ��Ӧ��ΪDWRR��SP"},
    {ERR_QOS_INVALID_QUE_IF,            "Invalid index of interface instance", "��Ч�Ľӿ�ʵ������"},
    {ERR_QOS_QUE_IF_NOT_EXIST,          "The interface instance does not exist", "�ӿ�ʵ��������"},
    {ERR_QOS_INVALID_QUE_LEN,           "Queue length should be 20-150packets or 30000-225000bytes",
                                        "���г���Ӧ��Ϊ20-150packets��30000-225000bytes"},
    {ERR_QOS_INVALID_QUE_LEN_TYPE,      "Invalid queue length type, should be 0(packets) or 1(bytes)",
                                        "��Ч�Ķ��г������ͣ�Ӧ��Ϊ0(packets)��1(bytes)"},
    {ERR_QOS_INVALID_QUE_WEIGHT,        "Invalid queue weight value,should be 1-99", "��Ч�Ķ���Ȩ��ֵ,Ӧ��Ϊ1-99"},
    {ERR_QOS_INVALID_DROP_ALGORITHM,    "Invalid drop algorithm, should be DT(drop tail)", "��Ч�Ķ����㷨,Ӧ��ΪDT(β����)"},
    {ERR_QOS_INVALID_CLASS_TYPE,        "Invalid classify type, should be 0(L2) or 1(L3)", "��Ч�ķ�������,Ӧ��Ϊ0(L2)��1(L3)"},
    {ERR_QOS_INVALID_CLASS_IF,          "Invalid ingress interface", "��Ч������ӿ�"},
    {ERR_QOS_INVALID_ETH_TYPE,          "Invalid ethertype field valid", "��Ч����̫�������ֶ�ֵ"},
    {ERR_QOS_INVALID_ETH_PRIO,          "Invalid ethernet priority value, shoud be 0-7", "��Ч����̫�����ȼ�,Ӧ��Ϊ0-7"},
    {ERR_QOS_INVALID_VLAN_ID,           "Invalid vlan id, shoud be 1-4094", "��Ч��VLAN ID,Ӧ��Ϊ1-4094"},
    {ERR_QOS_INVALID_IP_LEN,            "Invalid ip length value,should be 20-1500", "��Ч��IP���ݳ���,Ӧ��Ϊ20-1500"},
    {ERR_QOS_INVALID_DSCP,              "Invalid DSCP value, should be PHB format", "��Ч��DSCPֵ,Ӧ��ΪPHB��ʽ"},
    {ERR_QOS_INVALID_PROTOCOL,          "Invalid protocol value, should be 0(tcp&udp),1(icmp),6(tcp),17(udp)",
                                        "��Ч��Э��ֵ��Ӧ��Ϊ0(tcp&udp), 1(icmp), 6(tcp), 17(udp)"},
    {ERR_QOS_INVALID_CLASS_QUE,         "Invalid index of queue instance", "��Ч�Ķ���ʵ������"},
    {ERR_QOS_CLASS_QUE_NOT_EXIST,       "The queue instance does not exist", "����ʵ��������"},
    {ERR_QOS_SHAPE_RATE_CONFLICT,       "Committed rate should be less than or equal to peak rate", "��ŵ����Ӧ��С�ڵ��ڷ�ֵ����"},
    {ERR_QOS_QUEUE_LEN_CONFLICT,        "Queue length should be 20-150packets or 30000-225000bytes",
                                        "���г���Ӧ��Ϊ20-150packets��30000-225000bytes"},
    {ERR_QOS_QUEUE_WEIGHT_CONFLICT,     "The weight sum of all enabled queue binding the same DWRR interface should be less than or equal to 100%",
                                        "�󶨵�ͬһ��DWRR�ӿڵ����������ö��е�Ȩ�غ�Ӧ��С�ڻ����100%"},
    {ERR_QOS_QUEUE_PRIO_CONFLICT,       "Two enabled queues binding the same SP interface should not have the same priority",
                                        "�����󶨵�ͬһSP�ӿڵ������ö��е����ȼ�������ͬ"},
    {ERR_QOS_IP_LENGTH_CONFLICT,        "The minimum ip length should be less than or equal to the maximum ip length",
                                        "IP������СֵӦ��С�ڻ����IP�������ֵ"},
    {ERR_QOS_PORT_CONFLICT,             "The minimum port value should be less than or equal to the maximum port value",
                                        "�˿���СֵӦ��С�ڻ���ڶ˿����ֵ"},
    {ERR_QOS_CLASS_CONFLICT,     "The same Classification Rules has existed", "����������ظ�"},
    {ERR_QOS_CLASS_EMPTY,          "Classification Rules should not be empty","���������Ϊ��"},
#endif
#if 0
    /* DYNAMIC ROUTE */
    {ERR_DYNAMIC_INVALID_VAL, "value invalid",  "ֵ��Ч"},
    {ERR_DYNAMIC_INVALID_CONFIG_FILE, "invalid configurate file",  "�����ļ���Ч"},
    {ERR_DYNAMIC_VERSION_INVALID, "RIP version invalid",  "��Ч��rip�汾"},
    {ERR_DYNAMIC_INVALID_PC_MSG, "PC inner message invalid",  "��Ч��PC�ڲ���Ϣ"},
    {ERR_DYNAMIC_INVALID_WAN, "WAN device invalid",  "WAN�豸��Ч"},
    {ERR_DYNAMIC_INVALID_INNER_PARA,  "invalid inner parament ",  "��Ч���ڲ�����"},
#endif

    /* DYNAMIC ROUTE */
    {ERR_DROUTE_INVALID_VAL, "value invalid",  "ֵ��Ч"},
    {ERR_DROUTE_INVALID_CONFIG_FILE, "invalid configurate file",  "�����ļ���Ч"},
    {ERR_DROUTE_VERSION_INVALID, "RIP version invalid",  "��Ч��rip�汾"},
    {ERR_DROUTE_INVALID_PC_MSG, "PC inner message invalid",  "��Ч��PC�ڲ���Ϣ"},
    {ERR_DROUTE_INVALID_WAN, "WAN device invalid",  "WAN�豸��Ч"},
    {ERR_DROUTE_INVALID_INNER_PARA,  "invalid inner parament ",  "��Ч���ڲ�����"},
    {ERR_DROUTE_INVALID_DIRECTION, "RIP direction invalid",  "��Ч��rip��Ч�ӿڷ���"},

    {ERR_STATIC_INVALID_INSTANCE      , "invalid instance",  "��Чʵ��"},
    {ERR_STATIC_INTERFACE             , "invalid interface ",  "��Ч�������豸"},
    {ERR_STATIC_NO_RESOURSE           , "has no resource",  "û����Դ"},
    {ERR_STATIC_INVALID_VAL           , "value invalid",  "ֵ��Ч"},
    {ERR_STATIC_ENABLE_INVALID        , "enable invalid",  "ʹ�ܲ�����Ч"},
    {ERR_STATIC_INVALID_PATH          , "invalid path",  "��Ч·��"},
    {ERR_STATIC_INVALID_DEFAULT_PARA  , "invalid default route parameter",  "��ЧĬ��·�ɲ���"},
    {ERR_STATIC_INVALID_ROUTE_PARA    , "invalid route para",  "��Ч·�ɲ���"},
    {ERR_STATIC_INVALID_PARA          , "invalid parameter",  "��Ч����"},
    {ERR_STATIC_DEL_INSTANCE          , "del instance error",  "ɾ��ʵ������"},
    {ERR_STATIC_INVALID_METRIC        , "invalid metric",  "��Чmetric"},
    {ERR_STATIC_INVALID_IP            , "Invalid IP address",  "��ЧIP"},
    {ERR_STATIC_IP_EMPTY              , "IP empty",  "IP ��ַ��"},
    {ERR_STATIC_INVALID_MASK          , "invalid netmask",  "��Ч������"},
    {ERR_STATIC_MASK_EMPTY            , "netmask empty",  "�������"},
    {ERR_STATIC_INVALID_GATEWAY       , "invalid gateway",  "��Ч����"},
    {ERR_STATIC_METRIC_OUT_LIMITED    , "metric is out of limits(0~255)",  "metric����������Χ"},
    {ERR_STATIC_INVALID_LOGFILE       , "invalid log file",  "��Ч��log�ļ�"},
    {ERR_STATIC_INVALID_INTERFACE     , "invalid interface", "��Ч������ӿ�"},
    {ERR_STATIC_DUPLICATE_ROUTING	  , "duplicate routing",  "�ظ���·��"},
    {ERR_STATIC_DUPLICATE_DESTIP	  , "duplicate destination address",  "�ظ���Ŀ�ĵ�ַ"},

   /* V6 Static routing */
//    {ERR_V6STATIC_ENABLE_INVALID        , "enable invalid",  "ʹ�ܲ�����Ч"},
//    {ERR_V6STATIC_BAD_PREFIX        , "Invalid Prefix! Valid range is 3-128",  "Prefix����ȷ����ȷ��ȡֵ��Χ��3-128"},
//    {ERR_V6STATIC_BAD_METRIC        , "Invalid Metric!",  "��Ч��metric����"},

    /* SNMPC */
    {ERR_SNMPC_INVALID_VERSION,         "invalid SNMP version", "��Ч��SNMP�汾��"},

    /* Firewall */
    {ERR_FW_RULES_OVER,                 "The number of rules can not be over 20",
                                        "������ӵĹ����ܳ���20��"},
    {ERR_FW_RULE_EXISTS,                "this rule already exists in filter table",
                                        "�����ظ��Ĺ������"},
    {ERR_FW_SRC_IP_EMPTY,                "Source IP Address can not be empty !",
                                        "ԴIP��ַ�ز���Ϊ��"},
    {ERR_FW_DEST_IP_EMPTY,                "Destination IP Address can not be empty !",
                                        "Ŀ��IP��ַ�ز���Ϊ��"},

    /* SNTP */
    {ERR_SNTP_INVALID_UP_INTERVAL,      "Invalid update interval value, shoud be 1-24(hours)",
                                        "��Ч�ĸ���ʱ����ֵ,Ӧ��Ϊ1-24(Сʱ)"},
    {ERR_SNTP_INVALID_RETRY_INTERVAL,   "Invalid retry interval value, should be 1-60(minutes)",
                                        "��Ч������ʱ����,Ӧ��Ϊ1-60(����)"},
    {ERR_SNTP_INVALID_NTP_SERVER,       "The Ip address or domain name of NTP server is invalid",
                                        "NTP��������IP��ַ����������Ч��"},
    {ERR_SNTP_INVALID_DATA_TIME,        "Invalid time format",   "ʱ���ʽ���Ϸ�"},
    {ERR_SNTP_NULL_NTPSERVER_STR,       "The NTP servers should not be null",
                                        "NTP����������Ϊ��"},
    {ERR_SNTP_NTPSERVERS_CONFLICT,      "Two NTP servers should not be same value",
                                        "����NTP������������ͬ"},
    {ERR_SNTP_DST_CONFLICT,             "Theres a conflict between the start and the end of daylight saving time",
                                        "����ʱ��ʼ�ͽ���ʱ��֮����ڳ�ͻ"},
    {ERR_SNTP_INVALID_DATA_YEAR,        "Invalid year,should be 2000-2036/02/5",   "��Ч��������Ӧ��Ϊ2000-2036/02/5"},

    /*tr069be*/
    {ERR_TR069BE_INVALID_URL,  "invalid URL", "�Ƿ���URL"},
    {ERR_TR069BE_INVALID_UPLOAD_URL, "unable to connect to the server", "���ӷ�����ʧ��"},
    {ERR_TR069BE_INVALID_DOWNLOAD_URL, "unable to connect to the server", "���ӷ�����ʧ��"},
    {ERR_USERNAME,  "invalid username", "�Ƿ����û���"},
    {ERR_PASSWORD,  "invalid password", "�Ƿ�������"},
    {ERROR_TR069_INVALID_CAFILE, "Invalid CAfile", "�Ƿ���֤��"},
	{ERR_WAN_SVC_ONE_MORE_TR069, "Only one TR069 service type can exist", "���ܽ�������tr069��wan����"},

    {ERR_URLFILTER_INVALID_INNER_PARA  ,  "inner parament error",   "�ڲ��������ݴ���"},
    {ERR_URLFILTER_INVALID_VAL         ,  "invalid value",   "ֵ��Ч"},
    {ERR_URLFILTER_ENABLE_EMPTY        ,  "value enable empty",   "ֵenableΪ��"},
    {ERR_URLFILTER_ENABLE_INVALID      ,  "value enable wrong",   "ֵenable����"},
    {ERR_URLFILTER_INVALID_CONFIG_FILE ,  "invalid config file",   "��Ч�����ļ�"},
    {ERR_URLFILTER_MODE_EMPTY          ,  "value mode empty",   "ֵģʽΪ��"},
    {ERR_URLFILTER_MODE_INVALID        ,  "value mode wrong",   "ֵģʽ����"},
    {ERR_URLFILTER_URL_CONFLICT        ,  "URL conflict",   "URL ��ͻ"},
    {ERR_URLFILTER_URLNUMBER_OVER      ,  "URL Number is over", "������url��Ŀ������Χ"},
    {ERR_URLFILTER_URL_LENGTH_OVER     ,  "URL length is over", "URL����"},
    {ERR_URLFILTER_URL_ERROR           ,  "URL value is error",  "URLֵ����"},
    {ERR_URLFILTER_INVALID_FILTERMODE           ,  "Invalid filter mode",  "��Ч��URL����ģʽ"},
    {ERR_URLFILTER_INVALID_TIME           ,  "Invalid time",  "��Чʱ��"},
    {ERR_URLFILTER_INVALID_DAY           ,  "Invalid day",  "��Ч������"},
    {ERR_URLFILTER_TIME_CONFLICT           ,  "Start time is higher than end time",  "��ʼʱ����ڽ���ʱ��"},
    {ERR_URLFILTER_MAC_CONFLICT           ,  "MAC already exists",  "MAC��ַ�Ѿ�����"},
    {ERR_USERNAMEORPASSWORD           ,  "invalid username or password",  "�Ƿ����û���������"},


    /* IPT */
    {ERR_IPT_CONFLICT_DMZ,            "DMZ host ip should not be null", "DMZ������ַ������Ϊ��"},

    /* WLAN */
    {ERR_WLAN_NO_RESOURSE,            "no resourse", "��Դȱ��"},
    {ERR_WLAN_INVALID_VAL,            "invalid value",   "ֵ��Ч"},
    {ERR_WLAN_INVALID_PATH,            "invalid path",  "��Ч·��"},
    {ERR_WLAN_VAL_EMPTY,            "value empty",   "ֵΪ��"},
    {ERR_WLAN_EXCEED_VAP_NUM,             "The number of Vap can not exceed 5",
                                                                "������ӵ�VAP���ܳ���5��"},
    {ERR_WLAN_INVALID_INSTANCE,            "invalid instance", "��Чʵ��"},
    {ERR_WLAN_INVALID_VAPIDX,            "invalid Vap index", "��Ч��Vap���"},
    {ERR_WLAN_NAME_REPEATED,            "name repeated", "���ظ�"},
    {ERR_WLAN_INVALID_SESSION,   "wps session is already started", "�Ѿ�������WPSЭ��"},
	{ERR_WLAN_IN_OOB,   "It is already in OOB status", "�Ѿ�����OOB״̬"},
    {ERR_WLAN_CONFLICT_ENCMODES_STANDARD, "WEP and TKIP encryption is not supported in gn/bgn/n/an mode", "wep & tkip ���ܵ�����²��ܽ�mode����Ϊgn/bgn/n/an"},
    {ERR_WLAN_INVALID_SSID_LENGTH,   "the length of SSID can not exceed 32 ","�������õ�SSID���Ȳ��ܳ���32"},
    //����Wlan_WDSģ��Ҫ����Խ�MAC��Ϊȫ�㣬���乫��ģ���MAC���󱨴���Ϣ��������WDSģ�飬�ʼӴ�����Ϣ
    {ERR_WLAN_INVALID_MAC_VALUE,  "mac must be \"xx:xx:xx:xx:xx:xx\" format and must not be broadcast or multicast",  "mac��ַӦ����xx:xx:xx:xx:xx:xx���ָ�ʽ�Ҳ����ǹ㲥���鲥"},
	{ERR_WLAN_INVALID_MAXSTATION_NUM, "the max  number of stations should be set between 1-32", "�û���������Ӧ��������1-32֮��"},
    {ERR_WLAN_GET_CHANNEL_AUTOCHANNEL, "Get channel or AutoChannelEnable failed", "��ȡ�ŵ������Զ��ŵ�ʧ��"},
    {ERR_WLAN_CONFLICT_CHANNEL_AUTOCHANNEL, "Channel and AutoChannelEnable conflict", "�ŵ����Զ��ŵ���ͻ"},
	{ERR_WLAN_INVALID_SSID_VAL_0, "The SSID should not be empty", "SSID ���Ȳ���Ϊ0"},
	{ERR_WLAN_INVALID_PSK_KEY_0, "The length of the passphrase (PSK) must be 8-63 characters or 64 Hex number.", "����PSK���������8-63���ַ�����64��16��������"},
	{ERR_WLAN_INVALID_WEP_KEY_0, "The length of the wep key must be 5 characters(10 Hex number) or 13 characters(26 Hex number)", "����wep���볤��Ӧ����5���ַ�(10��16��������) ���� 13���ַ�(26��16��������)"},
		
    /* PORTMAPPING */
    {ERR_PTMP_INVALID_DESCRIPTION,    "Invalid characters inclued in description", "������Ϣ�к��зǷ��ַ�"},
    {ERR_PTMP_CONFLICT_PORTRANGE,     "The minimum port value should be less than or equal to the maximum port value",
                                      "�˿���СֵӦ��С�ڻ���ڶ˿����ֵ"},
    {ERR_PTMP_CONFLICT_TUPLE,         "Two instances conflict for the conflicting tuple composed of RemoteHost, ExternalPort and Protocol ",
                                      "��������ʵ������Ԫ��(Զ������, �ⲿ�˿ں�Э��)������ͻ"},
    {ERR_PTMP_CONFLICT_PORTRANGE_7547,"Port 7547 is in use by TR069, please choose another port",
	                                  "�˿�7547�Ѿ���TR069ռ��,��ʹ�������˿�"},
    {ERR_PTMP_INVALID_HOST_NAME,    "Invalid Host Name" , "������������"},
	{ERR_DESCRIPTION_DUPLICATE,	  "Service Name DUPLICATE, please check the Service Name!",
										  "�������ֳ�ͻ"},
     //����ulInternPortֵ�����ֵ���жϺ��������ʾ��Ϣ���ʼ�������һ��
    {ERR_PTMP_CONFLICT_PORTRANGE_ulInternPort,"The port value of  Internal Port End is too big ",
	                                  "�˿�Internal Port End ��ֵ����"},
    {ERR_PTMP_WRONG_HOST_IP,    "Host IP can not for empty and 255.255.255.255" , "����IP����Ϊ���Լ�255.255.255.255"},

    {ERR_PTMP_NOTINSAMESUBNET,              "Service IP and LAN IP are not in the same subnet",
                                            "����IP��ַ��LAN IP��ַ������ͬһ����"},
    {ERR_PORT_HAS_OCCUPIED,         "The port is occupied","�˿��ѱ�ռ��"},

	/* USB3G */
	{ERR_USB3G_INVALID_PIN_LENGTH,                "The length of PIN code must be 4",
                                        "PIN��ĳ��ȱ���Ϊ4"},
    {ERR_USB3G_INVALID_PUK_LENGTH,                "The length of PUK code must be 8",
                                        "PUK��ĳ��ȱ���Ϊ8"},
	{ERR_USB3G_INVALID_PINPUKCODE,            "The PIN/PUK code must be decimal",
                                        "PIN/PUK�������ʮ�����������"},

    /* ACL */
    {ERR_ACL_RULES_OVER,                "The number of rules can not be over 10",
                                        "������ӵķ����ܳ���10��"},
    {ERR_ACL_RULE_EXISTS,               "This open port has been occupied",
                                        "�˿��Ŷ˿��Ѿ���ռ��"},
    {ERR_ACL_PORT_BE_ZERO,              "When protocol is ICMP, the port value must be zero",
                                        "��Э��ΪICMPʱ,�˿�ֵ����Ϊ0"},
    {ERR_ACL_PORT_CANOT_ZERO,           "When protocol is not ICMP, The port value can not be zero",
                                        "��Э�鲻ΪICMPʱ,�˿�ֵ����Ϊ0"},
    {ERR_ACL_PORT_IN_USE,               "This port is in use",
                                        "�˶˿��Ѿ���ʹ��"},

    /* LOGGER */
    {ERR_LOGGER_UPLOAD_FAILED,          "Log file upload failed",  "��־�ļ��ϴ�ʧ��"},
    {ERR_LOGGER_INVALID_TFTP_SERVER,    "Invalid TFTP server",  "�Ƿ���TFTP������"},
    {ERR_LOGGER_CLEAR_FAILED,           "Log file clear failed",  "��־�ļ����ʧ��"},
    {ERR_LOGGER_EMPTY_TFTP_SERVER,      "Server address cannot be empty","��������ַ����Ϊ��"},
    {ERR_LOGGER_INVALID_SERVER,         "Invalid server","�Ƿ��ķ�����"},

    /* ATM */
    {ERR_ATM_PVC,                       "VPI/VCI is invalid", "PVC���ò��Ϸ�������VPI/VCI�ĸ�ʽ��д"},
    {ERR_ATM_VPI,                       "Invalid VPI value (0~255)", "VPIֵ�Ƿ�(0~255)"},
    {ERR_ATM_VCI,                       "Invalid VCI value (32~65535)", "VCIֵ�Ƿ�(32~65535)"},
    {ERR_ATM_UNKNOWN_ENCAP,             "Unkown encapsulation", "δ֪��װ����"},
    {ERR_ATM_UNKNOWN_QOS_TYPE,          "Unkown Qos type", "δ֪Qos����"},
    {ERR_ATM_PCR,                       "Invalid PCR value (0~7100)", "��ֵ��Ԫ����/��Ԫ�ӳٱ仯�ʷǷ�(0~7100)"},
    {ERR_ATM_MBS,                       "Invalid MBR value (0~1000000)", "���ͻ���ߴ�Ƿ�(0~1000000)"},
    {ERR_ATM_SCR,                       "Invalid SCR value (0~7099)", "ƽ����Ԫ���ʷǷ�"},
    {ERR_ATM_MIN_PCR,                   "Wrong MCR value (0~pcr)", "��С��Ԫ���ʷǷ�"},
    {ERR_ATM_VLAN_ID,                   "Wrong vlan id (0~4095)", "VLAN ID�Ƿ�(0~4095)"},
    {ERR_ATM_VLAN_PRIORITY,             "Wrong vlan priority(0~7)", "VLAN���ȼ��Ƿ�(0~7)"},
    {ERR_ATM_VLAN_ID_CONFLICT,          "Same vlan id", "VLAN ID�����ظ�"},

    /* UPNP */
    {ERR_UPNP_SRCIP_EXIST,              "This IP address is already in the list","��IP��ַ�����б���"},

    /* DDNS */
    {ERR_DDNS_INVALID_DDNSPROVIDER,     "Invalid DDNS provider","��Ч�Ķ�̬DNS������"},
    {ERR_DDNS_INVALID_HOSTNAME,         "Invalid hostname","��Ч��ע������"},
    {ERR_DDNS_HOSTNAME_LENGTH_OVER,     "Hostname length is high","ע����������"},
    {ERR_DDNS_INVALID_WAN_CONN_SID,     "Invalid WAN connection ID","��Ч��WAN����"},
    {ERR_DDNS_INVALID_USERNAME,         "Invalid username","��Ч���û���"},
    {ERR_DDNS_USERNAME_LENGTH_OVER,     "Username length is too high","�û�������"},
    {ERR_DDNS_INVALID_PASSWORD,         "Invalid password","��Ч������"},
    {ERR_DDNS_PASSWORD_LENGTH_OVER,     "Password length is high","���볬��"},
    {ERR_DDNS_INVALID_EMAIL,            "Invalid email address","��Ч�������ַ"},
    {ERR_DDNS_EMAIL_LENGTH_OVER,        "Email address length is high","�����ַ����"},
    {ERR_DDNS_INVALID_KEY,              "Invalid key","��Ч����Կ"},
    {ERR_DDNS_KEY_LENGTH_OVER,          "Key length is too high","��Կ����"},
    {ERR_DDNS_HOSTNAME_CONFLICT,        "Hostname already exists","ע�������Ѿ�����"},
    {ERR_DDNS_INVALID_DDNS_HOST,         "Invalid DDNS host","��Ч��DDNS ������"},
    {ERR_DDNS_DDNS_HOST_LENGTH_OVER,     "DDNS host length is too high","DDNS������������ʶ����"},
    {ERR_DDNS_PRIVODER_CONFLICT,        "www.oray.cn exists already","www.oray.cn�Ѿ�����"},

    /* SUPP */
    {ERR_SUPP_USERNAME_OR_PASSWORD_BE_EMPTY, "username or password empty", "�û���������Ϊ��"},

    /* MACFILTER */
    {ERR_MACFILTER_INVALID_USERNAME,  "Invalid username","��Ч�û���"},
    {ERR_MACFILTER_INVALID_FILTERMODE,  "Invalid filter mode","��Ч�Ĺ���ģʽ"},
    {ERR_MACFILTER_INVALID_TIME,        "Invalid time","��Ч��ʱ��"},
    {ERR_MACFILTER_INVALID_DAY,         "Invalid day","��Ч������"},
    {ERR_MACFILTER_TIME_CONFLICT,       "Start time is higher than end time","��ʼʱ����ڽ���ʱ��"},
    {ERR_MACFILTER_MAC_CONFLICT,        "MAC already exists","MAC��ַ�Ѿ�����"},
    {ERR_MACFILTER_INVALID_INNER_PARA,  "Invalid inner parameter","��Ч�ڲ�����"},
	{ERR_MACFILTER_ERR_DEL_WHITE_LIST,	"In allow mode, not allow to delete the only one white list.","������ģʽ�£�ֻ��һ��������ʱ���ֹɾ��"},

    /*RADVD*/
    {ERR_HEX_FORMAT_WRONG,          "invalid hex number","������Ч��ʮ��������"},
    {ERR_GLOBALID_OUT_OF_RANGE, "invalid global id,must be 40bit hex number","global id������ʮ��������"},
    {ERR_CONFIG_TYPE_INVALID_VALUE, "invalid config type,must be Static or Delegated","�������ͱ����� Static��Delegated"},
    {ERR_ROUTE_LIFETIME_OUT_OF_RANGE, "lifetime range is 1~90 minutes","��������Ϊ0~90����"},
    {ERR_ROUTE_PREFERENCE_INVALID_VALUE, "route preference must be low,medium or high","���ȼ�����������Ϊlow,medium or high"},
    {ERR_SUBNET_ID_OUT_OF_RANGE, "SubnetID range is 0~255","������ΧΪ0~255"},
    {ERR_INVALID_PREFIX_LEN, "Invalid prefix length, it should be 0~128","��Ч�ĵ�ַǰ׺���ȣ���ΧΪ0~128"},
    {ERR_INVALID_PREFIX, "Invalid prefix","��Ч�ĵ�ַǰ׺"},
	{ERR_RADVD_VALID_LIFE_TIME_SMALLER, "PreferredLifeTime must smaller than ValidLifeTime","PreferredLifeTime����С��ValidLifeTime"},


    /* TRACEROUT */
    {ERR_TRACERT_NOSTART,               "Traceroute is not started","����δ����"},
    {ERR_TRACERT_BUSY,                  "Traceroute is running","������������"},

    /* URLFILTER */
    {ERR_URLFILTER_INVALID_FILTERMODE,  "Invalid filter mode","��Ч�Ĺ���ģʽ"},
    {ERR_URLFILTER_INVALID_TIME,        "Invalid time","��Ч��ʱ��"},
    {ERR_URLFILTER_INVALID_DAY,         "Invalid day","��Ч������"},
    {ERR_URLFILTER_TIME_CONFLICT,       "Start time is higher than end time","��ʼʱ����ڽ���ʱ��"},
    {ERR_URLFILTER_INVALID_INNER_PARA,  "Invalid inner parameter","��Ч�ڲ�����"},

    /* SCHEDULE */
    {ERR_SCHEDULE_INVALID_NAME,             "Name is invalid.", "���ֲ��Ϸ�"},
    {ERR_SCHEDULE_INVALID_ENABLEALLDAYS,    "Day select is invalid.","����ѡ�񲻺Ϸ�"},
    {ERR_SCHEDULE_INVALID_STARTTIME,        "The Start Time entered is invalid","��ʼʱ�䲻�Ϸ�"},
    {ERR_SCHEDULE_INVALID_ENDTIME,          "The End Time entered is invalid","����ʱ�䲻�Ϸ�"},
    {ERR_SCHEDULE_INVALID_SPICIFIEDDAY,     "No days are specified.","δָ������"},
    {ERR_SCHEDULE_INVALID_SCHEDULEDAYS,     "Day select is invalid.","����ѡ�񲻺Ϸ�"},
    {ERR_SCHEDULE_INVALID_ENABLEALLTIME,    "Start time is invalid.","��ʼʱ�䲻�Ϸ�"},
    /* Diagnostics */
    {ERR_DG_INVALID_STATE,              "Diagnostics state is invalid.",    "���״ֵ̬���Ϸ�"},
    {ERR_DG_INVALID_INTERFACE,          "Interface is invalid.",            "�ӿڲ��Ϸ�"},
    {ERR_DG_PVC_TOO_MUCH,               "Too many PVC pairs in search list.", "�����б���PVC����̫��"},
    {ERR_DG_PVC_INVALID,               "PVC INVALID.", "�����PVC����ȷ"},


    //default gateway
    {ERR_DEGW_EMPTY_IP,              "IP address is empty","IP��ַΪ��"},
    {ERR_DEGW_INVALID_IP,              "IP address is invalid","IP��ַ��Ч"},
    {ERR_DEGW_EMPTY_USERINIF,              "User interface is empty","�û��ӿ�Ϊ��"},
    {ERR_DEGW_INVALID_USERINIF,              "User interface is invalid","�û��ӿ���Ч"},
    {ERR_DEGW_EMPTY_SELECT,              "Mode is empty","ģʽΪ��"},
    {ERR_DEGW_INVALID_SELECT,              "Mode must be one of AutoDeftGW,GWIP andUserInIf","ģʽ����ΪAutoDeftGW,GWIP��UserInIf"},

    //IP ACL
    {ERR_IP_ACL_EMPTY_IP,              "IP address is empty","IP��ַΪ��"},
    {ERR_IP_ACL_INVALID_IP,              "IP address is invalid","IP��ַ��Ч"},
    {ERR_IP_ACL_IP_ZERO,              "You can use this function unless there is at least one IP","������һ����¼����ʹ���������"},
    {ERR_IP_ACL_IP_SAME,              "Have the same entry","�Ѿ���������ͬ��ip"},

    /* IPTUNNEL */
    {ERR_IPTUNNEL_NO_RESOURSE,                "no resourse.", "û����Դ����"},
    {ERR_IPTUNNEL_VAL_EMPTY,                  "value empty.", "ֵΪ��"},
    {ERR_IPTUNNEL_NAME_OVERLENGTH,            "name over maxlength.", "ֵ������󳤶�"},
    {ERR_IPTUNNEL_VALID_STRING,               "name is valid string.", "ֵ����Ч�ַ���"},
    {ERR_IPTUNNEL_EXIST_NAME,                 "Already exist tunnel name.", "�Ѵ��ڵ�����"},
    {ERR_IPTUNNEL_EXIST_WAN_IF,               "Wan connection is in use.", "��ʹ�õ�Wan����"},
    {ERR_IPTUNNEL_NOEXIST_WAN_IF,             "There is not Wan connection.", "û�п��õ�WAN����"},
    {ERR_IPTUNNEL_WRONY_IPV6_MODLE,           "IPTunnel ipv6 prefix must be XXXXX:XXXX/X model.", "IPV6 ��ʽ����ȷ"},
    {ERR_IPTUNNEL_WRONY_MASKLEN_EMPTY,        "IPTunnel ipv4 mask length can not be empty.", "IPV4 ���벻��Ϊ��"},
    {ERR_IPTUNNEL_WRONY_MASKLEN,              "IPTunnel ipv4 mask length must be 0~32.", "IPV4 ����ȷ"},
    {ERR_IPTUNNEL_INVALID_PARA,               "invalid parameter",  "��Ч����"},
    {ERR_IPTUNNEL_IP_EMPTY,                   "IP empty",  "IP ��ַ��"},
    {ERR_IPTUNNEL_INVALID_IP,                 "Invalid IP address",  "��ЧIP"},
    {ERR_IPTUNNEL_INVALID_PREFIX_LEN,         "IPTunnel ipv6 prefix length must be 1~64",  "��Чǰ׺��ַ����"},
    {ERR_IPTUNNEL_INVALID_IPV6_ADDR,          "IPTunnel ipv6 address format error",  "IPv6��ַ��ʽ����"}, 
	{ERR_IPTUNNEL_INVALID_CREATE_PREFIX,      "The total of ipv6 prefix length and ipv4 mask length must be 1~64",  "IPv6ǰ׺������IPv4�������볤��֮�Ͳ���ȷ"}, 

    /* v6sroute */
    {ERR_V6SROUTE_BAD_ADDR,                   "Invalid IP ! Please input a right IPv6 address!", "v6��ַ������������ȷ��v6��ַ"},
    {ERR_V6SROUTE_REPEAT,                   "Route Had Exist!", "��̬v6·���Ѿ�����"},
    {ERR_V6SROUTE_GET_FAIL,                   "Failed to get static ip,Geteway or Interface!", "��ȡ��̬IP�����أ����߽��ʧ��"},
    {ERR_V6ROUTE_IP_NULL,                   "ipv6 address can not be empty!", "ipv6 ��ַ����Ϊ��"},
    {ERR_V6ROUTE_COUNT_ERROR,                   "V6_addr count error!", "V6_addr ��������"},

    /* DHCPv6 */
    {ERR_DHCP6S_LIFE_TIME_OUT_RANGE, "the life time is out of range","��Чʱ�䳬����Χ"},
    {ERR_DHCP6S_START_ADDR_BIGGER, "the start address is higher than the end one","��ʼ��ַ���ڽ�����ַ"},
    {ERR_DHCP6S_INVALID_LIFE_TIME, "the life time is out of range","�������ڳ�����Χ"},
    {ERR_DHCP6S_VALID_LIFE_TIME_SMALLER, "DHCPv6LeaseTime must be lower than DHCPv6ValidTime","DHCPv6LeaseTime����С��DHCPv6ValidTime"},
    {ERR_DHCP6S_RANGE_ERROR, "IPv6 Interface ID range error", "IPv6�ӿ�ID��Χ����"},

    /* ipv6  add by lining 2012.9.25*/
    {ERR_IPV6_INVALID_FORMAT,     "Invalid ipv6 address format", "�Ƿ���ipv6��ַ��ʽ"},
    {ERR_IPV6_ADDR_REPEAT,        "Repetitive ipv6 address exists", "�����ظ���ipv6��ַ"},
    {ERR_IPV6_INVALID_PREFIX_LEN, "Invalid ipv6 prefix length", "�Ƿ���ipv6ǰ׺����"},
    {ERR_IPV6_INVALID_DUID,       "Invalid DUID", "�Ƿ���DUID"},
    {ERR_INVALID_PROTO_TYPE,        "IPv6 invalid prototype",  "IPv6�Ƿ���Э������"},
    {ERR_IPV6_PROFIXLEN_NOT_MATCH,  "IPv6 profixlen not match", "IPv6ǰ׺��ƥ��"},


    /*V6CONN*/
    {ERR_V6CONN_INVALID_STATIC_ADDR,  "the ipv6 static addr is wrong","��̬IPV6��ַ����"},
    {ERR_V6CONN_INVALID_DNS_ADDR,         "Invalid DNS Address", "DNS��ַ���ʹ���"},
    {ERR_V6CONN_INVALID_ROUTER_ADDR, "the ipv6 router addr is wrong","ipv6��·�ɵ�ַ����"},
    {ERR_V6CONN_INVALID_ADDR_CONF_TYPE,   "Invalid IP Address Config Type", "IP��ַ����������Ч"},
    {ERR_V6CONN_INVALID_ROUTER_CONF_TYPE, "Invalid Route Config Type", "·������������Ч"},
    {ERR_V6CONN_INVALID_DUID_CONF_TYPE, "the duid type is wrong","DUID������Ч"},
    {ERR_V6CONN_INVALID_PREFIX_ADDR, "Invalid ipv6 prefix address,eg:2003::", "�Ƿ���ipv6ǰ׺��ַ������:2003::"},
    {ERR_V6CONN_PREFIX_ADDR_LEN_CONFLICT, "Ipv6 prefix address conflict with prefix len", "IPv6 ǰ׺��ַ��ǰ׺���ȳ�ͻ"},
    {ERR_V6CONN_STATIC_ADDR_LEN_CONFLICT, "Ipv6 static address conflict with prefix len", "IPv6 ��̬��ַ��ǰ׺���ȳ�ͻ"},

    // WEB -> Browser
    {ERR_WEB_VAR_VALUE_ERROR, "Web security vulnerabilities, illegal cross-site script",  "WEB��ȫ����,�Ƿ��ű�"},
    {0, NULL, NULL}
};


/*============================================================================
                      Debug Utils
=============================================================================*/
/*
*	��������: 	tbsGetMsgTypeName
*	��������: 	���Ը�������������ֵ��ʽMsgTypeת��Ϊ�ַ����ɶ���ʽ��
*	�������: 	usMsgType - MsgType��ֵ
*	����ֵ  :	��Ӧ��MsgType�ַ������֡�������ں���ʶ��Χ�У�����
*				UNKNOWN_MSGTYPE(ddd)��ʽ��
*	����    : 	������ / 2008-11-10
*/
const char* tbsGetMsgTypeName(unsigned short usMsgType)
{
	#define MSG_TYPE_NAME(name) case name: return #name

	switch (usMsgType)
    {

        MSG_TYPE_NAME(MSG_POLL_EXIT);
        MSG_TYPE_NAME(MSG_POLL_EXIT_ACK);

        MSG_TYPE_NAME(MSG_PC_START_PROC);
        MSG_TYPE_NAME(MSG_PC_START_PROC_ACK);
        MSG_TYPE_NAME(MSG_PC_SEND_SIGNAL);
        MSG_TYPE_NAME(MSG_PC_SEND_SIGNAL_ACK);
        MSG_TYPE_NAME(MSG_PC_PROC_STATE);
        MSG_TYPE_NAME(MSG_PC_PROC_STATE_ACK);
        MSG_TYPE_NAME(MSG_PC_PROC_OUTPUT);
        MSG_TYPE_NAME(MSG_PC_PROC_OUTPUT_ACK);
        MSG_TYPE_NAME(MSG_PC_EXIT);
        MSG_TYPE_NAME(MSG_PC_EXIT_ACK);

        MSG_TYPE_NAME(MSG_CMM_GET_VAL);
        MSG_TYPE_NAME(MSG_CMM_GET_VAL_ACK);
        MSG_TYPE_NAME(MSG_CMM_SET_VAL);
        MSG_TYPE_NAME(MSG_CMM_SET_VAL_ACK);
        MSG_TYPE_NAME(MSG_CMM_ADD_NODE);
        MSG_TYPE_NAME(MSG_CMM_ADD_NODE_ACK);
        MSG_TYPE_NAME(MSG_CMM_DEL_NODE);
        MSG_TYPE_NAME(MSG_CMM_DEL_NODE_ACK);
        MSG_TYPE_NAME(MSG_CMM_GET_NAME);
        MSG_TYPE_NAME(MSG_CMM_GET_NAME_ACK);
        MSG_TYPE_NAME(MSG_CMM_GET_ATTR);
        MSG_TYPE_NAME(MSG_CMM_GET_ATTR_ACK);
        MSG_TYPE_NAME(MSG_CMM_SET_ATTR);
        MSG_TYPE_NAME(MSG_CMM_SET_ATTR_ACK);
        MSG_TYPE_NAME(MSG_CMM_INFORM_NOTI);
        MSG_TYPE_NAME(MSG_CMM_INFORM_NOTI_ACK);
        MSG_TYPE_NAME(MSG_CMM_GET_NOTI);
        MSG_TYPE_NAME(MSG_CMM_GET_NOTI_ACK);
        MSG_TYPE_NAME(MSG_CMM_CLEAR_NOTI);
        MSG_TYPE_NAME(MSG_CMM_CLEAR_NOTI_ACK);
        MSG_TYPE_NAME(MSG_CMM_COMMIT);
        MSG_TYPE_NAME(MSG_CMM_COMMIT_ACK);
        MSG_TYPE_NAME(MSG_CMM_CANCEL);
        MSG_TYPE_NAME(MSG_CMM_CANCEL_ACK);
        MSG_TYPE_NAME(MSG_CMM_UPDATE);
        MSG_TYPE_NAME(MSG_CMM_UPDATE_ACK);
        MSG_TYPE_NAME(MSG_CMM_SAVE_CFG);
        MSG_TYPE_NAME(MSG_CMM_SAVE_CFG_ACK);
        MSG_TYPE_NAME(MSG_CMM_RECOVER_CFG);
        MSG_TYPE_NAME(MSG_CMM_RECOVER_CFG_ACK);
        MSG_TYPE_NAME(MSG_CMM_UPGRADE);
        MSG_TYPE_NAME(MSG_CMM_UPGRADE_ACK);
        MSG_TYPE_NAME(MSG_CMM_BACKUP);
        MSG_TYPE_NAME(MSG_CMM_BACKUP_ACK);
        MSG_TYPE_NAME(MSG_CMM_REBOOT);
        MSG_TYPE_NAME(MSG_CMM_REBOOT_ACK);

        MSG_TYPE_NAME(MSG_TIMER_REGSTER);
        MSG_TYPE_NAME(MSG_TIMER_REGSTER_ACK);
        MSG_TYPE_NAME(MSG_TIMER_UNREGSTER);
        MSG_TYPE_NAME(MSG_TIMER_UNREGSTER_ACK);
        MSG_TYPE_NAME(MSG_TIMER_TIMEOUT);
        MSG_TYPE_NAME(MSG_TIMER_TIMEOUT_ACK);

        MSG_TYPE_NAME(MSG_WAN_INTERFACE_STATE);
        MSG_TYPE_NAME(MSG_WAN_INTERFACE_STATE_ACK);
        MSG_TYPE_NAME(MSG_WAN_CONN_EST);
        MSG_TYPE_NAME(MSG_WAN_CONN_EST_ACK);
        MSG_TYPE_NAME(MSG_WAN_CONN_FIN);
        MSG_TYPE_NAME(MSG_WAN_CONN_FIN_ACK);
        MSG_TYPE_NAME(MSG_WAN_LINK_CHG);
        MSG_TYPE_NAME(MSG_WAN_LINK_CHG_ACK);
		MSG_TYPE_NAME(MSG_WAN_PROT_CHG);
        MSG_TYPE_NAME(MSG_WAN_PROT_CHG_ACK);

        MSG_TYPE_NAME(MSG_LAN_IP_UPDATE);
        MSG_TYPE_NAME(MSG_LAN_IP_UPDATE_ACK);
        MSG_TYPE_NAME(MSG_LAN_MACLIST_UPDATE);
        MSG_TYPE_NAME(MSG_LAN_MACLIST_UPDATE_ACK);
        MSG_TYPE_NAME(MSG_LAN_DOMAIN_NAME_UPDATE);
        MSG_TYPE_NAME(MSG_LAN_DOMAIN_NAME_UPDATE_ACK);

        MSG_TYPE_NAME(MSG_GET_DHOST_LIST);
        MSG_TYPE_NAME(MSG_GET_DHOST_LIST_ACK);
        MSG_TYPE_NAME(MSG_DNS_CHANGE);
        MSG_TYPE_NAME(MSG_DNS_CHANGE_ACK);
        MSG_TYPE_NAME(MSG_WLAN_ADD_DEVICE);
        MSG_TYPE_NAME(MSG_WLAN_ADD_DEVICE_ACK);
        MSG_TYPE_NAME(MSG_WLAN_DEL_DEVICE);
        MSG_TYPE_NAME(MSG_WLAN_DEL_DEVICE_ACK);

        MSG_TYPE_NAME(MSG_TR069_SET_PARKEY);
        MSG_TYPE_NAME(MSG_TR069_SET_PARKEY_ACK);
        MSG_TYPE_NAME(MSG_TR069_GET_EVENTS);
        MSG_TYPE_NAME(MSG_TR069_GET_EVENTS_ACK);
        MSG_TYPE_NAME(MSG_TR069_CLEAR_EVENTS);
        MSG_TYPE_NAME(MSG_TR069_CLEAR_EVENTS_ACK);
        MSG_TYPE_NAME(MSG_TR069_DOWNLOAD);
        MSG_TYPE_NAME(MSG_TR069_DOWNLOAD_ACK);
        MSG_TYPE_NAME(MSG_TR069_UPLOADLOAD);
        MSG_TYPE_NAME(MSG_TR069_UPLOADLOAD_ACK);
        MSG_TYPE_NAME(MSG_TR069_REBOOT);
        MSG_TYPE_NAME(MSG_TR069_REBOOT_ACK);
        MSG_TYPE_NAME(MSG_TR069_FACTORYRESET);
        MSG_TYPE_NAME(MSG_TR069_FACTORYRESET_ACK);
        MSG_TYPE_NAME(MSG_TR069_SCHEDULEINFORM);
        MSG_TYPE_NAME(MSG_TR069_SCHEDULEINFORM_ACK);
        MSG_TYPE_NAME(MSG_TR069_GET_TRANSFERCOMPLETEINFO);
        MSG_TYPE_NAME(MSG_TR069_GET_TRANSFERCOMPLETEINFO_ACK);

        MSG_TYPE_NAME(MSG_LBT_SET_STATE);
        MSG_TYPE_NAME(MSG_LBT_SET_STATE_ACK);
        MSG_TYPE_NAME(MSG_LBT_GET);
        MSG_TYPE_NAME(MSG_LBT_GET_ACK);

        MSG_TYPE_NAME(MSG_TR069_HTTPD_CONNECT);
        MSG_TYPE_NAME(MSG_TR069_HTTPD_CONNECT_ACK);
        MSG_TYPE_NAME(MSG_TR069_HTTPD_CREATE_UPFILE);
        MSG_TYPE_NAME(MSG_TR069_HTTPD_CREATE_UPFILE_ACK);

        MSG_TYPE_NAME(MSG_ETHNET_LINK_STATE);
        MSG_TYPE_NAME(MSG_ETHNET_LINK_STATE_ACK);
        MSG_TYPE_NAME(MSG_ETHWAN_INTF_NAME);
        MSG_TYPE_NAME(MSG_ETHWAN_INTF_NAME_ACK);

        MSG_TYPE_NAME(MSG_MON_INTF_REGISTER);
        MSG_TYPE_NAME(MSG_MON_INTF_REGISTER_ACK);
        MSG_TYPE_NAME(MSG_MON_INTF_UNREGISTER);
        MSG_TYPE_NAME(MSG_MON_INTF_UNREGISTER_ACK);
        MSG_TYPE_NAME(MSG_MON_INTF_STATUS_QUERY);
        MSG_TYPE_NAME(MSG_MON_INTF_STATUS_QUERY_ACK);
        MSG_TYPE_NAME(MSG_MON_INTF_STATUS_INFORM);
        MSG_TYPE_NAME(MSG_MON_INTF_STATUS_INFORM_ACK);

        MSG_TYPE_NAME(MSG_AUTH);
        MSG_TYPE_NAME(MSG_AUTH_ACK);

        MSG_TYPE_NAME(MSG_WAN_DEL_INST);
        MSG_TYPE_NAME(MSG_WAN_DEL_INST_ACK);
        MSG_TYPE_NAME(MSG_WAN_ADD_INST);
        MSG_TYPE_NAME(MSG_WAN_ADD_INST_ACK);
        MSG_TYPE_NAME(MSG_LAN_DEL_INST);
        MSG_TYPE_NAME(MSG_LAN_DEL_INST_ACK);
        MSG_TYPE_NAME(MSG_LAN_ADD_INST);
        MSG_TYPE_NAME(MSG_LAN_ADD_INST_ACK);
        MSG_TYPE_NAME(MSG_WLAN_DEL_INST);
        MSG_TYPE_NAME(MSG_WLAN_DEL_INST_ACK);
        MSG_TYPE_NAME(MSG_WLAN_ADD_INST);
        MSG_TYPE_NAME(MSG_WLAN_ADD_INST_ACK);

        MSG_TYPE_NAME(MSG_CMD);
        MSG_TYPE_NAME(MSG_CMD_ACK);
        MSG_TYPE_NAME(MSG_RESP);
        MSG_TYPE_NAME(MSG_RESP_ACK);
        MSG_TYPE_NAME(MSG_RESP_FRAGMENT);
        MSG_TYPE_NAME(MSG_RESP_FRAGMENT_ACK);
        MSG_TYPE_NAME(MSG_RESP_ERR);
        MSG_TYPE_NAME(MSG_RESP_ERR_ACK);

        MSG_TYPE_NAME(MSG_CMM_CLEAR_CFG);
        MSG_TYPE_NAME(MSG_CMM_CLEAR_CFG_ACK);

        MSG_TYPE_NAME(MSG_BUTTON);
        MSG_TYPE_NAME(MSG_BUTTON_ACK);

        MSG_TYPE_NAME(MSG_SNTP_UPDATE_TIME);
        MSG_TYPE_NAME(MSG_SNTP_UPDATE_TIME_ACK);
        MSG_TYPE_NAME(MSG_SNTP_TIME_CHANGED);
        MSG_TYPE_NAME(MSG_SNTP_TIME_CHANGED_ACK);
        MSG_TYPE_NAME(MSG_NTPS_CHANGED);
        MSG_TYPE_NAME(MSG_NTPS_CHANGED_ACK);

        MSG_TYPE_NAME(MSG_TR111_DEVICE_ADD);
        MSG_TYPE_NAME(MSG_TR111_DEVICE_ADD_ACK);
        MSG_TYPE_NAME(MSG_TR111_DEVICE_DEL);
        MSG_TYPE_NAME(MSG_TR111_DEVICE_DEL_ACK);

        MSG_TYPE_NAME(MSG_DIAGNOSTIC_COMPLETE);
        MSG_TYPE_NAME(MSG_DIAGNOSTIC_COMPLETE_ACK);

        MSG_TYPE_NAME(MSG_MSG4UDP_REGISTER);
        MSG_TYPE_NAME(MSG_MSG4UDP_REGISTER_ACK);
        MSG_TYPE_NAME(MSG_MSG4UDP_UNREGISTER);
        MSG_TYPE_NAME(MSG_MSG4UDP_UNREGISTER_ACK);
        MSG_TYPE_NAME(MSG_MSG4UDP_MESSAGE);
        MSG_TYPE_NAME(MSG_MSG4UDP_MESSAGE_ACK);

        MSG_TYPE_NAME(MSG_WAN_CONN_ENABLE_UPDATE);
        MSG_TYPE_NAME(MSG_WAN_CONN_ENABLE_UPDATE_ACK);

        MSG_TYPE_NAME(MSG_VLAN_ADD_BRIDGE);
        MSG_TYPE_NAME(MSG_VLAN_ADD_BRIDGE_ACK);
        MSG_TYPE_NAME(MSG_VLAN_DEL_BRIDGE);
        MSG_TYPE_NAME(MSG_VLAN_DEL_BRIDGE_ACK);
        MSG_TYPE_NAME(MSG_VLAN_BIND_PORT);
        MSG_TYPE_NAME(MSG_VLAN_BIND_PORT_ACK);
        MSG_TYPE_NAME(MSG_VLAN_UNBIND_PORT);
        MSG_TYPE_NAME(MSG_VLAN_UNBIND_PORT_ACK);


        MSG_TYPE_NAME(MSG_VDSL_BRIEF_STATUS_OF_PORT);
        MSG_TYPE_NAME(MSG_VDSL_BRIEF_STATUS_OF_PORT_ACK);
        MSG_TYPE_NAME(MSG_VDSL_EXTENDED_STATUS_OF_PORT);
        MSG_TYPE_NAME(MSG_VDSL_EXTENDED_STATUS_OF_PORT_ACK);
        MSG_TYPE_NAME(MSG_VDSL_STATUS);
        MSG_TYPE_NAME(MSG_VDSL_STATUS_ACK);
        MSG_TYPE_NAME(MSG_VDSL_SNR_STATUS);
        MSG_TYPE_NAME(MSG_VDSL_SNR_STATUS_ACK);
        MSG_TYPE_NAME(MSG_VDSL_BME_FIRMWARE_VERSION);
        MSG_TYPE_NAME(MSG_VDSL_BME_FIRMWARE_VERSION_ACK);

        MSG_TYPE_NAME(MSG_LAN_ETH_DEL_INST);
        MSG_TYPE_NAME(MSG_LAN_ETH_ADD_INST);

        MSG_TYPE_NAME(MSG_PORTOFF_TRIGGER);

        MSG_TYPE_NAME(MSG_CMM_INST_ADDED);
        MSG_TYPE_NAME(MSG_CMM_INST_DELED);

        MSG_TYPE_NAME(MSG_CMM_GET_VAL_IGNORE_ERR);
        MSG_TYPE_NAME(MSG_CMM_GET_VAL_IGNORE_ERR_ACK);

        MSG_TYPE_NAME(MSG_WAN_CONN_SET_CHNG);

        MSG_TYPE_NAME(MSG_WAN_ADD_LINK_DEV);
        MSG_TYPE_NAME(MSG_WAN_DEL_LINK_DEV);
        MSG_TYPE_NAME(MSG_WAN_LINK_DEV_STAT_INFORM);

        MSG_TYPE_NAME(MSG_PPPV6_CONN_EST);
        MSG_TYPE_NAME(MSG_PPPV6_CONN_FIN);
        MSG_TYPE_NAME(MSG_IP6AAC_NOTIFY);
        MSG_TYPE_NAME(MSG_IP6AAC_REQUEST);
        MSG_TYPE_NAME(MSG_IP6AAC_REQUEST_ACK);
        MSG_TYPE_NAME(MSG_IP6AAC_RELEASE);
        MSG_TYPE_NAME(MSG_IP6CONN_STAT_CHANGED);
        MSG_TYPE_NAME(MSG_IP6CONN_ADDR_CHANGED);
        MSG_TYPE_NAME(MSG_IP6CONN_DNS_CHANGED);
        MSG_TYPE_NAME(MSG_IP6CONN_NTP_CHANGED);
        MSG_TYPE_NAME(MSG_IP6CONN_PREFIX_CHANGED);
        MSG_TYPE_NAME(MSG_IP6CONN_ROUTER_CHANGED);

        MSG_TYPE_NAME(MSG_IPV6_ADDR_NOTIFY);
        MSG_TYPE_NAME(MSG_IPV6_DFLTGW_NOTIFY);
        MSG_TYPE_NAME(MSG_RS_FAILED);
        MSG_TYPE_NAME(MSG_RA_UPDATE);

        MSG_TYPE_NAME(MSG_DFLT_IP6CONN_CHANGED);
        MSG_TYPE_NAME(MSG_IP6LAN_DOMAIN_NAME_UPDATE);
        MSG_TYPE_NAME(MSG_IP6LAN_PREFIX_CHANGED);
        MSG_TYPE_NAME(MSG_IP6LAN_DNS_CHANGED);
        MSG_TYPE_NAME(MSG_IP6LAN_ADDR_CHANGED);

        MSG_TYPE_NAME(MSG_TR069_UPGRADE_ERR);

        MSG_TYPE_NAME(MSG_IPV6RD_PREFIX_SET);
        MSG_TYPE_NAME(MSG_IPV6RD_PREFIX_UNSET);

        MSG_TYPE_NAME(MSG_WLAN_ENABLE_UPDATE);

        MSG_TYPE_NAME(MSG_IPV6_SERVERTYPE_CHANGED);
        MSG_TYPE_NAME(MSG_CONN_DNS_UPDATE);
        MSG_TYPE_NAME(MSG_CONN_DNS_UPDATE_ACK);

     	default:
		{
			static char s_szUnsupported[64];
			sprintf(s_szUnsupported, "UNKNOWN_MSGTYPE(%04x)", usMsgType);
			return s_szUnsupported;
		}
	}
}

/*
*	��������: 	tbsGetMIDName
*	��������: 	���Ը�������������ֵ��ʽMIDת��Ϊ�ַ����ɶ���ʽ��
*	�������: 	usMID - MID��ֵ
*	����ֵ  :	��Ӧ��MID�ַ������֡�������ں�����ʶ��Χ�У�����
*				UNKNOWN_MID(ddd)��ʽ��
*	����    : 	������ / 2008-11-10
*/
const char* tbsGetMIDName(unsigned short usMID)
{
	#define MID_NAME(name) case name: return #name
	switch (usMID)
    {
        MID_NAME(MID_CCP);
        MID_NAME(MID_AUTH);
        MID_NAME(MID_IGMP);
        MID_NAME(MID_CMM);
        MID_NAME(MID_LAN);
        MID_NAME(MID_IPT);
        MID_NAME(MID_ETHLAN);
        MID_NAME(MID_ETHWAN);
        MID_NAME(MID_PPPOE);
        MID_NAME(MID_WLAN);
        MID_NAME(MID_TR069BE);
        MID_NAME(MID_DGN);
        MID_NAME(MID_DHCPR);
        MID_NAME(MID_DHCPS);
        MID_NAME(MID_TIMER);
        MID_NAME(MID_IPCONN);
        MID_NAME(MID_FIREWALL);
        MID_NAME(MID_SNMPC);
        MID_NAME(MID_QOS);
        MID_NAME(MID_STATIC_ROUTING);
        MID_NAME(MID_VDSL);
        MID_NAME(MID_DNS);
        MID_NAME(MID_ALG);
#if (defined(CONFIG_RT63365) || defined(CONFIG_RT63368))
        MID_NAME(MID_UTMPROXY);
        MID_NAME(MID_AUTOUPGRADE);
#endif
        MID_NAME(MID_WAN);
        MID_NAME(MID_DROUTE);
        MID_NAME(MID_SNTP);
        MID_NAME(MID_VLAN);
        MID_NAME(MID_USB_MASS);
        MID_NAME(MID_LOG);
        MID_NAME(MID_FTPD);
        MID_NAME(MID_NATPRIO);
        MID_NAME(MID_WPS);
        MID_NAME(MID_ACL);
        MID_NAME(MID_UPNP);
        MID_NAME(MID_LSVLAN);
        MID_NAME(MID_PORTOFF);
        MID_NAME(MID_ANTIATTACK);
        MID_NAME(MID_PORTMAPPING);
        MID_NAME(MID_URLFILTER);
        MID_NAME(MID_ATM);
	    MID_NAME(MID_SPT);
        MID_NAME(MID_SUPP);
        MID_NAME(MID_WEBP);
        MID_NAME(MID_PORTTRIGGER);
        MID_NAME(MID_DHCP6S);
        MID_NAME(MID_V6CONN);
        MID_NAME(MID_RAD);
		MID_NAME(MID_TRACERT);
        MID_NAME(MID_DIAG);
	    MID_NAME(MID_DEVCONFIG);
        MID_NAME(MID_WEB);
        MID_NAME(MID_LNB);
        MID_NAME(MID_CLI);
        MID_NAME(MID_TR069FE);
        MID_NAME(MID_DDNS);
        MID_NAME(MID_CFG);
        MID_NAME(MID_MACFILTER);
        MID_NAME(MID_BRIDGE_FILTER);
        MID_NAME(MID_IP_ACL);
        MID_NAME(MID_DEFAULTGW);
        MID_NAME(MID_SAMBA);
        MID_NAME(MID_TF_PORTMAPPING);
        MID_NAME(MID_TF_FIREWALL);
        MID_NAME(MID_OS_INFO);
        MID_NAME(MID_IPMACFILTER);
        MID_NAME(MID_TR069_HTTPD);
        MID_NAME(MID_SNMPA);
		MID_NAME(MID_TR064FE);
		MID_NAME(MID_MDNS);

        MID_NAME(MID_PC);
        MID_NAME(MID_ELM);
        MID_NAME(MID_UPG);
        MID_NAME(MID_VDSLD);
        MID_NAME(MID_DSL);
        MID_NAME(MID_TM);
		MID_NAME(MID_IPTUNNEL);
		MID_NAME(MID_MLD);
		MID_NAME(MID_TF_GUI);
        MID_NAME(MID_MON);
        MID_NAME(MID_LBT);
        MID_NAME(MID_UPGCGI);
        MID_NAME(MID_FTPUPG);
        MID_NAME(MID_PTI);
        MID_NAME(MID_MSG4UDP);
        MID_NAME(MID_TFTPUPG);
        MID_NAME(MID_RAMON);
        MID_NAME(MID_WANSELECT);
        MID_NAME(MID_DEVINFO);
        MID_NAME(MID_IP6MON);
        MID_NAME(MID_IP6AAC);
        MID_NAME(MID_WANMIRROR);

        MID_NAME(MID_GRP_CCP);
        MID_NAME(MID_GRP_BASE);
        MID_NAME(MID_GRP_MACLIST_UPDATE);
        MID_NAME(MID_GRP_MON_LINK_UPDATE);
        MID_NAME(MID_GRP_WAN_LINK_UPDATE);
        MID_NAME(MID_GRP_WAN_CONN_UPDATE);
        MID_NAME(MID_GRP_DNS_UPDATE);
        MID_NAME(MID_GRP_WAN_DEL_INST);
        MID_NAME(MID_GRP_WAN_ADD_INST);
        MID_NAME(MID_GRP_TIME_CHANGED);
        MID_NAME(MID_GRP_WAN_CONN_ENABLE_UPDATE);
        MID_NAME(MID_GRP_LAN_DEL_INST);
        MID_NAME(MID_GRP_LAN_ADD_INST);

        MID_NAME(MID_GRP_WLAN_DEL_INST);
        MID_NAME(MID_GRP_WLAN_ADD_INST);
        MID_NAME(MID_GRP_LAN_ETH_INST_CHG);

        MID_NAME( MID_GRP_LAN_IP_UPDATE);
        MID_NAME( MID_GRP_WAN_CONN_SET_CHNG);

        MID_NAME( MID_GRP_DEFAULT_ROUTE_CHANGE);
        MID_NAME( MID_GRP_IP6CONN_STAT_CHANGED);
        MID_NAME( MID_GRP_IP6CONN_ADDR_CHANGED);
        MID_NAME( MID_GRP_IP6LAN_ADDR_CHANGED);
        MID_NAME( MID_GRP_DFLT_IP6CONN_CHANGED);
        MID_NAME( MID_GRP_IP6LAN_PREFIX_CHANGED);
        MID_NAME( MID_GRP_IP6CONN_DNS_CHANGED);
        MID_NAME( MID_GRP_IP6CONN_ROUTER_CHANGED);
		MID_NAME( MID_GRP_WAN_PROTOCOL_CHANGED);
		
        MID_NAME(MID_NULL);

	 	default:
		{
			static char s_szUnsupported[64];
			sprintf(s_szUnsupported, "UNKNOWN_MID(%04x)", usMID);
			return s_szUnsupported;
		}
	}
}


