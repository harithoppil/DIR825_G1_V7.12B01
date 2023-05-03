
include $(TOPDIR)/config.mk
include $(BUILDDIR)/.config

$(shell rm ${BUILDDIR}/boot/tbsboot/cpu/${CPU}/product.c -f)
$(shell ln -s ${BUILDDIR}/product/product.c ${BUILDDIR}/boot/tbsboot/cpu/${CPU}/product.c)
LIB	= $(obj)lib$(CPU).a



START	= start.o 
COBJS	=  
AOBJS  = cpu.o serial.o spi_flash.o skbuff.o mt7620_eth.o gpio_boot.o led.o button.o product.o
SOBJS	=  

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c) $(AOBJS:.o=.S)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS) $(AOBJS))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
