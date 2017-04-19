#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <cstdlib>
#include <json/json.h>
#include <cpprest/ws_client.h>

#include <unistd.h>

#include "misc.h"
#include "puapi.h"

using namespace std;
using namespace web;
using namespace web::websockets::client;

int maxtitle=50, maxbody=160;
map<string,string> iden2devname;
string pdir=getenv("HOME"), apikey, ftimestamp, timestamp, traypipe;
bool systray=true, enable_sms_alert=false;

void handler(Json::Value& M ){
        string type=M["type"].asString();
        if( type == "nop" ){
                fprintf(stderr, "nop " );
        }else if( type == "tickle" ){
	        Json::Value tmpM = str_json( get_pushes(apikey,  timestamp) );
		int index=0;
	        for( Json::ValueIterator it = tmpM["pushes"].begin() ; it != tmpM["pushes"].end() ; it++, index++ ){
			bool dismissed = tmpM["pushes"][ index ]["dismissed"].asBool();
			if( dismissed == false ){
				type = tmpM["pushes"][ index ][ "type" ].asString();

				vector<string> PU(5);		// 0body 1from 2to 3title 4url
				vector<string> RAW={"body","source_device_iden","target_device_iden","title","url"};

				for(int i=0,sz=RAW.size(); i<sz; i++)
					PU[i] = tmpM["pushes"][ index ][ RAW[i] ].asString();

				// decode identity
				PU[1] = iden2devname[ PU[1] ];
				PU[2] = iden2devname[ PU[2] ];
				if( PU[1] == "" ) PU[1] = tmpM["pushes"][ index ][ "sender_name" ].asString();
				if( PU[2] == "" ) PU[2] = "ALL\n";

				// max length of title & body
        	                if( PU[3].size() > maxtitle  ) PU[3] = utf8_cut( PU[3], maxtitle ) + "....";
        	                if( PU[0].size() > maxbody ) PU[0] = utf8_cut( PU[0], maxbody ) + "....";

				cout << endl << "pushes: " << PU[1] << '>' << PU[2] << " :: " << PU[3] << endl;

				for(auto& s:PU)	sanitize(s);
        	                string cmds, cmd=pdir+"handler/push.sh";
				if( PU[4] != "" )
		                        cmds="cat <<< '"+ PU[0] +"'|" + cmd + " \"" + type + "\" \"" + PU[1] + "\" \"" + PU[2] + "\" \"" + PU[3] + "\" \"" + PU[4] + "\" &";
				else
		                        cmds="cat <<< '"+ PU[0] +"'|" + cmd + " \"" + type + "\" \"" + PU[1] + "\" \"" + PU[2] + "\" \"" + PU[3] + "\" &";
        	                system( cmds.c_str() );
			}
			// will update both timestamp variable write it to file
			update_timestamp(ftimestamp, timestamp);
			if(systray) update_icon(traypipe);
		}
        }else if( type == "push" ){
		bool encrypted = M["push"]["encrypted"].asBool();
		if( encrypted ){
			cout << "encrypted message" << endl;
			string cmds="echo |" +pdir + "handler/mirror.sh \"Bashbullet\" \"Warning\" \"End to End encryption is currently not supported\"";
			system( cmds.c_str());
			type = "dismissal";
		}else{
			type = M["push"]["type"].asString();
		}

		if( type == "dismissal"){
			/* do nothimg */
		}else if( type == "mirror" ){
			vector<string> PU(5);		// 0body 1from 2app 3title 4icon
			vector<string> RAW={"body","source_device_iden","application_name","title","icon"};

			for(int i=0,sz=RAW.size(); i<sz; i++)
				PU[i] = M["push"][ RAW[i] ].asString();

			PU[1] = iden2devname[ PU[1] ];
                        if( PU[3].size() > maxtitle  ) PU[3] = utf8_cut( PU[3], maxtitle ) + "....";
                        if( PU[0].size() > maxbody ) PU[0] = utf8_cut( PU[0], maxbody ) + "....";

			for(auto& s:PU)	sanitize(s);
			string cmd=pdir+"handler/mirror.sh";
			string cmds = "cat <<< '"+ PU[0] +"'|" + cmd + " \"" + PU[1] + "\" \"" + PU[2] + "\" \"" + PU[3] + "\" \"" + PU[4] + "\" &";
			system( cmds.c_str() );

			cout << endl << "mirror: " << PU[1] << ':' << PU[2] << " :: " << PU[3] << endl;
		}else if( type == "clip" ){
			// untested, this is now payed function
			string body = M["push"][ "body" ].asString();
			string cmd="cat <<< '" + sanitize(body) + "'|" + pdir + "handler/clip.sh";
			system( cmd.c_str() );
		}else if( type == "sms_changed" && enable_sms_alert ){
			string from= iden2devname[ M["push"][ "source_device_iden" ].asString() ];
			string cmd="echo |" +pdir + "handler/mirror.sh \"" + sanitize(from) + "\" \"SMS Received/Sent\"";
			system( cmd.c_str());
		}
        }

}


int main() {
	// env
	pdir+="/.bashbullet2/";
	ftimestamp=pdir+".TIMESTAMP", timestamp;
	traypipe=pdir+".systray_pipe";
	ifstream is( traypipe.c_str() );
	if( is.good() ) systray=true;
	is.close();

        Json::Value JStmp;

	// parse config, timestamp
	JStmp = file_json( pdir+"config.json" );
	apikey=JStmp["access_token"].asString();
	if( apikey == "" ){
		cout << " Access-Token is missing, stopping now. \n";
		return 0;
	}
	if( ! JStmp["title_max_length"].isNull() ) maxtitle=stoi( JStmp["title_max_length"].asString() );
	if( ! JStmp["body_max_length"].isNull() ) maxbody=stoi( JStmp["body_max_length"].asString() );
	if( ! JStmp["enable_sms_alert"].isNull() ) enable_sms_alert=JStmp["enable_sms_alert"].asBool();
	timestamp=get_timestamp(ftimestamp);

	// Getting devices
	string rawdev;
	for(int trial=1; trial<=2; trial++ ){
		rawdev=get_devices(apikey);
		if( rawdev != "" ) break;
		else if(trial == 2 ){
			cout << "ERROR: fail to get devices list. Connection issue ? \n";
			return 0;
		}
		sleep(5);
	}
	// Parse devices, get more devices if device list has multiple pages (cursor)
	// (arbitrarily setting a max 10-cursor limit)
        JStmp = str_json( rawdev );
	vector<string> sms_capab;
	for(int i=0; i<10; i++){
	        int index=0;
	        for( Json::ValueIterator it = JStmp["devices"].begin() ; it != JStmp["devices"].end() ; it++, index++ ){
			iden2devname[ JStmp["devices"][ index ]["iden"].asString() ] = JStmp["devices"][ index ]["nickname"].asString();
			if( JStmp["devices"][ index ]["has_sms"].asBool() ) sms_capab.push_back( JStmp["devices"][ index ]["nickname"].asString() );
	        }
		string cursor = JStmp["cursor"].asString();
	        if( cursor=="" ) break;
		else JStmp = str_json( get_cursor_devices(apikey, cursor) );
	}

	bool has_bashbullet=false;
	for(auto& kv:iden2devname)
		if( kv.second == "bashbullet" ) has_bashbullet=true;
	if(has_bashbullet=false){
		cout << "ERROR: bashbullet device does not exist. \n";
		cout << "Run \"create_device.sh\" once if it's never been done. ::  \n";
		return 0;
	}

	// save device cache for YAD GUI
	save_devices( iden2devname , pdir ,sms_capab );

	// begin websocket
	for(int retry=0; retry<2; retry++){
		websocket_client client;
		try{
			client.connect("wss://stream.pushbullet.com/websocket/"+apikey ).wait();
			while(1){
				try{
					client.receive().then([](websocket_incoming_message in_msg) {
						return in_msg.extract_string();
					}).then([](string wsmsg) {
/*						// debug
						string fdebug="/tmp/rawmsg";
				        	ofstream rawdeb(fdebug.c_str(), std::ofstream::out | std::ofstream::app);
						rawdeb << wsmsg << "\n";
*/
						// parse
						Json::Value M=str_json(wsmsg);
					        handler(M);

					}).wait();
				}catch(const std::exception &e){
					// break loop and reconnect when encountering an error
					printf("Error exception:%s\n", e.what());
					break;
				}
			}
		}catch(const std::exception &e){
			// if failed to re-connect (give up at 2nd failure);
			printf("Error exception:%s\n", e.what());
		}
		client.close().then([](){ /* close connection and retry */ });
		sleep(5);
	}

	return 0;
}

/*
// write to websocket ?? Ephermeral from PC ??
  websocket_outgoing_message out_msg;
  out_msg.set_utf8_message("test");
  client.send(out_msg).wait();
*/
