#CROSS_COMPILE=mips-linux-gnu-

#CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
LD=$(CROSS_COMPILE)ld
STRIP=$(CROSS_COMPILE)strip
RANLIB=$(CROSS_COMPILE)ranlib
SIZE=$(CROSS_COMPILE)size
include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
export CFLAGS +=${EXTRA_AFLAGS}

INCLUDE = -I$(KERNEL_DIR)/include

KERNEL_INCLUDES:=$(KERNEL_DIR)/include/

#CFLAGS= -mips1 -msoft-float 
#CFLAGS= -mips1 -msoft-float 
#CFLAGS= -mips32r2 -msoft-float -DTRENDCHIP -muclibc
ifeq ($(TC2031_SUPPORT),1)
CFLAGS += -DTC2031_DEBUG
endif



#TBS 编译框架编译选项转换成ralink驱动所需宏
ifeq ($(CONFIG_TCSUPPORT_PRODUCTIONLINE), y)
CFLAGS += -DTCSUPPORT_PRODUCTIONLINE
endif

ifeq ($(CONFIG_TCSUPPORT_PRODUCTIONLINE_CONTENT), y)
CFLAGS += -DTCSUPPORT_PRODUCTIONLINE_CONTENT
endif

ifeq ($(CONFIG_TCSUPPORT_PRODUCTIONLINE_DATE), y)
CFLAGS += -DTCSUPPORT_PRODUCTIONLINE_DATE
endif


ifneq ($(strip $(TC_CFLAGS)),)
CFLAGS += $(TC_CFLAGS)
endif

ATMOBJS=atmcmd.o
ADSLOBJS=adslcmd.o
WLANOBJS=wlancmd.o

all: atmcmd adslcmd wlancmd

.c.o:
	$(CC) -c $(CFLAGS) $< -I$(KERNEL_INCLUDES)

atmcmd: $(ATMOBJS) 
	$(CC) $(ATMOBJS) -o $@
	#$(STRIP) --strip-unneeded atmcmd

adslcmd: $(ADSLOBJS) 
	$(CC) $(ADSLOBJS) -o $@
	#$(STRIP) --strip-unneeded adslcmd
	
wlancmd: $(WLANOBJS) 
	$(CC) $(WLANOBJS) -o $@
	#$(STRIP) --strip-unneeded wlancmd

install:
	install -dv $(FSROOT)/etc/wireless
	ln -fs /var/RT2860AP.dat $(FSROOT)/etc/wireless/RT2860AP.dat
	cp -rf atmcmd $(FSROOT)/usr/bin/atmphxcmd
	cp -rf adslcmd $(FSROOT)/usr/bin/adslphxcmd
	cp -rf wlancmd $(FSROOT)/usr/bin/wlanphxcmd

clean:
	rm -f *.o *.a atmcmd adslcmd wlancmd
