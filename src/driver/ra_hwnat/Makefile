include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
export EXTRA_CFLAGS +=${EXTRA_AFLAGS}

KERNELDIR=$(KERNEL_DIR)
LINUX_SRC_MODULE=${FSROOT}/lib/modules/driver/
MOD_NAME = hw_nat
EXTRA_CFLAGS+= -I../../vendor/ralink/include/ 

#obj-$(CONFIG_RA_HW_NAT) += hw_nat.o
obj-m = hw_nat.o
hw_nat-objs := ra_nat.o foe_fdb.o util.o acl_policy.o ac_policy.o mtr_policy.o
hw_nat-objs += ac_ioctl.o hwnat_ioctl.o acl_ioctl.o mtr_ioctl.o

ifeq ($(CONFIG_RALINK_RT3052),y)
hw_nat-objs += ra_rfrw.o
endif

#uncomment this flag if you want to speed up WiFi traffic by HNAT
#EXTRA_CFLAGS += -DCONFIG_RA_HW_NAT_WIFI

all:
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules
	mips-linux-uclibc-strip --strip-unneeded hw_nat.ko

clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean

install:
	install -m 644 -c $(addsuffix .ko,$(MOD_NAME)) $(LINUX_SRC_MODULE)	
