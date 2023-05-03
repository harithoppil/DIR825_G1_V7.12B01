#
# logmonitor: Monitoring system events log file, sompress and save it to file
#     target: /usr/bin/logmonitor
#

ifeq "$(ROOTDIR)" ""
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif
include $(ROOTDIR)/.config
include $(BUILDDIR)/product/arch.mk

EXTRA_CFLAGS=  -I../../include -I${BUILDDIR} -I../../ssap/msg -I${BUILDDIR}/include\
	       -I../../ssap/lib -I$(TMPDISTDIR)/include\
	       -DPRODUCT=\"$(PRODUCT)\"\
	       -DDATE=\"$(DATE)\"\
	       -DTBS_VERSION=\"$(TBS_VERSION)\"

EXTRA_LDFLAGS=-lutil -lssap -L$(BUILDDIR)/apps/ssap/lib -L$(TMPDISTDIR)/lib -lexpat -lz


ifeq ("$(CONFIG_FEATURE_RECORD_SYSEVENTS)","y")
EXTRA_CFLAGS += -DENABLE_FEATURE_RECORD_SYSEVENTS
endif

ifeq ("$(CONFIG_FEATURE_RECORD_MESSAGE)","y")
EXTRA_CFLAGS += -DENABLE_FEATURE_RECORD_MESSAGE
endif

ifeq ("$(CONFIG_FEATURE_REMOTE_LOG)","y")
EXTRA_CFLAGS += -DENABLE_FEATURE_REMOTE_LOG
endif

E_TARGET=logmonitor

obj-y= logger_sysevt.o
obj-y+= syslogd.o

include $(ROOTDIR)/Rules.make

installme::
	$(Q)cp $(E_TARGET) $(FSROOT)/usr/bin/
	$(Q)$(STRIP) $(FSROOT)/usr/bin/$(E_TARGET)

