include $(ROOTDIR)/.config
export MODULEPATH:=lib/modules
#export MODULEPATH:=lib/modules/${KERNELRELEASE}
#export PATH_ROOTFS_LIB:=$(FSROOT)/$(LIBDIR)
export PATH_ROOTFS_MOD:=$(FSROOT)/$(MODULEPATH)

SRC_DIR = $(shell pwd | sed "s/build.$(PRODUCT)/src/")

ifeq ($(CONFIG_RALINK_MT751X_I2C), y)
MODULE_NAME=mt7510/sif.ko
endif

all:
	

create_dir:
	if [ ! -d $(PATH_ROOTFS_MOD)/driver ]; then mkdir -p $(PATH_ROOTFS_MOD)/driver; fi

install:create_dir
	@cp -fv -L $(SRC_DIR)/$(MODULE_NAME) $(PATH_ROOTFS_MOD)/driver/

clean:

uninstall:
	rm -f $(PATH_ROOTFS_MOD)/driver/sif.ko
