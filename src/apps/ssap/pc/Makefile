ifeq "$(ROOTDIR)" ""
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif

include $(ROOTDIR)/.config
EXTRA_CFLAGS=-fPIC -I../msg -I../mon -I$(BUILDDIR) -I$(BUILDDIR)/apps/include  -I$(TMPDISTDIR)/include -I../syslog -Wall -I$(BUILDDIR)/include -I$(ROOTDIR)/build/kernel/${TBS_KERNEL_VERSION}/include #-D_PC_DEBUG

ifeq ("$(CONFIG_APPS_LOGIC_DSL)","y")
EXTRA_CFLAGS+=-DENABLE_DSL -I$(BUILDDIR)/apps/logic/dsl -I$(BUILDDIR)/driver/dsl/infineon/drv_dsl_cpe_api/src/include
endif

ifeq ("$(CONFIG_TBS_APPS_DEBUG_PC)","y")
EXTRA_CFLAGS += -D_PC_DEBUG
endif

ifeq ("$(CONFIG_TBS_APPS_DEBUG_MON)","y")
EXTRA_CFLAGS += -DMON_DEBUG
endif

EXTRA_LDFLAGS=-lutil -lssap -L$(BUILDDIR)/apps/ssap/lib -L$(TMPDISTDIR)/lib -lexpat -lz
E_TARGET=pc

obj-y = pc_pcb.o pc_mon.o pc_main.o main.o
obj-y += ../mon/mon_func.o
obj-$(CONFIG_APPS_LOGIC_DSL) += $(BUILDDIR)/apps/logic/dsl/dsl.o
obj-$(CONFIG_APPS_LOGIC_DSL) += $(BUILDDIR)/apps/logic/dsl/dsl_api.o
obj-$(CONFIG_APPS_LOGIC_DSL) += $(BUILDDIR)/apps/logic/dsl/rt65168/rt65168_dsl_api_wrapper.o
obj-$(CONFIG_APPS_SSAP_BUTTON) += ../button/btn_main.o ../button/app_btn_cfg.o


include $(ROOTDIR)/Rules.make
installme::
	$(Q)cp pc $(FSROOT)/usr/bin/
	$(Q)$(STRIP) $(FSROOT)/usr/bin/pc

