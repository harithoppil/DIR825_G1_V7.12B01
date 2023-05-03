#CROSS=mips-linux-

#CC=$(CROSS)gcc
AR=$(CROSS)ar
LD=$(CROSS)ld
STRIP=$(CROSS)strip
RANLIB=$(CROSS)ranlib
SIZE=$(CROSS)size
LIBDIR=$(CROSS_PATH)/lib

KERNEL_DIR=../../../kernel/linux-2.6.36/

INCLUDE = -I$(KERNEL_DIR)/arch/mips/include -I$(KERNEL_DIR)/include

include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
export CFLAGS +=${EXTRA_AFLAGS}

#CFLAGS= -mips1 -msoft-float -DETHCMD_DBG -DETHCMD_EXTENSION
#CFLAGS= -mips1 -msoft-float 
ifeq ($(TCSUPPORT_ETH4_WAN_PORT),)
  	CFLAGS += -DVPORT
endif

ifneq ($(TC_CFLAGS), )
        CFLAGS+=$(TC_CFLAGS)
endif

ifneq ($(strip $(TCSUPPORT_CODE_SHRINK)),)
CFLAGS += -Os
endif

OBJS= ethcmd.o libcmd.o ethercmd.o libmii.o ether.o
EXECS= ethcmd

all: ethcmd

.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDE) $<

ethcmd: $(OBJS) 
	$(CC) $(OBJS) $(INCLUDE) -o $@
#	$(STRIP) --strip-unneeded $(EXECS)

install:
	cp -rf ethcmd $(FSROOT)/usr/bin/
	cp -rf hw_nat $(FSROOT)/usr/bin/

clean:
	rm -f *.o *.a $(EXECS)
