
ifeq "$(ROOTDIR)" ""
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif

include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config

ifeq ($(CONFIG_RT65168), y)
include $(ROOTDIR)/build/product/tcsupport.mk
export EXTRA_CFLAGS +=${EXTRA_AFLAGS}
endif
ifeq ($(CONFIG_RT63365), y)
include $(ROOTDIR)/build/product/tcsupport.mk
export EXTRA_CFLAGS +=${EXTRA_AFLAGS}
endif
ifeq ($(CONFIG_RT63368), y)
include $(ROOTDIR)/build/product/tcsupport.mk
export EXTRA_CFLAGS +=${EXTRA_AFLAGS}
endif

EXTRA_CFLAGS += -I$(ROOTDIR)/product/$(PRODUCT) -DPRODUCT=\"$(PRODUCT)\" -I./ -I$(KERNEL_DIR)/include -I$(KERNEL_DIR)/include/linux -I$(ROOTDIR)/src/include 

ifeq ($(strip $(CONFIG_BUTTON_TRIGGER_TIMER)),y)
EXTRA_CFLAGS += -DBUTTON_TRIGGER_TIMER
endif
# Button Platform Drivers

BTNOBJS = btn-core.o
ifeq ($(strip $(CONFIG_BUTTON_MV88F6560P)),y)
BTNOBJS += btn_mv88f6560p.o
endif
ifeq ($(strip $(CONFIG_BUTTON_BCM5358)),y)
BTNOBJS += btn_bcm5358.o
endif
ifeq ($(strip $(CONFIG_BUTTON_AR7130)),y)
BTNOBJS += btn_ar7130.o
endif
ifeq ($(strip $(CONFIG_BUTTON_AMAZON_SE)),y)
BTNOBJS += btn_amazon_se.o
endif
ifeq ($(strip $(CONFIG_BUTTON_RT3052)),y)
BTNOBJS += btn_rt3052.o
endif
ifeq ($(strip $(CONFIG_BUTTON_VX180)),y)
BTNOBJS += btn_vx180.o
endif
ifeq ($(strip $(CONFIG_BUTTON_RTL8672)),y)
BTNOBJS += btn_rtl8672.o
endif
ifeq ($(strip $(CONFIG_BUTTON_RTL8196C)),y)
BTNOBJS += btn_rtl8196c.o
endif
ifeq ($(strip $(CONFIG_BUTTON_C1000)),y)
BTNOBJS += btn_c1k.o
endif
ifeq ($(strip $(CONFIG_BUTTON_C1000_INT)),y)
BTNOBJS += btn_c1k_int.o
endif
ifeq ($(strip $(CONFIG_BUTTON_RT65168)),y)
BTNOBJS += btn_rt65168.o
endif
ifeq ($(strip $(CONFIG_BUTTON_RT63365)),y)
BTNOBJS += btn_rt63365.o
endif
ifeq ($(strip $(CONFIG_BUTTON_RT63368)),y)
BTNOBJS += btn_rt63368.o
endif
ifeq ($(strip $(CONFIG_BUTTON_MT7620)),y)
BTNOBJS += btn_mt7620.o
endif
ifeq ($(strip $(CONFIG_BUTTON_MT751X)),y)
BTNOBJS += btn_mt751x.o
endif
# Button Core
btn-objs:=$(BTNOBJS)
obj-m:=btn.o

export MODULEPATH:=lib/modules
export PATH_ROOTFS_MOD:=$(FSROOT)/$(MODULEPATH)

all:
	make -C $(KERNEL_DIR) M=$(shell pwd) modules

install:
	if [ ! -d $(PATH_ROOTFS_MOD)/driver/ ]; then mkdir -p $(PATH_ROOTFS_MOD)/driver/; fi
	cp -f btn.ko $(PATH_ROOTFS_MOD)/driver/
	
clean:
	rm -rf *.o *.ko .btn* *.mod.c .tmp_versions modules.order Module.symvers

distclean:clean
	
uninstall:
	rm -f $(PATH_ROOTFS_MOD)/driver/btn.ko


