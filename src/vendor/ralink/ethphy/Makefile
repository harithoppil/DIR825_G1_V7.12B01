AR=$(CROSS_COMPILE)ar
LD=$(CROSS_COMPILE)ld
STRIP=$(CROSS_COMPILE)strip
RANLIB=$(CROSS_COMPILE)ranlib
SIZE=$(CROSS_COMPILE)size
LIBDIR=$(TOOLCHAINS_PATH)/lib

EXEC = phyval
OBJS = phy_val.o
CFLAGS += -I$(KERNEL_DIR)/include
CFLAGS += -I$(KERNEL_DIR)/drivers/net/raeth

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

install:
	cp -rf $(EXEC) $(FSROOT)/usr/bin/

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

