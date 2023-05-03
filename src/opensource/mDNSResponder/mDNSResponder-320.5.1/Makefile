ifeq "$(ROOTDIR)" "" 
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif
include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk
all:build install 
build:
	$(MAKE) -C mDNSPosix os=linux SAResponder        
	                                                                                           
install:
	$(MAKE) -C mDNSPosix os=linux install_tmpdistdir

clean:
	$(MAKE) -C mDNSPosix os=linux clean