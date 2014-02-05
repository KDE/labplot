#! /bin/sh
$EXTRACTRC `find . -name \*.rc -o -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.h -o -name \*.cpp` -o $podir/LabPlot2.pot
