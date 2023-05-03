#!/bin/sh
OBJCPY=${TOOLCHAINS_PATH}/${OBJCOPY}

if [ "${ENDIANNESS}" = "big" ];
then
ENDIAN=be
ENDIAN_OPT=b
fi

if [ "${ENDIANNESS}" = "little" ];
then
ENDIAN=le
ENDIAN_OPT=l
fi

PINANDOTHER_ENABLED=`grep CONFIG_PINANDOTHER_ENABLED ${BUILDDIR}/.config | awk -F"=" '{print $2}'`
DOUBLE_BACKUP=`grep CONFIG_DOUBLE_BACKUP ${BUILDDIR}/.config | awk -F"=" '{print $2}'`
FLASH_ERASESIZE=`grep CONFIG_FLASH_ERASESIZE  ${BUILDDIR}/.config | awk -F"=" '{print $2}'`
MKSQUASHFS_BLOCK=`grep CONFIG_MKSQUASHFS_BLOCK ${BUILDDIR}/.config | awk -F"=" '{print $2}'`
BOARD_ID=`grep CONFIG_BOARD_ID ${BUILDDIR}/.config | awk -F"=" '{print $2}' | awk -F"\"" '{print $2}'`
SW_REGION=`grep CONFIG_SW_REGION ${BUILDDIR}/.config | awk -F"=" '{print $2}' | awk -F"\"" '{print $2}'`
SW_VERSION=`grep CONFIG_SW_VERSION ${BUILDDIR}/.config | awk -F"=" '{print $2}' | awk -F"\"" '{print $2}'`
SW_MODEL=`grep CONFIG_SW_MODEL ${BUILDDIR}/.config | awk -F"=" '{print $2}' | awk -F"\"" '{print $2}'`
SYSTEM_VERSION=`grep CONFIG_SYSTEM_VERSION ${BUILDDIR}/.config | awk -F"=" '{print $2}' | awk -F"\"" '{print $2}'`
SDRAM_TRANSFER=`grep CONFIG_SDRAM_TRANSFER ${BUILDDIR}/.config | awk -F"=" '{print $2}'`
FLASH_TRANSFER=`grep CONFIG_FLASH_TRANSFER ${BUILDDIR}/.config | awk -F"=" '{print $2}'`
GENERATE_SSID_PASSPHRASE=`grep CONFIG_GENERATE_SSID_PASSPHRASE ${BUILDDIR}/.config | awk -F"=" '{print $2}'`
MAC_OUI=`grep CONFIG_MAC_OUI ${BUILDDIR}/.config | awk -F"=" '{print $2}' | awk -F"\"" '{print $2}'`
TBS_APP_CFG_NAME=`grep TBS_APP_CFG ${ROOTDIR}/src/include/flash_layout.h  | awk -F"\"" {'print $2'}`
DRAM_SIZE=`grep CONFIG_DRAM_SIZE ${BUILDDIR}/.config | awk -F"=" '{print $2}'`
SUPPORT_JFFS2=`grep CONFIG_MTD_JFFS2 ${BUILDDIR}/.config -w | awk -F"=" '{print $2}'`
IMAGE_ENCRYPT_SUPPORT=`grep CONFIG_VENDOR_IMGCRYPT ${BUILDDIR}/.config | awk -F"=" '{print $2}'`


if [ "${DOUBLE_BACKUP}" = "y" ];
then
IMG_TYPE=imgd
else
IMG_TYPE=imgs
fi

if [ "${SDRAM_TRANSFER}" = "y" ];
then
SYSCONFIG_TRANSFER_MODE=sdram
fi

if [ "${FLASH_TRANSFER}" = "y" ];
then
SYSCONFIG_TRANSFER_MODE=flash
fi

# Create random MAC address
MAC="$MAC_OUI:`printf %x $[($RANDOM % 256)]`:`printf %x $[($RANDOM % 256)]`:`printf %x $[($RANDOM % 256)]`"
# Create random SSID and Passphrase
if [ "${GENERATE_SSID_PASSPHRASE}" = "y" ];
then
TMP="12icyhat123"
fi

cd  $ROOTDIR/tools/
# Create Wireless Pin
PIN=`./bin/generatepin $MAC| awk '{print $2}'`$TMP
echo $PIN

if [ "${ARCH}" = "um" ];
then
    ./bin/mksquashfs  ../build/romfs/rootfs  ../build/romfs/rootfs.squashfs.img  -noI -noD -noF -no-fragments -noappend -all-root -${ENDIAN}

    exit 0

else

   ${OBJCPY} -S -O binary ${ROMFS}/vmlinux ${ROMFS}/vmlinux.bin
   ./bin/lzma e ${ROMFS}/vmlinux.bin ${ROMFS}/vmlinux.bin.lzma

fi

	./bin/mksquashfs  ${ROMFS}/rootfs ${ROMFS}/rootfs.img -comp lzma -noappend -b ${MKSQUASHFS_BLOCK}
    #./bin/mksquashfs  ${ROMFS}/rootfs ${ROMFS}/rootfs.img -${ENDIAN} -noappend -b ${MKSQUASHFS_BLOCK}

	FLASH_ERASESIZE=64
	EXTFS_SIZE=128
	REAL_ERASE_SIZE=`expr ${FLASH_ERASESIZE} \* 1024`
	REAL_EXTFS_SIZE=`expr ${EXTFS_SIZE} \* 1024`
	/usr/sbin/mkfs.jffs2 -d ${ROMFS}/multi_lang -o ${ROMFS}/multi_lang.img -${ENDIAN_OPT} -e ${REAL_ERASE_SIZE} --pad=${REAL_EXTFS_SIZE}
    ./bin/mkimage -b ${ROMFS}/bootloader.bin -k ${ROMFS}/vmlinux.bin.lzma -r ${ROMFS}/rootfs.img \
-mac $MAC -pin $PIN -ip 192.168.1.1 -${ENDIAN} -obin ${ROMFS}/${PRODUCT}.bin -oimg ${ROMFS}/${PRODUCT}.img \
-id ${BOARD_ID} -region ${SW_REGION} -sw ${SW_VERSION} -model ${SW_MODEL} -blk ${FLASH_ERASESIZE}  -v ${SYSTEM_VERSION} -img_type ${IMG_TYPE} -p $PRODUCT \
-t $SYSCONFIG_TRANSFER_MODE -app_cfg ${TBS_APP_CFG_NAME} ${ROMFS}/tbs_app_cfg
#-multi_lang  -fss ${EXTFS_SIZE} -f ${ROMFS}/multi_lang.img

if [ "${IMAGE_ENCRYPT_SUPPORT}" = "y" ];
then
    ./bin/imgcrypt ${ROMFS}/${PRODUCT}.img ${ROMFS}/${PRODUCT}_firmware.img ${BUILDDIR}/vendor/imgcrypt/key.pem "12345678"
fi

cd ${ROMFS}
ln -sf ${PRODUCT}.bin image.bin
ln -sf ${PRODUCT}.img image.img

echo
echo
echo  "==================================================="
echo  "|                    Image Info                   |"
echo  "---------------------------------------------------"
echo  "| PRODUCT:             ${PRODUCT}"
echo  "| Endian:              ${ENDIANNESS}"
echo  "| DRAM Size:           ${DRAM_SIZE} Mbytes"
echo  "| Flash erase size:    ${FLASH_ERASESIZE} Kbytes"
echo  "| Cfg repeat mode:     ${SYSCONFIG_TRANSFER_MODE}"
echo  "| Double Backup:       ${DOUBLE_BACKUP}"
echo  "| IMG_TYPE:            ${IMG_TYPE}"
echo  "| Support Jffs2:       ${SUPPORT_JFFS2}"
echo  "| Squashfs block:      ${MKSQUASHFS_BLOCK}"
echo  "| Board ID:            ${BOARD_ID}"
echo  "| System version:      ${SYSTEM_VERSION}"
echo  "| MAC OUI:             ${MAC_OUI}"
echo  "| Original MAC:        ${MAC}"
if [ "${PINANDOTHER_ENABLED}" = "y" ];
then
echo  "| Wirless PIN:         ${PIN}"
fi
if [ "${IMAGE_ENCRYPT_SUPPORT}" = "y" ];
then
echo  "| Image Encryption:    YES"
fi
echo  "---------------------------------------------------"
