#! /bin/sh
$EXTRACTRC `find . -name \*.rc -o -name \*.ui | grep -v '/tests/'` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.h -o -name \*.c -o -name \*.cpp | grep -v '/tests/'` -o $podir/labplot2.pot
