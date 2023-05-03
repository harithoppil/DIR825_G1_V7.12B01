KERNELDIR=$(KERNEL_DIR)
include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
export EXTRA_CFLAGS +=${EXTRA_AFLAGS}

EXTRA_CFLAGS += -O0 -save-temps
EXTRA_CFLAGS += -D CONFIG_TX_WB_DONE \
                -D CONFIG_RX_2B_OFFSET \
                -D CONFIG_TX_POLLING_BY_MAC \
             	-D CONFIG_DEBUG \
               	-D CONFIG_SUPPORT_SELF_TEST 

EXTRA_CFLAGS += -DCONFIG_NEW_QDMA_CSR_OFFSET

BUFMGR_SRC += qdma_bmgr.c qdma_api.c qdma_dev.c qdma_dvt.c
BUFMGR_OBJ := $(BUFMGR_SRC:%.c=%.o)

ifneq ($(TC_CFLAGS),)
EXTRA_CFLAGS+=$(TC_CFLAGS)
endif

include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk

BUILDDIR :=$(ROOTDIR)/build
VENDOR_DIR := ${BUILDDIR}/vendor

qdma-objs 	:= $(BUFMGR_OBJ)
obj-m		+= qdma.o


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

.PHONY: all build clean distclean
TOP_DIR = $(shell pwd)

#CFLAGS += -Werror  -I $(KERNEL_DIR) -I${BUILDDIR}

EXTRA_CFLAGS += -I $(VENDOR_DIR)  -I $(KERNEL_DIR)  -I${BUILDDIR}

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(TOP_DIR)  modules
	mips-linux-uclibc-strip --strip-unneeded qdma.ko

create_dir:
	echo $(PATH_ROOTFS_MOD)
	${Q}test -d $(PATH_ROOTFS_MOD)/driver || \
	    mkdir -p $(PATH_ROOTFS_MOD)/driver

install: create_dir
	${Q}cp -f -L *.ko $(PATH_ROOTFS_MOD)/driver/

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(TOP_DIR) clean
	rm -rf *.o

distclean: clean
