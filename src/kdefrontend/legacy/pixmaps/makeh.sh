#!/bin/bash

> pixmap.h

################################################

# for mainwin

echo "//LabPlot : pixmap.h">>pixmap.h
echo >> pixmap.h
echo "#ifndef PIXMAP_H">>pixmap.h
echo "#define PIXMAP_H">>pixmap.h
echo >> pixmap.h

for i in `ls *.xpm`
do
echo "#include \"$i\"" >> pixmap.h
done

echo >> pixmap.h
echo "#endif // PIXMAP_H">>pixmap.h

################################################
