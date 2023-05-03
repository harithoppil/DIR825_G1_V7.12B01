#CROSS=mips-linux-

#CC=$(CROSS)gcc
AR=$(CROSS)ar
LD=$(CROSS)ld
STRIP=$(CROSS)strip
RANLIB=$(CROSS)ranlib
SIZE=$(CROSS)size
LIBDIR=$(CROSS_PATH)/lib

include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
export CFLAGS +=${EXTRA_AFLAGS}
INCLUDE = -I$(KERNEL_DIR)/include 

#CFLAGS= -mips1 -msoft-float -DETHCMD_DBG -DETHCMD_EXTENSION
#CFLAGS= -mips1 -msoft-float 

EXEC = hw_nat acl ac mtr

HWNAT_OBJS = hw_nat.o hwnat_api.o util.o
AC_OBJS = ac.o ac_api.o util.o
ACL_OBJS = acl.o acl_api.o util.o
MTR_OBJS = mtr.o mtr_api.o util.o
CONF_H  = $(KERNEL_DIR)/include/linux/autoconf.h
CFLAGS += -I$(KERNEL_DIR)/net/nat/hw_nat
#CONF_H  = $(ROOTDIR)/$(LINUXDIR)/include/linux/autoconf.h
#CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/net/nat/hw_nat 

all: $(EXEC)

hw_nat: $(HWNAT_OBJS) $(CONF_H)
	$(CC) $(LDFLAGS) -o $@ $(HWNAT_OBJS) 

acl: $(ACL_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(ACL_OBJS) 

ac: $(AC_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(AC_OBJS) 

mtr: $(MTR_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(MTR_OBJS) 

install:
	cp -rf hw_nat $(FSROOT)/usr/bin/
	cp -rf acl $(FSROOT)/usr/bin/
	cp -rf mtr $(FSROOT)/usr/bin/
	cp -rf ac $(FSROOT)/usr/bin/

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

