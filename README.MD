Bashbullet2  
===================================  
 Bashbullet2 is a pushbullet notification daemon for pushes and Phone-PC notification mirrorring.
 A simple GUI for sending push is also included.
  
 This is the development version of Bashbullet.
 I've yet to decide what function should be included in this new version.
 The old one is ugly and full of bugs and will eventually be replaced.
 Most of the core functions will be rewritten in C++, but the scripting capability will still be supported.
  
### Prerequisite:   
	libnotify   
	websocketpp  
	cpprestsdk  
	jsoncpp  
	curl  
	jshon  
	yad   
	xclip (clipboard support)  
    
### Build:  
	cd src   
	make   
   
### Install:   
	# 1. copy "config_dir" to $HOME and rename to .bashbullet2   
	cp -a ./config_dir $HOME/.bashbullet2   
   
	# 2. edit $HOME/.bashbullet2/config.json and put your access token in this file   
	# 3. run create_devices.sh ONCE to create bashbullet device ( skip this step if upgrade from old bashbullet)   
	$HOME/.bashbullet2/lib/create_device.sh   
   
	# 4. edit $HOME/.bashbullet2/handler_config according to your preference   
	# 5. scripting can be easily done by editing $HOME/.bashbullet/handler/*   
   
### Run:   
	# To receive notification   
	./libbashbullet   
   
	# (Optional) to write a new push   
	$HOME/.bashbullet2/handler/new_push.sh   
