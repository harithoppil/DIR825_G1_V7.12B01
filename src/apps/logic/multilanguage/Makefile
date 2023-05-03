ifeq "$(ROOTDIR)" ""
	export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif

include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk


SRCDIR=$(CURDIR)/httpget


all: build

build:
	make -C $(SRCDIR)

clean:
	cd $(SRCDIR); \
	make clean
distclean:
	cd $(SRCDIR); \
	make distclean

install:
	cp $(SRCDIR)/httpget $(FSROOT)/sbin/httpget
	$(STRIP) $(FSROOT)/sbin/httpget
