#!/bin/bash
#The file use for sitecom Product GPL
#Date: 2014/07/24
#Written by zym
if [ $# -ne 0 ]
then
 echo "======================================================"
 echo "ERROR: This script not need parameter. For example:"
 echo "$0"
 echo "======================================================"
 exit;
fi

if [  -z $MYROOTDIR ];then
while true
do
	if [ -f BaseVar.mk ];then
		MYROOTDIR=`pwd`
		break
	else
		cd ..
	fi
done
echo $MYROOTDIR
fi

MyPath=$MYROOTDIR/GplMakeCode
echo $MyPath

KERNEL_VERSION=linux-2.6.36.x
IsKernelHide=0

#############################################################################################
#								please define the Global var here
#############################################################################################
SkinsStyle="DIR825G1"

sed -i '/^[[:space:]][[:space:]]*for/,+7d' $MYROOTDIR/src/opensource/dlna/dlna_usb/libs/Makefile
sed -i '/^[[:space:]][[:space:]]*for/,+7d' $MYROOTDIR/src/opensource/dlna/dlna_bcm/libs/Makefile
sed -i "/^[[:space:]][[:space:]]*rm -rf/d" $MYROOTDIR/src/opensource/qrencode/Makefile
sed -i '/.*rm -rf configure.*/d' $MYROOTDIR/src/opensource/libnfnetlink/Makefile
sed -i '/.*rm -rf configure.*/d' $MYROOTDIR/src/opensource/libnetfilter_conntrack/Makefile

sed -i '/rm -rf \$(SRC_DIR)/d' $MYROOTDIR/src/opensource/libpng/Makefile
sed -i '/^[[:space:]][[:space:]]*-make -C/d' $MYROOTDIR/src/opensource/openssl/Makefile.101
sed -i '/^[[:space:]][[:space:]]*rm -f configure/d' $MYROOTDIR/src/opensource/openssl/Makefile.101
#############################################################################################
#    					run the follow should make tools && make prepare
#                						compile all code
#############################################################################################
cp $MyPath/gpl_makefile/tools/mkbuilddir $MYROOTDIR/tools -f
:<<!
mv -f src/boot/tbsboot/config.mk src/boot/tbsboot/config_bk.mk 
cp $MyPath/gpl_makefile/boot/config.mk src/boot/tbsboot -f
!
#mv -f $MYROOTDIR/src/boot/Makefile $MYROOTDIR/src/boot/Makefile_bk
#cp $MyPath/gpl_makefile/boot/Makefile $MYROOTDIR/src/boot -f
#cp $MyPath/gpl_makefile/kernel/linux-2.6.36.x/MT7620A/Makefile	$MYROOTDIR/src/kernel/linux-2.6.36.x/	-f
cp -f $MYROOTDIR/src/kernel/linux-2.6.30/Makefile $MYROOTDIR/src/kernel/linux-2.6.30/Makefile_bk


cp $MYROOTDIR/src/opensource/nbns/Makefile $MyPath/gpl_makefile/opensource/Makefile_nbns  -f
sed -i '/.*rm.*nbnslisten/d' $MyPath/gpl_makefile/opensource/Makefile_nbns 
cp $MYROOTDIR/src/opensource/nbsmtp/Makefile $MyPath/gpl_makefile/opensource/Makefile_nbsmtp  -f
sed -i '/^clean/,+2d' $MyPath/gpl_makefile/opensource/Makefile_nbsmtp 
cp $MYROOTDIR/src/opensource/siproxd/Makefile $MyPath/gpl_makefile/opensource/Makefile_siproxd  -f
sed -i '/^clean/,+6d' $MyPath/gpl_makefile/opensource/Makefile_siproxd 
sed -i 's%\(.*cp.*\)\(nbnslisten\)\(.*$\)%\1$(ROOTDIR)/src/opensource/nbns/\2\3%' $MyPath/gpl_makefile/opensource/Makefile_nbns

sed -i's%\(.*cp.*\)\$(SRCDIR)\(/nbsmtp\)\(.*$\)%\1$(ROOTDIR)/src/opensource/nbsmtp\2\3%' $MyPath/gpl_makefile/opensource/Makefile_nbsmtp 
sed -i 's%\(.*cp.*\)\$(SRCDIR)/src\(/siproxd\)\(.*$\)%\1$(ROOTDIR)/src/opensource\2/siproxd\3%' $MyPath/gpl_makefile/opensource/Makefile_siproxd
rm -rf $MYROOTDIR/product/RTL8197DL_AC1200/
make prepare && \
make tools && \
make && \

Product=`awk -F= '{printf $2}' $MYROOTDIR/build/.product`
echo "=====>product:$Product"
cp $MYROOTDIR/product/$Product/app_btn_cfg.c $MYROOTDIR/src/apps/ssap/button -rf

##############################################################################################
#	     						make application layer file
##############################################################################################
cp $MYROOTDIR/src/apps/ssap/lib $MyPath/lib -rf
cp $MYROOTDIR/src/apps/ssap/cfg $MyPath/cfg -rf
cp $MYROOTDIR/src/apps/ssap/syslog $MyPath/syslog -rf
cp $MYROOTDIR/src/apps/ssap/msg $MyPath/msg -rf
cp $MYROOTDIR/src/apps/include $MyPath/include -rf
mkdir $MyPath/html
cp $MYROOTDIR/src/apps/html/html.c $MyPath/html -f
mkdir $MyPath/tbserror
cp $MYROOTDIR/src/apps/ssap/lib/tbserror.c $MyPath/tbserror -f



mkdir $MyPath/en_us
mkdir $MyPath/zh_cn
cp $MYROOTDIR/src/apps/html/standard/languages/en_us/error.js $MyPath/en_us -f
cp $MYROOTDIR/src/apps/html/standard/languages/zh_cn/error.js $MyPath/zh_cn -f


###############################################################################################
#									deal with kernel files
#
###############################################################################################
echo "==================== deal with kernel files ===================="

rm -rf $MYROOTDIR/src/kernel/linux-2.6.30/drivers/net/wireless/rtlmac80211Drv
rm -rf $MYROOTDIR/src/kernel/linux-2.6.30/drivers/net/wireless/prism54
rm -rf $MYROOTDIR/src/kernel/linux-2.6.30/drivers/net/wireless/rtl818x
rm -rf $MYROOTDIR/src/kernel/linux-2.6.30/drivers/net/wireless/libertas
rm -rf $MYROOTDIR/src/kernel/linux-2.6.30/drivers/net/wireless/libertas_tf

if [ -f $MyPath/release_dir ]
then
  while read line
  do
    echo $line|grep "\/rtl8192cd">/dev/null
    if [ $? -eq 0 ];then

	build_dir_kernel=`echo $line|sed 's/src\//build\//g'`
	build_dir_kernel_linux="build/kernel/linux-2.6.30"
	for file in `find ${build_dir_kernel} -name *.o`;
	do
	    src_o_kernel=`echo $file|sed 's/build\//src\//g'`
	    cp $file $src_o_kernel -f

	done
#	mv -f $build_dir_kernel_linux/*.s $line

#	for file in `find ${line} -name *.s`;
#	do
#	    src_S_file=`echo $file|sed 's/\.s$/.S/g'`
#	    mv $file $src_S_file -f
#	done

	mv $line/Makefile_bak $line/Makefile -f
	
	rm $build_dir_kernel -rf
	
	cp $line $build_dir_kernel -rf
#	rm $line -rf
#	cp $build_dir_kernel  $line -rf
	
	echo "enter kernel!"
	find $line -name '*.c'|xargs rm -rf
    find $line -name '*.h'|xargs rm -rf
	IsKernelHide=1
    fi 
    echo $line|grep "\/MT7610">/dev/null
    if [ $? -eq 0 ];then

	build_dir_kernel=`echo $line|sed 's/src\//build\//g'`
	build_dir_kernel_linux="build/kernel/linux-2.6.36.x"
	for file in `find ${build_dir_kernel} -name *.o`;
	do
	    src_o_kernel=`echo $file|sed 's/build\//src\//g'`
	    cp $file $src_o_kernel -f

	done
	
	echo "enter kernel!"
	find $line -name '*.c'|xargs rm -rf
    find $line -name '*.h'|xargs rm -rf
	IsKernelHide=1
    fi 
    
# echo $line|grep "\/MT7612E">/dev/null
#    if [ $? -eq 0 ];then
#    build_dir_kernel=`echo $line|sed 's/src\//build\//g'`
#		build_dir_kernel_linux="build/kernel/linux-2.6.36.x"
		
#		cd $build_dir_kernel/rlt_wifi_ap
#		mv	rlt_wifi.o  rlt_wifi.mod.o  -f
#		cd -
#		for file in `find ${build_dir_kernel} -name *.o`;
#		do
#	    src_o_kernel=`echo $file|sed 's/build\//src\//g'`
#	    cp $file $src_o_kernel -f
#		done
#		
#    echo "enter MT7612E!"
#		find $line -name '*.c'|xargs rm -rf
#    find $line -name '*.h'|xargs rm -rf 
#    fi
  done <$MyPath/release_dir
fi

echo "&&&&&&&&&&&&&&&&&&&& deal with kernel files OK! &&&&&&&&&&&&&&&&&&&&"

###############################################################################################
#									deal with driver files
sed -i "/^endif/a CONFIG_DRIVERS_WLAN=y" $MYROOTDIR/src/driver/Makefile
#
###############################################################################################

##########################################################################################
#									deal with uboot files
#
##########################################################################################
echo "==================== deal with uboot files ===================="
	cp $MYROOTDIR/build/romfs/bootloader.bin $MYROOTDIR/src/boot/
	sed -i "/^install/,+3d" $MYROOTDIR/src/boot/Makefile
	sed -i "20a install:\\n\\t\${Q}cp \$(ROOTDIR)/src/boot/bootloader.bin \${ROMFS}" $MYROOTDIR/src/boot/Makefile
	sed -i "6,19d" $MYROOTDIR/src/boot/Makefile
	sed -i "5a all:" $MYROOTDIR/src/boot/Makefile
	rm -rf src/boot/u-boot-1.1.6 -rf
	rm -rf src/boot/tbsboot -rf


echo "&&&&&&&&&&&&&&&&&&&& deal with uboot files OK! &&&&&&&&&&&&&&&&&&&&"

###########################################################################################
#									deal with apps files
#
###########################################################################################
echo "==================== deal with apps files ===================="

if [ -f $MyPath/release_dir ]
then
    while read line
    do
	echo $line|grep "\/apps">/dev/null
	if [ $? -eq 0 ];then
		build_dir_apps=`echo $line|sed 's/src\//build\//g'`
		echo $line
		echo $line|grep "\/include">/dev/null

		if [ $? -ne 0 ];then
			make -C $build_dir_apps ROOTDIR=`pwd` BUILDDIR=`pwd`/build
		fi

			echo $line|grep "\/ssap">/dev/null
			if [ $? -eq 0 ];then
				cp $build_dir_apps/lib/libssap.so $line/lib/ -f
				cp $build_dir_apps/pc/pc $line/pc/ -f
				cp $build_dir_apps/msg/*.o $line/msg/ -f
				cp $build_dir_apps/cfg/*.o $line/cfg/ -f
				cp $build_dir_apps/pc/*.o $line/pc/ -f
				cp $build_dir_apps/led/*.o $line/led/ -f
				cp $build_dir_apps/mon/*.o $line/mon/ -f 
				cp $build_dir_apps/button/*.o $line/button/ -f
				cp $build_dir_apps/upg/*.o $line/upg/ -f
				cp $build_dir_apps/tftpu/*.o $line/tftpu/ -f 
				cp $build_dir_apps/pti/*.o $line/pti/ -f  
				cp $build_dir_apps/getsmaps/*.o $line/getsmaps/ -f 
				cp $build_dir_apps/tftpu/*.o $line/tftpu/ -f
				cp $build_dir_apps/udpserver/*.o $line/udpserver/ -f
				cp $build_dir_apps/flash_test/*.o $line/flash_test/ -f
				cp $build_dir_apps/syslog/*.o $line/syslog/ -f
				cp $build_dir_apps/ipt_common/*.o $line/ipt_common/ -f
				cp $build_dir_apps/flash_test/flash_test $line/flash_test/ -f
				cp $build_dir_apps/getpagemap/getpagemap $line/getpagemap/ -f

				grep "CONFIG_APPS_SSAP_UPGRADE=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					cp $build_dir_apps/upg/upgrader $line/upg/ -f
				fi
				
				grep "CONFIG_APPS_SSAP_PLANETTOOLS=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					cp $build_dir_apps/planet_swctrl/swctrl $line/upg/ -f
				fi
				
				grep "CONFIG_APPS_SSAP_TFTPUPG=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					cp $build_dir_apps/tftpu/tftpd $line/tftpu/ -f
				fi

				grep "CONFIG_APPS_SSAP_PROTEST=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					cp $build_dir_apps/pti/protest $line/pti/ -f
				fi

				grep "CONFIG_APPS_SSAP_GETSMAPS=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					cp $build_dir_apps/getsmaps/getsmaps $line/getsmaps/ -f
				fi
				
				grep "CONFIG_APPS_SSAP_BUTTON=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					cp $build_dir_apps/button/button $line/button/ -f
				fi								
				
			fi

			echo $line|grep "\/logic">/dev/null
			if [ $? -eq 0 ];then
				echo "enter logic!"
				cp $build_dir_apps/logic $line/ -f
				cp $MYROOTDIR/build/romfs/rootfs/etc/config_full.xml $line/ -f
				cp $MYROOTDIR/build/romfs/rootfs/etc/config.xml $line/ -f
				cp $MYROOTDIR/build/romfs/rootfs/etc/config_flash.xml $line/ -f
				cp  $build_dir_apps/usb/hotplug/hotplug $line/usb/hotplug/ -f
				grep "CONFIG_APPS_LOGIC_DIAG=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					cp $build_dir_apps/diagnostics/diag $line/diagnostics/ -f
					cp $build_dir_apps/diagnostics/ralink/diagtest $line/diagnostics/ralink/ -f
				fi
				grep "CONFIG_APPS_LOGIC_MULTILANG=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					cp $build_dir_apps/multilanguage/httpget/httpget $line/multilanguage/httpget/ -f
				fi
				grep "CONFIG_APPS_LOGIC_LOGGER=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					cp $build_dir_apps/logger/logmonitor $line/logger/ -f
				fi
		
			fi
			echo $line|grep "\/upgcgi">/dev/null
			if [ $? -eq 0 ];then
				cp $build_dir_apps/webupg $line/ -f
			fi
			
			echo $line|grep "\/ipv6">/dev/null
			if [ $? -eq 0 ];then
				cp $build_dir_apps/ip6aac $line/ -f
				cp $build_dir_apps/ramon $line/ -f
				cp $build_dir_apps/ip6mon $line/ -f
				cp $build_dir_apps/ifip6 $line/ -f
			fi

			echo $line|grep "\/cli">/dev/null
			if [ $? -eq 0 ];then
				grep "CONFIG_APPS_CLI=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					cp $build_dir_apps/cmd $line/ -f
				fi
			fi

		echo $line|grep "\/CTC_UPnP_DM_FE">/dev/null
			if [ $? -eq 0 ];then
				grep "CONFIG_APPS_CTC_UPnP_DM_FE=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					cp $build_dir_apps/cmd $line/ -f
				fi
			fi
		
			echo $line|grep "\/omlib">/dev/null
			if [ $? -eq 0 ];then
				grep "CONFIG_APPS_LIB=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					echo "omlib do nothing!"
				fi
			fi

			echo $line|grep "\/tr069fe">/dev/null
			if [ $? -eq 0 ];then
				grep "CONFIG_APPS_TR069=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					grep "CONFIG_TR069_SSL=y" .config>/dev/null;
					if [ $? -eq 0 ];then
						cp $build_dir_apps/tb_tr069 $line/ -f
						cp $build_dir_apps/verify $line/ -f
					else
						cp $build_dir_apps/tb_tr069 $line/ -f
					fi
				fi
			fi

			echo $line|grep "\/web">/dev/null
			if [ $? -eq 0 ];then
				grep "CONFIG_APPS_WEB=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					cp $build_dir_apps/webproc $line/ -f
					
					cp $MYROOTDIR/build/modlist.h $line/modlist -f
					cp $MYROOTDIR/build/langlist.h $line/langlist -f
					cp $MYROOTDIR/build/lang.h $line/lang -f
				fi
			fi

			echo $line|grep "\/html">/dev/null
			if [ $? -eq 0 ];then
				#html_file=$line/standard
				#if [ -d $html_file ];then
				#    rm $html_file -rf
				#fi
				
				html_file=$line/skins
				if [ -d $html_file ];then
				    cd $html_file
				    ls | grep -v $SkinsStyle| xargs rm -rf
				    cd -
				fi
			fi

			echo $line|grep "\/tm">/dev/null
			if [ $? -eq 0 ];then
				grep "CONFIG_APPS_TM=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					cp $build_dir_apps/tm $line/ -f
				fi
			fi

			echo $line|grep "\/supp">/dev/null
			if [ $? -eq 0 ];then
				grep "CONFIG_APPS_SUPP=y" .config>/dev/null;
				if [ $? -eq 0 ];then
					cp $build_dir_apps/freesupplicant/supp $line/ -f
				fi
			fi

			echo $line|grep "\/test">/dev/null
			if [ $? -eq 0 ];then
				echo "enter test dir and do nothing!"
			fi
			
			echo $line|grep "\/include">/dev/null
			if [ $? -eq 0 ];then
				echo "enter include dir"
				find $line -name '*.h'|xargs rm -rf
			fi

	    find $line -name '*.c'|xargs rm -rf
	    find $line -name '*.h'|xargs rm -rf

	fi
	
    done < $MyPath/release_dir
fi

cp $MyPath/lib/* $MYROOTDIR/src/apps/ssap/lib -f
rm -rf $MyPath/lib
cp $MyPath/cfg/* $MYROOTDIR/src/apps/ssap/cfg -f
rm -rf $MyPath/cfg
cp $MyPath/syslog/* $MYROOTDIR/src/apps/ssap/syslog -f
rm -rf $MyPath/syslog
cp $MyPath/msg/* $MYROOTDIR/src/apps/ssap/msg -f
rm -rf $MyPath/msg
cp $MyPath/include/* $MYROOTDIR/src/apps/include -rf
rm -rf $MyPath/include
cp $MyPath/html/html.c $MYROOTDIR/src/apps/html -f
rm -rf $MyPath/html
cp $MyPath/tbserror/tbserror.c $MYROOTDIR/src/apps/ssap/lib -f
rm -rf $MyPath/tbserror



rm -rf $MyPath/tbsboot

echo "&&&&&&&&&&&&&&&&&&&& deal with apps files OK! &&&&&&&&&&&&&&&&&&&&"

############################################################################################
#   #   #  #   # 	
#    ###    ###  			remove useless vendor source
#      #   #
#       ###
############################################################################################
echo "==================== remove useless vendor source ===================="

grep "CONFIG_VENDOR_ATHERNOS=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/vendor/atheros
fi

grep "CONFIG_VENDOR_INFINEON=y" .config>/dev/null;
if [ $? -ne 0 ];then
  #find ./src/vendor/infineon -name '*.c'|xargs rm -rf
  #find ./src/vendor/infineon -name '*.h'|xargs rm -rf
  #sed -e 'infineon/d' ./src/vendor/Config.in > .tmp
  #mv .tmp ./src/vendor/Config.in
	echo "=============delete /vendor/infineon========================="
  rm -rf $MYROOTDIR/src/vendor/infineon
fi

grep "CONFIG_VENDOR_RALINK=y" .config>/dev/null;
if [ $? -eq 0 ];then
  
sed -i 's%source.*ralink/Config.in$%#source src/vendor/ralink/Config.in%' $MyPath/gpl_config/vendor/Config.in
  echo "=============delete /vendor/ralink========================="
  rm -rf $MYROOTDIR/src/vendor/ralink
fi

grep "CONFIG_VENDOR_IKANOS=y" .config>/dev/null;
if [ $? -ne 0 ];then
  #find ./src/vendor/ikanos -name '*.c'|xargs rm -rf
  #find ./src/vendor/ikanos -name '*.h'|xargs rm -rf
  #sed -e 'ikanos/d' ./src/vendor/Config.in > .tmp
  #mv .tmp ./src/vendor/Config.in
  echo "=============delete /vendor/ikanos========================="
  rm -rf $MYROOTDIR/src/vendor/ikanos
fi

grep "CONFIG_VENDOR_MINDSPEED=y" .config>/dev/null;
if [ $? -ne 0 ];then
  #find ./src/vendor/mindspeed -name '*.c'|xargs rm -rf
  #find ./src/vendor/mindspeed -name '*.h'|xargs rm -rf
  #sed -e '/mindspeed/d' ./src/vendor/Config.in > .tmp
  #mv .tmp ./src/vendor/Config.in
  
   echo "=============delete /vendor/mindspeed========================="
  rm -rf $MYROOTDIR/src/vendor/mindspeed
fi

grep "CONFIG_VENDOR_REALTEK=y" .config>/dev/null;
if [ $? -eq 0 ];then
  #find ./src/vendor/realtek -name '*.c'|xargs rm -rf
  #find ./src/vendor/realtek -name '*.h'|xargs rm -rf
  #sed -e 'realtek/d' ./src/vendor/Config.in > .tmp
  #mv .tmp ./src/vendor/Config.in
  sed -i 's%#source.*realtek/Config.in$%source src/vendor/realtek/Config.in%' $MyPath/gpl_config/vendor/Config.in 
fi
grep "CONFIG_REALTEK_AUTH_APP=y" .config>/dev/null;
if [ $? -eq 0 ];then
	sed -i "s@^include.*Rules.make@install:\\n\\tmake -C \$(subdir-y) install@" $MYROOTDIR/src/vendor/realtek/auth/Makefile 
	cp -f $MYROOTDIR/build/vendor/realtek/auth/src/auth $MYROOTDIR/src/vendor/realtek/auth/ 
	sed -i "s@\(cp -Lrf \)auth\(.*bin/auth\)@\1\$(ROOTDIR)/src/vendor/realtek/auth/auth\2@" $MYROOTDIR/src/vendor/realtek/auth/src/Makefile
fi
grep "CONFIG_REALTEK_FLASH_TOOLS=y" .config>/dev/null;
if [ $? -eq 0 ];then
	sed -i "/obj-y = flash.o/d" $MYROOTDIR/src/vendor/realtek/flash/Makefile
	cp -f $MYROOTDIR/build/vendor/realtek/flash/flash $MYROOTDIR/src/vendor/realtek/flash/
	sed -i "s@\(.*755 \)flash\(.*\)@\1\$(ROOTDIR)/src/vendor/realtek/flash/flash\2@" $MYROOTDIR/src/vendor/realtek/flash/Makefile
fi
grep "CONFIG_REALTEK_WPS_APP=y" .config>/dev/null;
if [ $? -eq 0 ];then
	sed -i "/.*make -C.*/gd" $MYROOTDIR/src/vendor/realtek/wsc/Makefile
	cp -f $MYROOTDIR/build/vendor/realtek/wsc/src/wscd $MYROOTDIR/src/vendor/realtek/wsc/src/
	sed -i "s@\(.*\)src/wscd\(.*\)@\1\$(ROOTDIR)/src/vendor/realtek/wsc/src/wscd\2@" $MYROOTDIR/src/vendor/realtek/wsc/Makefile
	cp -f $MYROOTDIR/build/vendor/realtek/wsc/iwcontrol $MYROOTDIR/src/vendor/realtek/wsc/ 
	sed -i "s@\(.*\)\./iwcontrol\(.*\)@\1\$(ROOTDIR)/src/vendor/realtek/wsc/iwcontrol\2@" $MYROOTDIR/src/vendor/realtek/wsc/Makefile
	#sed -i "/.*mini_upnp$/d" $MYROOTDIR/src/vendor/realtek/Makefile
	sed -i "/endif$/a CONFIG_DRIVERS_WLAN_REALTEK=y" $MYROOTDIR/src/driver/wireless/Makefile
fi
echo "&&&&&&&&&&&&&&&&&&&& remove vendor source OK! &&&&&&&&&&&&&&&&&&&&"

#########################################################################################
#								remove driver source
#
#########################################################################################
echo "==================== remove useless driver source ===================="

grep "CONFIG_DRIVERS_WLAN_ATHEROS=y" .config>/dev/null;
if [ $? -ne 0 ];then
  #find ./src/driver/wireless/atheros -name '*.c'|xargs rm -rf
  #find ./src/driver/wireless/atheros -name '*.h'|xargs rm -rf
  #sed -e 'atheros/d' ./src/driver/wireless/Config.in > .tmp
  #mv .tmp ./src/driver/wireless/Config.in
  rm -rf $MYROOTDIR/src/driver/wireless/atheros
fi

grep "CONFIG_DRIVERS_WLAN_RALINK=y" .config>/dev/null;
if [ $? -ne 0 ];then
  #find ./src/driver/wireless/ralink -name '*.c'|xargs rm -rf
  #find ./src/driver/wireless/ralink -name '*.h'|xargs rm -rf
  #sed -e 'ralink/d' ./src/driver/wireless/Config.in > .tmp
  #mv .tmp ./src/driver/wireless/Config.in
  rm -rf $MYROOTDIR/src/driver/wireless/ralink
fi

grep "CONFIG_DRIVERS_WLAN_REALTEK=y" .config>/dev/null;
if [ $? -ne 0 ];then
  #find ./src/driver/wireless/realtek -name '*.c'|xargs rm -rf
  #find ./src/driver/wireless/realtek -name '*.h'|xargs rm -rf
  #sed -e 'realtek/d' ./src/driver/wireless/Config.in > .tmp
  #mv .tmp ./src/driver/wireless/Config.in
  rm -rf $MYROOTDIR/src/driver/wireless/realtek
fi

grep "CONFIG_DRIVERS_DSL=y" .config>/dev/null;
if [ $? -ne 0 ];then
  #rm -rf ./src/driver/dsl/infineon
  #rm -rf ./src/driver/dsl/ikanos
  #find ./src/driver/dsl/infineon -name '*.c'|xargs rm -rf
  #find ./src/driver/dsl/infineon -name '*.h'|xargs rm -rf
  #find ./src/driver/dsl/ikanos -name '*.c'|xargs rm -rf
  #find ./src/driver/dsl/ikanos -name '*.h'|xargs rm -rf
  rm -rf $MYROOTDIR/src/driver/dsl
fi

echo "&&&&&&&&&&&&&&&&&&&& remove driver source OK! &&&&&&&&&&&&&&&&&&&&"

############################################################################################
#								remove product dir and files
#
############################################################################################
echo "==================== remove useless product dir ===================="

Product=`awk -F= '{printf $2}' $MYROOTDIR/build/.product`
cd product
ls | grep -v $Product | xargs rm -rf
cd ..

echo "&&&&&&&&&&&&&&&&&&&& remove product dir OK! &&&&&&&&&&&&&&&&&&&&"

##########################################################################################
#	  modify kernel Makefile opensource Makefile apps Makefile and fastpath Makefile
#
##########################################################################################
echo "==================== modify Makefile ===================="

#kernel
#cp $MyPath/gpl_makefile/kernel/Makefile $MYROOTDIR/src/kernel -f

#apps
cp $MyPath/gpl_makefile/apps/Makefile $MYROOTDIR/src/apps -f
sed -i "/libssap\.so$/d" $MYROOTDIR/src/apps/Makefile
sed -i '/.*getpagemap.*$/d' $MYROOTDIR/src/apps/Makefile
echo  -e "ifeq ("\$\(CONFIG_APPS_LOGIC_DIAG\)","y")
\tcp \$(ROOTDIR)/src/apps/logic/diagnostics/diag \$(FSROOT)/sbin/diag
\tcp \$(ROOTDIR)/src/apps/logic/diagnostics/ralink/diagtest \$(FSROOT)/sbin/diagtest
\t\$(STRIP) \$(FSROOT)/sbin/diagtest
endif
ifeq ("\$\(CONFIG_APPS_LOGIC_MULTILANG\)","y")
\tcp \$(ROOTDIR)/src/apps/logic/multilanguage/httpget/httpget \$(FSROOT)/sbin/httpget
\t\$(STRIP) \$(FSROOT)/sbin/httpget
endif
\tcp \$(ROOTDIR)/src/apps/logic/usb/hotplug/hotplug \$(FSROOT)/sbin/hotplug
\t\$(STRIP) \$(FSROOT)/sbin/hotplug
ifeq ("\$\(CONFIG_APPS_LOGIC_LOGGER\)","y")
\tcp \$(ROOTDIR)/src/apps/logic/logger/logmonitor \$(FSROOT)/usr/bin/logmonitor
\t\$(STRIP) \$(FSROOT)/usr/bin/logmonitor
endif
ifeq ("\$\(CONFIG_APPS_IPV6_IFIPV6\)","y")
\t\$(Q)cp \$(ROOTDIR)/src/apps/ipv6/ifip6 \$(FSROOT)/usr/bin/ 
\t\$(STRIP) \$(FSROOT)/usr/bin/ifip6
endif
"  >> $MYROOTDIR/src/apps/Makefile

#opensource
	rm -rf	$MYROOTDIR/src/opensource/utmproxy/Makefile
	rm -rf	$MYROOTDIR/src/opensource/autoFWupgrade/Makefile
sed -i "s/all:.*configure exe/all:exe/" $MYROOTDIR/build/opensource/openssl/openssl-1.0.1p/apps/Makefile 

#kernel
cp -f $MYROOTDIR/src/kernel/linux-2.6.30/Makefile_bk $MYROOTDIR/src/kernel/linux-2.6.30/Makefile
cp $MyPath/gpl_makefile/opensource/Makefile_nbns $MYROOTDIR/src/opensource/nbns/Makefile -f
sed -i "s@^all.*@& install@" $MYROOTDIR/src/opensource/nbns/Makefile
cp $MyPath/gpl_makefile/opensource/Makefile_nbsmtp $MYROOTDIR/src/opensource/nbsmtp/Makefile   -f
sed -i "s@^all.*@& install@" $MYROOTDIR/src/opensource/nbsmtp/Makefile 

sed -i "/^\.PHONY/a CONFIG_OPENSOURCE_SAMBA=y" $MYROOTDIR/src/filesystem/basefilesystem/Makefile
rm -rf $MYROOTDIR/src/opensource/samba/samba-1.9.18p8
rm -rf $MYROOTDIR/src/opensource/samba/samba-3.027
cp $MyPath/gpl_makefile/opensource/Makefile_siproxd $MYROOTDIR/src/opensource/siproxd/Makefile   -f
cp $MyPath/gpl_makefile/opensource/httpd/Makefile $MYROOTDIR/src/opensource/httpd -f
cp $MyPath/gpl_makefile/opensource/dproxy/Makefile $MYROOTDIR/src/opensource/dproxy -f
cp $MyPath/gpl_makefile/opensource/dproxy/Makefile $MYROOTDIR/src/opensource/dproxy -f
cp $MyPath/gpl_makefile/opensource/dproxy/Makefile $MYROOTDIR/src/opensource/dproxy -f
cp $MyPath/gpl_makefile/opensource/utmproxy/Makefile	$MYROOTDIR/src/opensource/utmproxy -f
cp $MyPath/gpl_makefile/opensource/autoFWupgrade/Makefile	$MYROOTDIR/src/opensource/autoFWupgrade -f 
#cp $MyPath/gpl_makefile/kernel/linux-2.6.36.x/MT7620A/Makefile	src/kernel/linux-2.6.36.x/	-f
#rm -rf $MYROOTDIR/src/kernel/linux-2.6.36.x/drivers/net/wireless/MT7620A/rt2860v2_ap/Makefile
#cp $MyPath/gpl_makefile/kernel/linux-2.6.36.x/wireless/Makefile $MYROOTDIR/src/kernel/linux-2.6.36.x/drivers/net/wireless/MT7620A/rt2860v2_ap/Makefile -f
#rm -rf $MYROOTDIR/src/kernel/linux-2.6.36.x/drivers/net/wireless/MT7612E/rlt_wifi_ap/Makefile
#cp $MyPath/gpl_makefile/kernel/linux-2.6.36.x/MT7612E/Makefile $MYROOTDIR/src/kernel/linux-2.6.36.x/drivers/net/wireless/MT7612E/rlt_wifi_ap/Makefile -f



#tools
cp $MyPath/gpl_makefile/tools/mkbuilddir $MYROOTDIR/tools -f

#opensource
cp $MyPath/gpl_makefile/opensource/Makefile $MYROOTDIR/src/opensource -f

echo "&&&&&&&&&&&&&&&&&&&& modify Makefile OK! &&&&&&&&&&&&&&&&&&&&"

sed -i "/\.config$/i CONFIG_OPENSOURCE_NTFS=y" $MYROOTDIR/src/opensource/Makefile
sed -i "5a CONFIG_OPENSOURCE_SAMBA=y" $MYROOTDIR/src/opensource/Makefile
############################################################################################
#										modify config.in
#
############################################################################################
echo "==================== modify config.in ===================="

#verdor
cp $MyPath/gpl_config/vendor/Config.in $MYROOTDIR/src/vendor -f

#driver
cp $MyPath/gpl_config/driver/Config.in $MYROOTDIR/src/driver -f
#cp $MyPath/gpl_config/driver/wireless/Config.in $MYROOTDIR/src/driver/wireless -f

#board
cp $MyPath/gpl_config/board/BoardConfig.in $MYROOTDIR/src -f

#opensource
cp $MyPath/gpl_config/opensource/Config.in $MYROOTDIR/src/opensource -f
sed -i 's%^source.*atm.*%#&%' $MYROOTDIR/src/opensource/Config.in
sed -i 's%^source.*snmp.*%#&%' $MYROOTDIR/src/opensource/Config.in
sed -i 's%^source.*ping.*%#&%' $MYROOTDIR/src/opensource/Config.in
sed -i 's%^source.*bftpd.*%#&%' $MYROOTDIR/src/opensource/Config.in
sed -i 's%^source.*ftpd.*%#&%' $MYROOTDIR/src/opensource/Config.in
sed -i 's%^source.*ippd.*%#&%' $MYROOTDIR/src/opensource/Config.in
sed -i 's%^source.*samba.*%#&%' $MYROOTDIR/src/opensource/Config.in
sed -i 's%^source.*hostapd-0.*%#&%' $MYROOTDIR/src/opensource/Config.in
sed -i 's%^source.*wpa_supplicant.*%#&%' $MYROOTDIR/src/opensource/Config.in
sed -i 's%^source.*strace.*%#&%' $MYROOTDIR/src/opensource/Config.in
sed -i 's%^source.*tcpdump.*%#&%' $MYROOTDIR/src/opensource/Config.in
sed -i 's%^source.*ttcp.*%#&%' $MYROOTDIR/src/opensource/Config.in
sed -i 's%^source.*gdb.*%#&%' $MYROOTDIR/src/opensource/Config.in
sed -i '67a subdir-${CONFIG_OPENSOURCE_NBNS} += nbns' $MYROOTDIR/src/opensource/Makefile
sed -i '67i subdir-${CONFIG_OPENSOURCE_NBSMTP} += nbsmtp' $MYROOTDIR/src/opensource/Makefile

#kernel
cp $MyPath/gpl_config/kernel/wireless/Kconfig_bak $MYROOTDIR/src/kernel/linux-2.6.30/drivers/net/wireless -f
#sed -i "/^install/i all:install" $MYROOTDIR/src/driver/wireless/realtek/Makefile
sed -i "/^include.*Rules.make/a all:\\n\\tmake -C realtek install" $MYROOTDIR/src/driver/wireless/Makefile
echo "&&&&&&&&&&&&&&&&&&&& modify config.in OK! &&&&&&&&&&&&&&&&&&&&"
#delete tar pack
rm -rf $MYROOTDIR/src/opensource/samba/samba-3.02_new/packaging/SuSE/samba-vscan-0.3.2b.tar.bz2
rm -rf $MYROOTDIR/src/opensource/samba/samba-3.02_new/packaging/SuSE/samba-3.0.0.files.tar.bz2
rm -rf $MYROOTDIR/src/kernel/linux-2.6.30/rtk_voip-sdk/voip_dsp/ivr/make_speech.tar.gz
rm -rf $MYROOTDIR/src/opensource/samba/samba-3.02_new/packaging/Example/skeleton.tar
rm -rf $MYROOTDIR/src/opensource/openssl/openssl-1.0.1p.tar.gz
cp -rf $MYROOTDIR/build/opensource/openssl/openssl-1.0.1p $MYROOTDIR/src/opensource/openssl/openssl-1.0.1p
sed -i '/.*\.\/Configure/i \\ttouch Makefile \\' $MYROOTDIR/src/opensource/openssl/Makefile.101
sed -i '/.*\.\/Configure/i \\ttouch Makefile' $MYROOTDIR/src/opensource/libnetfilter_conntrack/Makefile

rm -rf $MYROOTDIR/src/opensource/openssl/openssl-0.9.7m.tar.bz2
rm -rf $MYROOTDIR/src/opensource/openssl/openssl-0.9.8h.tar.gz
rm -rf $MYROOTDIR/src/opensource/openssl/Makefile.098
rm -rf $MYROOTDIR/src/opensource/openssl/Makefile.097

rm -rf $MYROOTDIR/src/opensource/libpng/libpng-1.2.44.tar.bz2
cp -rf $MYROOTDIR/build/opensource/libpng/libpng-1.2.44 $MYROOTDIR/src/opensource/libpng/libpng-1.2.44
rm -rf $MYROOTDIR/src/opensource/zebra/zebra-0.95a.tar.gz
 
rm -rf $MYROOTDIR/src/opensource/libnetfilter_conntrack/libnetfilter_conntrack-0.0.89.tar.bz2
cp -rf $MYROOTDIR/build/opensource/libnetfilter_conntrack/libnetfilter_conntrack-0.0.89 $MYROOTDIR/src/opensource/libnetfilter_conntrack/libnetfilter_conntrack-0.0.89
rm -rf $MYROOTDIR/src/opensource/libnfnetlink/libnfnetlink-0.0.33.tar.bz2
cp -rf $MYROOTDIR/build/opensource/libnfnetlink/libnfnetlink-0.0.33 $MYROOTDIR/src/opensource/libnfnetlink/libnfnetlink-0.0.33
rm -rf $MYROOTDIR/src/opensource/qrencode/qrencode-3.1.1.tar.bz2
cp -rf $MYROOTDIR/build/opensource/qrencode/qrencode-3.1.1 $MYROOTDIR/src/opensource/qrencode/qrencode-3.1.1
rm -rf $MYROOTDIR/src/opensource/dropbear/dropbear-0.51.tar.bz2
cp -rf $MYROOTDIR/build/opensource/dropbear/dropbear-0.51 $MYROOTDIR/src/opensource/dropbear/dropbear-0.51
rm -rf $MYROOTDIR/src/opensource/busybox/busybox-1.6.1/shell/susv3_doc.tar.bz2 


rm -rf $MYROOTDIR/src/opensource/dlna/dlna_bcm/libs/ffmpeg-0.5.tar.bz2
cp -rf $MYROOTDIR/build/opensource/dlna/dlna_bcm/libs/ffmpeg-0.5  $MYROOTDIR/src/opensource/dlna/dlna_bcm/libs/ffmpeg-0.5
rm -rf $MYROOTDIR/src/opensource/dlna/dlna_usb/libs/ffmpeg-0.5.tar.bz2
cp -rf $MYROOTDIR/build/opensource/dlna/dlna_bcm/libs/ffmpeg-0.5  $MYROOTDIR/src/opensource/dlna/dlna_usb/libs/ffmpeg-0.5
sed -i 's@\(.*\[AM_AUTOMAKE_VERSION(\[1.7\).9\(\])\])\)@\1\2@' $MYROOTDIR/src/opensource/libnetfilter_conntrack/libnetfilter_conntrack-0.0.89/aclocal.m4

############################################################################################
#									modify *.c files
#
############################################################################################
echo "==================== modify *.c files ===================="

cp $MyPath/gpl_.c/scripts/mconf.c $MYROOTDIR/scripts -f

echo "&&&&&&&&&&&&&&&&&&&& modify *.c files OK! &&&&&&&&&&&&&&&&&&&&"

############################################################################################
#									remove useless dirs and files
#
############################################################################################
echo "==================== remove useless dirs ===================="

#test
rm -rf $MYROOTDIR/test

#VTP_proj
grep "CONFIG_TBS_SUPPORT_VOIP=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/VTP_proj
fi

#SDK_Gpl_patch
rm -rf $MYROOTDIR/SDK_GPL_patch



#html
rm -rf $MYROOTDIR/src/apps/html/standard/*
mkdir $MYROOTDIR/src/apps/html/standard/languages
mkdir $MYROOTDIR/src/apps/html/standard/languages/en_us
mkdir $MYROOTDIR/src/apps/html/standard/languages/zh_cn
cp $MyPath/en_us/error.js $MYROOTDIR/src/apps/html/standard/languages/en_us -f
cp $MyPath/zh_cn/error.js $MYROOTDIR/src/apps/html/standard/languages/zh_cn -f
rm -rf $MyPath/en_us
rm -rf $MyPath/zh_cn

echo "&&&&&&&&&&&&&&&&&&&& remove useless dirs OK! &&&&&&&&&&&&&&&&&&&&"

############################################################################################
#									deal with menuconfig
#
############################################################################################

#rm -rf $MYROOTDIR/scripts/conf
#rm -rf $MYROOTDIR/scripts/mconf
#rm -rf $MYROOTDIR/scripts/lxdialog/lxdialog

############################################################################################
#								remove useless opensource code
#
############################################################################################
echo "==================== remove useless opensource code ===================="

grep "CONFIG_OPENSOURCE_OPENSSL=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/openssl
fi

grep "CONFIG_OPENSOURCE_ATED=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/ated
fi

grep "CONFIG_OPENSOURCE_BUSYBOX=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/busybox
fi

grep "CONFIG_OPENSOURCE_BRCTL=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/brctl
fi

grep "CONFIG_OPENSOURCE_NBTSCAN=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/nbtscan
fi

grep "CONFIG_OPENSOURCE_HTTPD=y" .config>/dev/null;
if [ $? -eq 0 ];then
  sed -i "s/all:build/& install/" $MYROOTDIR/src/opensource/httpd/Makefile
  sed -i "/.*cp -f.*sbin/a \\\tcp -f \$(SRCDIR)\/mini_httpd\.cnf \$(FSROOT)\/etc" $MYROOTDIR/src/opensource/httpd/Makefile
fi

grep "CONFIG_OPENSOURCE_PPPD=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/ppp-pppoe
fi

grep "CONFIG_OPENSOURCE_DPROXY=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/dproxy
fi

grep "CONFIG_OPENSOURCE_ETHTOOL=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/ethtool
fi

grep "CONFIG_OPENSOURCE_UDHCP=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/udhcp
fi

grep "CONFIG_OPENSOURCE_RADVD=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/radvd
fi

grep "CONFIG_OPENSOURCE_NBSMTP=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/nbsmtp
fi

grep "CONFIG_OPENSOURCE_INADYN=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/inadynd
fi

grep "CONFIG_OPENSOURCE_DHCPV6=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/dhcpv6
fi

grep "CONFIG_OPENSOURCE_DHCPR=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/dhcpr
fi

grep "CONFIG_OPENSOURCE_LIBNFNETLINK=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/libnfnetlink
fi

grep "CONFIG_OPENSOURCE_LIBNETFILTER_CONNTRACK=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/libnetfilter_conntrack
fi



grep "CONFIG_OPENSOURCE_PPPDS=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/ppps
fi

grep "CONFIG_OPENSOURCE_ATM=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/atm
fi

grep "CONFIG_OPENSOURCE_GDB=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/gdb
fi

grep "CONFIG_OPENSOURCE_UPDATEDD=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/updatedd
fi

grep "CONFIG_OPENSOURCE_EBTABLES=y" .config>/dev/null;
if [ $? -ne 0 ];then
  sed -i 's%source src/opensource/ebtables/Config.in%#source src/opensource/ebtables/Config.in%g' $MYROOTDIR/src/opensource/Config.in
  rm -rf $MYROOTDIR/src/opensource/ebtables
fi

grep "CONFIG_OPENSOURCE_IGMPPROXY=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/igmp-proxy
fi

grep "CONFIG_OPENSOURCE_IGMPPROXYV3=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/igmpproxy
fi

grep "CONFIG_OPENSOURCE_L2TPD=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/l2tpd
fi

grep "CONFIG_OPENSOURCE_L2TPD=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/openl2tp
fi

grep "CONFIG_OPENSOURCE_PPTP=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/pptp
fi

grep "CONFIG_OPENSOURCE_SNMPA=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/snmp
fi

grep "CONFIG_OPENSOURCE_PING=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/ping
fi

grep "CONFIG_OPENSOURCE_IPROUTE2=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/iproute2
fi

grep "CONFIG_OPENSOURCE_ZEBRA=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/zebra
fi


grep "CONFIG_OPENSOURCE_MSNTP=y" .config>/dev/null;
if [ $? -eq 0 ];then
	sed -i "/msntp$/d" $MYROOTDIR/src/opensource/Makefile
fi

#grep "CONFIG_OPENSOURCE_DROPBEAR=y" .config>/dev/null;
#if [ $? -ne 0 ];then
  #rm -rf $MYROOTDIR/src/opensource/dropbear
#fi

grep "CONFIG_OPENSOURCE_BFTPD=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/bftpd
fi


grep "CONFIG_OPENSOURCE_FTPD=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/ftpd
fi

grep "CONFIG_OPENSOURCE_UPnP=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/upnp
fi

sed -i "/\.config$/CONFIG_OPENSOURCE_SAMBA=y" $MYROOTDIR/src/opensource/Makefile 
sed -i "/^include.*Rules.make/i subdir-y +=ntpclient" $MYROOTDIR/src/opensource/Makefile
grep "CONFIG_OPENSOURCE_WGET=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/wget
fi

grep "CONFIG_OPENSOURCE_TCPDUMP=y" .config>/dev/null;
if [ $? -ne 0 ];then
  sed -i 's%source src/opensource/tcpdump/Config.in%#source src/opensource/tcpdump/Config.in%g' $MYROOTDIR/src/opensource/Config.in
  rm -rf $MYROOTDIR/src/opensource/tcpdump
fi

grep "CONFIG_OPENSOURCE_TTCP=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/ttcp
fi

grep "CONFIG_OPENSOURCE_HOSTAPD=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/hostapd-0.4.8
fi

grep "CONFIG_OPENSOURCE_SUPPLICANT=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/wpa_supplicant-0.5.5
fi

grep "CONFIG_OPENSOURCE_WIRELESS_TOOLS=y" .config>/dev/null;
rm -rf $MYROOTDIR/src/opensource/wireless_tools.28
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/wireless_tools
fi

grep "CONFIG_OPENSOURCE_IPPD=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/ippd
fi

grep "CONFIG_OPENSOURCE_LIB_USB=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/libusb
fi

grep "CONFIG_OPENSOURCE_USB_MODE_SWITCH=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/usb_modeswitch
fi

grep "CONFIG_OPENSOURCE_ARGP_STANDALONE=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/argp-standalone
fi

grep "CONFIG_OPENSOURCE_SIPROXD=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/libosip2
fi

grep "CONFIG_OPENSOURCE_PEANUTHULL=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/peanuthull
fi

grep "CONFIG_OPENSOURCE_PEANUTHULL2=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/peanuthull2
fi

grep "CONFIG_TBS_STRACE_DEBUG=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/strace
fi

grep "CONFIG_OPENSOURCE_UPDATEDD_PLANET=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/updatedd_planet
fi

#grep "CONFIG_APPS_LOGIC_WAN_BIND=y" .config>/dev/null;
#if [ $? -ne 0 ];then
#  rm -rf $MYROOTDIR/src/opensource/dproxy/dproxy-e8
#  rm -rf $MYROOTDIR/src/opensource/igmpproxy/igmpproxy-e8
#fi
cp $MYROOTDIR/build/opensource/nbns/nbnslisten $MYROOTDIR/src/opensource/nbns/ -f
sed -i "/\.config$/i CONFIG_OPENSOURCE_NBNS=y" $MYROOTDIR/src/opensource/Makefile
cp $MYROOTDIR/build/opensource/nbsmtp/nbsmtp-1.00/nbsmtp $MYROOTDIR/src/opensource/nbsmtp/ -f
sed -i "/\.config$/i CONFIG_OPENSOURCE_NBSMTP=y" $MYROOTDIR/src/opensource/Makefile


grep "CONFIG_OPENSOURCE_SIPROXD=y" .config>/dev/null;
if [ $? -eq 0 ];then
	cp $MYROOTDIR/build/opensource/siproxd/siproxd-0.5.1/src/siproxd $MYROOTDIR/src/opensource/siproxd/ -f
fi

grep "CONFIG_OPENSOURCE_NBNS_LISTEN=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf $MYROOTDIR/src/opensource/nbns_listen
fi

grep "CONFIG_IPTABLES_1_4_4=y" .config>/dev/null;
if [ $? -ne 0 ];then
#rm -rf $MYROOTDIR/src/opensource/iptables/iptables-1.4.4
#rm -rf $MYROOTDIR/src/opensource/iptables/Makefile_1.4.4
rm -rf $MYROOTDIR/src/opensource/iptables/iptables-1.4.4.tar.bz2
fi

grep "CONFIG_IPTABLES_1_3_8=y" .config>/dev/null;
if [ $? -ne 0 ];then
#rm -rf $MYROOTDIR/src/opensource/iptables/iptables-1.3.8
#rm -rf $MYROOTDIR/src/opensource/iptables/iptables-1.3.8
rm -rf $MYROOTDIR/src/opensource/iptables/iptables-1.3.8.tar.bz2
fi

grep "CONFIG_IPTABLES_1_4_6=y" .config>/dev/null;
if [ $? -ne 0 ];then
#rm -rf $MYROOTDIR/src/opensource/iptables/iptables-1.4.6
#rm -rf $MYROOTDIR/src/opensource/iptables/iptables-1.4.6
rm -rf $MYROOTDIR/src/opensource/iptables/iptables-1.4.6.tar.bz2
fi

#rm -rf $MYROOTDIR/src/opensource/libosip2/libosip2-3.2.0

rm -rf $MYROOTDIR/src/opensource/siproxd/siproxd-0.7.2


echo "&&&&&&&&&&&&&&&&&&&& remove useless opensource OK! &&&&&&&&&&&&&&&&&&&&"

############################################################################################
#								copy the INSTALL files
#
############################################################################################

cp $MyPath/gpl_install/top-level/INSTALL $MYROOTDIR -f
cp $MyPath/gpl_install/top-level/BT-Repeater_Information.xls $MYROOTDIR -f
cp $MyPath/tarProduct ../ -f
cp $MyPath/gpl_install/toolchain/README ../ -f


############################################################################################
#								remove build dir and .svn files
#
############################################################################################
echo "==================== remove build dir and .svn files ===================="

rm -rf build build.* arch.mk;
rm -f .config
find ./ -name '*.svn'|xargs rm -rf

echo "&&&&&&&&&&&&&&&&&&&& remove build dir and .svn files OK! &&&&&&&&&&&&&&&&&&&&"

