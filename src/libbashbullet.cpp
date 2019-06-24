#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <cstdlib>
#include <json/json.h>

#include <unistd.h>

#include "misc.h"
#include "puapi.h"
#include "aead_aes_gcm.h"

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

using namespace std;


typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;


int maxtitle=50, maxbody=160, loopcount=0;
vector<string> target_whitelist;
map<string,string> iden2devname;
string pdir=getenv("HOME"), apikey, ftimestamp, timestamp, traypipe, key="";
bool systray=true, enable_sms_alert=false, enable_whitelist=false;

void handler(Json::Value& M ){
	if( loopcount++ == 6 ){
		// update timestamp atleast every 3 minutes
		string cmd="touch "+ftimestamp;
		run_bash( cmd.c_str() );
		loopcount=0;
	}
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

				vector<string> PU(7);		// 0body 1from 2to 3title 4url 5fileurl 6imageurl
				vector<string> RAW={"body","source_device_iden","target_device_iden","title","url","file_url","image_url"};

				for(int i=0,sz=RAW.size(); i<sz; i++)
					PU[i] = tmpM["pushes"][ index ][ RAW[i] ].asString();

				if( PU[5] != "" ) PU[4]=PU[5];
				if( PU[6] != "" ) PU[4]=PU[6];

				// decode identity
				PU[1] = iden2devname[ PU[1] ];
				PU[2] = iden2devname[ PU[2] ];
				if( PU[1] == "" ) PU[1] = tmpM["pushes"][ index ][ "sender_name" ].asString();
				if( PU[2] == "" ) PU[2] = "ALL\n";

				bool display=false;
				if(enable_whitelist){
					for(auto& d:target_whitelist)
						if( d == PU[2] ) display=true;
				}else{
					display=true;
				}

				if(display){
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
	                                run_bash( cmds.c_str() );
					if(systray) update_icon(traypipe);
				}
			}
			// will update timestamp variable and write it to file
			update_timestamp(ftimestamp, timestamp);
		}
        }else if( type == "push" ){
		bool encrypted = M["push"]["encrypted"].asBool();
		if( encrypted ){
			cout << "decrypting message" << endl;
			if( key == "" ){
				string cmds="echo |" +pdir + "handler/mirror.sh \"Bashbullet\" \"ERROR\" \"End to End encryption is on, but decryption key is missing\"";
				run_bash( cmds.c_str());
				type = "dismissal";
			}else{
				string dec = pushbullet_decrypt(key, M["push"]["ciphertext"].asString() );
				M["push"] = str_json( dec );
				type = M["push"]["type"].asString();
			}
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
			run_bash( cmds.c_str() );

			cout << endl << "mirror: " << PU[1] << ':' << PU[2] << " :: " << PU[3] << endl;
		}else if( type == "clip" ){
			// untested, this is now payed function
			string body = M["push"][ "body" ].asString();
			string cmd="cat <<< '" + sanitize(body) + "'|" + pdir + "handler/clip.sh";
			run_bash( cmd.c_str() );
		}else if( type == "sms_changed" && enable_sms_alert ){
			string from= iden2devname[ M["push"][ "source_device_iden" ].asString() ];
			string cmd="echo |" +pdir + "handler/mirror.sh \"" + sanitize(from) + "\" \"SMS Received/Sent\"";
			run_bash( cmd.c_str());
		}
        }

}

// call back function for libwebsocketpp client
void on_message(client* c, websocketpp::connection_hdl hdl, message_ptr msg) {
	Json::Value M=str_json(msg->get_payload());
        handler(M);
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
	if( ! JStmp["key"].isNull() ){
		key = JStmp["key"].asString();
		if( key != "" ){
			Json::Value USR = str_json( get_user_iden(apikey) );
			cout << key << ' ' << USR["iden"].asString() << endl;
			key = pushbullet_key(key, USR["iden"].asString() );
		}
	}
	if( ! JStmp["title_max_length"].isNull() ) maxtitle=stoi( JStmp["title_max_length"].asString() );
	if( ! JStmp["body_max_length"].isNull() ) maxbody=stoi( JStmp["body_max_length"].asString() );
	if( ! JStmp["enable_sms_alert"].isNull() ) enable_sms_alert=JStmp["enable_sms_alert"].asBool();
        if( ! JStmp["enable_whitelist"].isNull() ) enable_whitelist=JStmp["enable_whitelist"].asBool();

	if( enable_whitelist ){
	        for(auto& v:JStmp["target_whitelist"])
	                target_whitelist.push_back( v.asString() );
	}

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
	if(has_bashbullet == false){
		cout << "ERROR: bashbullet device does not exist. \n";
		cout << "Run \"create_device.sh\" once if it's never been done. ::  \n";
		return 0;
	}

	// save device cache for YAD GUI
	save_devices( iden2devname , pdir ,sms_capab );

	// force refresh on start up
	timestamp = to_string( std::time(nullptr) );
	update_timestamp(ftimestamp,timestamp);
	Json::Value M=str_json("{ \"subtype\": \"push\", \"type\": \"tickle\" }");
	handler(M);

	// begin initialize websocket client
	client c;
	// tls handler for libwebsocketpp
	c.set_tls_init_handler([](websocketpp::connection_hdl) {
		return websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv1);
	});

	std::string uri = "wss://websocket.pushbullet.com/subscribe/"+apikey;

	try {
		c.init_asio();
		// Register our message handler
		c.set_message_handler(bind(&on_message,&c,::_1,::_2));

		websocketpp::lib::error_code ec;
		client::connection_ptr con = c.get_connection(uri, ec);
		if (ec) {
			std::cout << "could not create connection because: " << ec.message() << std::endl;
		return 0;
	        }

	        // Note that connect here only requests a connection. No network messages are
	        // exchanged until the event loop starts running in the next line.
	        c.connect(con);
	        // Start the ASIO io_service run loop
	        // this will cause a single connection to be made to the server. c.run()
	        // will exit when this connection is closed.
	        c.run();
	} catch (websocketpp::exception const & e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}

