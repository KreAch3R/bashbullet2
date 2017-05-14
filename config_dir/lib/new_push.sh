#!/bin/bash
IFS='|'

if [ -f ~/.bashbullet2/.device_cache ];then
	source ~/.bashbullet2/.device_cache
else
	echo "run libbashbullet once to generate device list cache"
	exit 0
fi

API_KEY=`grep access_token ~/.bashbullet2/config.json|cut -d '"' -f4`

# string to array
AIDEN=($IDEN)
ADEV=($DEV)
ASMSDEV=($SMSDEV)

name2iden(){
	for(( i=0 ; i<${#AIDEN[@]} ; i++ ));do
		if [ "$1" == "${ADEV[$i]}" ];then
			echo "${AIDEN[$i]}"
			return
		fi
	done
}

_ME=`name2iden bashbullet`

PU(){
	if curl -s --header "Access-Token: $API_KEY" -X POST https://api.pushbullet.com/v2/pushes \
		--header "Content-Type: application/json" \
		--data-binary '{"source_device_iden":"'"$_ME"'", "device_iden":"'"$1"'", "type": "'"$2"'", "title": "'"$3"'", "body": "'$4'"}' \
	grep true >/dev/null;then notify-send "Pushed";fi

}

YADPKEY=${RANDOM}
_p_note=$(mktemp --tmpdir push.note.XXXXXX)
_p_sms=$(mktemp --tmpdir push.sms.XXXXXX)
_p_file=$(mktemp --tmpdir push.file.XXXXXX)

       yad --form --field="Send to":CB "${DEV}" \
       	--field="Title" "" --field="Note":TXT "" --item-separator='|' \
	--plug=$YADPKEY --tabnum=1 > $_p_note &

       yad --form --field="Send to":CB "${DEV}" --item-separator='|' \
	--plug=$YADPKEY --tabnum=2 > $_p_file &

       yad --form --field="Sent SMS from":CB "${SMSDEV}" \
		--field="to phone number" "" --field="Note":TXT "" --item-separator='|' \
	--plug=$YADPKEY --tabnum=3 > $_p_sms &

       yad --notebook --key=$YADPKEY --mouse --title "Bashbullet :: new push"  --height=300 --width=400 \
               --window-icon="${SRC}/pushbullet.svg" --button="Discard:1" --button="Send:2" \
	--tab="Push" --tab="File" --tab="SMS"

if [ $? == 2 ] ;then
	SMS=(`cat $_p_sms`)
	FILE=(`cat $_p_file`)
	NOTE=(`cat $_p_note`)

	if [ ! -z "${SMS[1]}" ];then
		curl --header "Access-Token: $API_KEY" -X POST https://api.pushbullet.com/v2/ephemerals \
		--header "Content-Type: application/json" --data-binary \
		'{"type": "push", "push": {"type":"messaging_extension_reply","package_name":"com.pushbullet.android","source_user_iden":"'$_ME'","target_device_iden":"'`name_iden ${SMS[0]}`'","conversation_iden":"'${SMS[1]}'","message":"'"${SMS[2]}"'"}}'
	elif [[ ! -z "${NOTE[1]}" || ! -z "${NOTE[2]}" ]];then
		[ -z ${NOTE[1]} ] && NOTE[1]="untitled"
               	PU `name2iden "${NOTE[0]}"`  note "${NOTE[1]}" "${NOTE[2]}"
	elif [ ! -z "${FILE[0]}" ];then
		FN=`yad --file --width=550 --height=400 --title "Send a file to ${FILE[0]}"`
		FNAME=`sed 's/.*\///g' <<< $FN`
		if echo $FNAME|egrep -i 'png|jpg|jpeg|gif' >/dev/null ;then
			mime=image/`sed -e 's/.*\.//' <<< $FNAME|tr [:upper:] [:lower:]`
		else
			mime=application/octet-stream
		fi
		notify-send "Uploading..., please wait" "Sending $FN to ${FILE[0]}"
                IFS=$'\n'
                RAW=(`curl -s --header "Access-Token: $API_KEY" -X POST https://api.pushbullet.com/v2/upload-request -d file_name="$FNAME" -d file_type=$mime |"$HOME/.bashbullet2/lib/helper_upload"`)

		curl -s "${RAW[0]}" -i -X POST "${RAW[1]}" "${RAW[2]}" -F file=@"${FN}"
                PU `name2iden ${FILE[0]}` file "$FNAME" "${RAW[0]}"
	fi
fi

rm "$_p_note" "$_p_sms" "$_p_file" 2>/dev/null
