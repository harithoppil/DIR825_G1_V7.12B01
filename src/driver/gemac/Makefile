include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
export EXTRA_AFLAGS
export EXTRA_CFLAGS +=${EXTRA_AFLAGS}

KERNELDIR := $(KERNEL_DIR)

LINUX_SRC_MODULE=${FSROOT}/lib/modules/driver/
MOD_NAME = raeth

EXTRA_CFLAGS+=-DLINUX_OS -I../../vendor/ralink/include/ -I${KERNEL_DIR}/include

raeth-objs := gemac.o
raeth-objs += tcconsole.o
ifeq ($(CONFIG_RTL8365), y)
EXTRA_CFLAGS += -DCONFIG_RTL8365
raeth-objs += smi.o
raeth-objs += rtk_api.o
raeth-objs += rtl8367b_asicdrv.o
raeth-objs += rtl8367b_asicdrv_cputag.o
raeth-objs += rtl8367b_asicdrv_igmp.o
raeth-objs += rtl8367b_asicdrv_interrupt.o
raeth-objs += rtl8367b_asicdrv_led.o
raeth-objs += rtl8367b_asicdrv_mib.o
raeth-objs += rtl8367b_asicdrv_phy.o
raeth-objs += rtl8367b_asicdrv_port.o
raeth-objs += rtl8367b_asicdrv_qos.o
raeth-objs += rtl8367b_asicdrv_vlan.o
raeth-objs += rtl8367b_asicdrv_lut.o
raeth-objs += rtl8367b_asicdrv_eee.o
raeth-objs += rtl8367b_asicdrv_eav.o
raeth-objs += rtl8367b_asicdrv_acl.o
raeth-objs += rtl8367b_asicdrv_hsb.o
raeth-objs += rtl8367b_asicdrv_svlan.o
raeth-objs += rtl8367b_asicdrv_storm.o
raeth-objs += rtl8367b_asicdrv_mirror.o
raeth-objs += rtl8367b_asicdrv_fc.o
raeth-objs += rtl8367b_asicdrv_green.o
raeth-objs += rtl8367b_asicdrv_misc.o
raeth-objs += rtl8367b_asicdrv_trunking.o
raeth-objs += rtl8367b_asicdrv_dot1x.o
raeth-objs += rtl8367b_asicdrv_rma.o
raeth-objs += rtl8367b_asicdrv_portIsolation.o
raeth-objs += rtl8367b_asicdrv_scheduling.o
raeth-objs += rtl8367b_asicdrv_meter.o
raeth-objs += rtl8367b_asicdrv_inbwctrl.o
raeth-objs += rtl8367b_asicdrv_unknownMulticast.o
else
raeth-objs += tcswitch.o
endif

obj-m = raeth.o
all:
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules
	mips-linux-uclibc-strip --strip-unneeded raeth.ko

install:
	install -dv $(LINUX_SRC_MODULE)
	install -m 644 -c $(addsuffix .ko,$(MOD_NAME)) $(LINUX_SRC_MODULE)

clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean

