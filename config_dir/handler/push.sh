#!/bin/bash
source ~/.bashbullet2/handler_config

#arg1	arg2	arg3	arg4	arg5
#type	from	to	title	url

# stdin:
# MSG/BODY
if [[ $# != 5 && $1 != "file" ]];then
	notify-send "${1} :: ${4}\n${2} >> ${3}" "$( cat - )"
else
	notify-send "${1} :: ${4}\n${2} >> ${3}" "$5"
	if [ $1 == "file" ];then
		url=`cat -`
	else
		url="$5"
	fi
	if(( $cp_link ));then echo "$url"|xclip -sel clip; fi
	if(( $xdg_open ));then xdg-open "$url"; fi
fi

if [ ! -z "$notify_command" ];then
	eval $notify_command
fi
