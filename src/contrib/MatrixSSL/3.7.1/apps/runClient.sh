#!/bin/sh

# Some cipher suites
CIPHER_SUITE="47"	#AES128-SHA
#CIPHER_SUITE="5"  	#RC4128-SHA
#CIPHER_SUITE="4"  	#RC4128-MD5
#CIPHER_SUITE="60" 	#AES128-SHA256
#CIPHER_SUITE="10"  #DES-CBC3-SHA
#CIPHER_SUITE="141"  #PSK_AES256-SHA
#CIPHER_SUITE="49156"  	#ECDH_ECDSA-AES128-SHA
#CIPHER_SUITE="49162"  	#ECDHE_ECDSA-AES256-SHA
#CIPHER_SUITE="1567"  	#RSA AES128-GCM-SHA256
#CIPHER_SUITE="57"  	#DHE_RSA AES256-SHA
#CIPHER_SUITE="49195"   #ECDHE_ECDSA-AES128-GCM-SHA256
#CIPHER_SUITE="49196"    #ECDHE_ECDSA-AES256-GCM-SHA384

#PROTOCOL_VERSION="1"	#TLS1.0
#PROTOCOL_VERSION="2"	#TLS1.1
PROTOCOL_VERSION="3"	#TLS1.2

IPADDR="127.0.0.1"
PORT="4433"
NEW_SESSIONS="1"
RESUMED_SESSIONS="1"
BYTES="1024"

./client -s${IPADDR} -p${PORT} -b${BYTES} -n${NEW_SESSIONS} -r${RESUMED_SESSIONS} -v${PROTOCOL_VERSION} -c${CIPHER_SUITE}
