Bashbullet2  
===================================  
 Bashbullet2 is a pushbullet notification daemon for pushes and Phone-PC notification mirrorring.
 A simple GUI for sending push is also included.
  
 The old version of bashbullet is ugly, full of bugs and will eventually be replaced.
 Most of the core functions will be rewritten in C++, but the scripting capability will still be supported.

![alt tag](https://boteium.github.io/bashbullet2/bashbullet_systray_inbox.png)

### Feature :   
	receive, send, mirror pushes and send files.   
	send sms from bashbullet through phone.   
	copy texts from phone's clipboard.   
	copy links automatically.   
	customizable script and notification.   
  
### Prerequisite:   
	libnotify   
	websocketpp  
	cpprestsdk  
	jsoncpp  
	curl  
	yad   
	xclip (clipboard support)  
    
### Build:  
	cd src   
	make   
   
### Configuration:   
	# 1. copy "config_dir" to $HOME and rename to .bashbullet2   
	cp -a ./config_dir $HOME/.bashbullet2   
   
	# 2. edit $HOME/.bashbullet2/config.json and put your access token in this file   
	# 3. run create_devices.sh ONCE to create bashbullet device ( skip this step if upgrade from old bashbullet)   
	$HOME/.bashbullet2/lib/create_device.sh   
   
	# 4. edit $HOME/.bashbullet2/handler_config according to your preference   
	# 5. scripting can be easily done by editing $HOME/.bashbullet/handler/*   
   
### Run:   
	# systray with daemon   
	./bashbullet2   
   
	# Or, using standalone daemon without systray to receive notification   
	./libbashbullet   
   
	# (Optional) to write a new push   
	$HOME/.bashbullet2/lib/new_push.sh   
