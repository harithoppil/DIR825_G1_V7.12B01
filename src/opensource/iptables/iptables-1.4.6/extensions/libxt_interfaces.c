/* Shared library add-on to iptables to add NFMARK matching support. */
#include <stdbool.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <xtables.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_interfaces.h>



static const struct option interfaces_opts[] = {
	{"in", 1, NULL, 'I'},
	{"out", 1, NULL, 'O'},
	{"both", 1, NULL, 'B'},
	{NULL},
};


static void interfaces_help(void)
{
	printf(
"interfaces match options:\n"
"in|out|both inteface_name\n");
}

static void interfaces_print(const void *ip,
                             const struct xt_entry_match *match, int numeric)
{
	const struct xt_interfaces_info *info = (const void *)match->data;

	printf("Interfaces match ");

	if (info->direction ==1)
		printf("in ");
        else if(info->direction ==2)
                printf("out ");
        else 
                printf("both ");

        if (info->invert)
		printf("!");
	
        printf("%s",info->interfaces);

}

static int
interfaces_parse(int c, char **argv, int invert, unsigned int *flags,
           const void *entry, struct xt_entry_match **match)
{
	char *err;
	int i;
        struct xt_interfaces_info *info = (struct xt_interfaces_info *)(*match)->data;
	switch (c) {
	case 'I':
		if (*flags & 0x1)
			iptables_exit_error(PARAMETER_PROBLEM,
				"--in may be given only once");
		*flags |= 0x1;
		xtables_check_inverse(optarg, &invert, &optind, 0,argv);
                info->direction =1;
                strcpy(info->interfaces,optarg);
//		info->   = strtoul(argv[optind-1], NULL, 0);
		info->invert = invert; 
		break;
	case 'O':
		if (*flags & 0x2)
			iptables_exit_error(PARAMETER_PROBLEM,
				"--out may be given only once");

		*flags |= 0x2;
        	xtables_check_inverse(optarg, &invert, &optind, 0,argv);
                info->direction =2;
                strcpy(info->interfaces,optarg);
		info->invert = invert; 
		break;
        case 'B':
                if (*flags & 0x4)
	             iptables_exit_error(PARAMETER_PROBLEM,
	                  "--both may be given only once");
		*flags |= 0x4;
		xtables_check_inverse(optarg, &invert, &optind, 0,argv);
                info->direction =3;
                strcpy(info->interfaces,optarg);
		info->invert = invert; 
            break;
	default:
		return 0;
       }
 //   printf("%s***%d*** %d** line:%d*\n",info->interfaces,info->direction,info->invert,__LINE__);
    return 1;
}

/*
static void interfaces_check(unsigned int flags)
{
	if (flags == 0)
		xtables_error(PARAMETER_PROBLEM,
			   "mark match: The --interfaces option is required");
}
*/
static struct xtables_match interfaces_mt_reg = {
	.name          = "interfaces",
	.family        = NFPROTO_UNSPEC,
	.version       = XTABLES_VERSION,
	.size          = XT_ALIGN(sizeof(struct xt_interfaces_info)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_interfaces_info)),
	.help          = interfaces_help,
	.parse         = interfaces_parse,
	.print         = interfaces_print,	
//	.final_check   = interfaces_check,	
	.extra_opts    = interfaces_opts,
};



void _init(void)
{
	xtables_register_match(&interfaces_mt_reg);
}


