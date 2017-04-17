#include <iostream>
#include <string>
#include <curl/curl.h>
using namespace std;

string get_n_pushes(string, int);
string get_cursor_pushes(string,string);
string get_pushes(string,string);

string get_cursor_devices(string,string);
string get_devices(string);

