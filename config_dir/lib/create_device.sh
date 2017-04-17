#!/bin/bash
echo ""
echo The script only needs to be run ONCE.
echo Otherwise it might create duplicate bashbullet entries.
echo ""

if [ ! -f  ~/.bashbullet2/config.json ];then
	echo Please put your access token in ~/.bashbullet2/config.json
	exit 0;
fi
API_KEY=`grep access_token ~/.bashbullet2/config.json|cut -d '"' -f4`

if [ -z $API_KEY ];then
	echo error :: access token does not exist.
fi


echo 'Create bashbullet device ? (y/n)'
read yesno

if [ "$yesno" == "y" ];then
	curl --header "Access-Token: $API_KEY" \
		-X POST https://api.pushbullet.com/v2/devices \
		-d nickname=bashbullet -d type=opera
fi
