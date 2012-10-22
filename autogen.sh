#!/bin/bash

set -e

DIR=`dirname $0`

args=$@

if [ -e /usr/bin/python2 ]; then
    args="$args PYTHON=/usr/bin/python2"
fi
autoreconf --install --force $DIR
$DIR/configure $args
