#!/usr/bin/bash

BROPATH=`which bro`
SCRIPTDIR=`dirname $BROPATH`/../share/bro

grunt
pushd build
rm -f ./bro && ln -s $SCRIPTDIR bro
cp ../data/bro-profile-function ./profile.json
echo "Bro path is $BROPATH and script directory is $SCRIPTDIR ..."
(sleep 1 && firefox 'http://localhost:8000')&
python -m SimpleHTTPServer

