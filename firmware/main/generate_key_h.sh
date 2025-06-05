#!/bin/bash

if [ -z "$1" ]; then
  echo "Usage: $0 <path/to/public/key.pem>"
  exit 1
fi

TEMP_KEY_NAME=/tmp/washer_key.der

openssl rsa -pubin -in $1 -outform DER -out $TEMP_KEY_NAME
xxd -i -n public_key $TEMP_KEY_NAME > key.h

rm $TEMP_KEY_NAME

echo "Done"