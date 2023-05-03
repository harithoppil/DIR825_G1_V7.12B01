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

EXEC = mii_mgr
OBJS = mii_mgr.o
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth

all: $(EXEC)
all:
$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

install:
	cp -rf $(EXEC) $(FSROOT)/usr/bin/

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

