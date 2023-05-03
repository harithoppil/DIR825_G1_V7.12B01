#!/bin/sh
version="AP699E8C.CW125A-5-DLINK-WW"


#auth="pengyao"
auth=""

#lastrev="21705"
lastrev=""

#revURL="GAN4.FR71A-B/trunk"
revURL=""

#flashList="{AT25DF641,0x1F4800}{A25L064,0x373017}{EN25P64,0x1C2017}{EN25Q64,0x1C3017}{M25P64,0x202017}{MX25L6405D,0xc22017}{W25X64,0xEF3017}{W25Q64,0xEF4017}S25FL064A,0x010216}{GD25Q64,0xC84017}"
flashList="{GD25Q128,0xC84018},{AT25DF128,0x1F4900},{A25L128,0x373018},{EN25P128,0x1C2018},{EN25Q128,0x1C3018},{N25Q128,0x20BA18},{N25Q128A,0x20BB18},{MX25L12805,0xC22018},{W25X128,0xEF3018},{W25Q128,0xEF4018},{S25FL128P,0x012018}"

LANGINIT=`echo $LANG`
export LANG="zh_CN.UTF-8"

svn info > temp
if [ $? -eq 0 ]; then
	lastrev=`cat temp | sed -n 's/最后修改的版本: //gp'`
	if [ -z "$lastrev" ]; then
	      lastrev=`cat temp | sed -n 's/Last Changed Rev: //gp'`
	fi
	revURL=`cat temp | sed -n 's/URL: //gp'`
else
	lastrev=`echo "None"`
	revURL=`echo "None"`
fi

rm temp -rf

auth=`who am i`
auth=`echo ${auth%pts*}`

export LANG=$LANGINIT

./build/product/hidenPage.sh "$version" "$auth" "$lastrev" "$revURL" "$flashList"
