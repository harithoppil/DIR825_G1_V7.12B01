#!/bin/bash
#CROSS_COMPILE="/home/geilpc/telechips-codes/gingerbread-20110425/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin/arm-eabi-"
CROSS_COMPILE="/opt/tbs_toolchains/marvell_uclibc/usr/bin/arm-linux-"
./configure \
RANLIB="$CROSS_COMPILE"ranlib \
--prefix=/data/data/samba \
--exec-prefix=/data/data/samba \
--enable-shared-libs=no \
--host=arm-linux \
--disable-cups \
--disable-iprint \
--disable-pie \
--disable-fam \
--with-ads=no \
--with-ldap=no \
--with-cifsmount=no \
--with-logfilebase=/data/data/samba/var/log \
--with-swatdir=/data/data/samba/usr/local/swat \
--with-rootsbindir=/data/data/samba/sbin \
--with-lockdir=/data/data/samba/var/lock \
--with-piddir=/data/data/samba/var/lock \
--with-privatedir=/data/data/samba/etc/samba \
--with-configdir=/data/data/samba/etc/samba \
--cache-file=armsel-linux.cache \
--with-libtalloc=no \
--with-libtdb=no \
--with-libnetapi=no \
--with-libsmbclient=no \
--with-libsmbsharemodes=no \
--with-libaddns=no \
--enable-static=Samba \
