include $(ROOTDIR)/.config
export MODULEPATH:=lib/modules
#export MODULEPATH:=lib/modules/${KERNELRELEASE}
#export PATH_ROOTFS_LIB:=$(FSROOT)/$(LIBDIR)
export PATH_ROOTFS_MOD:=$(FSROOT)/$(MODULEPATH)

SRC_DIR = $(shell pwd | sed "s/build.$(PRODUCT)/src/")

ifeq ($(CONFIG_RT65168), y)
MODULE_NAME=tc3162_dmt_rt65168.ko
endif

ifeq ($(CONFIG_RT63365), y)
MODULE_NAME=tc3162_dmt_rt63365.ko
endif

ifeq ($(CONFIG_RT63368), y)
MODULE_NAME=tc3162_dmt_rt63365.ko
endif

ifeq ($(CONFIG_MT751X),y)
ifeq ($(TCSUPPORT_MT7510_E1),y)
    MODULE_NAME=tc3162_dmt_mt7510_e1.ko
else
    MODULE_NAME=tc3162_dmt_mt7510.ko
endif
endif

all:


create_dir:
	if [ ! -d $(PATH_ROOTFS_MOD)/driver ]; then mkdir -p $(PATH_ROOTFS_MOD)/driver; fi

install:create_dir
	-cp -fv -L $(SRC_DIR)/$(MODULE_NAME) $(PATH_ROOTFS_MOD)/driver/tc3162_dmt.ko

clean:
	rm -f $(MODULE_NAME)

uninstall:
	rm -f $(PATH_ROOTFS_MOD)/driver/tc3162_dmt.ko
