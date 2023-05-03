include $(ROOTDIR)/BaseVar.mk
include $(ROOTDIR)/.config

SRCDIR=$(CURDIR)/iproute2-2.6.18
ifdef CONFIG_CPU_BIG_ENDIAN
EXTRA_CFLAGS += -DCONFIG_CPU_BIG_ENDIAN
endif
export EXTRA_CFLAGS = -fPIC -Dlinux -D__linux__ -Dunix -D_LINUX_2_6_ -I. -I$(TOOLCHAINS_SYSROOT)/include -I$(KERNEL_DIR)/include

.PHONY: all build clean distclean

all: build

build:
	make -C $(SRCDIR)

clean:
	make -C $(SRCDIR) clean

distclean:	
	make -C $(SRCDIR) distclean
	
install:
	make -C $(SRCDIR) install
ifdef CONFIG_OPENSOURCE_TC_UTIL
	$(STRIP) $(FSROOT)/usr/sbin/tc
endif
ifdef CONFIG_OPENSOURCE_IP_UTIL
	$(STRIP) $(FSROOT)/usr/sbin/ip
endif
