CONFIG_SHELL=sh
check-lxdialog=check-lxdialog.sh
HOSTCC=gcc
CFLAGS += $(shell $(CONFIG_SHELL) $(check-lxdialog) -ccflags)
LDFLAGS += $(shell $(CONFIG_SHELL) $(check-lxdialog) -ldflags $(HOSTCC))
lxdialog:checklist.o menubox.o textbox.o yesno.o inputbox.o  util.o lxdialog.o msgbox.o

