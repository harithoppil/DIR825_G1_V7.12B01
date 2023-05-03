ifeq "$(ROOTDIR)" "" 
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif
include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk
.PHONY: all build install  
SRCDIR=$(CURDIR)/mDNSResponder-320.5.1

all: build install


build:
	cd $(SRCDIR); \
	make all

clean:
	cd $(SRCDIR); \
	make clean

install:
	cp $(TMPDISTDIR)/sbin/mDNSResponderPosix $(FSROOT)/sbin/mDNSResponderPosix
	$(STRIP) $(FSROOT)/sbin/mDNSResponderPosix
