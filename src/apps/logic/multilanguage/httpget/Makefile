ifeq "$(ROOTDIR)" ""
	export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif

include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk

CFLAGS= -I$(BUILDDIR) -I$(BUILDDIR)/include -I$(BUILDDIR)/kernel/${TBS_KERNEL_VERSION}/include -I$(BUILDDIR)/apps/include -I$(CURDIR) -g -ggdb
LDFLAGS= #-lssap -L$(BUILDDIR)/apps/ssap/lib -L$(TMPDISTDIR)/lib -lscew -lexpat -lz

ifeq "$(ROOTDIR)" "y"
CFLAGS += -DDG_DEBUG
endif



all: httpget

httpget: httpget.o 
	$(CC) $(LDFLAGS) -o $@ $^


clean: $(EXTRA_CLEANS)
	rm -f httpget.o


distclean: clean
	@rm -f TAGS `find . -name '*~' -o -name '.*~'` `find . -name '*.rej'` `find . -name '*.d'` .makefirst

install:
	cp httpget $(FSROOT)/sbin/httpget
	$(STRIP) $(FSROOT)/sbin/httpget
