#if 0
/* this block has been defined in arch/mips/include/asm/tc3162/TCIfSetQuery_os.h */
 */
#define VDSL2_QUERY_BONDING_BACP_SUPPORT    0x2003
#define TCIF_SET_BONDING_BACP_SUPPORT       0x3001
#define BONDING_OFF_BACP_OFF	0x0
#define BONDING_ON_BACP_OFF		0x1
#define BONDING_OFF_BACP_ON		0x2
#define BONDING_ON_BACP_ON		0x3
#endif


/* !!!!!! Do the following in ASIC !!!!!!!!
 * after loaing dmt.ko (dmt_slave.ko), wait 1 sec 
 * and then do "w ad bondingBacp 3" ("w adsl_slave bondingBacp 3")
 * in rcS to let dmt.ko know that CPE is bonding & BACP
 * supported! 
 * Note: "w ad bondingBacp 3" is implemented in doBondingBacp()
 * in tcci/tcadslcmd.c 
 */


