#!/bin/bash
source ~/.bashbullet2/handler_config

# stdin:
# MSG/BODY

if(( $cp_link ));then
	cat -|xclip -sel clip
fi
