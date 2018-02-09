#include "base64.h"
#include <iostream>
#include <string>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/evp.h>
using namespace std;

string pushbullet_decrypt(string key,string cipher);
string pushbullet_key(string pass, string salt);

