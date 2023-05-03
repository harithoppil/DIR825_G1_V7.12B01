#!/bin/sh
export ROOTDIR=$(while true; do if [ -f BaseVar.mk ]; then pwd;exit; else cd ..;fi;done;)
echo $ROOTDIR

#############en_us##################
for file in $ROOTDIR/src/apps/html/skins/DIR825G1/languages/en_us/* 
do
echo $file
afile=`echo $file | awk -F /en_us/ '{print $2}'`
echo $afile
echo ln -sf ../../../../../var/multi_lang/$afile $ROOTDIR/build/romfs/rootfs/usr/www/html/languages/en_us/$afile
ln -sf ../../../../../var/multi_lang/$afile $ROOTDIR/build/romfs/rootfs/usr/www/html/languages/en_us/$afile
echo $file
done


#############zh_tw##################
for file in $ROOTDIR/src/apps/html/skins/DIR825G1/languages/zh_tw/* 
do
echo $file
afile=`echo $file | awk -F /zh_tw/ '{print $2}'`
echo $afile
echo ln -sf ../../../../../var/multi_lang/$afile $ROOTDIR/build/romfs/rootfs/usr/www/html/languages/zh_tw/$afile
ln -sf ../../../../../var/multi_lang/$afile $ROOTDIR/build/romfs/rootfs/usr/www/html/languages/zh_tw/$afile
echo $file
done

#############es##################
for file in $ROOTDIR/src/apps/html/skins/DIR825G1/languages/es/* 
do
echo $file
afile=`echo $file | awk -F /es/ '{print $2}'`
echo $afile
echo ln -sf ../../../../../var/multi_lang/$afile $ROOTDIR/build/romfs/rootfs/usr/www/html/languages/es/$afile
ln -sf ../../../../../var/multi_lang/$afile $ROOTDIR/build/romfs/rootfs/usr/www/html/languages/es/$afile
echo $file
done


#############pt##################
for file in $ROOTDIR/src/apps/html/skins/DIR825G1/languages/pt/* 
do
echo $file
afile=`echo $file | awk -F /pt/ '{print $2}'`
echo $afile
echo ln -sf ../../../../../var/multi_lang/$afile $ROOTDIR/build/romfs/rootfs/usr/www/html/languages/pt/$afile
ln -sf ../../../../../var/multi_lang/$afile $ROOTDIR/build/romfs/rootfs/usr/www/html/languages/pt/$afile
echo $file
done

#rm $ROOTDIR/src/apps/html/skins/DIR809/languages/en_us -rf

