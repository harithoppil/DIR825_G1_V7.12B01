#!/bin/sh

TMP_VER_FILE=$1"/skins/"$2"/page/version.template"
TAG_VER_FILE=$1"/skins/"$2"/page/version.html"
SWP_VER_FILE=$1"/skins/"$2"/.verstion"

PRJ_SVN_REVISION=` LANG=C svn info $1 | grep "Revision:"| cut -d ' ' -f  2 `
LAST_CHANGED_AUTHOR=` LANG=C svn info $1 | grep "Last Changed Author:"| cut -d ' ' -f  4 `
echo 'svn last changed rev: '$PRJ_SVN_REVISION
cp -f $TMP_VER_FILE $TAG_VER_FILE
cat $TAG_VER_FILE | sed -e "s/(%_SVN_ID_%)/`echo $PRJ_SVN_REVISION`/g" > $SWP_VER_FILE
#cat $SWP_VER_FILE | sed -e "s/(%_PRODUCT_ID_%)/`echo $PRODUCT_ID`/g" > $SWP_VER_FILE
cat $SWP_VER_FILE | sed -e "s/(%_DATE_%)/`date`/g" > $TAG_VER_FILE
rm -f $SWP_VER_FILE
#cp $1"skins/"$2"/page/version.html" $3 -f
cp $1"skins/"$2"/page/version.html" $3"page" -f