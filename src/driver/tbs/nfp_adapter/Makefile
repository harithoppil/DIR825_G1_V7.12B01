

include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
include $(ROOTDIR)/BaseVar.mk


export EXTRA_CFLAGS +=${EXTRA_AFLAGS}

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


NFP_OBJS := nfp_adapter.o nfp_conntrack.o nfp_route.o nfp_neighbour.o  nfp_interface.o nfp_bridge.o
obj-m:= tbs_nfp_adapter.o
MODULE_NAME = tbs_nfp_adapter.ko

tbs_nfp_adapter-objs:= ${NFP_OBJS}

#CFLAGS += -Werror  -I $(KERNEL_DIR) -I${BUILDDIR}

EXTRA_CFLAGS +=-Werror  -I $(KERNEL_DIR)  -I${BUILDDIR}
#all: ${TARGART}
all:$
	$(MAKE) -C $(KERNEL_DIR) M=$(TOP_DIR)  modules

create_dir:
	echo $(PATH_ROOTFS_MOD)
	${Q}test -d $(PATH_ROOTFS_MOD)/kernel/net/nfp_adapter || \
	    mkdir -p $(PATH_ROOTFS_MOD)/kernel/net/nfp_adapter

install:create_dir
	${Q}cp -f -L $(MODULE_NAME)	\
	    $(PATH_ROOTFS_MOD)/kernel/net/nfp_adapter

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(TOP_DIR) clean
	rm -rf *.o

distclean: clean
