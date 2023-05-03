ifeq "$(ROOTDIR)" "" 
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif
include $(ROOTDIR)/.config

all:

install:
	${Q}cp $(ROOTDIR)/src/boot/bootloader.bin ${ROMFS}
  
clean:
ifneq (${PRODUCT}, UML)
	make -C $(BOOTDIR) clean
endif

distclean:
ifneq (${PRODUCT}, UML)
	make -C $(BOOTDIR) unconfig
endif
	rm -f $(BUILD_BOOT_DIR)/build/config

#include $(ROOTDIR)/Rules.make
