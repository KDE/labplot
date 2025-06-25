#! /bin/sh
$EXTRACTRC `find . -name \*.rc -o -name \*.ui | grep -v '/tests/' | grep -v '/lib/examples/'` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.h -o -name \*.c -o -name \*.cpp | grep -v '/tests/' | grep -v '/lib/examples/'` -o $podir/labplot.pot
