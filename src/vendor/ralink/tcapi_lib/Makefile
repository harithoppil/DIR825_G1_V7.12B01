#
#Configured directories
#
#prefix	=	/usr/local
#exec_prefix	=	/usr/local
#libdir	=	$(exec_prefix)/lib
libdir=	lib
#
#Install commands
#
INSTALL=	/usr/bin/install -c
INSTALL_DIR	=	$(INSTALL) -d
INSTALL_LIB	=	$(LIBTOOL)	$(INSTALL) -m 755

#CROSS_COMPILE=mips-linux-gnu-
include ${ROOTDIR}/build/arch.mk
include $(ROOTDIR)/.config
include $(ROOTDIR)/build/product/tcsupport.mk
export CFLAGS +=${EXTRA_AFLAGS}

#CC=$(CROSS)gcc -mips32r2 -msoft-float -muclibc
AR=$(CROSS_COMPILE)ar
LD=$(CROSS_COMPILE)ld 
STRIP=$(CROSS_COMPILE)strip
RANLIB=$(CROSS_COMPILE)ranlib
SIZE=$(CROSS_COMPILE)size
LIBDIR=$(TOOLCHAINS_SYSROOT)/lib

DSOFLAGS=	-Wl,-soname,libtcapi.so.1,-rpath,$(libdir) -shared $(OPTIM)
LN      =	/bin/ln -fs
RM      =	/bin/rm -f
OPTIM	=	-fPIC -Os -g

#KERNEL_DIR=../../linux

#INCLUDE = -I$(KERNEL_DIR)/include 

#CFLAGS= -mips1 -msoft-float 
CFLAGS +=-fPIC -Os -g  -Wall

.SUFFIXES:  .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<

LIBOBJS =       tcapi.o
LIBTCAPI=       libtcapi.so.1.4
TARGETS =       $(LIBTCAPI)
SOFTLINK=		libtcapi.so libtcapi.so.1

all:  $(TARGETS)

$(LIBOBJS): tcapi.h
tcapi.o:tcapi.h

libtcapi.so.1.4: $(LIBOBJS)
	$(CC) $(CFLAGS) $(DSOFLAGS) -o libtcapi.so.1.4 $(LIBOBJS)
	$(STRIP) --strip-unneeded $(LIBTCAPI)
	$(RM) libtcapi.so libtcapi.so.1
	$(LN) libtcapi.so.1.4 libtcapi.so
	$(LN) libtcapi.so.1.4 libtcapi.so.1
	$(INSTALL_DIR) $(TMPDISTDIR)/lib
	$(INSTALL_LIB) libtcapi.so.1.4 $(TMPDISTDIR)/lib
	$(RM) $(TMPDISTDIR)/lib/libtcapi.so
	$(LN) libtcapi.so.1.4 $(TMPDISTDIR)/lib/libtcapi.so
	$(RM) $(TMPDISTDIR)/libtcapi.so.1
	$(LN) libtcapi.so.1.4 $(TMPDISTDIR)/lib/libtcapi.so.1
	cp libtcapi.h $(TMPDISTDIR)/include

install:
	cp -Pf libtcapi.so.1.4 $(FSROOT)/lib/
	ln -fs libtcapi.so.1.4 $(FSROOT)/lib/libtcapi.so
	ln -fs libtcapi.so.1.4 $(FSROOT)/lib/libtcapi.so.1
	
	
clean:
	$(RM) $(LIBTCAPI) $(SOFTLINK)
