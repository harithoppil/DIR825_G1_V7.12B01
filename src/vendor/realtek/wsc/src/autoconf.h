/*
 * Automatically generated C config: don't edit
 */
#define AUTOCONF_INCLUDED

/*
 * Core Applications
 */
#define CONFIG_USER_INIT_INIT 1
#undef  CONFIG_USER_INIT_CONSOLE_SH
#undef  CONFIG_USER_INIT_RUN_FIREWALL
#undef  CONFIG_USER_SASH_SH
#undef  CONFIG_USER_SH_SH
#undef  CONFIG_USER_NWSH_SH
#undef  CONFIG_USER_BASH_BASH
#define CONFIG_USER_OTHER_SH 1
#undef  CONFIG_USER_SASH_REBOOT
#undef  CONFIG_USER_SASH_SHUTDOWN
#undef  CONFIG_USER_INIT_EXPAND
#undef  CONFIG_USER_VERSION_VERSION
#define CONFIG_USER_LOGIN_LOGIN 1
#undef  CONFIG_USER_OLD_PASSWORDS
#define CONFIG_USER_CLI 1
#define CONFIG_USER_MENU_CLI 1
#undef  CONFIG_USER_CMD_CLI
#undef  CONFIG_USER_AGETTY_AGETTY
#undef  CONFIG_USER_GETTYD_GETTYD
#undef  CONFIG_USER_LOGIN_PASSWD
#undef  CONFIG_USER_CRON_CRON
#define CONFIG_USER_WATCHDOG_WDG 1

/*
 * Real Time Clock
 */
#undef  CONFIG_USER_HWCLOCK_HWCLOCK
#undef  CONFIG_USER_RTC_M41T11
#undef  CONFIG_USER_RTC_DS1302

/*
 * Vixie-cron
 */
#undef  CONFIG_USER_VIXIECRON_CRON
#undef  CONFIG_USER_VIXIECRON_CRONTAB

/*
 * at
 */
#undef  CONFIG_USER_AT_AT
#undef  CONFIG_USER_AT_ATD
#undef  CONFIG_USER_AT_ATRUN

/*
 * memory utility
 */
#undef  CONFIG_USER_SFREE

/*
 * Library Configuration
 */

/*
 * Force build (Normally built when required)
 */
#undef  CONFIG_LIB_LIBAES_FORCE
#undef  CONFIG_LIB_LIBDES_FORCE
#undef  CONFIG_LIB_LIBSSL_FORCE
#undef  CONFIG_LIB_LIBGMP_FORCE
#undef  CONFIG_LIB_LIBG_FORCE
#undef  CONFIG_LIB_LIBPAM_FORCE
#undef  CONFIG_LIB_LIBPCAP_FORCE
#undef  CONFIG_LIB_ZLIB_FORCE
#undef  CONFIG_LIB_LIBATM_FORCE
#undef  CONFIG_LIB_LIBPNG_FORCE
#undef  CONFIG_LIB_LIBJPEG_FORCE
#undef  CONFIG_LIB_NCURSES_FORCE

/*
 * Library Configuration
 */
#undef  CONFIG_LIB_UC_LIBC_TIMEZONE

/*
 * Flash Tools
 */
#undef  CONFIG_USER_FLASHW_FLASHW
#undef  CONFIG_USER_NETFLASH_NETFLASH
#undef  CONFIG_USER_RECOVER_RECOVER
#undef  CONFIG_USER_BOOTTOOLS_FLASHLOADER
#undef  CONFIG_USER_BOOTTOOLS_HIMEMLOADER

/*
 * MTD utils
 */
#undef  CONFIG_USER_MTDUTILS
#undef  CONFIG_USER_MTDUTILS_ERASE
#undef  CONFIG_USER_MTDUTILS_ERASEALL
#undef  CONFIG_USER_MTDUTILS_FTL_CHECK
#undef  CONFIG_USER_MTDUTILS_FTL_FORMAT
#undef  CONFIG_USER_MTDUTILS_MKFSJFFS
#undef  CONFIG_USER_MTDUTILS_MKFSJFFS2
#undef  CONFIG_USER_MTDUTILS_NFTLDUMP
#undef  CONFIG_USER_MTDUTILS_NFTL_FORMAT
#undef  CONFIG_USER_MTDUTILS_NANDDUMP
#undef  CONFIG_USER_MTDUTILS_NANDTEST
#undef  CONFIG_USER_MTDUTILS_NANDWRITE
#undef  CONFIG_USER_MTDUTILS_DOC_LOADBIOS
#undef  CONFIG_USER_MTDUTILS_DOC_LOADIPL

/*
 * Filesystem Applications
 */
#define CONFIG_USER_FLATFSD_XXX 1
#define CONFIG_USER_FLATFSD_AUTO 1
#undef  CONFIG_USER_FLATFSD_USE_FLASH_FS
#undef  CONFIG_USER_FLATFSD_DISKLIKE
#undef  CONFIG_USER_FLATFSD_COMPRESSED
#undef  CONFIG_USER_FLATFSD_HAS_RTC
#undef  CONFIG_USER_MOUNT_MOUNT
#undef  CONFIG_USER_MOUNT_UMOUNT
#undef  CONFIG_USER_FDISK_FDISK

/*
 * EXT2
 */
#undef  CONFIG_USER_E2FSPROGS_E2FSCK_E2FSCK
#undef  CONFIG_USER_E2FSPROGS_MISC_MKE2FS
#undef  CONFIG_USER_E2FSPROGS_MISC_BADBLOCKS
#undef  CONFIG_USER_E2FSPROGS_MISC_CHATTR
#undef  CONFIG_USER_E2FSPROGS_MISC_DUMPE2FS
#undef  CONFIG_USER_E2FSPROGS_MISC_E2LABEL
#undef  CONFIG_USER_E2FSPROGS_MISC_FSCK
#undef  CONFIG_USER_E2FSPROGS_MISC_LSATTR
#undef  CONFIG_USER_E2FSPROGS_MISC_MKLOST_FOUND
#undef  CONFIG_USER_E2FSPROGS_MISC_TUNE2FS
#undef  CONFIG_USER_E2FSPROGS_MISC_UUIDGEN

/*
 * RESIERFS
 */
#undef  CONFIG_USER_REISERFSPROGS
#undef  CONFIG_USER_REISERFSPROGS_DEBUGRESIERFS
#undef  CONFIG_USER_REISERFSPROGS_MKREISERFS
#undef  CONFIG_USER_REISERFSPROGS_REISERFSCK
#undef  CONFIG_USER_REISERFSPROGS_RESIZE_REISERFS
#undef  CONFIG_USER_REISERFSPROGS_UNPACK

/*
 * SAMBA-3.2.4
 */
#undef  CONFIG_USER_SAMBA_3_2_4

/*
 * SAMBA
 */
#undef  CONFIG_USER_SAMBA
#undef  CONFIG_USER_SAMBA_SMBD
#undef  CONFIG_USER_SAMBA_NMBD
#undef  CONFIG_USER_SAMBA_SMBMOUNT
#undef  CONFIG_USER_SAMBA_SMBUMOUNT

/*
 * SMBFS
 */
#undef  CONFIG_USER_SMBMOUNT_SMBMOUNT
#undef  CONFIG_USER_SMBMOUNT_SMBUMOUNT

/*
 * CRAMFS
 */
#undef  CONFIG_USER_CRAMFS_CRAMFSCK
#undef  CONFIG_USER_CRAMFS_MKCRAMFS

/*
 * NTFS
 */
#undef  CONFIG_USER_NTFS_NTFS3G

/*
 * Network Applications
 */
#define CONFIG_USER_ROUTE_ARP 1
#undef  CONFIG_USER_AUTO_PROVISIONING
#define CONFIG_USER_BOA_SRC_BOA 1
#undef  CONFIG_USER_BOA_WITH_SSL
#define CONFIG_DEFAULT_WEB 1
#undef  CONFIG_GUI_WEB
#undef  ZTE_531B_BRIDGE_SC
#undef  ZTE_GENERAL_ROUTER_SC
#undef  ZTE_GENERAL_ROUTER_EN
#undef  CONFIG_BOA_WEB_E8B_CH
#undef  CONFIG_USER_BPALOGIN_BPALOGIN
#define CONFIG_USER_BR2684CTL_BR2684CTL 1
#undef  CONFIG_USER_BRCFG_BRCFG
#define CONFIG_USER_BRCTL_BRCTL 1
#define CONFIG_USER_CWMP_TR069 1
#undef  CONFIG_MIDDLEWARE
#undef  CONFIG_USER_TR143
#undef  CONFIG_USER_CWMP_WITHOUT_SSL
#undef  CONFIG_USER_CWMP_WITH_OPENSSL
#define CONFIG_USER_CWMP_WITH_MATRIXSSL 1
#define CONFIG_USER_CWMP_WITH_SSL 1
#define CONFIG_LIB_LIBMATRIXSSL 1
#define CONFIG_USER_FLATFSD_XXX 1
#define CONFIG_USER_FLATFSD_AUTO 1
#define CONFIG_USER_DDNS 1
#undef  CONFIG_USER_DHCPCD_DHCPCD
#undef  CONFIG_USER_DHCPCD_NEW_DHCPCD
#undef  CONFIG_USER_DHCPD_DHCPD
#undef  CONFIG_USER_DHCP_ISC_SERVER_DHCPD
#undef  CONFIG_USER_DHCP_ISC_CLIENT_DHCLIENT
#define CONFIG_USER_DHCP_ISC_RELAY_DHCRELAY 1
#undef  CONFIG_USER_DIALD_DIALD
#undef  CONFIG_USER_DISCARD_DISCARD
#define CONFIG_USER_DNSMASQ_DNSMASQ 1
#undef  CONFIG_USER_SSH_DROPBEAR
#undef  CONFIG_USER_ETHATTACH_ETHATTACH
#undef  CONFIG_USER_EZIPUPDATE_EZIPUPDATE
#define CONFIG_USER_FTP_FTP_FTP 1
#define CONFIG_USER_FTPD_FTPD 1
#undef  CONFIG_USER_BFTPD_BFTPD
#undef  CONFIG_USER_FREESWAN
#undef  CONFIG_USER_HTTPD_HTTPD
#undef  CONFIG_USER_IDB_IDB
#undef  CONFIG_USER_IFATTACH_IFATTACH
#undef  CONFIG_USER_ROUTE_IFCONFIG
#undef  CONFIG_USER_IFMOND2_IFMOND
#define CONFIG_USER_IGMPPROXY 1
#define CONFIG_USER_INETD_INETD 1
#undef  CONFIG_USER_IPCHAINS_IPCHAINS
#undef  CONFIG_USER_IPFWADM_IPFWADM
#undef  CONFIG_USER_IPMASQADM_IPMASQADM
#undef  CONFIG_USER_IPPORTFW_IPPORTFW
#undef  CONFIG_USER_IPREDIR_IPREDIR
#define CONFIG_USER_IPROUTE2 1
#define CONFIG_USER_IPROUTE2_TC_TC 1
#undef  CONFIG_USER_IPROUTE2_IP_IFCFG
#undef  CONFIG_USER_IPROUTE2_IP_IP
#undef  CONFIG_USER_IPROUTE2_IP_ROUTEF
#undef  CONFIG_USER_IPROUTE2_IP_ROUTEL
#undef  CONFIG_USER_IPROUTE2_IP_RTACCT
#undef  CONFIG_USER_IPROUTE2_IP_RTMON
#undef  CONFIG_USER_IPROUTE2_IP_RTPR
#define CONFIG_USER_IPTABLES_IPTABLES 1
#undef  CONFIG_BRIDGE_NF_EBTABLES
#undef  CONFIG_USER_KLAXON_KLAXON
#undef  CONFIG_USER_L2TPD_L2TPD
#undef  CONFIG_USER_LOATTACH_LOATTACH
#undef  CONFIG_USER_SMTP_SMTPCLIENT
#undef  CONFIG_USER_MAIL_MAIL_IP
#undef  CONFIG_USER_MINI_HTTPD_MINI_HTTPD
#undef  CONFIG_USER_MSNTP_MSNTP
#undef  CONFIG_USER_ROUTE_NETSTAT
#undef  CONFIG_USER_NETSTAT_NAT_NETSTAT_NAT
#undef  CONFIG_USER_ROUTE_MIITOOL
#define CONFIG_USER_NETLOGGER_SUPPORT 1
#undef  CONFIG_USER_OPENSSL_APPS
#undef  CONFIG_USER_PARENTAL_CONTROL
#undef  CONFIG_USER_PING_PING
#undef  CONFIG_USER_PKTDELAY_PKTDELAY
#undef  CONFIG_USER_PLUG_PLUG
#undef  CONFIG_USER_PORTMAP_PORTMAP
#undef  CONFIG_USER_PPPOE_PROXY
#undef  CONFIG_USER_PPPD_PPPD_PPPD
#undef  CONFIG_USER_PPTPD_PPTPCTRL
#undef  CONFIG_USER_PPTPD_PPTPD
#undef  CONFIG_USER_PPTP_CLIENT_PPTP
#undef  CONFIG_USER_PPTP_CLIENT_PPTP_CALLMGR
#undef  CONFIG_USER_REMOTE_MANAGEMENT
#undef  CONFIG_USER_RDATE_RDATE
#undef  CONFIG_USER_ROUTE_ROUTE
#define CONFIG_USER_ROUTED_ROUTED 1
#undef  CONFIG_USER_RP_PPPOE_PPPOE
#undef  CONFIG_USER_SETHDLC_SETHDLC
#undef  CONFIG_USER_SLATTACH_SLATTACH
#undef  CONFIG_USER_SNMPD_SNMPD
#undef  CONFIG_USER_SNMPD_SNMPD_V2C
#define CONFIG_USER_SNMPD_SNMPD_V2CTRAP 1
#undef  CONFIG_USER_STUNNEL_STUNNEL
#undef  CONFIG_USER_SQUID_SQUID
#undef  CONFIG_USER_SSH_SSH
#undef  CONFIG_USER_SSH_SSHD
#undef  CONFIG_USER_SSH_SSHKEYGEN
#undef  CONFIG_USER_STP_STP
#undef  CONFIG_USER_TCPWRAP_TCPD
#undef  CONFIG_USER_TCPBLAST_TCPBLAST
#undef  CONFIG_USER_TCPDUMP_TCPDUMP
#define CONFIG_USER_TELNETD_TELNETD 1
#undef  CONFIG_USER_TELNET_TELNET
#undef  CONFIG_USER_TFTP_TFTP
#define CONFIG_USER_TFTPD_TFTPD 1
#undef  CONFIG_USER_THTTPD_THTTPD
#undef  CONFIG_USER_TRACEROUTE_TRACEROUTE
#undef  CONFIG_USER_UCDSNMP_SNMPD
#undef  CONFIG_USER_UPNPD
#define CONFIG_USER_MINIUPNPD 1
#define CONFIG_USE_SHARED_LIB 1
#define CONFIG_USER_MINI_UPNPD 1
#undef  CONFIG_USER_VPNLED_VPNLED
#define CONFIG_USER_VSNTP 1
#undef  CONFIG_USER_WGET
#undef  CONFIG_USER_WGET_MANAGE
#undef  CONFIG_USER_ZEBRA_ZEBRA_ZEBRA

/*
 * new feature
 */

/*
 * Net-tools
 */
#undef  CONFIG_USER_NET_TOOLS_ARP
#undef  CONFIG_USER_NET_TOOLS_HOSTNAME
#undef  CONFIG_USER_NET_TOOLS_IFCONFIG
#undef  CONFIG_USER_NET_TOOLS_NAMEIF
#undef  CONFIG_USER_NET_TOOLS_NETSTAT
#undef  CONFIG_USER_NET_TOOLS_PLIPCONFIG
#undef  CONFIG_USER_NET_TOOLS_RARP
#undef  CONFIG_USER_NET_TOOLS_ROUTE
#undef  CONFIG_USER_NET_TOOLS_SLATTACH
#undef  CONFIG_USER_NET_TOOLS_MII_TOOL

/*
 * Wireless-tools
 */
#define CONFIG_USER_WIRELESS_TOOLS 1
#undef  CONFIG_USER_WIRELESS_TOOLS_IWCONFIG
#undef  CONFIG_USER_WIRELESS_TOOLS_IWGETID
#undef  CONFIG_USER_WIRELESS_TOOLS_IWLIST
#define CONFIG_USER_WIRELESS_TOOLS_IWPRIV 1
#undef  CONFIG_USER_WIRELESS_TOOLS_IWSPY
#undef  CONFIG_USER_WIRELESS_MBSSID
#define CONFIG_USER_WIRELESS_TOOLS_RTL8185_AUTH 1
#undef  CONFIG_USER_WIRELESS_TOOLS_RTL8185_IAPP
#define CONFIG_USER_BUSYBOX_CP 1
#define CONFIG_USER_MINI_UPNPD 1
#undef  CONFIG_USER_WIRELESS_WDS

/*
 * Miscellaneous Applications
 */
#undef  CONFIG_USER_LANG_A60
#undef  CONFIG_USER_CAL_CAL
#undef  CONFIG_USER_CHAT_CHAT
#undef  CONFIG_USER_CKSUM_CKSUM
#undef  CONFIG_USER_CLOCK_CLOCK
#undef  CONFIG_USER_CPU_CPU
#undef  CONFIG_USER_CAL_DATE
#undef  CONFIG_USER_DHRYSTONE_DHRYSTONE
#undef  CONFIG_USER_FROB_LED_FROB_LED
#undef  CONFIG_USER_GDBSERVER_GDBREPLAY
#undef  CONFIG_USER_GDBSERVER_GDBSERVER
#undef  CONFIG_USER_HD_HD
#undef  CONFIG_USER_LCD_LCD
#undef  CONFIG_USER_LEDCON_LEDCON
#undef  CONFIG_USER_LILO_LILO
#undef  CONFIG_USER_LISSA_LISSA
#undef  CONFIG_USER_MATH_TEST
#undef  CONFIG_USER_MAWK_AWK
#undef  CONFIG_USER_NULL_NULL
#undef  CONFIG_USER_PALMBOT_PALMBOT
#undef  CONFIG_USER_PCMCIA_CS
#undef  CONFIG_USER_PERL_PERL
#undef  CONFIG_USER_PYTHON_PYTHON
#undef  CONFIG_USER_READPROFILE_READPROFILE
#undef  CONFIG_USER_ROOTLOADER_ROOTLOADER
#undef  CONFIG_USER_SETSERIAL_SETSERIAL
#undef  CONFIG_USER_TRIPWIRE_SIGGEN
#undef  CONFIG_USER_STRACE_STRACE
#undef  CONFIG_USER_STTY_STTY
#undef  CONFIG_USER_TCSH_TCSH
#undef  CONFIG_USER_THREADDEMOS_THREADDEMOS
#undef  CONFIG_USER_TIP_TIP
#undef  CONFIG_USER_TRIPWIRE_TRIPWIRE
#undef  CONFIG_USER_USBMOUNT_USBMOUNT

/*
 * LIRC
 */
#undef  CONFIG_USER_LIRC
#undef  CONFIG_USER_LIRC_LIRCD
#undef  CONFIG_USER_LIRC_IRRECORD
#undef  CONFIG_USER_LIRC_LIRCMD
#undef  CONFIG_USER_LIRC_IREXEC
#undef  CONFIG_USER_LIRC_IRW
#undef  CONFIG_USER_LIRC_MODE2

/*
 * Editors
 */
#undef  CONFIG_USER_LEVEE_VI
#undef  CONFIG_USER_ELVISTINY_VI

/*
 * Audio tools
 */
#undef  CONFIG_USER_MP3PLAY_MP3PLAY
#undef  CONFIG_USER_OGGPLAY_OGG123
#undef  CONFIG_USER_OGGPLAY_EXAMPLE
#undef  CONFIG_USER_MUSICBOX_MUSICBOX
#undef  CONFIG_USER_PLAY_PLAY
#undef  CONFIG_USER_PLAY_TONE
#undef  CONFIG_USER_VPLAY_VPLAY
#undef  CONFIG_USER_VPLAY_VREC
#undef  CONFIG_USER_VPLAY_MIXER
#undef  CONFIG_USER_PLAYRT_PLAYRT

/*
 * Video tools
 */
#undef  CONFIG_USER_W3CAM_VIDCAT
#undef  CONFIG_USER_W3CAM_W3CAMD

/*
 * Fileutils tools
 */
#undef  CONFIG_USER_FILEUTILS_CAT
#undef  CONFIG_USER_FILEUTILS_CHGRP
#undef  CONFIG_USER_FILEUTILS_CHMOD
#undef  CONFIG_USER_FILEUTILS_CHOWN
#undef  CONFIG_USER_FILEUTILS_CMP
#undef  CONFIG_USER_FILEUTILS_CP
#undef  CONFIG_USER_FILEUTILS_DD
#undef  CONFIG_USER_FILEUTILS_GREP
#undef  CONFIG_USER_FILEUTILS_L
#undef  CONFIG_USER_FILEUTILS_LN
#undef  CONFIG_USER_FILEUTILS_LS
#undef  CONFIG_USER_FILEUTILS_MKDIR
#undef  CONFIG_USER_FILEUTILS_MKFIFO
#undef  CONFIG_USER_FILEUTILS_MKNOD
#undef  CONFIG_USER_FILEUTILS_MORE
#undef  CONFIG_USER_FILEUTILS_MV
#undef  CONFIG_USER_FILEUTILS_RM
#undef  CONFIG_USER_FILEUTILS_RMDIR
#undef  CONFIG_USER_FILEUTILS_SYNC
#undef  CONFIG_USER_FILEUTILS_TOUCH

/*
 * Shutils tools
 */
#undef  CONFIG_USER_SHUTILS_BASENAME
#undef  CONFIG_USER_SHUTILS_DATE
#undef  CONFIG_USER_SHUTILS_DIRNAME
#undef  CONFIG_USER_SHUTILS_ECHO
#undef  CONFIG_USER_SHUTILS_FALSE
#undef  CONFIG_USER_SHUTILS_LOGNAME
#undef  CONFIG_USER_SHUTILS_PRINTENV
#undef  CONFIG_USER_SHUTILS_PWD
#undef  CONFIG_USER_SHUTILS_TRUE
#undef  CONFIG_USER_SHUTILS_UNAME
#undef  CONFIG_USER_SHUTILS_WHICH
#undef  CONFIG_USER_SHUTILS_WHOAMI
#undef  CONFIG_USER_SHUTILS_YES

/*
 * Sysutils tools
 */
#undef  CONFIG_USER_SYSUTILS_REBOOT
#undef  CONFIG_USER_SYSUTILS_SHUTDOWN
#undef  CONFIG_USER_SYSUTILS_DF
#undef  CONFIG_USER_SYSUTILS_FREE
#undef  CONFIG_USER_SYSUTILS_HOSTNAME
#undef  CONFIG_USER_SYSUTILS_KILL
#undef  CONFIG_USER_SYSUTILS_PS

/*
 * Procps tools
 */
#undef  CONFIG_USER_PROCPS_FREE
#undef  CONFIG_USER_PROCPS_KILL
#undef  CONFIG_USER_PROCPS_PGREP
#undef  CONFIG_USER_PROCPS_PKILL
#undef  CONFIG_USER_PROCPS_PS
#undef  CONFIG_USER_PROCPS_SNICE
#undef  CONFIG_USER_PROCPS_SYSCTL
#undef  CONFIG_USER_PROCPS_TLOAD
#undef  CONFIG_USER_PROCPS_TOP
#undef  CONFIG_USER_PROCPS_UPTIME
#undef  CONFIG_USER_PROCPS_VMSTAT
#undef  CONFIG_USER_PROCPS_W
#undef  CONFIG_USER_PROCPS_WATCH

/*
 * PCI utilities
 */
#undef  CONFIG_USER_PCIUTILS_LSPCI
#undef  CONFIG_USER_PCIUTILS_SETPCI

/*
 * BusyBox
 */
#define CONFIG_USER_BUSYBOX_BUSYBOX 1
#undef  CONFIG_USER_BUSYBOX_BUSYBOX100PRE3

/*
 * Applets
 */
#undef  CONFIG_USER_BUSYBOX_ADJTIMEX
#undef  CONFIG_USER_BUSYBOX_AR
#undef  CONFIG_USER_BUSYBOX_BASENAME
#define CONFIG_USER_BUSYBOX_CAT 1
#undef  CONFIG_USER_BUSYBOX_CHGRP
#undef  CONFIG_USER_BUSYBOX_CHMOD
#undef  CONFIG_USER_BUSYBOX_CHOWN
#undef  CONFIG_USER_BUSYBOX_CHROOT
#undef  CONFIG_USER_BUSYBOX_CLEAR
#undef  CONFIG_USER_BUSYBOX_CMP
#define CONFIG_USER_BUSYBOX_CP 1
#undef  CONFIG_USER_BUSYBOX_CUT
#undef  CONFIG_USER_BUSYBOX_DATE
#undef  CONFIG_USER_BUSYBOX_DC
#undef  CONFIG_USER_BUSYBOX_DD
#undef  CONFIG_USER_BUSYBOX_DF
#undef  CONFIG_USER_BUSYBOX_DIRNAME
#undef  CONFIG_USER_BUSYBOX_DMESG
#undef  CONFIG_USER_BUSYBOX_DUTMP
#undef  CONFIG_USER_BUSYBOX_DU
#define CONFIG_USER_BUSYBOX_ECHO 1
#undef  CONFIG_USER_BUSYBOX_ENV
#define CONFIG_USER_BUSYBOX_EXPR 1
#undef  CONFIG_USER_BUSYBOX_FIND
#undef  CONFIG_USER_BUSYBOX_FREE
#undef  CONFIG_USER_BUSYBOX_FREERAMDISK
#undef  CONFIG_USER_BUSYBOX_FSCK_MINIX
#undef  CONFIG_USER_BUSYBOX_GETOPT
#undef  CONFIG_USER_BUSYBOX_GREP
#undef  CONFIG_USER_BUSYBOX_GUNZIP
#undef  CONFIG_USER_BUSYBOX_GZIP
#undef  CONFIG_USER_BUSYBOX_HALT
#undef  CONFIG_USER_BUSYBOX_HEAD
#undef  CONFIG_USER_BUSYBOX_HOSTNAME
#undef  CONFIG_USER_BUSYBOX_ID
#define CONFIG_USER_BUSYBOX_IFCONFIG 1
#define CONFIG_USER_BUSYBOX_IFCONFIG_STATUS 1
#undef  CONFIG_USER_BUSYBOX_IFCONFIG_SLIP
#define CONFIG_USER_BUSYBOX_IFCONFIG_HW 1
#undef  CONFIG_USER_BUSYBOX_IFCONFIG_MEMSTART_IOADDR_IRQ
#undef  CONFIG_USER_BUSYBOX_INIT
#undef  CONFIG_USER_BUSYBOX_INSMOD
#define CONFIG_USER_BUSYBOX_KILL 1
#undef  CONFIG_USER_BUSYBOX_KILLALL
#undef  CONFIG_USER_BUSYBOX_KLOGD
#undef  CONFIG_USER_BUSYBOX_LENGTH
#undef  CONFIG_USER_BUSYBOX_LN
#undef  CONFIG_USER_BUSYBOX_LOGGER
#undef  CONFIG_USER_BUSYBOX_LOGNAME
#define CONFIG_USER_BUSYBOX_LS 1
#undef  CONFIG_USER_BUSYBOX_LS_USERNAME
#undef  CONFIG_USER_BUSYBOX_LS_TIMESTAMPS
#undef  CONFIG_USER_BUSYBOX_LS_FILETYPES
#undef  CONFIG_USER_BUSYBOX_LS_SORTFILES
#undef  CONFIG_USER_BUSYBOX_LS_RECURSIVE
#undef  CONFIG_USER_BUSYBOX_LS_FOLLOWLINKS
#undef  CONFIG_USER_BUSYBOX_LSMOD
#undef  CONFIG_USER_BUSYBOX_MAKEDEVS
#undef  CONFIG_USER_BUSYBOX_MD5SUM
#define CONFIG_USER_BUSYBOX_MKDIR 1
#undef  CONFIG_USER_BUSYBOX_MKFS_MINIX
#undef  CONFIG_USER_BUSYBOX_MKNOD
#undef  CONFIG_USER_BUSYBOX_MKTEMP
#undef  CONFIG_USER_BUSYBOX_MODPROBE
#undef  CONFIG_USER_BUSYBOX_MORE
#define CONFIG_USER_BUSYBOX_MOUNT 1
#undef  CONFIG_USER_BUSYBOX_MOUNT_LOOP
#undef  CONFIG_USER_BUSYBOX_MTAB_SUPPORT
#undef  CONFIG_USER_BUSYBOX_NFSMOUNT
#undef  CONFIG_USER_BUSYBOX_MV
#undef  CONFIG_USER_BUSYBOX_NC
#undef  CONFIG_USER_BUSYBOX_NSLOOKUP
#undef  CONFIG_USER_BUSYBOX_PIDOF
#undef  CONFIG_USER_BUSYBOX_PING
#undef  CONFIG_USER_BUSYBOX_PIVOT_ROOT
#undef  CONFIG_USER_BUSYBOX_POWEROFF
#undef  CONFIG_USER_BUSYBOX_PRINTF
#define CONFIG_USER_BUSYBOX_PS 1
#undef  CONFIG_USER_BUSYBOX_PWD
#undef  CONFIG_USER_BUSYBOX_RDATE
#undef  CONFIG_USER_BUSYBOX_READLINK
#define CONFIG_USER_BUSYBOX_REBOOT 1
#undef  CONFIG_USER_BUSYBOX_RENICE
#undef  CONFIG_USER_BUSYBOX_RESET
#define CONFIG_USER_BUSYBOX_RM 1
#undef  CONFIG_USER_BUSYBOX_RMDIR
#undef  CONFIG_USER_BUSYBOX_RMMOD
#define CONFIG_USER_BUSYBOX_ROUTE 1
#undef  CONFIG_USER_BUSYBOX_RPM2CPIO
#undef  CONFIG_USER_BUSYBOX_SED
#undef  CONFIG_USER_BUSYBOX_SHELL
#undef  CONFIG_USER_BUSYBOX_SLEEP
#undef  CONFIG_USER_BUSYBOX_SORT
#undef  CONFIG_USER_BUSYBOX_STTY
#undef  CONFIG_USER_BUSYBOX_SYNC
#undef  CONFIG_USER_BUSYBOX_SYSLOGD
#undef  CONFIG_USER_BUSYBOX_TAIL
#undef  CONFIG_USER_BUSYBOX_TAR
#undef  CONFIG_USER_BUSYBOX_TEE
#undef  CONFIG_USER_BUSYBOX_TELNET
#undef  CONFIG_USER_BUSYBOX_TEST
#undef  CONFIG_USER_BUSYBOX_TFTP
#undef  CONFIG_USER_BUSYBOX_TOP
#undef  CONFIG_USER_BUSYBOX_TOUCH
#undef  CONFIG_USER_BUSYBOX_TR
#undef  CONFIG_USER_BUSYBOX_TRACEROUTE
#undef  CONFIG_USER_BUSYBOX_TRUE_FALSE
#undef  CONFIG_USER_BUSYBOX_TTY
#undef  CONFIG_USER_BUSYBOX_UMOUNT
#undef  CONFIG_USER_BUSYBOX_UNAME
#undef  CONFIG_USER_BUSYBOX_UNIQ
#undef  CONFIG_USER_BUSYBOX_UNIX2DOS
#undef  CONFIG_USER_BUSYBOX_UPTIME
#undef  CONFIG_USER_BUSYBOX_USLEEP
#undef  CONFIG_USER_BUSYBOX_UUDECODE
#undef  CONFIG_USER_BUSYBOX_UUENCODE
#undef  CONFIG_USER_BUSYBOX_VI
#undef  CONFIG_USER_BUSYBOX_WATCHDOG
#undef  CONFIG_USER_BUSYBOX_WC
#undef  CONFIG_USER_BUSYBOX_WGET
#undef  CONFIG_USER_BUSYBOX_WHICH
#undef  CONFIG_USER_BUSYBOX_WHOAMI
#undef  CONFIG_USER_BUSYBOX_XARGS
#undef  CONFIG_USER_BUSYBOX_YES

/*
 * other features
 */
#undef  CONFIG_USER_BUSYBOX_VERBOSE_USAGE
#undef  CONFIG_USER_BUSYBOX_AUTOWIDTH
#undef  CONFIG_USER_BUSYBOX_NEW_MODULE_INTERFACE
#undef  CONFIG_USER_BUSYBOX_OLD_MODULE_INTERFACE
#undef  CONFIG_USER_BUSYBOX_INSMOD_VERSION_CHECKING
#undef  CONFIG_USER_BUSYBOX_HUMAN_READABLE

/*
 * Tinylogin
 */
#undef  CONFIG_USER_TINYLOGIN_TINYLOGIN

/*
 * MicroWindows
 */
#undef  CONFIG_USER_MICROWIN

/*
 * RTK VoIP User applications
 */
#undef  CONFIG_USER_RTK_VOIP

/*
 * Games
 */
#undef  CONFIG_USER_GAMES_ADVENT4
#undef  CONFIG_USER_GAMES_DUNGEON
#undef  CONFIG_USER_GAMES_XMAME

/*
 * Miscellaneous Configuration
 */
#define CONFIG_USER_RAMIMAGE_NONE 1
#undef  CONFIG_USER_RAMIMAGE_RAMFS64
#undef  CONFIG_USER_RAMIMAGE_RAMFS128
#undef  CONFIG_USER_RAMIMAGE_RAMFS256
#undef  CONFIG_USER_RAMIMAGE_RAMFS512
#undef  CONFIG_USER_CGI_GENERIC
#undef  CONFIG_USER_DEMO_BUTTON
#undef  CONFIG_USER_DEMO_LATCH
#undef  CONFIG_USER_DEMO_MORSE
#undef  CONFIG_USER_DEMO_R2100

/*
 * Debug Builds
 */
#define CONFIG_LIB_DEBUG 1
#define CONFIG_USER_DEBUG 1

/*
 * Debug tools
 */
#undef  CONFIG_USER_TIMEPEG_TPT

/*
 * Debug libraries
 */
#undef  CONFIG_LIB_LIBCCMALLOC
#define CONFIG_VENDOR "Realtek"
#define CONFIG_PRODUCT "RTL8670"
#define CONFIG_LINUXDIR "linux-2.6.x"
#define CONFIG_LIBCDIR "uClibc"
#define CONFIG_LANGUAGE ""
#define VENDORS_AUTOCONF_INCLUDED
#undef AUTOCONF_INCLUDED
