
include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
export CFLAGS +=${EXTRA_AFLAGS}
AR=$(CROSS_COMPILE)ar
STRIP=$(CROSS_COMPILE)strip
RANLIB=$(CROSS_COMPILE)ranlib


INCLUDE = -I$(KERNEL_DIR)/include -I../include -I $(TMPDISTDIR)/include

#CFLAGS= -mips1 -msoft-float 
#CFLAGS= -mips1 -msoft-float 
#CFLAGS= -mips32r2 -msoft-float -DTRENDCHIP -muclibc
ifeq ($(strip $(CONFIG_TC2031_SUPPORT)),y)
CFLAGS += -DTC2031_DEBUG
endif

ifneq ($(strip $(TC_CFLAGS)), )
CFLAGS += $(TC_CFLAGS)
endif



LDFLAGS += -L $(TMPDISTDIR)/lib -ltcapi

OBJS=tcci.o

all: tcci

.c.o:
	$(CC) -c $(CFLAGS) ${INCLUDE} $<

tcci: $(OBJS) 
	$(CC) $(CFLAGS) $(OBJS) ${INCLUDE} -o $@ $(LDFLAGS)
	$(STRIP) tcci

install:
	cp -rf tcci $(FSROOT)/usr/bin/
ifeq ($(CONFIG_RT65168),y)
	ln -fs tcci $(FSROOT)/usr/bin/hwnat
endif
	ln -fs tcci $(FSROOT)/usr/bin/wan

clean:
	rm -f *.o *.a tcci
