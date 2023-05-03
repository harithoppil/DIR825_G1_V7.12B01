#CROSS=mips-linux-

#CC=$(CROSS)gcc
AR=$(CROSS)ar
LD=$(CROSS)ld
STRIP=$(CROSS)strip
RANLIB=$(CROSS)ranlib
SIZE=$(CROSS)size
LIBDIR=$(CROSS_PATH)/lib

KERNEL_DIR=../../linux

INCLUDE = -I$(KERNEL_DIR)/include 

#CFLAGS= -mips1 -msoft-float -DETHCMD_DBG -DETHCMD_EXTENSION
#CFLAGS= -mips1 -msoft-float 

ifneq ($(strip $(TCSUPPORT_CODE_SHRINK)),)
CFLAGS += -Os -DDBG
else
CFLAGS += -DDBG
endif

# If you want to support multiple radius server for multiple bssid, add following line
CFLAGS +=  -DMULTIPLE_RADIUS

ifneq ($(TC_CFLAGS), )
	CFLAGS+=$(TC_CFLAGS)
endif
ifeq ($(RT_8021X_EXT),1)
  	CFLAGS+= -DRT_8021X_EXT
endif

OBJS = rtdot1x.o eloop.o eapol_sm.o radius.o md5.o  \
	config.o ieee802_1x.o  \
	sta_info.o radius_client.o
EXECS= rtdot1xd

all: rtdot1xd

#.c.o:
#	$(CC) -c $(CFLAGS) $<

rtdot1xd: $(OBJS) 
	$(CC) $(OBJS) -o $@

install:
	$(Q)cp $(EXECS) $(FSROOT)/usr/sbin/
	$(Q)ln -s  ./rtdot1xd  $(FSROOT)/usr/sbin/rtdot1xd-24G	
	$(Q)ln -s  ./rtdot1xd  $(FSROOT)/usr/sbin/rtdot1xd-5G

clean:
	rm -f *.o *.a $(EXECS)
