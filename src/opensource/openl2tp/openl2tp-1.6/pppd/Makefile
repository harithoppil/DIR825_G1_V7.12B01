# BEGIN CONFIGURABLE SETTINGS

ifdef PPPD_SRCDIR
CFLAGS.pppd=	-I$(PPPD_SRCDIR)/include -I$(PPPD_SRCDIR)/pppd
endif
ifndef PPPD_LIBDIR
PPPD_LIBDIR=	$(SYS_LIBDIR)/pppd/$(PPPD_VERSION)
endif
INCLUDE_DIR?=	/usr/include

# END CONFIGURABLE SETTINGS

CFLAGS	+= $(CFLAGS.optimize) -I$(L2TP_KERNELDIR)/include -I$(KERNEL_BLDDIR)/include \
	-I$(KERNEL_SRCDIR)/include -I.. \
	-I$(INCLUDE_DIR)/pppd \
	$(CFLAGS.pppd) -DHAVE_MULTILINK -fPIC
LDFLAGS	+= -shared

PLUGINS=	pppol2tp.so
PLUGINS+=	openl2tp.so

all:	$(PLUGINS)

pppol2tp.so: pppol2tp.c
	echo 1231313
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) -D_linux_=1 $^

openl2tp.so: openl2tp.c
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) -D_linux_=1 $^

install: pppol2tp.so
	install pppol2tp.so $(TMPDISTDIR)/lib/pppol2tp.so
	install openl2tp.so $(TMPDISTDIR)/lib/openl2tp.so
	

clean:
	-rm -rf *.o *.so *.d
