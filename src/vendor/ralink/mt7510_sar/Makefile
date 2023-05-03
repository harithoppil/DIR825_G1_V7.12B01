KERNELDIR=$(KERNEL_DIR)
include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
export EXTRA_CFLAGS +=${EXTRA_AFLAGS}

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

#CFLAGS += -Werror  -I $(KERNEL_DIR) -I${BUILDDIR}

EXTRA_CFLAGS += -I $(VENDOR_DIR)  -I $(KERNEL_DIR)  -I${BUILDDIR}
obj-m += mt7510sar.o

mt7510sar-objs := mt7510_sar.o

EXTRA_CFLAGS += -DWAN2LAN
EXTRA_CFLAGS += -DCONFIG_TX_POLLING_BY_MAC
EXTRA_CFLAGS += -DCMD_API -DCONFIG_QOS -DCWMP

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
