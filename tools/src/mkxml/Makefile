# copy and complie xmlparser

ROOT_DIR=../../..
SRC_PATH=$(ROOT_DIR)/src
XML_PATH=$(SRC_PATH)/opensource/xmlparser
TMPDISTDIR=$(shell pwd)/xmlparser/dist
EXPATDIR=$(shell pwd)/xmlparser/expat
SCEWDIR=$(shell pwd)/xmlparser/scew
TARGET=

APP_PATH=$(SRC_PATH)/apps
LOGIC_PATH=$(APP_PATH)/logic
SSAP_PATH=$(APP_PATH)/ssap

xml:xml_flash prepare expat scew xmld
	rm -rf ./xmlparser configure*
	cp -f ./xmltool ../../bin/
	cp -f ./xml_flash_tool ../../bin/
#	rm -f ./xmltool ./xml_flash_tool

prepare:
	cp -rf $(XML_PATH) ./
	mkdir -p $(TMPDISTDIR)

configure_expat:
	cd $(EXPATDIR); \
	find | xargs touch; \
	./configure --prefix=$(TMPDISTDIR) --host=$(TARGET);
	touch configure_expat

expat:configure_expat
	make all install -C $(EXPATDIR)

configure_scew:
	cd $(SCEWDIR); \
	find -name Makefile.in | xargs touch; \
	CPPFLAGS=-I$(TMPDISTDIR)/include LDFLAGS=-L$(TMPDISTDIR)/lib ./configure --prefix=$(TMPDISTDIR) --host=$(TARGET);
	touch configure_scew

scew:configure_scew
	make all install -C $(SCEWDIR)
	cp -f $(SCEWDIR)/scew/x*.h $(TMPDISTDIR)/include/scew

INCLUDE=-I$(PWD) -I$(SRC_PATH)/include -I$(APP_PATH)/include  -I$(SSAP_PATH)/msg -I$(APP_PATH)/omlib
CFLAGS=-Wall -g -pipe
CFLAGS += -D_CFG_BYTE_NEED_CVT

xmld:
	gcc -o xmltool -g xml.c \
	    $(CFLAGS) -D_CFG_DEBUG -D_TBS_ENV_PC \
	    $(SSAP_PATH)/cfg/cfg_get.c $(SSAP_PATH)/cfg/cfg_set.c $(SSAP_PATH)/cfg/cfg_accessor.c \
            $(SSAP_PATH)/cfg/cfg_comm.c $(SSAP_PATH)/cfg/cfg_file.c $(SSAP_PATH)/cfg/cfg_mid.c \
            $(SSAP_PATH)/cfg/cfg_r2.c ./pc_stub.c $(SSAP_PATH)/lib/tbsutil.c $(SSAP_PATH)/lib/tbsflash.c $(SSAP_PATH)/syslog/warnlog.c \
	    $(INCLUDE) -I./ \
	    -I$(SSAP_PATH)/cfg -I$(TMPDISTDIR)/include \
	    $(TMPDISTDIR)/lib/libscew.a \
	    $(TMPDISTDIR)/lib/libexpat.a \
	    -lz

xml_flash:
	gcc -o xml_flash_tool -g xml_flash.c \
	    $(CFLAGS) -D_CFG_DEBUG -D_TBS_ENV_PC \
	    $(SSAP_PATH)/cfg/cfg_file.c $(SSAP_PATH)/lib/tbsutil.c $(SSAP_PATH)/lib/tbsflash.c $(SSAP_PATH)/syslog/warnlog.c \
	    $(INCLUDE) -I./ -I$(SSAP_PATH)/cfg -lz
	cp -f ./xml_flash_tool ../../bin/

clean:
	rm -rf ./xmlparser configure*
#	cp -f ./xmltool ../../bin/
#	cp -f ./xml_flash_tool ../../bin/
	rm -f ./xmltool ./xml_flash_tool
	
