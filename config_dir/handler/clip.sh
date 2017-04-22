#!/bin/bash
source ~/.bashbullet2/handler_config

# stdin:
# MSG/BODY

if(( $cp_link ));then
	# use sed to remove backslash
	cat -|sed 's/\(\\\)\(.\)/\2/g'|xclip -sel clip
fi
