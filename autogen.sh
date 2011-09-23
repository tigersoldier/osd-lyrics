#!/bin/bash

set -e

args=$@

if grep -q Arch /etc/issue; then
    args="$args PYTHON=python2"
fi
autoreconf --install
./configure $args
