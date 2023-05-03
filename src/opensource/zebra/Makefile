ifeq "$(ROOTDIR)" "" 
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif
include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk
.PHONY: all build clean distclean 
SRCDIR=$(CURDIR)/zebra-0.95a

ifeq ($(CONFIG_IPV6), y)
CONFIG_OPTION = --disable-FEATURE --disable-bgpd --disable-ospfd --disable-ospf6d --disable-bgp-announce
DST_OBJ = zebra ripd ripngd
else
CONFIG_OPTION = --disable-FEATURE --disable-ipv6 --disable-bgpd --disable-ripngd --disable-ospfd --disable-ospf6d --disable-bgp-announce
DST_OBJ = zebra ripd
endif

all:configure build
configure:
	cd $(SRCDIR); ./configure --prefix=$(TMPDISTDIR) --host=$(TARGET) $(CONFIG_OPTION)
	touch configure
build:
	make -C $(SRCDIR)

clean:
	make -C $(SRCDIR) clean

distclean:
	make -C $(SRCDIR) distclean
	rm -f configure

install:
	if [ ! -d $(FSROOT)/sbin/ ]; then mkdir -p $(FSROOT)/sbin/; fi
	make -C $(SRCDIR) install
	cd $(TMPDISTDIR)/sbin/; cp -f $(DST_OBJ) $(FSROOT)/sbin/
	cd $(FSROOT)/sbin/; $(STRIP) $(DST_OBJ)

