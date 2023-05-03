include $(ROOTDIR)/BaseVar.mk
.PHONY: all build clean

SRCDIR=$(CURDIR)/libosip2-2.0.9
#SRCDIR=$(CURDIR)/libosip2-3.2.0

all: configure build


configure:
	cd $(SRCDIR); \
	SIP_CFLAGS="-Os -s -fomit-frame-pointer -fno-exceptions" \
	./configure --prefix=$(TMPDISTDIR) --host=${TARGET} --disable-shared --enable-static;
	touch configure

build:
	make -C $(SRCDIR) all install

clean:
	cd $(SRCDIR); \
	make clean

distclean:
	rm configure
	cd $(SRCDIR); \
	make distclean

install:
	make -C $(SRCDIR) install
