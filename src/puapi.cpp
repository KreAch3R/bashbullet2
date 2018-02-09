#include "puapi.h"
#include <iostream>
#include <string>
#include <curl/curl.h>
using namespace std;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
string get_raw_json(string apikey,string url){
	string readBuffer;
	CURL *curl = curl_easy_init();

	if(curl){
		string token="Access-Token:"+apikey;
		struct curl_slist *list = NULL;
		list = curl_slist_append(list, token.c_str() ) ;

		CURLcode res;

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str() );
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	return readBuffer;
}
string get_cursor_pushes(string apikey,string cursor){
	string url="https://api.pushbullet.com/v2/pushes?active=true&cursor="+cursor;
	return get_raw_json(apikey, url);
}

string get_n_pushes(string apikey,int n){
	string limit=to_string(n);
	string url="https://api.pushbullet.com/v2/pushes?active=true&limit="+limit;
	return get_raw_json(apikey, url);
}
string get_pushes(string apikey,string lasttime){
	string url="https://api.pushbullet.com/v2/pushes?active=true&modified_after="+lasttime;
	return get_raw_json(apikey, url);
}
string get_cursor_devices(string apikey,string cursor){
	string url="https://api.pushbullet.com/v2/devices?active=true&cursor="+cursor;
	return get_raw_json(apikey, url);
}
string get_devices(string apikey){
	string url="https://api.pushbullet.com/v2/devices?active=true";
	return get_raw_json(apikey, url);
}
string get_user_iden(string apikey){
	string url="https://api.pushbullet.com/v2/users/me";
	return get_raw_json(apikey, url);
}
