#!/bin/bash
#The file use for sitecom Product GPL
#Date: 2014/07/24
#Written by zym
if [ $# -ne 0 ]
then
 echo "========================================"
 echo "ERROR: This script not need parameter. For example:"
 echo "$0"
 echo "========================================"
 exit;
fi

MyPath=`pwd`
IsKernelHide=0

#####################################################
#	please define the Global var here
#####################################################
#SkinsStyle="Sitecom"


######################################################
#    run the follow should make tools && make prepare
#                compile all code
######################################################
#make tools && \
#make prepare && \

Product=`awk -F= '{printf $2}' ./build/.product`
cp ./product/$Product/app_btn_cfg.c ./src/apps/ssap/button -rf

######################################################
#	     make application layer file
######################################################
#cp ./src/apps/ssap/lib/tbserror.c .tbserror.c
#cp ./src/apps/ssap/cfg cfg -rf
#cp ./src/apps/ssap/syslog syslog -rf
#cp ./src/apps/ssap/msg msg -rf
#cp ./src/apps/ssap/lib/tbsutil.c .tbsutil.c


#######################################################
#		deal with kernel files
#
#######################################################
if [ -f $MyPath/release_dir.txt ]
then
  while read line
  do
    echo $line|grep "\/kernel">/dev/null
        if [ $? -ne 0 ];then

	   echo $line|grep "\/boot">/dev/null
	   if [ $? -ne 0 ];then
	     find $line -name '*.d'|xargs rm -rf
	     find $line -name '*.flag'|xargs rm -rf
	   fi
	else
	   build_dir=`echo $line|sed 's/src\//build\//g'`
		for file in `find ${build_dir} -name *.o`;
		do
		    src_lib_file=`echo $file|sed 's/build\//src\//g'|sed 's/\.o$/.lib/g'`
		    cp $file ${src_lib_file}
		    
		    src_makefile=`echo ${src_lib_file}|sed -n 's:/[^/]*$::p'|sed 's/$/\/Makefile/g'`
		    sed 's/\.o/.lib/g' ${src_makefile} > .tmp
		    mv .tmp ${src_makefile}
		done

		for file in `find ${build_dir} -name *.cmd`;
		do
		    src_cmd_file=`echo $file|sed 's/build\//src\//g'`
		    cp $file ${src_cmd_file}
		done
		   
		find $line -name '*.c'|xargs rm -rf
                find $line -name '*.h'|xargs rm -rf
		IsKernelHide=1

	fi 
  done <$MyPath/release_dir.txt
fi

######################################################################
#		deal with uboot files
#
######################################################################
if [ -f $MyPath/release_dir.txt ]
then
  while read line
  do
      echo $line|grep "\/boot">/dev/null
      if [ $? -ne 0 ];then
         echo "Enter none boot content"
      else
	 build_boot_dir=`echo $line|sed 's/src\//build\//g'`
	 make boot
	 build_boot_build_dir=$build_boot_dir/build
	 
	 for file in `find ${build_boot_build_dir} -name *.o`;
         do
	    src_boot_file=`echo $file|sed 's/build\//src\//'|sed 's/build\//tbsboot\//'`
	    cp $file ${src_boot_file}
	 done
	
         for file in `find ${build_boot_build_dir} -name *.a`;
         do
            src_boot_file=`echo $file|sed 's/build\//src\//'|sed 's/build\//tbsboot\//'`
            cp $file ${src_boot_file}
         done
	 
	 src_boot_dir=$line/tbsboot
	 find $src_boot_dir -name '*.c'|xargs rm -rf
	 find $src_boot_dir -name '*.h'|xargs rm -rf
	 find $src_boot_dir -name '*.S'|xargs rm -rf
	
      fi
  done <$MyPath/release_dir.txt
fi



#######################################################################
#		deal with apps files
#delete the code file that not in the kernel path and had contant of in the release_dir.txt
#######################################################################
if [ -f $MyPath/release_dir.txt ]
then
    while read line
    do
	echo $line|grep "\/kernel">/dev/null
	
	if [ $? -ne 0 ];then
	    echo $line|grep "\/html">/dev/null
	    
	    if [ $? -ne 0 ];then
		echo $line|grep "\/include">/dev/null

		if [ $? -ne 0 ];then
			echo $line|grep "\/boot">/dev/null

			if [ $? -ne 0 ];then
			    build_dir_two=`echo $line|sed 's/src\//build\//g'`
            		    make -C $build_dir_two ROOTDIR=`pwd` BUILDDIR=`pwd`/build

            		    for file in `find ${build_dir_two} -name *.o`;
                	    do
                    	        src_o_file=`echo $file|sed 's/build\//src\//g'`
                    	        cp $file ${src_o_file}
                	    done
		
            		    find $line -name '*.c'|xargs rm -rf
            		    find $line -name '*.h'|xargs rm -rf
			fi
		else
			find $line -name '*.h'|xargs rm -rf
		fi
	    else
		html_file=$line/standard
		if [ -d $html_file ];then
		    rm $html_file -rf
		fi
		
		html_file=$line/skins
		if [ -d $html_file ];then
		    cd $html_file
		    #ls | grep -v $SkinsStyle| xargs rm -rf
		    ls | xargs rm -rf
		    cd -
		fi
	    fi
	fi
    done < $MyPath/release_dir.txt
fi

#mv .tbserror.c ./src/apps/ssap/lib/tbserror.c
#cp cfg/* ./src/apps/ssap/cfg
#rm -rf cfg
#cp syslog/* ./src/apps/ssap/syslog
#rm -rf syslog
#cp msg/* ./src/apps/ssap/msg
#rm -rf msg
#mv .tbsutil.c ./src/apps/ssap/lib/tbsutil.c

##############################################################
#   #   #  #   # compile all code
#    ###    ###  remove vendor source
#      #   #
#       ###
##############################################################

grep "CONFIG_VENDOR_ATHERNOS=y" .config>/dev/null;
if [ $? -ne 0 ];then
  find ./src/vendor/atheros -name '*.c'|xargs rm -rf
  find ./src/vendor/atheros -name '*.h'|xargs rm -rf
  sed -e 'atheros/d' ./src/vendor/Config.in > .tmp
  mv .tmp ./src/vendor/Config.in
fi

grep "CONFIG_VENDOR_INFINEON=y" .config>/dev/null;
if [ $? -ne 0 ];then
  find ./src/vendor/infineon -name '*.c'|xargs rm -rf
  find ./src/vendor/infineon -name '*.h'|xargs rm -rf
  sed -e 'infineon/d' ./src/vendor/Config.in > .tmp
  mv .tmp ./src/vendor/Config.in
fi

grep "CONFIG_VENDOR_RALINK=y" .config>/dev/null;
if [ $? -ne 0 ];then
  find ./src/vendor/ralink -name '*.c'|xargs rm -rf
  find ./src/vendor/ralink -name '*.h'|xargs rm -rf
  sed -e 'ralink/d' ./src/vendor/Config.in > .tmp
  mv .tmp ./src/vendor/Config.in
fi

grep "CONFIG_VENDOR_IKANOS=y" .config>/dev/null;
if [ $? -ne 0 ];then
  find ./src/vendor/ikanos -name '*.c'|xargs rm -rf
  find ./src/vendor/ikanos -name '*.h'|xargs rm -rf
  sed -e 'ikanos/d' ./src/vendor/Config.in > .tmp
  mv .tmp ./src/vendor/Config.in
fi

grep "CONFIG_VENDOR_MINDSPEED=y" .config>/dev/null;
if [ $? -ne 0 ];then
  find ./src/vendor/mindspeed -name '*.c'|xargs rm -rf
  find ./src/vendor/mindspeed -name '*.h'|xargs rm -rf
  sed -e '/mindspeed/d' ./src/vendor/Config.in > .tmp
  mv .tmp ./src/vendor/Config.in
fi

grep "CONFIG_VENDOR_REALTEK=y" .config>/dev/null;
if [ $? -ne 0 ];then
  find ./src/vendor/realtek -name '*.c'|xargs rm -rf
  find ./src/vendor/realtek -name '*.h'|xargs rm -rf
  sed -e 'realtek/d' ./src/vendor/Config.in > .tmp
  mv .tmp ./src/vendor/Config.in
fi

###########################################################
#		remove driver source
#
###########################################################

grep "CONFIG_DRIVERS_WLAN_ATHEROS=y" .config>/dev/null;
if [ $? -ne 0 ];then
  find ./src/driver/wireless/atheros -name '*.c'|xargs rm -rf
  find ./src/driver/wireless/atheros -name '*.h'|xargs rm -rf
  sed -e 'atheros/d' ./src/driver/wireless/Config.in > .tmp
  mv .tmp ./src/driver/wireless/Config.in
fi

grep "CONFIG_DRIVERS_WLAN_RALINK=y" .config>/dev/null;
if [ $? -ne 0 ];then
  find ./src/driver/wireless/ralink -name '*.c'|xargs rm -rf
  find ./src/driver/wireless/ralink -name '*.h'|xargs rm -rf
  sed -e 'ralink/d' ./src/driver/wireless/Config.in > .tmp
  mv .tmp ./src/driver/wireless/Config.in
fi

grep "CONFIG_DRIVERS_WLAN_REALTEK=y" .config>/dev/null;
if [ $? -ne 0 ];then
  find ./src/driver/wireless/realtek -name '*.c'|xargs rm -rf
  find ./src/driver/wireless/realtek -name '*.h'|xargs rm -rf
  sed -e 'realtek/d' ./src/driver/wireless/Config.in > .tmp
  mv .tmp ./src/driver/wireless/Config.in
fi

grep "CONFIG_DRIVERS_DSL=y" .config>/dev/null;
if [ $? -ne 0 ];then
  rm -rf ./src/driver/dsl/infineon
  rm -rf ./src/driver/dsl/ikanos
  find ./src/driver/dsl/infineon -name '*.c'|xargs rm -rf
  find ./src/driver/dsl/infineon -name '*.h'|xargs rm -rf
  find ./src/driver/dsl/ikanos -name '*.c'|xargs rm -rf
  find ./src/driver/dsl/ikanos -name '*.h'|xargs rm -rf
fi


##############################################################
#		remove product dir
#
##############################################################
Product=`awk -F= '{printf $2}' ./build/.product`
cd product
ls | grep -v $Product | xargs rm -rf
cd ..

##############################################################
#		remove build dir
#
##############################################################
rm -rf build build.* arch.mk;
rm -f .config
find ./ -name '*.svn'|xargs rm -rf
