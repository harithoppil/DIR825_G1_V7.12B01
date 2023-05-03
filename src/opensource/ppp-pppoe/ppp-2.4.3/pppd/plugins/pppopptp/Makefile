# Generated automatically from Makefile.in by configure.
#***********************************************************************
#
# Makefile
#
# Makefile for Roaring Penguin's Linux PPPoE plugin.
#
# Copyright (C) 2001 Roaring Penguin Software Inc.
#
# This program may be distributed according to the terms of the GNU
# General Public License, version 2 or (at your option) any later version.
#
# $Id: Makefile.linux,v 1.6 2004/11/14 07:58:37 paulus Exp $
#***********************************************************************
DESTDIR = $(FSROOT)/usr
LIBDIR = $(DESTDIR)/lib/pppd/$(VERSION)
VERSION = $(shell awk -F '"' '/VERSION/ { print $$2; }' ../../patchlevel.h)

INSTALL	= install
# Version is set ONLY IN THE MAKEFILE!  Don't delete this!
VERSION=3.3

COPTS = -O2
CFLAGS = $(COPTS) -I. -I../../.. -I$(ROOTDIR)/$(LINUXDIR)/include -fPIC  -I../../../include/linux $(EXTRA_CFLAGS)

PLUGINS := pptp.so

all: $(PLUGINS)

OBJS = dirutil.o pptp.o pptp_ctrl.o util.o \
       orckit_quirks.o pptp_callmgr.o pptp_quirks.o vector.o

%.so: $(OBJS)
	$(CC) $(CFLAGS) -o $@ -shared $^ $(LIBS)

install: all
	$(INSTALL) -d -m 755 $(LIBDIR)
	$(INSTALL) -s -c -m 755 pptp.so $(LIBDIR)

clean:
	rm -f *.o *.so

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
