

include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
include $(ROOTDIR)/BaseVar.mk


export EXTRA_CFLAGS +=${EXTRA_AFLAGS}

BUILDDIR :=$(ROOTDIR)/build
VENDOR_DIR := ${BUILDDIR}/vendor

##################################################
# For install
##################################################
ifndef KERNELRELEASE
VERSION := $(strip $(shell head -n 1 ${KERNEL_DIR}/Makefile | cut -f 2 -d'='))
PATCHLEVEL := $(strip $(shell head -n 2 ${KERNEL_DIR}/Makefile | tail -1 | cut -f 2 -d'='))
SUBVERSION := $(strip $(shell head -n 3 ${KERNEL_DIR}/Makefile | tail -1 | cut -f 2 -d'='))
EXTRAVERSION ?= $(strip $(shell head -n 4 ${KERNEL_DIR}/Makefile | tail -1 | cut -f 2 -d'='))
KERNELRELEASE := $(strip ${VERSION}.${PATCHLEVEL}.${SUBVERSION}${EXTRAVERSION})
endif

export LIBDIR:=lib
export BINDIR:=usr/sbin
export MODULEPATH:=lib/modules/${KERNELRELEASE}
export PATH_ROOTFS_LIB:=$(FSROOT)/$(LIBDIR)
export PATH_ROOTFS_MOD:=$(FSROOT)/$(MODULEPATH)
export PATH_ROOTFS_RC:=$(FSROOT)/etc/init.d
export PATH_ROOTFS_BIN:=$(FSROOT)/$(BINDIR)



.PHONY: all build clean distclean
all:build
TOP_DIR = $(shell pwd)

NFP_OBJS-y += tbs_nfp.o tbs_nfp_itf.o tbs_nfp_mng.o tbs_nfp_skb_parser.o \
		tbs_nfp_proc.o tbs_nfp_itf_proc.o 
NFP_OBJS-${CONFIG_TBS_NFP_CT} += tbs_nfp_ct.o tbs_nfp_ct_proc.o
NFP_OBJS-${CONFIG_TBS_NFP_FIB} += tbs_nfp_fib.o tbs_nfp_fib_proc.o tbs_nfp_arp.o tbs_nfp_arp_proc.o
NFP_OBJS-${CONFIG_TBS_NFP_BRIDGE} += tbs_nfp_bridge.o tbs_nfp_bridge_proc.o


#NFP_OBJS := tbs_nfp.o tbs_nfp_ct.o tbs_nfp_fib.o tbs_nfp_bridge.o  tbs_nfp_itf.o tbs_nfp_mng.o
obj-m:= tbs_nfp_module.o

MODULE_NAME = tbs_nfp_module.ko

#tbs_nfp_module-objs:= ${NFP_OBJS}
tbs_nfp_module-objs:= ${NFP_OBJS-y}

#CFLAGS += -Werror  -I $(KERNEL_DIR) -I${BUILDDIR}

EXTRA_CFLAGS += -Werror -I $(VENDOR_DIR)  -I $(KERNEL_DIR)  -I${BUILDDIR}
#all: ${TARGART}
all:$
	$(MAKE) -C $(KERNEL_DIR) M=$(TOP_DIR)  modules

create_dir:
	echo $(PATH_ROOTFS_MOD)
	${Q}test -d $(PATH_ROOTFS_MOD)/kernel/net/tbs_nfp || \
	    mkdir -p $(PATH_ROOTFS_MOD)/kernel/net/tbs_nfp

install:create_dir
	${Q}cp -f -L $(MODULE_NAME)	\
	    $(PATH_ROOTFS_MOD)/kernel/net/tbs_nfp/

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(TOP_DIR) clean
	rm -rf *.o

distclean: clean
