include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
export EXTRA_CFLAGS +=${EXTRA_AFLAGS}
KERNELDIR := $(KERNEL_DIR)
LINUX_SRC_MODULE=${FSROOT}/lib/modules/driver/
MOD_NAME = hw_nat
EXTRA_CFLAGS+= -I../../vendor/ralink/include/ -I../../vendor/ralink/fe_api/
#obj-$(CONFIG_RA_HW_NAT) += hw_nat.o
obj-m = hw_nat.o
hw_nat-objs := ra_nat.o foe_fdb.o util.o hwnat_ioctl.o
EXTRA_CFLAGS += -DCONFIG_HNAT_V2 -DCONFIG_RA_HW_NAT_IPV6


#ifeq ($(CONFIG_RALINK_RT3052),y)
#hw_nat-objs += sys_rfrw.o
#endif

#ifneq ($(CONFIG_HNAT_V2),y)
#hw_nat-objs += acl_policy.o acl_ioctl.o
#hw_nat-objs += ac_policy.o ac_ioctl.o
#hw_nat-objs += mtr_policy.o mtr_ioctl.o
#endif

all:
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules
	mips-linux-uclibc-strip --strip-unneeded hw_nat.ko

clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean

install:
	install -m 644 -c $(addsuffix .ko,$(MOD_NAME)) $(LINUX_SRC_MODULE)	
