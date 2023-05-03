#
# Makefile for the Linux VNET layer.
#

obj-$(CONFIG_VNET) += vnet_mod.o

vnet_mod-objs := vnet.o vnet_dev.o vnet_fdb.o

ifeq ($(CONFIG_PROC_FS),y)
vnet_mod-objs += vnet_proc.o
endif

