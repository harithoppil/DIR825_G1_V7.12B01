include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk
.PHONY: all build clean  

SRCDIR=$(CURDIR)/openl2tp-1.6
export PPPD_SRCDIR=$(ROOTDIR)/build/opensource/ppp-pppoe/ppp-2.4.3
#EXTRA_CFLAGS=-g
all: build

build:
	mkdir -p  $(FSROOT)/sbin $(TMPDISTDIR)/sbin
	mkdir -p  $(FSROOT)/etc $(TMPDISTDIR)/etc
	echo " " >> $(SRCDIR)/l2tp_config_token.c
	make -C $(SRCDIR) TBS_filesystem=$(TMPDISTDIR) CC=$(CROSS_COMPILE)gcc
	
	

clean:
	cd $(SRCDIR); \
	make clean

install:
	make -C $(SRCDIR) install
	
	
	cp $(TMPDISTDIR)/sbin/openl2tpd $(FSROOT)/sbin/openl2tpd
	cp $(TMPDISTDIR)/lib/ppp_unix.so $(FSROOT)/lib/ppp_unix.so
	-cp $(SRCDIR)/pppd/pppol2tp.so $(FSROOT)/lib/pppol2tp.so
	-cp $(SRCDIR)/pppd/openl2tp.so $(FSROOT)/lib/openl2tp.so
	cp $(TMPDISTDIR)/lib/libusl.a $(FSROOT)/lib/libusl.a
	#$(STRIP) $(FSROOT)/sbin/l2tpd
	
	#cd $(FSROOT)/etc; ln -sf ../var/l2tpd.conf l2tpd.conf

