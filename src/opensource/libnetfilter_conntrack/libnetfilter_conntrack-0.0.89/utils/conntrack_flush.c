#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include "internal.h"

static int count = 0,pcount = 0;

static int cb(enum nf_conntrack_msg_type type,
	      struct nf_conntrack *ct,
	      void *data)
{
	int ret;
	struct nfct_handle *h;
	char buf[1024];


	count ++;
	pcount ++;

	nfct_snprintf(buf, 1024, ct, NFCT_T_UNKNOWN, NFCT_O_DEFAULT, NFCT_OF_SHOW_LAYER3);
	//printf("count:%4d,buf:%s\n", count,buf);

	h = nfct_open(CONNTRACK, 0);
	if (!h) {
		perror("nfct_open");
		return -1;
	}

	ret = nfct_query(h, NFCT_Q_DESTROY, ct);
	//printf("TBS: flush conntrack (%d)(%s)\n", ret, strerror(errno));
	nfct_close(h);
	return NFCT_CB_CONTINUE;
}

int main()
{
	int i = 0;
	int ret;
	u_int8_t family = AF_INET;
	struct nfct_handle *h;
	char buf[1024];
	pcount = 0;
	h = nfct_open(CONNTRACK, 0);
	if (!h) {
		perror("nfct_open");
		return -1;
	}

	nfct_callback_register(h, NFCT_T_ALL, cb, NULL);	
	
	while (1)
	{	
		count = 0;
		i ++;
		
		ret = nfct_query(h, NFCT_Q_DUMP, &family);
		//printf("TBS: dump conntrack (%d)(%s)\n", ret, strerror(errno));

		if (ret == -1 || count == 0 || i>=3)
			break ;
	}
	printf("conntrack_flush:%d,i:%d\n",pcount,i);
	if (ret == -1)
		exit(EXIT_FAILURE);

	nfct_close(h);
}
