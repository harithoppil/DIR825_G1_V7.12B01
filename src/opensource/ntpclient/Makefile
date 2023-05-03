ifeq "$(ROOTDIR)" "" 
export ROOTDIR=$(shell while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
endif
include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk
CURDIR = $(shell pwd)
TARGET=ntpclient

all: ntpclient
objects: ntpclient.o phaselock.o

ntpclient: ntpclient.o phaselock.o
	$(Q)$(CC) -o ntpclient ntpclient.o phaselock.o

ntpclient.o: Makefile ntpclient.c
	$(Q)$(CC) -c $(CFLAGS) ntpclient.c

phaselock.o: Makefile phaselock.c
	$(Q)$(CC) -c $(CFLAGS) phaselock.c

clean:
	$(Q)rm -f *.o *.gdb *.elf ntpclient

install:
	cp -f $(TARGET) $(FSROOT)/sbin
	$(STRIP) $(FSROOT)/sbin/$(TARGET)
	
.PHONY: all fresetd clean
