AR=$(CROSS_COMPILE)ar
LD=$(CROSS_COMPILE)ld
STRIP=$(CROSS_COMPILE)strip
RANLIB=$(CROSS_COMPILE)ranlib
SIZE=$(CROSS_COMPILE)size
LIBDIR=$(TOOLCHAINS_PATH)/lib

#KERNEL_DIR=../../linux

INCLUDE = -I$(KERNEL_DIR)/include
include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk

EXEC = switch
CFLAGS += -I$(KERNEL_DIR)
CFLAGS += -I$(KERNEL_DIR)/drivers/net/raeth
CFLAGS += -I$(BUILDDIR)
#CONF_H  = $(BUILDDIR)/autoconf.h

export CFLAGS +=${EXTRA_AFLAGS}

ifeq ($(CONFIG_RALINK_RT6855),y)
SRC=switch_gsw.c
else
ifeq ($(CONFIG_RALINK_MT7620),y)
SRC=switch_gsw.c
else
ifeq ($(CONFIG_RALINK_MT7621),y)
SRC=switch_gsw.c
else
ifeq ($(CONFIG_RALINK_RT6855A),y)
SRC=switch_gsw.c
else
SRC=switch.c
endif
endif
endif
endif

all: $(EXEC)
switch: $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(LDLIBS)

install:
	cp -rf $(EXEC) $(FSROOT)/usr/sbin/
	cp -f ./config-vlan.sh $(FSROOT)/usr/sbin/
	cp -rf $(BUILDDIR)/.config $(FSROOT)/usr/sbin/config.sh 
clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

