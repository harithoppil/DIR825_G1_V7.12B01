#!/bin/bash
#this is a test file
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

echo "==================== deal with uboot files ===================="

	mkdir $MyPath/tbsboot
	mkdir $MyPath/tbsboot/include
	mkdir $MyPath/tbsboot/cpu
	mkdir $MyPath/tbsboot/cpu/mt7620a
	mkdir $MyPath/tbsboot/bootstart
	mkdir $MyPath/tbsboot/bootstart/mt7620a

	cp $MYROOTDIR/src/boot/tbsboot/include/configs/mt7620a.h $MyPath/tbsboot/include -f
	cp $MYROOTDIR/src/boot/tbsboot/cpu/mt7620a $MyPath/tbsboot/cpu -rf
	cp $MYROOTDIR/src/boot/tbsboot/bootstart/cpu/mt7620a $MyPath/tbsboot/bootstart -rf




if [ -f $MyPath/release_dir ]
then
  while read line
  do
    echo $line|grep "\/boot">/dev/null
    if [ $? -eq 0 ];then
		build_boot_dir=`echo $line|sed 's/src\//build\//g'`
    
		#U-boot-1.1.6
		rm -rf $line/u-boot-1.1.6

		#tbsboot/include/configs
		rm $line/tbsboot/include/configs/* -rf

		#tbsboot/cpu
		rm $line/tbsboot/cpu/* -rf

		#tbsboot/bootstart/cpu
		rm $line/tbsboot/bootstart/cpu/* -rf

		cp $MyPath/tbsboot/include/mt7620a.h $MYROOTDIR/src/boot/tbsboot/include/configs -f
		cp $MyPath/tbsboot/cpu/mt7620a $MYROOTDIR/src/boot/tbsboot/cpu/ -rf
		cp $MyPath/tbsboot/bootstart/mt7620a $MYROOTDIR/src/boot/tbsboot/bootstart/cpu/ -rf

		rm -f $line/tbsboot/config.mk
		mv -f $line/tbsboot/config_bk.mk $line/tbsboot/config.mk
		rm -f $line/Makefile
		mv -f src/boot/Makefile_bk src/boot/Makefile
		
		#tbsboot/cpu
		build_boot_build_cpu=$build_boot_dir/tbsboot/cpu/mt7620a
		src_boot_tbsboot_cpu=$line/tbsboot/cpu/mt7620a

		mv -f $build_boot_build_cpu/*.s $src_boot_tbsboot_cpu
		
		for file in `find ${src_boot_tbsboot_cpu} -name *.s`;
        do
	    	src_S_file=`echo $file|sed 's/\.s$/.S/g'`
	    	mv $file $src_S_file -f
	 	done

		find $src_boot_tbsboot_cpu -name '*.c'|xargs rm -rf
	 	find $src_boot_tbsboot_cpu -name '*.h'|xargs rm -rf
	 	rm -f $src_boot_tbsboot_cpu/Makefile
	 	cp -f $MyPath/gpl_makefile/boot/cpu/Makefile $src_boot_tbsboot_cpu
	 	

		#tbsboot/bootstart/cpu
	 	#build_boot_build_bootstart=$build_boot_dir/build/bootstart/cpu/rtl8196c
	 	
	 	#for file in `find ${build_boot_build_bootstart} -name *.o`;
        #do
	    #	src_boot_tbsboot_bootstart=`echo $file|sed 's/build\//src\//'|sed 's/build\//tbsboot\//'`
	    #	Build_S_file=`echo $file|sed 's/\.s$/.S/g'`
	    #	SRC_S_file=`echo $src_boot_tbsboot_bootstart|sed 's/\.s$/.S/g'`
	    	
	    #	$OBJDUMP -D $file > $Build_S_file
	    #	cp $Build_S_file $SRC_S_file -f
	 	#done

	 	#for file in `find ${build_boot_build_bootstart} -name *.a`;
        #do
	    #	src_boot_tbsboot_bootstart=`echo $file|sed 's/build\//src\//'|sed 's/build\//tbsboot\//'`
	    #	cp $file ${src_boot_tbsboot_bootstart} -f
	 	#done

		#src_boot_tbsboot_bootstart=`echo $build_boot_build_bootstart|sed 's/build\//src\//'|sed 's/build\//tbsboot\//'`
	 	#find $src_boot_tbsboot_bootstart -name '*.c'|xargs rm -rf
	 	#find $src_boot_tbsboot_bootstart -name '*.h'|xargs rm -rf
	 	#find $src_boot_tbsboot_bootstart -name '*.S'|xargs rm -rf


	 	#tbsboot/common
		build_boot_build_common=$build_boot_dir/tbsboot/common
		src_boot_tbsboot_common=`echo $build_boot_build_common|sed 's/build\//src\//'`

		rm -f $src_boot_tbsboot_common/flash_layout.c
		rm -f $src_boot_tbsboot_common/cmd_modify_sysc.c
		mv -f $build_boot_build_common/flash_layout.s $src_boot_tbsboot_common/flash_layout.S
		mv -f $build_boot_build_common/cmd_modify_sysc.s $src_boot_tbsboot_common/cmd_modify_sysc.S

		rm -f $MYROOTDIR/src/boot/tbsboot/common/Makefile
	 	cp $MyPath/gpl_makefile/boot/common/Makefile $MYROOTDIR/src/boot/tbsboot/common -f

	 	

    fi 
  done <$MyPath/release_dir
fi

echo "&&&&&&&&&&&&&&&&&&&& deal with uboot files OK! &&&&&&&&&&&&&&&&&&&&"