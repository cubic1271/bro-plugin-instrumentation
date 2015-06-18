#!/usr/bin/bash

BROPATH=`which bro`
SCRIPTDIR=`dirname $BROPATH`/../share/bro

grunt
pushd bin
rm -f ./bro && ln -s $SCRIPTDIR bro
cp ../data/bro-profile-function ./profile.json
cp ../data/bro-profile-collection ./collection.json
cp ../data/callgraph.png ./callgraph.png
echo "Bro path is $BROPATH and script directory is $SCRIPTDIR ..."
python -m SimpleHTTPServer

