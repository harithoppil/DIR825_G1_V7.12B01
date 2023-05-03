#include ${ROOTDIR}/.config
#ifeq ($(CONFIG_DOUBLE_BACKUP),y)
#CFLAGS += -DCONFIG_DOUBLE_BACKUP
#endif

ifeq "$(ROOTDIR)" ""
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif


ifeq "$(CONFIG_DONE)" ""
CONFIG_DONE=$(shell if [ -f $(ROOTDIR)/.config ]; then echo yes;exit; else echo no;fi;)
endif

#restore the change
ifeq "$(CONFIG_DONE)" "no"
include ${ROOTDIR}/product/RTL8196C_Genie2P/defconfig
else
include ${ROOTDIR}/.config
endif

CC=gcc
PROGS = mkimage 

OBJS += imgbuildermain.o imgbuilder.o cksum.o
CFLAGS += -I${ROOTDIR}/src/include

ifeq ("$(CONFIG_PINANDOTHER_ENABLED)","y")
CFLAGS += -DPINANDOTHER_ENABLED
endif

ifeq ("$(CONFIG_GENERATE_SSID_PASSPHRASE)","y")
CFLAGS += -DCONFIG_GENERATE_SSID_PASSPHRASE
endif

all : mkimage 
$(OBJS):imgbuilder.h  $(ROOTDIR)/src/include/flash_layout.h 
mkimage:$(OBJS) 
	$(CC) -o ${PROGS} $(OBJS)

clean:
	rm -f $(PROGS) $(OBJS)
