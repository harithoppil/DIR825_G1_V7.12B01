KERNELDIR := $(KERNEL_DIR)

EXTRA_CFLAGS +=-DTCPHY_SUPPORT -DRT63365_SUPPORT -DMII_INTERFACE -DWAN2LAN

EXTRA_CFLAGS += -DLINUX_OS 

ifneq ($(TCSUPPORT_AUTOBENCH),)
EXTRA_CFLAGS += -DTCPHY_DEBUG
endif
#EXTRA_CFLAGS+=-DLOOPBACK_SUPPORT

raeth-objs := femac.o
raeth-objs += ../tcphy/tcetherphy.o ../tcphy/tcswitch.o ../tcphy/tcconsole.o 
#ifneq ($(TCSUPPORT_MT7510_FE),)
raeth-objs += fe_api.o
#endif
#raeth-objs += fe_verify.o
ifneq ($(strip $(TCSUPPORT_MT7530_SWITCH_API)),)
raeth-objs += ../tcphy/mtkswitch_api_krl.o
endif
ifneq ($(TCSUPPORT_AUTOBENCH),)
raeth-objs += ../auto_bench/femac_autobench.o
endif
ifneq ($(TCSUPPORT_HEC_6906),)
raeth-objs += access_module_cl.o
endif
obj-m = raeth.o

ifneq ($(TCSUPPORT_POWERSAVE_ENABLE),)
obj-m += pwm.o
endif
all:
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules
	mips-linux-uclibc-strip --strip-unneeded *.ko

clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean

