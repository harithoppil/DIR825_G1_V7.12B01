ifeq "$(ROOTDIR)" ""  
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif
include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk
#include config.mk

O_TARGET=wlan_module.o


#LIBDIR=$(ROOTDIR)/build/tmpdist/lib

ifeq ($(CONFIG_DRIVERS_WLAN_ATHEROS),y)
PRODUCT_DIR=atheros
endif


ifeq ($(CONFIG_DRIVERS_WLAN_REALTEK),y)
PRODUCT_DIR=realtek
endif

ifeq ($(strip $(CONFIG_DRIVERS_WLAN_RALINK)),y)
PRODUCT_DIR=ralink
endif

subdir-y=${PRODUCT_DIR}
obj-y=wl_api.o wlan.o wl_channel.o

ifeq ($(strip $(CONFIG_APPS_LOGIC_WPS)), y)
#obj-y += $(PRODUCT_DIR)/$(PRODUCT_NAME)_wl_wps.o
endif

ifneq (${PRODUCT}, UML)
lib-y += ${PRODUCT_DIR}/wl_api_vendor_wrapper.o
endif

EXTRA_LDFLAGS =  

EXTRA_CFLAGS +=  -I$(BUILDDIR)  -I$(BUILDDIR)/include -I$(BUILDDIR)/kernel/$(TBS_KERNEL_VERSION)/include -I$(BUILDDIR)/apps/include -I$(BUILDDIR)/opensource/wireless_tools.28
EXTRA_CFLAGS += -I$(BUILDDIR)/hal/wireless -I$(BUILDDIR)/apps/ssap/msg -I$(BUILDDIR)/apps/ssap/lib -I${BUILDDIR}/apps/logic/include


ifeq (${PRODUCT}, UML)
	EXTRA_CFLAGS +=
endif
ifdef CONFIG_TBS_APPS_DEBUG_WLAN
EXTRA_CFLAGS += -DWLAN_DEBUG
endif
ifdef CONFIG_TBS_APPS_DEBUG_WPS
EXTRA_CFLAGS += -DWLAN_WPS_DEBUG
endif

include $(ROOTDIR)/Rules.make


