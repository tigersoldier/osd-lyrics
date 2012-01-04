#!/bin/bash

set -e

args=$@

if [ -e /usr/bin/python2 ]; then
    args="$args PYTHON=/usr/bin/python2"
fi
autoreconf --install --force
./configure $args
