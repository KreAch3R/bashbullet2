#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <cstdlib>
#include <json/json.h>
#include <cpprest/ws_client.h>

#include "misc.h"
#include "puapi.h"

using namespace std;
using namespace web;
using namespace web::websockets::client;

string pdir=getenv("HOME");
string ftimestamp, timestamp;
map<string,string> iden2devname;
string apikey;

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
        	                if( PU[3].size() > 50  ) PU[3] = utf8_cut( PU[3], 50 ) + "....";
        	                if( PU[0].size() > 160 ) PU[0] = utf8_cut( PU[0], 160);

				cout << endl << "pushes: " << PU[1] << '>' << PU[2] << " :: " << PU[3] << endl;

				for(auto& s:PU)	sanitize(s);
        	                string cmds, cmd=pdir+"handler/push.sh";
				if( PU[4] != "" )
		                        cmds="cat <<< '"+ PU[0] +"'|" + cmd + " \"" + type + "\" \"" + PU[1] + "\" \"" + PU[2] + "\" \"" + PU[3] + "\" \"" + PU[4] + "\" &";
				else
		                        cmds="cat <<< '"+ PU[0] +"'|" + cmd + " \"" + type + "\" \"" + PU[1] + "\" \"" + PU[2] + "\" \"" + PU[3] + "\" &";
        	                system( cmds.c_str() );
			}
		}
		// will update both timestamp variable write it to file
		update_timestamp(ftimestamp, timestamp);
        }else if( type == "push" ){
                type = M["push"]["type"].asString();
		if( type == "dismissal"){
			/* do nothimg */
		}else if( type == "mirror" ){
			vector<string> PU(5);		// 0body 1from 2app 3title 4icon
			vector<string> RAW={"body","source_device_iden","application_name","title","icon"};

			for(int i=0,sz=RAW.size(); i<sz; i++)
				PU[i] = M["push"][ RAW[i] ].asString();

			PU[1] = iden2devname[ PU[1] ];
                        if( PU[3].size() > 50  ) PU[3] = utf8_cut( PU[3], 50 ) + "....";
                        if( PU[0].size() > 160 ) PU[0] = utf8_cut( PU[0], 160);

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
//		}else if( type == "sms_changed" && enable_sms_alert ){
		}else if( type == "sms_changed" ){
			string from=M["push"][ "source_device_iden" ].asString();
			string cmd="echo |" +pdir + "handler/mirror.sh \"" + sanitize(from) + "\" SMS Received/Sent ";
			system( cmd.c_str());
		}
        }

}


int main() {
	// env
	pdir+="/.bashbullet2/";
	ftimestamp=pdir+".TIMESTAMP", timestamp;
        Json::Value JStmp;

	// parse config, timestamp
	JStmp = file_json( pdir+"config.json" );
	apikey=JStmp["access_token"].asString();
	if( apikey == "" ){
		cout << " Access-Token is missing, stopping now. \n";
		return 0;
	}

	timestamp=get_timestamp(ftimestamp);

	// parse device
	vector<string> sms_capab;
        JStmp = str_json( get_devices(apikey) );
        int index=0;
        for( Json::ValueIterator it = JStmp["devices"].begin() ; it != JStmp["devices"].end() ; it++, index++ ){
		iden2devname[ JStmp["devices"][ index ]["iden"].asString() ] = JStmp["devices"][ index ]["nickname"].asString();
		if( JStmp["devices"][ index ]["has_sms"].asBool() ) sms_capab.push_back( JStmp["devices"][ index ]["nickname"].asString() );
        }
	// save device cache for YAD GUI
	save_devices( iden2devname , pdir ,sms_capab );

	// begin websocket
	while(1){
		websocket_client client;
		try{
			client.connect("wss://stream.pushbullet.com/websocket/"+apikey ).wait();
			while(1){
				try{
					client.receive().then([](websocket_incoming_message in_msg) {
						return in_msg.extract_string();
					}).then([](string wsmsg) {
						// debug
						string fdebug="/tmp/rawmsg";
				        	ofstream rawdeb(fdebug.c_str(), std::ofstream::out | std::ofstream::app);
						rawdeb << wsmsg << "\n";

						// parse
						Json::Value M=str_json(wsmsg);
					        handler(M);

					}).wait();
				}catch(const std::exception &e){
					printf("Error exception:%s\n", e.what());
					break;
				}
			}
		}catch(const std::exception &e){
			printf("Error exception:%s\n", e.what());
			break;
		}
		client.close().then([](){ /* close connection and retry */ });
	}

	return 0;
}

/*
// write to websocket ?? Ephermeral from PC ??
  websocket_outgoing_message out_msg;
  out_msg.set_utf8_message("test");
  client.send(out_msg).wait();
*/
