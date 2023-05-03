include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
export EXTRA_CFLAGS +=${EXTRA_AFLAGS}
obj-m += tccicmd.o
tccicmd-objs := tccimain.o tcadslcmd.o tcsyscmd.o

EXTRA_CFLAGS+=-DDMA_API
#ifeq ($(TC3262),1)
#STRIP=mips-linux-uclibc-strip
#else
#STRIP=mips-linux-strip
#endif

EXTRA_CFLAGS+= -DL2_AUTOPVC
tc3162l2sar-objs += autoPVC.o




EXTRA_CFLAGS+=-DMII_INTERFACE



#Default the mac driver is supported TC2206 Switch IC.
#ifeq ($(TC2206_SUPPORT),1)
EXTRA_CFLAGS+= -DTCPHY_SUPPORT -DTCPHY_4PORT -DWAN2LAN
tc3162l2mac-objs += ../tcetherphy.o tcethercmd.o
#endif

CMD_API=1

ifeq ($(CMD_API),1)
EXTRA_CFLAGS+= -DCMD_API
endif
EXTRA_CFLAGS+= -DCWMP

ifeq ($(CONFIG_MIPS_MT7510), y)
EXTRA_CFLAGS+= -DTCSUPPORT_CPU_MT7510
endif


ifneq ($(TC_CFLAGS),)
EXTRA_CFLAGS+=$(TC_CFLAGS)
endif


include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk

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

#export LIBDIR:=lib
#export BINDIR:=usr/sbin
export MODULEPATH:=lib/modules
#export MODULEPATH:=lib/modules/${KERNELRELEASE}
#export PATH_ROOTFS_LIB:=$(FSROOT)/$(LIBDIR)
export PATH_ROOTFS_MOD:=$(FSROOT)/$(MODULEPATH)
#export PATH_ROOTFS_RC:=$(FSROOT)/etc/init.d
#export PATH_ROOTFS_BIN:=$(FSROOT)/$(BINDIR)



.PHONY: all build clean distclean
all:build
TOP_DIR = $(shell pwd)





#CFLAGS += -Werror  -I $(KERNEL_DIR) -I${BUILDDIR}

#EXTRA_CFLAGS += -Werror -I $(VENDOR_DIR)  -I $(KERNEL_DIR)  -I${BUILDDIR}
EXTRA_CFLAGS += -I $(VENDOR_DIR)  -I $(KERNEL_DIR)  -I${BUILDDIR}
#all: ${TARGART}
all:$
	$(MAKE) -C $(KERNEL_DIR) M=$(TOP_DIR)  modules

create_dir:
	echo $(PATH_ROOTFS_MOD)
	${Q}test -d $(PATH_ROOTFS_MOD)/driver || \
	    mkdir -p $(PATH_ROOTFS_MOD)/driver

install:create_dir
	${Q}cp -f -L tccicmd.ko	$(PATH_ROOTFS_MOD)/driver/

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(TOP_DIR) clean
	rm -rf *.o

distclean: clean
