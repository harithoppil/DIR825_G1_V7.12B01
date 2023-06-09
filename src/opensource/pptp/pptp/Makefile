# $Id: Makefile,v 1.47 2008/05/14 06:32:52 quozl Exp $
VERSION=1.7.2
RELEASE=

#################################################################
# CHANGE THIS LINE to point to the location of your pppd binary.
PPPD = /usr/sbin/pppd
PPPD_OPTIONS = /var/ppp/options.pptp
#################################################################

BINDIR=$(DESTDIR)/usr/sbin
MANDIR=$(DESTDIR)/usr/share/man/man8
PPPDIR=$(DESTDIR)/etc/ppp

#CC	= gcc
RM	= rm -f
OPTIMIZE= -O0
DEBUG	= -g
INCLUDE =
CFLAGS  = -Wall $(OPTIMIZE) $(DEBUG) $(INCLUDE)
LIBS	= -lutil
# Solaris 10
# LIBS	= -lnsl -lsocket -lresolv
# Solaris Nevada build 14 or above
# LIBS    = -lnsl -lsocket
LDFLAGS	=

PPTP_BIN = pptp

PPTP_OBJS = pptp.o pptp_gre.o ppp_fcs.o \
            pptp_ctrl.o dirutil.o vector.o \
	    inststr.o util.o version.o test.o \
	    pptp_quirks.o orckit_quirks.o pqueue.o pptp_callmgr.o routing.o \
	    pptp_compat.o

PPTP_DEPS = pptp_callmgr.h pptp_gre.h ppp_fcs.h util.h test.h \
	    pptp_quirks.h orckit_quirks.h config.h pqueue.h routing.h

all: config.h $(PPTP_BIN)

$(PPTP_BIN): $(PPTP_OBJS) $(PPTP_DEPS)
	$(CC) -o $(PPTP_BIN) $(PPTP_OBJS) $(LDFLAGS) $(LIBS)

pptpsetup.8: pptpsetup
	pod2man $? > $@

config.h: 
	echo "/* text added by Makefile target config.h */" > config.h
	echo "#define PPTP_LINUX_VERSION \"$(VERSION)$(RELEASE)\"" >> config.h
	echo "#define PPPD_OPTIONS \"$(PPPD_OPTIONS)\"" >> config.h
	echo "#define PPPD_BINARY \"$(PPPD)\"" >> config.h

vector_test: vector_test.o vector.o
	$(CC) -o vector_test vector_test.o vector.o

clean:
	$(RM) *.o config.h pptp

clobber: clean
	$(RM) $(PPTP_BIN) vector_test

distclean: clobber

test: vector_test

dynamic: all install_bcm

install_bcm:
	install -m 755 pptp $(INSTALL_DIR)/bin
	$(STRIP) $(INSTALL_DIR)/bin/pptp

install:
	mkdir -p $(BINDIR)
	install -o root -m 555 pptp $(BINDIR)
	install -o root -m 555 pptpsetup $(BINDIR)
	mkdir -p $(MANDIR)
	install -m 644 pptp.8 $(MANDIR)
	install -m 644 pptpsetup.8 $(MANDIR)
	mkdir -p $(PPPDIR)
	install -m 644 options.pptp $(PPPDIR)

uninstall:
	$(RM) $(BINDIR)/pptp $(MANDIR)/pptp.8

dist: clobber
	$(RM) pptp-$(VERSION)$(RELEASE).tar.gz
	$(RM) -r pptp-$(VERSION)
	mkdir pptp-$(VERSION)
	cp --recursive ChangeLog Makefile *.c *.h options.pptp pptp.8 \
		pptpsetup Documentation AUTHORS COPYING INSTALL NEWS \
		README DEVELOPERS TODO USING PROTOCOL-SECURITY \
		pptp-$(VERSION)/
	$(RM) -r pptp-$(VERSION)/CVS pptp-$(VERSION)/*/CVS
	tar czf pptp-$(VERSION)$(RELEASE).tar.gz pptp-$(VERSION)
	$(RM) -r pptp-$(VERSION)
	md5sum pptp-$(VERSION)$(RELEASE).tar.gz

deb:
	chmod +x debian/rules 
	fakeroot dpkg-buildpackage -us -uc
	mv ../pptp_$(VERSION)-0_i386.deb .

WEB=~/public_html/external/mine/pptp/pptpconfig
release:
	cp pptp_$(VERSION)-0_i386.deb $(WEB)
	cd $(WEB);make
