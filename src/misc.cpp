#include "misc.h"
#include <fstream>
#include <string>
#include <ctime>
#include <algorithm>
#include <json/json.h>
#include <boost/locale.hpp>

using namespace std;

string utf8_cut(string& in, int end){
        int ascii_len=0;
        for(int i=0; i<end; i++)
                if(isprint(in[i])) ascii_len++;
        end = (end+ascii_len)/2;

        u32string tmp = boost::locale::conv::utf_to_utf<char32_t>(in);
        tmp = tmp.substr(0,end);
        return boost::locale::conv::utf_to_utf<char>( tmp );
}

string sanitize(string& in){
        static string illegal=" \\/\"';<>|$()&`#@!~^*/.";
        string out;
        for(auto& c:in){
		for(auto& ci:illegal){
			if(ci==c){
	                        out+='\\';
				break;
			}
                }
                out+=c;
        }
        return out;
}

void update_timestamp(string ftimestamp,string& timestamp){
        timestamp = to_string( std::time(nullptr) );
        ofstream os(ftimestamp.c_str());
        os << timestamp;
        os.close();
}

string get_timestamp(string ftimestamp){
	string timestamp;
        ifstream is(ftimestamp.c_str());
        is >> timestamp;
        is.close();
	return timestamp;
}

Json::Value file_json(string file){
        ifstream is(file.c_str());
        Json::Reader reader;
	Json::Value M;
	reader.parse(is,M);
	return M;
}

Json::Value str_json(string str){
        Json::Reader reader;
	Json::Value M;
	reader.parse(str,M);
	return M;
}

void save_devices( map<string,string>& iden2device, string pdir, vector<string> sms_capab){
	pdir += ".device_cache";
	ofstream os(pdir.c_str());

	string iden="IDEN=\"", dev="DEV=\"", sms="SMSDEV=\"";
	for(auto& kv:iden2device){
		iden = iden + kv.first+"|";
		dev = dev + kv.second+"|";
	}
	for(auto& s:sms_capab){
		sms = sms+s+"|";
	}

	iden.pop_back();
	iden+="\"";
	os << iden << endl;

	dev.pop_back();
	dev+="\"";
	os << dev << endl;

	sms.pop_back();
	sms+="\"";
	os << sms << endl;

	os.close();
}
