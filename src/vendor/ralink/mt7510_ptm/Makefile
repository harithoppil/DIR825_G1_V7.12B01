KERNELDIR=$(KERNEL_DIR)
include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk

export EXTRA_CFLAGS +=${EXTRA_AFLAGS}

EXTRA_CFLAGS += -DWAN2LAN
EXTRA_CFLAGS += -DCONFIG_TX_POLLING_BY_MAC -DCONFIG_NEW_QDMA_CSR_OFFSET

ifneq ($(TC_CFLAGS),)
EXTRA_CFLAGS+=$(TC_CFLAGS)
endif

include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk

BUILDDIR :=$(ROOTDIR)/build
VENDOR_DIR := ${BUILDDIR}/vendor

ifndef KERNELRELEASE
VERSION := $(strip $(shell head -n 1 ${KERNEL_DIR}/Makefile | cut -f 2 -d'='))
PATCHLEVEL := $(strip $(shell head -n 2 ${KERNEL_DIR}/Makefile | tail -1 | cut -f 2 -d'='))
SUBVERSION := $(strip $(shell head -n 3 ${KERNEL_DIR}/Makefile | tail -1 | cut -f 2 -d'='))
EXTRAVERSION ?= $(strip $(shell head -n 4 ${KERNEL_DIR}/Makefile | tail -1 | cut -f 2 -d'='))
KERNELRELEASE := $(strip ${VERSION}.${PATCHLEVEL}.${SUBVERSION}${EXTRAVERSION})
endif

#export LIBDIR:=lib
#export BINDIR:=usr/sbin
export MODULEPATH:=lib/modules
#export MODULEPATH:=lib/modules/${KERNELRELEASE}
#export PATH_ROOTFS_LIB:=$(FSROOT)/$(LIBDIR)
export PATH_ROOTFS_MOD:=$(FSROOT)/$(MODULEPATH)
#export PATH_ROOTFS_RC:=$(FSROOT)/etc/init.d
#export PATH_ROOTFS_BIN:=$(FSROOT)/$(BINDIR)

TOP_DIR = $(shell pwd)

#####################
# for loopback tests
#####################
# EXTRA_CFLAGS += -DLOOPBACK_SUPPORT
# EXTRA_CFLAGS += -DEXTERNAL_LOOPBACK

####################
#for line bonding
####################
ifneq ($(TCSUPPORT_BONDING),)
#EXTRA_CFLAGS += -DLINE_BONDING
endif

###################
#for FPGA STAGE only, comment it in ASIC !!!
##################
#EXTRA_CFLAGS += -DFPGA_STAGE

###################
# for debug
##################
EXTRA_CFLAGS += -DPTM_DEBUG

##################
# for UNH L2 Encap
##################
UNH_L2ENCAP := 0
ifeq ($(UNH_L2ENCAP),1)
EXTRA_CFLAGS += -DUNH_L2ENCAP
endif

#######################
# for cached/uncached message type
# Using cached Tx/Rx messages has to waste 20480 btyes, but gain 8~9% upstream throughput
# (Note: there are 1024+256=1280 Tx/Rx messages, so have to waste 1280*16=20480 bytes)
#######################
EXTRA_CFLAGS += -DMSG_CACHED


mt7510ptm-objs := 7510ptm.o

mt7510ptm-objs += 7510ptm_loopback.o

ifeq ($(UNH_L2ENCAP),1)
mt7510ptm-objs += 7510ptm_L2encap.o
endif

ifneq ($(TCSUPPORT_BONDING),)
#mt7510ptm-objs += 7510ptm_lineBonding.o
endif

EXTRA_CFLAGS += -DTCSUPPORT_CPU_MT7510
EXTRA_CFLAGS += -DTCSUPPORT_QOS
ifeq ($(CONFIG_RALINK_MT751X_RA_HWNAT), y)
EXTRA_CFLAGS += -DTCSUPPORT_RA_HWNAT
endif

obj-m = mt7510ptm.o

.PHONY: all build clean distclean

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(TOP_DIR)  modules
	mips-linux-uclibc-strip --strip-unneeded *.ko

create_dir:
	echo $(PATH_ROOTFS_MOD)
	@test -d $(PATH_ROOTFS_MOD)/driver || \
	    mkdir -p $(PATH_ROOTFS_MOD)/driver

install: create_dir
	@cp -fv -L *.ko $(PATH_ROOTFS_MOD)/driver/

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(TOP_DIR) clean
	rm -rf *.o

distclean: clean
