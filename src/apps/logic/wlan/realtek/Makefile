ifeq "$(ROOTDIR)" "" 
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif
include $(ROOTDIR)/.config
EXTRA_CFLAGS=  -I$(BUILDDIR) -I$(BUILDDIR)/kernel/${TBS_KERNEL_VERSION}/include -I$(BUILDDIR)/apps/include 
EXTRA_CFLAGS += -I$(BUILDDIR)/hal/wireless -I$(BUILDDIR)/apps/ssap/msg -I$(BUILDDIR)/apps/ssap/lib

ifeq ($(strip $(SUPPORT_WAPI)), y)
EXTRA_CFLAGS += -DTBS_WAPI_ENABLE
endif


EXTRA_CFLAGS=  -I$(BUILDDIR) -I$(BUILDDIR)/include -I$(BUILDDIR)/kernel/${TBS_KERNEL_VERSION}/include -I$(BUILDDIR)/apps/include
EXTRA_CFLAGS += -I$(BUILDDIR)/hal/wireless -I$(BUILDDIR)/apps/ssap/msg -I$(BUILDDIR)/apps/ssap/lib  
O_TARGET = wl_api_vendor_wrapper.o
obj-y += rtl_wl_api_wrapper.o
obj-y += rtl_wl_wps.o
include $(ROOTDIR)/Rules.make
