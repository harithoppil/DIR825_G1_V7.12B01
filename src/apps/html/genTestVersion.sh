#!/bin/sh
version=""
auth=""
lastrev=""
revURL=""
date=`date -R|cut -b1-26`

LANGINIT=`echo $LANG`

export LANG="zh_CN.UTF-8"

svn info $1 > temp
cat temp
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
echo "*****************Create test_version.html Begain***********************************"
echo "" > test_version.html

echo -e "	<td>\n\
			<p>Version Information</p>\n\
                   <p>Subversion rev:&nbsp;$lastrev</p>\n \
                   <p>Build By:&nbsp;$auth</p>\n \
                   <p>Build Date:&nbsp;$date</p>\n \
                   <p>Changed Rev URL:&nbsp;$revURL</p>\n \
		 </td>\n\
	</tr>\n\
</table>\n" >> test_version.html
echo "*****************Create test_version.html successful!******************************"
