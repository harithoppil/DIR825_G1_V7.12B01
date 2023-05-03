include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk
.PHONY: all build clean distclean 
SRCDIR=$(CURDIR)/mini_httpd-1.19

export CFLAGS = -I$(BUILDDIR)
#all:configure build
all:build install
ifeq ("$(CONFIG_OPENSOURCE_HTTPD_SSL)","y")
SSL_TREE =      $(TMPDISTDIR)/ssl
SSL_DEFS =      -DUSE_SSL
SSL_INC =       -I$(TMPDISTDIR)/include
SSL_LIBS =      -L$(TMPDISTDIR)/lib -lssl -lcrypto
export SSL_TREE SSL_DEFS SSL_INC SSL_LIBS
endif

#configure:ssap

#ssap:
#	 make -C $(ROOTDIR) ssap

build:
	make -C $(SRCDIR) all CC=$(CC) LD=$(LD) 

clean:
	cd $(SRCDIR); \
	make clean

distclean:clean	
#	cd $(SRCDIR); \
#	make distclean
#	rm -f configure
	
install:
	mkdir -p  $(FSROOT)/usr/sbin
	mkdir -p  $(FSROOT)/usr/www
	mkdir -p  $(FSROOT)/etc
ifeq ("$(CONFIG_OPENSOURCE_HTTPD_SSL)","y")
	cp -f ${SRCDIR}/mini_httpd.pem $(FSROOT)/etc/
endif
	cp -f $(SRCDIR)/mini_httpd $(FSROOT)/usr/sbin
	cp -f $(SRCDIR)/mini_httpd.cnf $(FSROOT)/etc
	cp -f ${SRCDIR}/.htdgst $(FSROOT)/usr/www
	cp -f ${SRCDIR}/.htpasswd $(FSROOT)/usr/www
	cp -f ${SRCDIR}/mini_httpd.cnf $(FSROOT)/etc
