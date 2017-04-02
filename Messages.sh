#! /bin/sh
$EXTRACTRC `find . -name \*.rc -o -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.h -o -name \*.c -o -name \*.cpp` -o $podir/labplot2.pot
