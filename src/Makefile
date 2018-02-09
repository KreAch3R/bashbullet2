.DEFAULT_GOAL := all

clean:
	rm ../libbashbullet ../config_dir/lib/helper ../config_dir/lib/helper_upload

libbashbullet: libbashbullet.cpp puapi.cpp misc.cpp
	g++ --std=c++11 libbashbullet.cpp misc.cpp puapi.cpp aead_aes_gcm.cpp base64.cpp -o ../libbashbullet -lboost_system -lpthread -lssl -lcrypto  -ljsoncpp -lcurl

helper: helper.cpp misc.cpp puapi.cpp
	g++ --std=c++11 helper.cpp misc.cpp puapi.cpp -o ../config_dir/lib/helper -lcurl -ljsoncpp

helper_upload: helper_upload.cpp
	g++ --std=c++11 helper_upload.cpp -o ../config_dir/lib/helper_upload -ljsoncpp

all: helper libbashbullet helper_upload
