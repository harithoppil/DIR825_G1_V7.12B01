include $(ROOTDIR)/BaseVar.mk
.PHONY: all build clean disclean install

SRCDIR=$(CURDIR)/siproxd-0.5.1
#SRCDIR=$(CURDIR)/siproxd-0.7.2

all: configure build


#configure:
#	cd $(SRCDIR); \
#	./configure --prefix=$(TMPDISTDIR) --host=$(TARGET) --with-libosip-prefix=$(TMPDISTDIR) \
#	--disable-shared --enable-static-libosip2 CFLAGS=-Os;
#	touch configure

configure:
	cd $(SRCDIR); \
	./configure --prefix=$(TMPDISTDIR) --host=$(TARGET) --with-libosip-prefix=$(TMPDISTDIR) \
	--disable-shared \
	CFLAGS="-Os -s -fomit-frame-pointer -fno-exceptions"
	touch configure
	#--enable-static 
build:
	make -C $(SRCDIR) all 


install:
#	make -C $(SRCDIR) install
	cp $(ROOTDIR)/src/opensource/siproxd/siproxd $(FSROOT)/sbin/siproxd
	$(STRIP) $(FSROOT)/sbin/siproxd	
