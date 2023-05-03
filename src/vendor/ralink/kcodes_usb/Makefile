include $(ROOTDIR)/.config
export MODULEPATH:=lib/modules
export PATH_ROOTFS_MOD:=$(FSROOT)/$(MODULEPATH)
SRC_DIR = $(shell pwd | sed "s/build.$(PRODUCT)/src/")

ifeq ($(strip $(CONFIG_RT63365_WLM3600)), y)
KCODES_SMB_TARGET +=KC_SMB_WLM-3600.dat
KCODES_NETUSB_TARGET +=NetUSB_WLM-3600.ko
endif
ifeq ($(strip $(CONFIG_RT63368_WLM4600)), y)
KCODES_SMB_TARGET +=KC_SMB_WLM-4600.dat
KCODES_NETUSB_TARGET +=NetUSB_WLM-4600.ko
endif
ifeq ($(strip $(CONFIG_RT63368_WLM5600)), y)
KCODES_SMB_TARGET +=KC_SMB_WLM-5600.dat
KCODES_NETUSB_TARGET +=NetUSB_WLM-5600.ko
endif
ifeq ($(strip $(CONFIG_RT63368_WLM6600)), y)
KCODES_SMB_TARGET +=KC_SMB_WLM-6600.dat
KCODES_NETUSB_TARGET +=NetUSB_WLM-6600.ko
endif
ifeq ($(strip $(CONFIG_RT63368_WLM5700)), y)
KCODES_SMB_TARGET +=KC_SMB_WLM-5700.dat
KCODES_NETUSB_TARGET +=NetUSB_WLM-5700.ko
endif
ifeq ($(strip $(CONFIG_DSL_AC750)), y)
KCODES_NETUSB_TARGET += NetUSB_DSL-AC750.ko
endif


all:
	

create_dir:
	if [ ! -d $(PATH_ROOTFS_MOD)/driver ]; then mkdir -p $(PATH_ROOTFS_MOD)/driver; fi

install:create_dir
	cp -f -L $(SRC_DIR)/GPL_NetUSB.ko $(PATH_ROOTFS_MOD)/driver/GPL_NetUSB.ko
	cp -f -L $(SRC_DIR)/$(KCODES_NETUSB_TARGET) $(PATH_ROOTFS_MOD)/driver/NetUSB.ko
	cp -f -L $(SRC_DIR)/$(KCODES_SMB_TARGET) $(FSROOT)/usr/bin/kc_smb

clean:
	rm -f GPL_NetUSB.ko $(KCODES_NETUSB_TARGET) $(KCODES_SMB_TARGET)

uninstall:
	rm -f $(PATH_ROOTFS_MOD)/driver/GPL_NetUSB.ko $(PATH_ROOTFS_MOD)/driver/NetUSB.ko $(FSROOT)/usr/bin/kc_smb
	
