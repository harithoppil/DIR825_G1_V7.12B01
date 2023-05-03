#ifndef _QDMA_TEST_H_
#define _QDMA_TEST_H_


#ifdef CONFIG_SUPPORT_SELF_TEST

typedef struct {
#ifdef __BIG_ENDIAN
	uint resv1				: 24 ;
	uint channel			: 4 ;
	uint queue				: 4 ;
#else
	uint queue				: 4 ;
	uint channel			: 4 ;
	uint resv1				: 24 ;
#endif /* __BIG_ENDIAN */

#ifdef __BIG_ENDIAN
	uint resv2				: 10 ;
	uint fport				: 3 ;
	uint resv3				: 19 ;
#else
	uint resv3				: 19 ;
	uint fport				: 3 ;
	uint resv2				: 10 ;
#endif /* __BIG_ENDIAN */
} QDMA_TxMsg_T ;

typedef struct {
#ifdef __BIG_ENDIAN
	uint resv1				: 28 ;
	uint channel			: 4 ;
#else
	uint channel			: 4 ;
	uint resv1				: 28 ;
#endif /* __BIG_ENDIAN */
	uint resv2 ;
	uint resv3 ;
	uint resv4 ;
} QDMA_RxMsg_T ;

typedef struct {
	uint tx_frames ;
	uint rx_frames ;
	uint rx_err_frames ;
} QDMA_DbgCounters_T ;

#endif /* CONFIG_SUPPORT_SELF_TEST */




int qdma_dvt_init(void) ;
int qdma_dvt_deinit(void) ;

#endif /* _QDMA_TEST_H_ */