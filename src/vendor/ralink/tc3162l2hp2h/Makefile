include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
export EXTRA_CFLAGS +=${EXTRA_AFLAGS}
KERNELDIR=$(KERNEL_DIR)
ifndef CONFIG_MIPS_TC3262
ifndef CONFIG_MIPS_MT7510
obj-m += tc3162l2mac.o
endif
endif

ifdef CONFIG_MIPS_MT7510
	EXTRA_CFLAGS += -DTCSUPPORT_CPU_MT7510
endif

LINUX_SRC_MODULE=${FSROOT}/lib/modules/driver/
MOD_NAME= module_sel tc3162l2sar
#MOD_NAME += tcledctrl
obj-m += tc3162l2sar.o
#obj-m += tcledctrl.o
obj-m += module_sel.o

ifneq ($(TARGET),THREADX)
#RT28xx_DIR = home directory of RT28xx source code
RT28xx_DIR = $(shell pwd)
endif

ifeq ($(strip $(CONFIG_TCSUPPORT_WLAN_WPS)), y)
export TCSUPPORT_WLAN_WPS
export WSC_AP_SUPPORT=1
EXTRA_CFLAGS+=-DWSC_AP_SUPPORT
ifeq ($(strip $(CONFIG_TCSUPPORT_WLAN_WPS_LEDSPEC)), y)
export TCSUPPORT_WLAN_WPS_LEDSPEC
export LED_WPSSPEC_COMPLY=1
EXTRA_CFLAGS+=-DLED_WPSSPEC_COMPLY
endif
ifeq ($(strip $(CONFIG_TCSUPPORT_AUTHENTICATION_TYPE)), y)
export TCSUPPORT_AUTHENTICATION_TYPE=1
EXTRA_CFLAGS+=-DTCSUPPORT_AUTHENTICATION_TYPE
endif
endif

ifeq ($(strip $(CONFIG_TCSUPPORT_WLAN_IGMPSNOOP)), y) 
export TCSUPPORT_WLAN_IGMPSNOOP
export IGMP_SNOOP_SUPPORT=1
EXTRA_CFLAGS+=-DIGMP_SNOOP_SUPPORT
endif
ifeq ($(strip $(CONFIG_TCSUPPORT_WLAN_ATE)), y)
export TCSUPPORT_WLAN_ATE
endif

ifeq ($(strip $(CONFIG_TCSUPPORT_WLAN_QATOOL)), y)
export TCSUPPORT_WLAN_QATOOL
endif

ifeq ($(strip $(CONFIG_TCSUPPORT_WLAN_RT5392)), y)
EXTRA_CFLAGS+=-DRT5392 -DBBUTOWBU
export TCSUPPORT_WLAN_RT5392
endif
ifneq ($(strip $(TCSUPPORT_WLAN_GPIO)),)
export TCSUPPORT_WLAN_GPIO
EXTRA_CFLAGS+=-DTCSUPPORT_WLAN_GPIO
endif

tc3162l2sar-objs := sar_tc3162l2.o
ifeq ($(PURE_BRIDGE),1)
tc3162l2mac-objs := dummy_mac.o
obj-m += tc3162_udc.o
else
tc3162l2mac-objs := mac_tc3162l2.o
endif

EXTRA_CFLAGS+=-DDMA_API
ifeq ($(TC3262),1)
STRIP=mips-linux-uclibc-strip
else
STRIP=mips-linux-strip
endif
ifeq ($(TC3262_FPGA),1)
EXTRA_CFLAGS+=-DTC3262_FPGA
endif
ifeq ($(SAR_VERIFY),1)
EXTRA_CFLAGS+= -DSAR_VERIFY
tc3162l2sar-objs += sar_verify.o
endif

EXTRA_CFLAGS+= -DL2_AUTOPVC
tc3162l2sar-objs += autoPVC.o

ifeq ($(PURE_BRIDGE),1)
EXTRA_CFLAGS += -DPURE_BRIDGE
endif

ifeq ($(WSC_AP_SUPPORT), 1)
EXTRA_CFLAGS += -DWSC_AP_SUPPORT
endif

ifeq ($(LED_WPSSPEC_COMPLY),1)
EXTRA_CFLAGS += -DLED_WPSSPEC_COMPLY
endif

EXTRA_CFLAGS+=-DMII_INTERFACE
EXTRA_CFLAGS+=-DLINUX_OS

ifeq ($(TC2031_SUPPORT),1)
#The orginal etherd root ci-cmd is changed to tce ci-cmd.
#If you want to IOT test.We must to add -DTCPHY_DEBUG compiler option 
EXTRA_CFLAGS+= -DTCPHY_SUPPORT -DTCPHY_1PORT -DWAN2LAN
tc3162l2mac-objs += ../tcphy/tcetherphy.o tcethercmd.o  ../tcphy/tcconsole.o
else
#Default the mac driver is supported TC2206 Switch IC.
EXTRA_CFLAGS+= -DTCPHY_SUPPORT -DTCPHY_4PORT -DWAN2LAN -DVPORT
tc3162l2mac-objs += ../tcphy/tcetherphy.o tcethercmd.o ../tcphy/tcswitch.o ../tcphy/tcconsole.o
endif

ifneq ($(strip $(CONFIG_TCSUPPORT_LED_BTN_CHECK)),)
EXTRA_CFLAGS+= -DCONFIG_TCSUPPORT_LED_BTN_CHECK
endif
ifneq ($(TCSUPPORT_AUTOBENCH),)
EXTRA_CFLAGS+= -DTCPHY_DEBUG
tc3162l2mac-objs  += ../auto_bench/mac_autobench.o
endif

ifeq ($(SAR_POLLING_MODE),1)
EXTRA_CFLAGS+= -DSAR_POLLING
endif

ifeq ($(EEE_SUPPORT),1)
EXTRA_CFLAGS+= -DEEE_SUPPORT -DTCPHY_SUPPORT -DTCPHY_1PORT
tc3162l2mac-objs += ../tcphy/tcetherphy.o tcethercmd.o psm_verify.o 
endif
ifeq ($(CMD_API),1)
EXTRA_CFLAGS+= -DCMD_API
endif
EXTRA_CFLAGS+= -DCWMP

ifeq ($(TR068_LED),1)
EXTRA_CFLAGS+= -DTR068_LED
endif

ifeq ($(WITHVOIP),1)
EXTRA_CFLAGS += -DWITHVOIP
endif

all:
#ifeq ($(SAR_VERIFY),1)
#	cp tc3162l2sar.c sar.c
#endif
#ifeq ($(TC2031_SUPPORT),1)
#	cp tc3162l2mac.c mac.c
#endif
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules
	$(STRIP) --strip-unneeded *.ko
#	$(STRIP) --strip-unneeded tc3162l2sar.ko
#	$(STRIP) --strip-unneeded tccicmd.ko	
#	$(STRIP) --strip-unneeded tcledctrl.ko
#ifneq ($(TC3262),1)
#	$(STRIP) --strip-unneeded tc3162l2mac.ko
#	$(STRIP) --strip-unneeded tc3162_udc.ko
#	$(STRIP) --strip-unneeded tc3162_udc20.ko
#endif

install:
ifdef CONFIG_MIPS_MT7510
	@cp -fv -L module_sel.ko $(LINUX_SRC_MODULE)
else
	install -dv $(LINUX_SRC_MODULE)
	install -m 644 -c $(addsuffix .ko,$(MOD_NAME)) $(LINUX_SRC_MODULE)
endif

clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean

