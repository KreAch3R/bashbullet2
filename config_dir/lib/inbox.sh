if ps -ef|grep -e yad |grep "$HOME/.bashbullet2/.show" >/dev/null;then
	exit 0
fi

timestamp=`cat $HOME/.bashbullet2/.TIMESTAMP`

if [ ! -f "$HOME/.bashbullet2/.show_${timestamp}" ];then
	echo Retrieving lastest pushes
	rm "$HOME/.bashbullet2/.show_*" 2>/dev/null
	"$HOME/.bashbullet2/lib/helper" > "$HOME/.bashbullet2/.show_${timestamp}"
	printf "\n" >> "$HOME/.bashbullet2/.show_${timestamp}"
	echo "Click Manage in Browser to show more/delete pushes" >> "$HOME/.bashbullet2/.show_${timestamp}"
fi

YADKEY=${RANDOM}

yad --fontname="Monospace Regular 12" --plug=$YADKEY --tabnum=1 --text-info --filename="$HOME/.bashbullet2/.show_${timestamp}" &
yad --key=$YADKEY --mouse --notebook --width=650 --height=350 --title "Bashbullet :: Inbox" \
    --tab="Lastest 10 Pushes" --fontname="Monospace 16" \
    --window-icon="${SRC}/pushbullet.svg" --button "Close:1" --button "Mark as Read:2" --button "New push:3" --button "Manage in Browser:4" 

case  $? in
	2)
		PIPE="$HOME/.bashbullet2/.systray_pipe"
		if [ -e "$PIPE" ];then
			echo icon:"$HOME/.bashbullet2/lib/pushbullet.svg" > "$PIPE"
		fi
		;;
	3)
		"$HOME/.bashbullet2/lib/new_push.sh"
		;;
	4)
		xdg-open "https://www.pushbullet.com/";
		;;
esac
