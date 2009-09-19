#!/bin/bash
cd /tmp
rm -rf osdlyrics
svn export --force http://osd-lyrics.googlecode.com/svn/trunk/ osdlyrics
cd osdlyrics
version=`grep AC_INIT configure.ac | cut -d , -f 2 | cut -d [ -f 2 | cut -d ] -f 1`
aclocal
autoconf
automake --add-missing --copy
make
make distclean
cd ..
mdirname=osdlyrics-$version
[ -e $dirname ] && rm -rf $mdirname
mv osdlyrics $mdirname
fullversion=$version.`date +%Y%m%d`
tarname=osdlyrics-$fullversion.tar.gz
[ -e $tarname ] && rm -rf $tarname
tar zcvf $tarname $mdirname
