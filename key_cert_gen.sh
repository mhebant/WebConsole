#!/bin/bash

keyfile="./WebConsole/bin/privatekey.key"
crtfile="./WebConsole/bin/serv.crt"

#Certificates
name="localhost"
organisation="Martin HEBANT"
country="FR"
serial=1
not_before=20160101000000
not_after=20260101000000

if [ $# -ge 1 ] && [ $1 = "regenpkey" ]
then
    mbedtls-2.2.1/programs/pkey/gen_key type=rsa rsa_keysize=4096 filename=$keyfile
fi

mbedtls-2.2.1/programs/x509/cert_write selfsign=1 issuer_key="$keyfile" issuer_name=CN="$name",O="$organisation",C="$country" serial=$serial not_before=$not_before not_after=$not_after output_file="$crtfile"