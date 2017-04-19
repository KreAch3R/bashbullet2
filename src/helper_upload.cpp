#include <iostream>
#include <json/json.h>
using namespace std;
int main(){

	string jsonstr;
	getline(cin,jsonstr);

	Json::Reader parser;
        Json::Value M;
	parser.parse(jsonstr,M);

	cout << M["file_url"].asString() << endl;
	cout << M["upload_url"].asString() << endl;

	int index=0;
	for(Json::Value::iterator it=M["data"].begin(); it !=M["data"].end(); it++){
		Json::Value key = it.key();
		cout << " -F " << key.asString() << ':' << it->asString() ;
	}
	cout << endl;

	return 0;
}
