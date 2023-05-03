KERNELDIR=$(KERNEL_DIR)
include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
export EXTRA_CFLAGS +=${EXTRA_AFLAGS}

FE_API_SRC += fe_api.c
FE_API_OBJ := $(FE_API_SRC:%.c=%.o)

ifneq ($(TC_CFLAGS),)
EXTRA_CFLAGS+=$(TC_CFLAGS)
endif

include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk

BUILDDIR :=$(ROOTDIR)/build
VENDOR_DIR := ${BUILDDIR}/vendor

# fe_api-objs := $(FE_API_OBJ)
obj-m += fe_api.o


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
	mips-linux-uclibc-strip --strip-unneeded *.ko

create_dir:
	echo $(PATH_ROOTFS_MOD)
	${Q}test -d $(PATH_ROOTFS_MOD)/driver || \
	    mkdir -p $(PATH_ROOTFS_MOD)/driver

install: create_dir
	@cp -fv -L *.ko $(PATH_ROOTFS_MOD)/driver/

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(TOP_DIR) clean
	rm -rf *.o

distclean: clean
