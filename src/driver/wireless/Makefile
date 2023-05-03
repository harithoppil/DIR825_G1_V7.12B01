ifeq "$(ROOTDIR)" "" 
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif
CONFIG_DRIVERS_WLAN_REALTEK=y
include $(ROOTDIR)/.config

subdir-$(CONFIG_DRIVERS_WLAN_ATHEROS) += atheros
subdir-$(CONFIG_DRIVERS_WLAN_RALINK) += ralink

subdir-$(CONFIG_DRIVERS_WLAN_REALTEK) +=realtek
include $(ROOTDIR)/Rules.make
all:
	make -C realtek install
