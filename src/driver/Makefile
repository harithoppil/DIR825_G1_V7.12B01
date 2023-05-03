ifeq "$(ROOTDIR)" "" 
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif
CONFIG_DRIVERS_WLAN=y
include $(ROOTDIR)/.config

subdir-$(CONFIG_DRIVERS_WLAN) += wireless 
subdir-$(CONFIG_DRIVERS_DSL) += dsl 
subdir-$(CONFIG_DRIVERS_TBSMOD) += tbs

subdir-${CONFIG_RT63365} += tc3262
subdir-${CONFIG_RT65168} += tc3262
subdir-${CONFIG_RT63368} += gemac
subdir-${CONFIG_DRIVERS_HWNAT} += hwnat
subdir-${CONFIG_RALINK_MT751X_RA_HWNAT} += ra_hwnat_7510

include $(ROOTDIR)/Rules.make
