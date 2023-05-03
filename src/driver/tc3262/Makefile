include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
export EXTRA_AFLAGS
export EXTRA_CFLAGS +=${EXTRA_AFLAGS}

KERNELDIR := $(KERNEL_DIR)

LINUX_SRC_MODULE=${FSROOT}/lib/modules/driver/
MOD_NAME = raeth

ifeq ($(CONFIG_RT65168), y)
MOD_NAME += tc3262ptm
MOD_NAME += pwm
MOD_NAME += tc3262gmac1
endif

EXTRA_CFLAGS +=-DTCPHY_SUPPORT -DMII_INTERFACE -DWAN2LAN
EXTRA_CFLAGS +=-DLINUX_OS -I../../vendor/ralink/include/ -I${KERNEL_DIR}/include
#CFLAGS+=-DTCPHY_DEBUG 

ifeq ($(CONFIG_RT63365), y)
EXTRA_CFLAGS += -DRT63365_SUPPORT
raeth-objs := femac.o
endif

ifeq ($(CONFIG_RT65168), y)
EXTRA_CFLAGS += -DVPORT -DTC2206_SUPPORT
raeth-objs := gmac_tc3262.o
endif

raeth-objs += tcphy/tcetherphy.o tcphy/tcswitch.o tcphy/tcconsole.o 

obj-m = raeth.o
ifeq ($(CONFIG_RT65168), y)
obj-m += tc3262ptm.o
obj-m += pwm.o
endif

CFLAGS_gmac1_tc3262.o := -DTC3262_GMAC1

tc3262gmac1-objs := gmac1_tc3262.o
tc3262gmac1-objs += tcphy/tcetherphy1.o tcphy/tcswitch.o
ifneq ($(TCSUPPORT_AUTOBENCH),)
tc3262gmac1-objs += ../auto_bench/mac1_autobench.o
endif

obj-m += tc3262gmac1.o

all:
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules
	mips-linux-uclibc-strip --strip-unneeded raeth.ko
	mips-linux-uclibc-strip --strip-unneeded tc3262gmac1.ko


install:
	install -dv $(LINUX_SRC_MODULE)
	install -m 755 -c $(addsuffix .ko,$(MOD_NAME)) $(LINUX_SRC_MODULE)

clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean

