CFLAGS += -I$(KERNEL_DIR)/include

#include $(ROOTDIR)/.config
#include $(ROOTDIR)/BaseVar.mk

EXEC = ated
SRCS = ated.c

#EXTRA_CFLAGS += -DDBG

all: $(EXEC)

$(EXEC): $(SRCS) $(KERNEL_DIR)/include/linux/autoconf.h
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $@.c 

install:
	cp -rf $(EXEC) $(FSROOT)/usr/sbin/

clean:
	-rm -f $(EXEC) *.o

