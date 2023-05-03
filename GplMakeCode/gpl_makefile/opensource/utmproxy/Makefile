include $(ROOTDIR)/.config
include $(ROOTDIR)/BaseVar.mk
.PHONY: all build clean

CC=$(CROSS_COMPILE)gcc

SRCDIR=$(CURDIR)

EXEC = utmproxy

#SOURCES=aes.c alloc.c address.c blocked.c board.c buffer.c cloud.c config.c crashlog.c debug.c dns.c dump.c endian.c fifo.c hashing.c hashmap.c hmac.c host.c hosts.c hrtime.c http.c license.c list.c main.c nvram.c pbkdf2.c pipeline.c proxy.c queue.c sha1.c socket.c device.c stack.c transfer.c udp.c url.c util.c #sc_cfg.c

#OBJS=$(SOURCES:.c=.o)
#OBJS = aes.o alloc.o address.o arp.o blocked.o board.o buffer.o cloud.o config.o crashlog.o debug.o dhcp.o dns.o dump.o endian.o exploit.o fifo.o hashing.o hashmap.o hmac.o host.o hosts.o hrtime.o http.o license.o list.o main.o  nvram.o pbkdf2.o pipeline.o proxy.o queue.o sha1.o socket.o device.o stack.o transfer.o udp.o url.o util.o sc_cfg.o

CFLAGS +=-fPIC -I$(BUILDDIR)/kernel/${TBS_KERNEL_VERSION}/include/ -I../msg -I../../include -I../../../include -I$(TMPDISTDIR)/include -I../syslog -Wall -O3 -D_GNU_SOURCE -I$(BUILDDIR)
LDFLAGS +=-lutil -lssap -L$(BUILDDIR)/apps/ssap/lib -L$(TMPDISTDIR)/lib -lscew -lexpat -lz

#CFLAGS += -I$(ROOTDIR)/build/kernel/$(TBS_KERNEL_VERSION)/include

#all: $(EXEC) build
all:  build

#$(EXEC): $(OBJS)
#	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS$(LDLIBS-$(@)))

build:
	mkdir -p  $(FSROOT)/sbin $(TMPDISTDIR)/sbin
	install -m 0755 $(SRCDIR)/utmproxy $(TMPDISTDIR)/sbin/utmproxy
	#make -C $(SRCDIR) all

clean:
	cd $(SRCDIR); \
	#rm -f $(EXEC) *.o *~

install:
	cp $(TMPDISTDIR)/sbin/utmproxy $(FSROOT)/sbin/utmproxy
	$(STRIP) $(FSROOT)/sbin/utmproxy
