#include <iostream>
#include <json/json.h>
#include "misc.h"
#include "puapi.h"

using namespace std;
int main(){

	string pdir=getenv("HOME");
	pdir += "/.bashbullet2/";

        Json::Value JStmp = file_json( pdir+"config.json" );
        string apikey=JStmp["access_token"].asString();

	Json::Value M = str_json( get_n_pushes(apikey, 1000) );

	int showlen=10;
	for(int j=0; j<8; j++){
	        string cursor = M["cursor"].asString();
		int index=0;
		for( Json::ValueIterator it = M["pushes"].begin() ; it != M["pushes"].end() ; it++, index++ ){
	                 bool dismissed = M["pushes"][ index ]["dismissed"].asBool();
	                 if( dismissed == false ){
	                        string type = M["pushes"][ index ][ "type" ].asString();

	                        vector<string> PU(5);           // 0body 1from 2to 3title 4url
	                        vector<string> RAW={"body","source_device_iden","target_device_iden","title","url"};

	                        for(int i=0,sz=RAW.size(); i<sz; i++)
	                                PU[i] = M["pushes"][ index ][ RAW[i] ].asString();

	                        // length of title & body
				PU[3]+="                              ";
	                        PU[3] = utf8_cut( PU[3], 30 );

				if( PU[0] != ""){
		                        PU[0] = utf8_cut( PU[0], 100 ) + "...";
		                        cout << type << ": " << PU[3] << " :: " << PU[0] << endl;
				}else{
		                        cout << type << ": " << PU[3] << " :: " << PU[4] << endl;
				}

				showlen--;
				if(showlen <= 0) break;
			}
		}
		if(showlen <= 0 || cursor == "" ) break;
		if( j == 7 && showlen >0 ){
			cout << "Stop retrieving... There might be more pushes, but bashbullet had already searched for 8 pages\n";
		}else{
			if(showlen && cursor!="" ) M = str_json( get_cursor_pushes(apikey, cursor) );
		}
	}

	return 0;
}
