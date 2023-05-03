/* Shared library add-on to iptables to add MAC address support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#if defined(__GLIBC__) && __GLIBC__ == 2
#include <net/ethernet.h>
#else
#include <linux/if_ether.h>
#endif
#include <xtables.h>
#include <linux/netfilter/xt_mac.h>

static void mac_help(void)
{
	printf(
"mac match options:\n"
"[!] --mac-source XX:XX:XX:XX:XX:XX\n"
"				Match source MAC address\n"
"mac match options:\n"
"[!] --mac-dest XX:XX:XX:XX:XX:XX\n"
"				Match dest MAC address\n");
}

static const struct option mac_opts[] = {
	{ "mac-source", 1, NULL, '1' },
	{ "mac-dest", 1, NULL, '2' },
	{ .name = NULL }
};

static void
parse_mac(const char *mac, struct xt_mac_info *info)
{
	unsigned int i = 0;

	if (strlen(mac) != ETH_ALEN*3-1)
		xtables_error(PARAMETER_PROBLEM, "Bad mac address \"%s\"", mac);

	for (i = 0; i < ETH_ALEN; i++) {
		long number;
		char *end;

		number = strtol(mac + i*3, &end, 16);
        if(info->flags & MAC_SRC)
        {
			if (end == mac + i*3 + 2
			    && number >= 0
			    && number <= 255)
				info->srcaddr.macaddr[i] = number;
			else
				xtables_error(PARAMETER_PROBLEM,
					   "Bad mac address `%s'", mac);
        }
		else if(info->flags & MAC_DST)
		{
			if (end == mac + i*3 + 2
			    && number >= 0
			    && number <= 255)
				info->dstaddr.macaddr[i] = number;
			else
				xtables_error(PARAMETER_PROBLEM,
					   "Bad mac address `%s'", mac);
        }
	}
}

static int
mac_parse(int c, char **argv, int invert, unsigned int *flags,
          const void *entry, struct xt_entry_match **match)
{
	struct xt_mac_info *macinfo = (struct xt_mac_info *)(*match)->data;

	switch (c) {
	case '1':
		macinfo->flags |= MAC_SRC;
		xtables_check_inverse(optarg, &invert, &optind, 0, argv);
		parse_mac(optarg, macinfo);
		if (invert)
			macinfo->flags |= MAC_SRC_INV;
		*flags = 1;
		
		break;
	case '2':
		macinfo->flags |= MAC_DST;
		xtables_check_inverse(optarg, &invert, &optind, 0, argv);
		parse_mac(optarg, macinfo);
		if (invert)
			macinfo->flags |= MAC_DST_INV;
		*flags = 1;
		
		break;

	default:
		return 0;
	}

	return 1;
}

static void print_mac(const unsigned char macaddress[ETH_ALEN])
{
	unsigned int i;

	printf("%02X", macaddress[0]);
	for (i = 1; i < ETH_ALEN; i++)
		printf(":%02X", macaddress[i]);
	printf(" ");
}

static void mac_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			   "You must specify `--mac-source'");
}

static void
mac_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_mac_info *info = (void *)match->data;
	printf("MAC ");

	if((info->flags & MAC_SRC_INV) || (info->flags & MAC_DST_INV))
		printf("! ");
	
	if(info->flags & MAC_SRC)
	{
	    print_mac(info->srcaddr.macaddr);
	}

	if(info->flags & MAC_DST)
	{
	    print_mac(info->dstaddr.macaddr);
	}
}

static void mac_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_mac_info *info = (void *)match->data;

	if((info->flags & MAC_SRC_INV) || (info->flags & MAC_DST_INV))
		printf("! ");
	
    if(info->flags & MAC_SRC)
    {
	    printf("--mac-source ");
	    print_mac(info->srcaddr.macaddr);
    }

	if(info->flags & MAC_DST)
    {
	    printf("--mac-dest ");
	    print_mac(info->dstaddr.macaddr);
    }
}

static struct xtables_match mac_match = {
	.family		= NFPROTO_UNSPEC,
 	.name		= "mac",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_mac_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_mac_info)),
	.help		= mac_help,
	.parse		= mac_parse,
	.final_check	= mac_check,
	.print		= mac_print,
	.save		= mac_save,
	.extra_opts	= mac_opts,
};

void _init(void)
{
	xtables_register_match(&mac_match);
}
