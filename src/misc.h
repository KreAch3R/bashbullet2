#include <fstream>
#include <string>
#include <boost/locale.hpp>
#include <json/json.h>
#include <algorithm>
using namespace std;

void update_timestamp(string, string&);
string get_timestamp(string);

string utf8_cut(string&, int);

string sanitize(string& in);

Json::Value file_json(string);
Json::Value str_json(string);

void save_devices( map<string,string>& , string, vector<string>);
