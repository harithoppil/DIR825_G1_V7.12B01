
ifeq "$(ROOTDIR)" ""
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif

include $(ROOTDIR)/.config
EXTRA_CFLAGS += -I$(ROOTDIR)/product/MARVELL_88F65XX -I./ -I$(KERNEL_DIR)/include -I$(KERNEL_DIR)/include/linux -I$(ROOTDIR)/src/include 

ifeq ($(strip $(CONFIG_BUTTON_TRIGGER_TIMER)),y)
EXTRA_CFLAGS += -DBUTTON_TRIGGER_TIMER
endif

# Link product.o to kernel
obj-m:= product.o

PWD:=$(shell pwd)


export MODULEPATH:=lib/modules
#export MODULEPATH:=lib/modules/${KERNELRELEASE}
#export PATH_ROOTFS_LIB:=$(FSROOT)/$(LIBDIR)
export PATH_ROOTFS_MOD:=$(FSROOT)/$(MODULEPATH)



all:
	@echo $(PWD)/../../../../product/$(PRODUCT)/product.c
	ln -sf $(PWD)/../../../../product/$(PRODUCT)/product.c product.c
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
	
create_dir:
	${Q}test -d $(PATH_ROOTFS_MOD)/driver || \
            mkdir -p $(PATH_ROOTFS_MOD)/driver

install:create_dir
	cp product.ko $(PATH_ROOTFS_MOD)/driver/
	
clean:
	-rm product.ko
