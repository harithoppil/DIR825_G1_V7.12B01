
include $(ROOTDIR)/build/arch.mk
include $(ROOTDIR)/.config

ifeq (${ARCH}, um)
CROSS_COMPILE=
else
CROSS_COMPILE:=$(TOOLCHAINS_PATH)$(if $(notdir $(TOOLCHAINS_PATH)),/)$(CROSS_COMPILE)
endif

#default strip kernel module
export INSTALL_MOD_STRIP=1

OSDIR=${BUILDDIR}/kernel/$(TBS_KERNEL_VERSION)
KERNEL_VERSION = $(shell cat $(OSDIR)/include/config/kernel.release)

all:
	rm -f ${OSDIR}/.config
	ln -sf $(BUILDDIR)/.config ${OSDIR}/.config
	make -C ${OSDIR} silentoldconfig ARCH=${ARCH} V=${V} CROSS_COMPILE=$(CROSS_COMPILE) KBUILD_OUTPUT=
ifeq ($(CPU),C1000)
	make -C ${OSDIR} ARCH=${ARCH} V=${V} CROSS_COMPILE=$(CROSS_COMPILE) KBUILD_OUTPUT= modules KBUILD_MODPOST_WARN=1
	make -C ${OSDIR} ARCH=${ARCH} V=${V} CROSS_COMPILE=$(CROSS_COMPILE) KBUILD_OUTPUT= uImage
else
	make -C ${OSDIR} ARCH=${ARCH} V=${V} CROSS_COMPILE=$(CROSS_COMPILE) KBUILD_OUTPUT=
endif

install:
	make -C ${OSDIR} modules_install ARCH=${ARCH} INSTALL_MOD_PATH=$(ROOTDIR)/build/romfs/rootfs \
		V=${V}	CROSS_COMPILE=$(CROSS_COMPILE) KBUILD_OUTPUT= DEPMOD=/bin/true
ifeq ($(CONFIG_MODULES), y)
	$(ROOTDIR)/tools/depmod.pl -b $(BUILDDIR)/romfs/rootfs/lib/modules/$(KERNEL_VERSION)/ -F $(OSDIR)/System.map 
endif
	cp ${BUILDDIR}/kernel/${TBS_KERNEL_VERSION}/vmlinux ${ROMFS}/vmlinux

clean:
	make -C ${OSDIR} clean ARCH=${ARCH} V=${V}  CROSS_COMPILE=$(CROSS_COMPILE) KBUILD_OUTPUT=
		
distclean:
	make -C ${OSDIR} distclean ARCH=${ARCH} V=${V}  CROSS_COMPILE=$(CROSS_COMPILE) KBUILD_OUTPUT=
	
