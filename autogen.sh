#!/bin/bash

aclocal
autoheader
intltoolize --force
autoconf
automake --add-missing --copy