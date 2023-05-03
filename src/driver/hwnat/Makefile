include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
export EXTRA_CFLAGS +=${EXTRA_AFLAGS}

KERNELDIR=$(KERNEL_DIR)
LINUX_SRC_MODULE=${FSROOT}/lib/modules/driver/
#EXTRA_CFLAGS += -DHWNAT_SW_MODEL -I$(obj)/../hwnat_emulation

MOD_NAME = hwnat

EXTRA_CFLAGS+= -I../../vendor/ralink/include/ 

obj-m += hwnat.o

#Default support the  TC2206 switch  IC
#CFLAGS +=-DTCPHY_DEBUG -DTCPHY_SUPPORT -DTCPHY_4PORT -DMII_INTERFACE


hwnat-objs := hwnat_main.o hwnat_pktflow.o hwnat_reg.o hwnat_itf.o hwnat_gpr.o hwnat_pce.o hwnat_tue.o hwnat_fte.o hwnat_mde.o hwnat_mac.o hwnat_qos.o hwnat_nfe.o hwnat_rxport.o

all:
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules
	mips-linux-uclibc-strip --strip-unneeded hwnat.ko

clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean

install:
	install -m 644 -c $(addsuffix .ko,$(MOD_NAME)) $(LINUX_SRC_MODULE)	
