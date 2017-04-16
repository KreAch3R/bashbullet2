#!/bin/bash
source ~/.bashbullet2/handler_config

#arg1	arg2	arg3	arg4
#from	apps	title	icon

# stdin:
# MSG/BODY

if [ $# == 4 ];then
	ICO=$(mktemp -u --suffix=pushicon)
	echo $4|base64 -d > "${ICO}"
	convert ${ICO} -resize 60x60 ${ICO}.png
	notify-send -i ${ICO}.png "$1 >> $2 \n$3" "$( cat - )"
	sleep 2;
	rm "${ICO}"
	rm "${ICO}.png"
else
	notify-send "$1 >> $2 \n$3" "$( cat - )"
fi

if [ ! -z "$notify_command_mirror" ];then
        eval $notify_command_mirror
fi
