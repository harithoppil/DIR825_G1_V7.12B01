#!/bin/bash
#Date: 2014/07/24
#Written by zym
#echo "==================== deal with kernel files ===================="
##Product=`awk -F= '{printf $2}' $MYROOTDIR/build/.product`
##if [$Product -eq "MTK7620N_N300_SitCOM"]; then
#rm -rf $MYROOTDIR/src/kernel/linux-2.6.36.x/drivers/net/wireless/MT7610
#rm -rf $MYROOTDIR/src/kernel/linux-2.6.36.x/drivers/net/wireless/MT7612E
##fi
#rm -rf $MYROOTDIR/src/kernel/linux-2.6.36.x/drivers/net/wireless/rtl818x
#
#if [ -f $MyPath/release_dir ]
#then
#  while read line
#  do
#    echo $line|grep "\/MT7620A">/dev/null
#    if [ $? -eq 0 ];then
#
#	build_dir_kernel=`echo $line|sed 's/src\//build\//g'`
#	build_dir_kernel_linux="build/kernel/linux-2.6.36.x"
#	for file in `find ${build_dir_kernel} -name *.o`;
#	echo $build_dir_kernel
#	echo $build_dir_kernel_linux
#	do
#	    src_o_kernel=`echo $file|sed 's/build\//src\//g'`
#	    cp $file $src_o_kernel -f
#	done
#
#	cp $line $build_dir_kernel -rf
#	
#	echo "enter kernel!"
#	find $line -name '*.c'|xargs echo >/home/zhangyuanmao/work/Sitecom_gpl/trunk/test.txt
#    find $line -name '*.h'|xargs >>/home/zhangyuanmao/work/Sitecom_gpl/trunk/test.txt
#	IsKernelHide=1
#
#    fi 
#  done <$MyPath/release_dir
#fi
#
echo "&&&&&&&&&&&&&&&&&&&& deal with kernel files OK! &&&&&&&&&&&&&&&&&&&&"