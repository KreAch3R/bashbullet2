.DEFAULT_GOAL := all

libbashbullet: libbashbullet.cpp puapi.cpp misc.cpp
	g++ --std=c++11 libbashbullet.cpp misc.cpp puapi.cpp -o ../libbashbullet -lboost_system -lpthread -lssl -lcrypto -lcpprest -ljsoncpp -lcurl

helper: helper.cpp misc.cpp puapi.cpp
	g++ --std=c++11 helper.cpp misc.cpp puapi.cpp -o ../config_dir/lib/helper -lcurl -ljsoncpp

all: helper libbashbullet
