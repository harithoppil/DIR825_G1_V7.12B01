ifeq "$(ROOTDIR)" ""
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif

include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config

EXTRA_CFLAGS += -I$(ROOTDIR)/product/$(PRODUCT) -DPRODUCT=\"$(PRODUCT)\" -I./ -I$(KERNEL_DIR)/include -I$(KERNEL_DIR)/include/linux -I$(ROOTDIR)/src/include 
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



ifeq ($(strip $(CONFIG_TBSMOD_LED)),y)
LEDOBJ  += led_tbscore.o
endif

ifeq ($(strip $(CONFIG_LED_RT65168)),y)
LEDOBJ  += leds_rt65168.o
endif

ifeq ($(strip $(CONFIG_LED_RT63365)),y)
LEDOBJ  += leds_rt63365.o
endif

ifeq ($(strip $(CONFIG_LED_RT63368)),y)
LEDOBJ  += leds_rt63368.o
endif

ifeq ($(strip $(CONFIG_LED_RTL8672)),y)
LEDOBJ  += leds_rtl8672.o
endif

ifeq ($(strip $(CONFIG_LED_RTL8676)),y)
LEDOBJ  += leds_rtl8676.o
endif

ifeq ($(strip $(CONFIG_LED_MT7620)),y)
LEDOBJ  += leds_mt7620.o
endif

ifeq ($(strip $(CONFIG_LED_MT751X)),y)
LEDOBJ  += leds_mt751x.o
endif

KBUILD_EXTRA_SYMBOLS += $(PWD)/../product/Module.symvers
export KBUILD_EXTRA_SYMBOLS

# Led Core
led-objs:= $(LEDOBJ)
obj-m:=led.o

PWD:=$(shell pwd)

export EXTRA_CFLAGS +=${EXTRA_AFLAGS}
export MODULEPATH:=lib/modules
export PATH_ROOTFS_MOD:=$(FSROOT)/$(MODULEPATH)


all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
	
install:
	if [ ! -d $(PATH_ROOTFS_MOD)/driver ]; then mkdir -p $(PATH_ROOTFS_MOD)/driver/; fi
	cp -f led.ko $(PATH_ROOTFS_MOD)/driver/
	
clean:
	rm -rf *.o *.ko .led* *.mod.c .tmp_versions modules.order Module.symvers

distclean:clean

uninstall:
	rm -f $(PATH_ROOTFS_MOD)/driver/led.ko
	

