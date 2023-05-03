/**********************************************************************
 Copyright (c), 1991-2008, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 �ļ����� : nfmark.h
 �ļ����� : skb->nfmark bitλ����

 �޶���¼ :
          1 ���� : pengyao
            ���� : 2010-10-25
            ���� :
**********************************************************************/
#ifndef __NFMARK_H__
#define __NFMARK_H__


/*********************************************************************
 *                              mark��غ궨��                       *
 *********************************************************************/

/*
skb->nfmark 32bitλ����:
                            1111    //0 - 3bit����ClsIdx
                         111        //4 - 6bit����L2/L3/FW/APP/POST����
                        1           //7 bit�������������������
                    1111            //8 - 11bit����bridgeʵ�����
                1111                //12 - 15bit����dummyport�豸��ű��
           11111                    //16 - 20bit���ڲ���·�ɱ��
          1                         //21 bit VLAN�鲥�����
         1                          //22 bit �����鲥���ȼ�����
111111111                           //23 - 31bitԤ��


0 - 3bit����ClsIdx
4 - 6bit����L2/L3/FW/APP/POST����
7 bit�������������������

8 - 11bit����bridgeʵ�����
12 - 15bit����dummy�豸��ű��
16 - 20bit���ڲ���·�ɱ��
21 bit VLAN�鲥�����
22 bit �����鲥���ȼ�����
23 - 31bitԤ��
*/

/*ClsIdxռ�õ�bitλ*/
#define CLASSIDX_NFMARK_START_BIT         (0)
#define CLASSIDX_NFMARK_END_BIT           (3)
/*ClsIdx��Ӧ��mask*/
#define CLASSIDX_NFMARK_MASK \
    ((0xFFFFFFFFUL<<CLASSIDX_NFMARK_START_BIT) - (0xFFFFFFFFUL<<(CLASSIDX_NFMARK_END_BIT + 1)))


/*L2/L3/FW/APP/POSTռ�õ�bitλ*/
#define TYPE_NFMARK_START_BIT             (4)
#define TYPE_NFMARK_END_BIT               (6)
/*L2/L3/FW/APP/POST��Ӧ��mask*/
#define TYPE_NFMARK_MASK \
    ((0xFFFFFFFFUL<<TYPE_NFMARK_START_BIT) - (0xFFFFFFFFUL<<(TYPE_NFMARK_END_BIT + 1)))


/*�����б�ǵ�bitλ*/
#define UP_DOWN_NFMARK_BIT                (7)
/*������mark���mask*/
#define UP_DOWN_NFMARK_MASK     (0x1UL<<UP_DOWN_NFMARK_BIT)
#define UP_DOWN_STREAM_MARK(iMark)  \
    (UP_DOWN_NFMARK_MASK&(iMark<<UP_DOWN_NFMARK_BIT))
#define DOWN_STREAM             (0x1UL<<UP_DOWN_NFMARK_BIT)
#define UP_STREAM               (0UL)


/*bridge markռ�õ�bitλ*/
#define VLAN_BRID_MARK_START_BIT          (8)
#define VLAN_BRID_MARK_END_BIT            (11)
/*bridgeʵ��id��Ӧ��mask*/
#define VLAN_BRKEY_MARK_MASK \
    ((0XFFFFFFFFUL<<VLAN_BRID_MARK_START_BIT) - (0XFFFFFFFFUL<<(VLAN_BRID_MARK_END_BIT + 1)))
/*ʮ����bridgeʵ��idת��:mask+mark��ʾ*/
#define VLAN_GET_BRKEY_MARK(iMark) \
            (VLAN_BRKEY_MARK_MASK&(iMark<<VLAN_BRID_MARK_START_BIT))


/*dummyport�豸���ռ�õ�bitλ*/
#define DUMMYPORTDEV_MARK_START_BIT       (12)
#define DUMMYPORTDEV_MARK_END_BIT         (15)
/*dummyport�豸��Ŷ�Ӧ��mask*/
#define DUMMYPORTDEV_MARK_MASK \
    ((0XFFFFFFFFUL<<DUMMYPORTDEV_MARK_START_BIT) - (0XFFFFFFFFUL<<(DUMMYPORTDEV_MARK_END_BIT + 1)))
/*ʮ����dummyport�豸���ת��:mask+mark��ʾ*/
#define DUMMYPORTDEV_MARK(iMark) \
            (DUMMYPORTDEV_MARK_MASK&(iMark<<DUMMYPORTDEV_MARK_START_BIT))


/*����·�ɱ��ռ�õ�bitλ*/
#define PROUTE_MARK_START_BIT             (16)
#define PROUTE_MARK_END_BIT               (20)
/*����·�ɱ��mark��Ӧ��mask*/
#define PROUTE_MARK_MASK \
    ((0XFFFFFFFFUL<<PROUTE_MARK_START_BIT) - (0XFFFFFFFFUL<<(PROUTE_MARK_END_BIT + 1)))
/* ����·��mark ��׼ֵ*/
#define PROUTE_MARK_BASE  200
/*ʮ���Ʋ���·�ɱ��ת��:mask+mark��ʾ����skb->mark��ȡ��ʱӦ����PROUTE_MARK_BASE*/
#define PROUTE_MARK(iMark) \
        (PROUTE_MARK_MASK&((iMark - PROUTE_MARK_BASE)<<PROUTE_MARK_START_BIT))
/*��skb->mark����ȡ����·�ɱ��*/
#define GET_PROUTE_MARK_BY_MASK(mark) \
            (((PROUTE_MARK_MASK&(mark))>>PROUTE_MARK_START_BIT) + PROUTE_MARK_BASE)

/*��sk->mark����ȡ��Ǹ�skb->mark��ֵ*/
#define GET_SK_MARK_BY_PROUTE_MASK(sk_mark) \
            (PROUTE_MARK_MASK&((sk_mark - PROUTE_MARK_BASE)<<PROUTE_MARK_START_BIT))


/*vlan�鲥�����ռ�õ�bitλ��Ŀǰ����mc_forward������*/
#define VLAN_MULTICAST_MARK_BIT           (21)
/*VLAN�鲥��Ǻ�mask��������ֱ��ת��vlan�鲥����*/
#define VLAN_MULTICAST_MASK     (0x1UL<<VLAN_MULTICAST_MARK_BIT)
#define VLAN_MULTICAST_MARK     (0x1UL<<VLAN_MULTICAST_MARK_BIT)


/*�����鲥���ȼ�����*/
#define WLAN_MULTICAST_PRIORITY_BIT       (22)
#define WLAN_MULTICAST_PRIORITY_MASK      (0x1UL<<WLAN_MULTICAST_PRIORITY_BIT)
#define WLAN_MULTICAST_PRIORITY_MARK      (0x1UL<<WLAN_MULTICAST_PRIORITY_BIT)


/******************************************************************************
 *                                 END                                        *
 ******************************************************************************/
#endif /* __NFMARK_H__ */

