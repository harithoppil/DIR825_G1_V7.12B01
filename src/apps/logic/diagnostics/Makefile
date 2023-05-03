ifeq "$(ROOTDIR)" ""
	export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif

include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk

#ifeq ($(CONFIG_DRIVERS_DSL_REALTEK),y)
SRCDIR=$(CURDIR)/rtl8672
#endif

#ifeq ($(CONFIG_DRIVERS_TC3262),y)
SRCDIR=$(CURDIR)/ralink
#endif

all: build

build:
	echo "diagnostics is ok!"
	$(CC) -o diag $(CURDIR)/diag_conn.c
	make -C $(SRCDIR)

clean:
	cd $(SRCDIR); \
	make clean
distclean:
	cd $(SRCDIR); \
	make distclean

install:
	cp diag $(FSROOT)/sbin/diag
	cp $(SRCDIR)/diagtest $(FSROOT)/sbin/diagtest
	$(STRIP) $(FSROOT)/sbin/diagtest
